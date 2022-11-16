/**
 * This code was tested against C++20
 *
 * @author Ludvik Jerabek
 * @package slanalyzer
 * @version 1.0.0
 * @license MIT
 */

#include "Analyzer.h"
#include "CsvParser.h"
#include <chrono>
#include <iostream>
#include <iomanip>
#include "re2/re2.h"
#include "Utils.h"

Proofpoint::Analyzer::Analyzer(const SafeList& safelist, PatternErrors& errors)
{
	for (std::size_t i = 0; i<safelist.safe_list.size(); i++) {
		std::shared_ptr<SafeList::Entry> sle = safelist.safe_list.at(i);
		switch (sle->field_type) {
		case FieldType::IP: ip.Add(sle->match_type, sle->pattern, i, errors);
			break;
		case FieldType::HOST: host.Add(sle->match_type, sle->pattern, i, errors);
			break;
		case FieldType::HELO: helo.Add(sle->match_type, sle->pattern, i, errors);
			break;
		case FieldType::FROM: from.Add(sle->match_type, sle->pattern, i, errors);
			break;
		case FieldType::HFROM: hfrom.Add(sle->match_type, sle->pattern, i, errors);
			break;
		case FieldType::RCPT: rcpt.Add(sle->match_type, sle->pattern, i, errors);
			break;
		case FieldType::UNKNOWN: break;
		}
	}
}
void Proofpoint::Analyzer::Process(const std::string& ss_file, SafeList& safelist)
{
	std::size_t count = 0;
	using std::chrono::high_resolution_clock;
	using std::chrono::microseconds;

	auto start = high_resolution_clock::now();
	std::ifstream f(ss_file);
	csv::CsvParser parser(f);
	std::vector<std::string> header;
	re2::StringPiece matches[2];
	RE2 hfrom_addr_only(R"(<?\s*([a-zA-Z0-9.!#$%&’*+\/=?^_`{|}~-]+@[a-zA-Z0-9-]+(?:\.[a-zA-Z0-9-]+)*)\s*>?\s*(?:;|$))");
	RE2 inbound_check(R"(\bdefault_inbound\b)");

	auto compare = [](const std::string& lhs, const std::string& rhs) -> bool {
	  return strcasecmp(lhs.c_str(), rhs.c_str())<0;
	};

	std::map<std::string, std::size_t, decltype(compare)> header_to_index;

	// Find the header first, get column names
	for (auto& row : parser) {
		size_t cols = row.size();
		if (cols==1 && Utils::trim_copy(row.at(0)).empty()) continue;
		header = row;
		for (std::size_t i = 0; i<header.size(); i++) {
			header_to_index.insert({header.at(i), i});
		}
		break;
	}

	for (auto& row : parser) {
		bool inbound = RE2::PartialMatch(row[header_to_index["Policy_Route"]],inbound_check);
		ip.Match(inbound,row[header_to_index["Sender_IP_Address"]], safelist.safe_list);
		host.Match(inbound,row[header_to_index["Sender_Host"]], safelist.safe_list);
		helo.Match(inbound,row[header_to_index["HELO"]], safelist.safe_list);
		// This single call has large impact on processing. Since we need to perform header from "address only"
		hfrom.Match(inbound,(hfrom_addr_only.Match(row[header_to_index["Header_From"]], 0,
				row[header_to_index["Header_From"]].length(), RE2::UNANCHORED, matches, 2))
				? matches[1].ToString() : row[header_to_index["Header_From"]], safelist.safe_list);
		from.Match(inbound,row[header_to_index["Sender"]], safelist.safe_list);
		rcpt.Match(inbound,row[header_to_index["Recipients"]], safelist.safe_list);
		count++;
	}

	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop-start);
	std::cout << std::left << std::setw(25) << "SS Processing Completed" << " "
			  << std::left << std::setw(25) << std::to_string(duration.count()) << " "
			  << std::setw(25) << std::setprecision(2) << std::to_string((double)duration.count()/1000000) << " "
			  << std::setw(25) << count << " "
			  << ss_file << std::endl;
}
