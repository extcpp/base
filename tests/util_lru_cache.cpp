// Copyright - 2020 - Jan Christoph Uhde <Jan@UhdeJC.com>
// Please see LICENSE.md for license or visit https://github.com/extcpp/basics
#include <ext/util/lru_cache.hpp>
#include <ext/macros/compiler.hpp>
#include <gtest/gtest.h>

namespace eu = ::ext::util;

#ifndef EXT_CHECKED
    static_assert(false, "this test must run with check macro active");
#endif

void times_2(int& x) { x=x*2; }
void times_100(int& x) { x=x*100; }
bool is_even(int const& x) { return x % 2 == 0; }

TEST(util_lru_cache, put_get) {
    eu::lru_cache<int,int> cache(3);
    ASSERT_EQ(cache.size(),0);

    //---------------------------------
    cache.put(1,1);
    cache.put(2,2);
    cache.put(3,3);

    ASSERT_EQ(cache.size(),  3);
    ASSERT_NE(cache.get(1), nullptr);
    ASSERT_EQ(*cache.get(1), 1);
    ASSERT_NE(cache.get(2), nullptr);
    ASSERT_EQ(*cache.get(2), 2);
    ASSERT_NE(cache.get(3), nullptr);
    ASSERT_EQ(*cache.get(3), 3);
    ASSERT_EQ(cache.get(4), nullptr);

    //---------------------------------
    cache.put(4,4);

    ASSERT_EQ(cache.size(),  3);
    ASSERT_NE(cache.get(2), nullptr);
    ASSERT_EQ(*cache.get(2), 2);
    ASSERT_NE(cache.get(3), nullptr);
    ASSERT_EQ(*cache.get(3), 3);
    ASSERT_NE(cache.get(4), nullptr);
    ASSERT_EQ(*cache.get(4), 4);

    //---------------------------------
    // trigger splice
    cache.put(3,3);
    cache.put(3,3);
    cache.put(4,4);

    ASSERT_EQ(cache.size(),  3);
    ASSERT_NE(cache.get(2), nullptr);
    ASSERT_EQ(*cache.get(2), 2);
    ASSERT_NE(cache.get(3), nullptr);
    ASSERT_EQ(*cache.get(3), 3);
    ASSERT_NE(cache.get(4), nullptr);
    ASSERT_EQ(*cache.get(4), 4);
}

TEST(util_lru_cache, get_update_and_get_remove) {
    eu::lru_cache<int,int> cache(3);
    cache.put(1,1);
    cache.put(2,2);
    cache.put(3,3);

    //---------------------------------
    ASSERT_EQ(*cache.get_update(1,times_2), 2);
    ASSERT_EQ(*cache.get_update(3,times_2), 6);
    ASSERT_EQ(*cache.get_update(2,times_2), 4);
    ASSERT_EQ(cache.get_remove(2).value(), 4);

    // 3:6 2:4 1:2
    ASSERT_EQ(cache.size(), 2);

    //---------------------------------
    ASSERT_EQ(cache.remove_all_matching(is_even), 2);
    ASSERT_EQ(cache.size(), 0);
}

TEST(util_lru_cache, update) {
    eu::lru_cache<int,int> cache(3);
    cache.put(1,1);
    cache.put(2,2);
    cache.put(3,3);

    //---------------------------------
    ASSERT_EQ(*cache.get_update(3,times_100), 300);
    // 3:300 2:2 1:1
    cache.put(4,4);
    // 4:4 3:300 2:2
    ASSERT_EQ(*cache.get_update(2, times_100), 200);

    // 2:200 4:4 3:300
    ASSERT_EQ(cache.size(), 3);
    ASSERT_EQ(cache.get(1), nullptr);
    ASSERT_EQ(*cache.get(2), 200);
    ASSERT_EQ(*cache.get(4), 4);
    ASSERT_EQ(*cache.get(3), 300);
}

TEST(util_lru_cache, remove) {
    eu::lru_cache<int,int> cache(3);
    cache.put(1,1);
    cache.put(2,2);
    cache.put(3,3);

    // 3:3 2:2 1:1
    ASSERT_EQ(cache.size(), 3);

    //---------------------------------
    ASSERT_TRUE(cache.remove(2));
    ASSERT_FALSE(cache.remove(4));

    // 3:3 1:1
    ASSERT_EQ(cache.size(), 2);
    ASSERT_EQ(*cache.get(1), 1);
    ASSERT_EQ(*cache.get(3), 3);
}

TEST(util_lru_cache, remove_predicates) {
    eu::lru_cache<int,int> cache(4);
    cache.put(1,1);
    cache.put(2,2);
    cache.put(3,3);
    cache.put(4,4);

    // 4:4 3:3 2:2 1:1
    ASSERT_EQ(cache.size(), 4);

    //---------------------------------
    ASSERT_EQ(cache.remove_all_matching(is_even),2);

    // 3:3 1:1
    ASSERT_EQ(cache.size(), 2);
}
