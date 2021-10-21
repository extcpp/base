// Copyright - 2018 - Jan Christoph Uhde <Jan@UhdeJC.com>
// Please see LICENSE.md for license or visit https://github.com/extcpp/basics
#ifndef EXT_UTIL_REGEX_HEADER
#define EXT_UTIL_REGEX_HEADER
#include <regex>
#include <string>
#include <vector>

#ifdef EXT_DEBUG
    #include <iostream>
#endif // EXT_DEBUG

namespace ext { namespace util {


#ifdef EXT_DEBUG
using re_vector = std::vector<std::pair<std::string, std::regex>>;
#else
using re_vector = std::vector<std::regex>;
#endif // EXT_DEBUG


inline void add_re(std::string const& re,
                   re_vector& re_vec,
                   std::regex_constants::syntax_option_type flag = std::regex_constants::ECMAScript) {
#ifdef EXT_DEBUG
    re_vec.push_back({re, std::regex(re, flag)});
#else
    re_vec.push_back(std::regex(re, flag));
#endif // EXT_DEBUG
}

inline bool match(std::string const& to_check, re_vector const& re_vec, bool all = false) {
    for (auto const& re : re_vec) {
#ifdef EXT_DEBUG
        bool matches = std::regex_match(to_check, re.second);
        // clang-format off
        std::cerr << "re: '" << re.first << "' "
                  << (matches ? "matches" : "does not match") << " string: '" << to_check
                  << "'\n";
        // clang-format on
        if (matches) {
#else
        if (std::regex_match(to_check, re)) {
#endif // EXT_DEBUG
            if (!all) {
                return true;
            }
        } else {
            if (all) {
                return false;
            }
        }
    }

    if (all) {
        return true;
    } else {
        return false;
    }
}

inline bool not_in_exclude(std::string const& to_check, re_vector const& exclude_vec) {
    return !match(to_check, exclude_vec, false);
}

inline bool must_include_one_of(std::string const& to_check, re_vector const& include_one_of_vec) {
    return match(to_check, include_one_of_vec, false);
}

inline bool must_include_all_of(std::string const& to_check, re_vector const& include_all_of_vec) {
    return match(to_check, include_all_of_vec, true);
}

}}     // namespace ext::util
#endif // EXT_UTIL_REGEX_HEADER
