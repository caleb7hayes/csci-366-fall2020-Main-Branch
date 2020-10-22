#include "gtest/gtest.h"

extern "C" {
extern long long find_max(long long *, int len);
}


/* PROBLEM 3: */
TEST(find_max, works) {
    long long arr1[] = {1, 2, 3};
    EXPECT_EQ(3, find_max(arr1, 3));
    long long arr2[] = {4, 2, 3};
    EXPECT_EQ(4, find_max(arr2, 3));
    long long arr3[] = {22, 18, 4, 42, 3};
    EXPECT_EQ(42, find_max(arr3, 5));
}

