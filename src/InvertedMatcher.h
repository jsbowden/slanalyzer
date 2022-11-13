/**
 * This code was tested against C++20
 *
 * @author Ludvik Jerabek
 * @package slanalyzer
 * @version 1.0.0
 * @license MIT
 */
#ifndef SLPARSER_INVERTEDMATCHER_H
#define SLPARSER_INVERTEDMATCHER_H

#include "IMatcher.h"
#include "re2/re2.h"
#include "re2/set.h"

namespace Proofpoint {
class InvertedMatcher : public IMatcher {
 public:
  explicit InvertedMatcher(bool literal = false,
						   bool case_sensitive = false,
						   RE2::Anchor anchor = RE2::ANCHOR_BOTH);
  void Add(const std::string &pattern, const std::size_t &index, std::vector<PatternError>& errors) override;
  bool Match(const std::string &pattern, std::vector<std::size_t> &match_indexes) override;
  std::size_t GetPatternCount() override;
 private:
  bool compiled;
  RE2::Options opt;
  std::unique_ptr<RE2::Set> match;
  std::unordered_map<int, std::size_t> map_to_sle;
};
}
#endif //SLPARSER_INVERTEDMATCHER_H
