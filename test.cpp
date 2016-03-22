//
// Created by joshm on 3/21/2016.
//

#include <gtest/gtest.h>

#include "utility.hpp"

/*
 * utility tests
 */
TEST (rangeLimit, withinRange) {
    EXPECT_EQ(3, mvis::util::range_limit(3, 0, 360));
    EXPECT_EQ(3, mvis::util::range_limit(3, 0.0, 360.0));
}

TEST (rangeLimit, outOfRange) {
    EXPECT_EQ(0, mvis::util::range_limit(-1, 0.0, 360.0));
    EXPECT_EQ(360, mvis::util::range_limit(362, 0.0, 360.0));
}

TEST (rangeLimit, impossibleRange) {
    EXPECT_EQ(1000, mvis::util::range_limit(1000, 360.0, 0.0));
}

TEST (ithMiddleTest, positive){
    EXPECT_EQ(45, mvis::util::ith_middle(3, 0, 360));
    EXPECT_DOUBLE_EQ(45.0, mvis::util::ith_middle(3, 0.0, 360.0));

    EXPECT_DOUBLE_EQ(180.0, mvis::util::ith_middle(0, 0.0, 360.0));

    //ith_middle can't take negative values
}

int main(int argc, char**argv){
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}