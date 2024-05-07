#define LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#include <libnetwrk.hpp>
#include <gtest/gtest.h>

using namespace libnetwrk;

TEST(tsdeque, create) {
    tsdeque<int> deque;

    EXPECT_TRUE(deque.count() == 0);
}

TEST(tsdeque, push_front) {
    tsdeque<int> deque;
    deque.push_front(1);
    deque.push_front(0);
    deque.push_front(2);

    EXPECT_TRUE(deque.pop_front() == 2);
    EXPECT_TRUE(deque.pop_front() == 0);
    EXPECT_TRUE(deque.pop_front() == 1);
}

TEST(tsdeque, push_back) {
    tsdeque<int> deque;
    deque.push_back(1);
    deque.push_back(0);
    deque.push_back(2);

    EXPECT_TRUE(deque.pop_front() == 1);
    EXPECT_TRUE(deque.pop_front() == 0);
    EXPECT_TRUE(deque.pop_front() == 2);
}

TEST(tsdeque, front) {
    tsdeque<int> deque;
    deque.push_back(1);
    deque.push_back(0);
    deque.push_back(2);

    EXPECT_TRUE(deque.front() == 1);
}

TEST(tsdeque, back) {
    tsdeque<int> deque;
    deque.push_back(1);
    deque.push_back(0);
    deque.push_back(2);

    EXPECT_TRUE(deque.back() == 2);
}

TEST(tsdeque, pop_front) {
    tsdeque<int> deque;
    deque.push_back(1);
    deque.push_back(0);
    deque.push_back(2);
    int i = deque.pop_front();

    EXPECT_TRUE(i == 1);
    EXPECT_TRUE(deque.front() == 0);
}


TEST(tsdeque, pop_back) {
    tsdeque<int> deque;
    deque.push_back(1);
    deque.push_back(0);
    deque.push_back(2);
    int i = deque.pop_back();

    EXPECT_TRUE(i == 2);
    EXPECT_TRUE(deque.back() == 0);
}

TEST(tsdeque, clear) {
    tsdeque<int> deque;
    deque.push_back(1);
    deque.push_back(0);
    deque.push_back(2);
    deque.clear();

    EXPECT_TRUE(deque.empty() == true);
}

TEST(tsdeque, empty) {
    tsdeque<int> deque;
    deque.push_back(1);
    deque.push_back(0);
    deque.push_back(2);

    EXPECT_TRUE(deque.empty() == false);

    deque.clear();

    EXPECT_TRUE(deque.empty() == true);
}

TEST(tsdeque, count) {
    tsdeque<int> deque;
    deque.push_back(1);
    deque.push_back(0);
    deque.push_back(2);

    EXPECT_TRUE(deque.count() == 3);
}
