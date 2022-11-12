#ifndef SLPARSER_PARSER_H
#define SLPARSER_PARSER_H

#include "re2/re2.h"
#include <string>

namespace Proofpoint {
	static inline void ltrim(std::string& s)
	{
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
			return !std::isspace(ch);
		}));
	}

	static inline void rtrim(std::string& s)
	{
		s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
			return !std::isspace(ch);
		}).base(), s.end());
	}

	static inline void trim(std::string& s)
	{
		rtrim(s);
		ltrim(s);
	}

	static inline std::string ltrim_copy(std::string s)
	{
		ltrim(s);
		return s;
	}

	static inline std::string rtrim_copy(std::string s)
	{
		rtrim(s);
		return s;
	}

	static inline std::string trim_copy(std::string s)
	{
		trim(s);
		return s;
	}
}

#include "Subnet.h"
#include "Matcher.h"
#include "IPMatcher.h"
#include "StringMatcher.h"
#include "SafeList.h"
#include "Processor.h"

#endif //SLPARSER_PARSER_H