/**
 * This code was tested against C++20
 *
 * @author Ludvik Jerabek
 * @package slanalyzer
 * @version 1.0.0
 * @license MIT
 */

#include "UserAnalyzer.h"
#include "CsvParser.h"
#include <chrono>
#include <iostream>
#include <iomanip>
#include "re2/re2.h"
#include "Utils.h"

void Proofpoint::UserAnalyzer::Load(const UserList& userlist, PatternErrors<UserMatch>& pattern_errors)
{
	std::size_t count = 0;
	addr_to_user.reserve(userlist.GetAddressCount());
	for ( auto user = userlist.begin() ; user != userlist.end() ; user++ ) {
		std::size_t index = std::distance(userlist.begin(),user);
		addr_to_user.emplace(user->mail,index);
		for ( const auto& email : user->proxy_addresses ) {
			addr_to_user.emplace(email, index);
		}
		for (std::size_t j = 0; j < user->safe.size(); j++) {
			safe_matcher.emplace(index, std::make_shared<Matcher<UserMatch>>(true,false,RE2::ANCHOR_START));
			safe_matcher.at(index)->Add(Utils::reverse_copy(user->safe[j].pattern), {index,j},pattern_errors);
			//std::cout << "Added: (" << user->safe[j].pattern << ") " << user->givenName << "," << user->sn << " " << user->mail << "" << index << " --> " << j << std::endl;
 		}
		for (std::size_t j = 0; j < user->block.size(); j++){
			block_matcher.emplace(index, std::make_shared<Matcher<UserMatch>>(true,false,RE2::ANCHOR_START));
			block_matcher.at(index)->Add(Utils::reverse_copy(user->block[j].pattern), {index,j},pattern_errors);
		}
		count++;
	}
}
std::size_t Proofpoint::UserAnalyzer::Process(const std::string& ss_file, UserList& userlist, std::size_t& records_processed)
{
	records_processed = 0;
	csv::HeaderIndex header_index;
	std::ifstream f(ss_file);
	csv::CsvParser parser(f);
	re2::StringPiece matches[2];
	RE2 hfrom_addr_only(R"(<?\s*([a-zA-Z0-9.!#$%&’*+\/=?^_`{|}~-]+@[a-zA-Z0-9-]+(?:\.[a-zA-Z0-9-]+)*)\s*>?\s*(?:;|$))");
	RE2 inbound_check(R"(\bdefault_inbound\b)");
	csv::HeaderMap header_map;
	csv::HeaderList required_headers{"Policy_Route","Header_From","Sender","Recipients"};
	// Validate there are headers we are interested in...
	header_index = parser.FindHeader(required_headers, header_map);

	//std::cout << std::setw(35) << "Highest Index" << " " << std::setw(25) << header_index << std::endl;
	// std::multimap is useful for CSVs where there may be duplicate headers.
	//for (auto i = header_map.begin(); i!= header_map.end(); i++){
	//	std::cout << std::setw(35) << i->first << " " << std::setw(25) << i->second  << " " << header_map.count(i->first) << std::endl;
	//}
	if( header_index > -1 ) {
		for (auto& row : parser) {
			bool inbound = RE2::PartialMatch(row[header_map.find("Policy_Route")->second], inbound_check);
			for (auto recipient : Utils::split(row[header_map.find("Recipients")->second], ',')) {
				auto user = addr_to_user.find(std::string(recipient));
				if (user != addr_to_user.end()) {
					auto smatcher = safe_matcher.find(user->second);
					if( smatcher != safe_matcher.end() ) {
						std::vector<UserMatch> user_matches;
						if( smatcher->second->Match(Utils::reverse_copy(row[header_map.find("Sender")->second]), user_matches) )
							userlist.safe_count++;
						for( auto m : user_matches ) {
							//std::cout << "Safe Matched: " << m.list_index << "-->" << m.user_index << std::endl;
							userlist.entries[m.user_index].safe[m.list_index].count++;
						}

					}

					auto bmatcher = block_matcher.find(user->second);
					if( bmatcher != block_matcher.end() ) {
						std::vector<UserMatch> user_matches;
						bmatcher->second->Match(Utils::reverse_copy(row[header_map.find("Sender")->second]), user_matches);
						for( auto m : user_matches ){
							//std::cout << "Block Matched: " << m.list_index << "-->" << m.user_index << std::endl;
							userlist.entries[m.user_index].block[m.list_index].count++;
						}
					}
				}
			}
			records_processed++;
		}
	}
	return header_index;
}
