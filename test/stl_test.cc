#include <gtest/gtest.h>
#include <unordered_map>
#include <set>
#include <map>
#include <list>
#include <deque>
#include <utils/common.h>


using namespace ors::utils::stl;

class stl_test : public ::testing::Test {

};

TEST_F(stl_test, sort) {
    auto vec = std::vector<int>{6,5,4,3,2,1};
    sort(vec);
    EXPECT_EQ(vec[0],1);
    EXPECT_EQ(vec[5],6);
    
}


TEST_F(stl_test, 1) {
    std::map<int,int> map;
    for(int i = 0; i < 10; ++i) {
        map[i] = i*2;
    }
    auto vec = keys(map);
    auto vec_vals = values(map);
    EXPECT_EQ(vec[0],0);
    EXPECT_EQ(vec_vals.back(),18);
    auto list = keys<std::list>(map);
    EXPECT_EQ(list.back(),9);

    auto deq = keys<std::deque>(map);
    EXPECT_EQ(deq.back(),9);
    std::set<int> set;

    auto kvs = pairs(map);
    EXPECT_EQ(kvs[1].first,1);
    EXPECT_EQ(kvs[1].second,2);
}