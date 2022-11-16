/**
 * This code was tested against C++20
 *
 * @author Ludvik Jerabek
 * @package slanalyzer
 * @version 1.0.0
 * @license MIT
 */
#ifndef SLANALYZER_MATCHER_H
#define SLANALYZER_MATCHER_H

#include "IMatcher.h"
#include "re2/re2.h"
#include "re2/set.h"
#include <memory>
#include <unordered_map>

namespace Proofpoint {
class Matcher : public IMatcher {
public:
	explicit Matcher(bool literal = false, bool case_sensitive = false, RE2::Anchor anchor = RE2::ANCHOR_BOTH);
	void Add(const std::string& pattern, const size_t& index, PatternErrors& errors) override;
	bool Match(const std::string& pattern, std::vector<std::size_t>& match_indexes) override;
	std::size_t GetPatternCount() override;
private:
	bool compiled;
	RE2::Options opt;
	std::unique_ptr<RE2::Set> match;
	std::unordered_map<int, std::size_t> map_to_sle;
};
}
#endif //SLANALYZER_MATCHER_H
