#define LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#include <libnetwrk.hpp>
#include <gtest/gtest.h>

TEST(buffer, create) {
    libnetwrk::buffer<libnetwrk::bin_serialize> b1;
    EXPECT_TRUE(b1.size() == 0);

    std::vector<uint8_t> v({ 1, 2, 3, 4, 5, 6 });
    libnetwrk::buffer<libnetwrk::bin_serialize> b2(v.begin(), v.end());
    EXPECT_TRUE(b2.size() == 6);
}

TEST(buffer, get_range) {
    libnetwrk::buffer<libnetwrk::bin_serialize> b;
    b << uint8_t(1) << uint8_t(2) << uint8_t(3) << uint8_t(4) << uint8_t(5);

    libnetwrk::buffer<libnetwrk::bin_serialize> range = b.get_range(3);
    EXPECT_TRUE(range.size() == 3);
    EXPECT_TRUE(range[0] == 1);
    EXPECT_TRUE(range[1] == 2);
    EXPECT_TRUE(range[2] == 3);

    uint8_t c1 = 0, c2 = 0;
    b.get_range(&c1, sizeof(uint8_t));
    b.get_range(&c2, sizeof(uint8_t));
    EXPECT_TRUE(c1 == 4);
    EXPECT_TRUE(c2 == 5);
}

TEST(buffer, get_range_exception) {
    libnetwrk::buffer<libnetwrk::bin_serialize> b1;
    int i1 = 253, i2 = 0;
    EXPECT_ANY_THROW(b1.get_range(sizeof(int)));
    EXPECT_ANY_THROW(b1.get_range(&i2, sizeof(int)));

    libnetwrk::buffer<libnetwrk::bin_serialize> b2; b2 << i1;
    b2.get_range(&i1, sizeof(int));
    EXPECT_ANY_THROW(b2.get_range(sizeof(char)));
}

TEST(buffer, push_back_buffer) {
    libnetwrk::buffer<libnetwrk::bin_serialize> b1;
    b1 << uint8_t(1) << uint8_t(2) << uint8_t(3);

    libnetwrk::buffer<libnetwrk::bin_serialize> b2;
    b2.push_back(b1);

    EXPECT_TRUE(b2.size() == 3);
    EXPECT_TRUE(b2[0] == 1);
    EXPECT_TRUE(b2[1] == 2);
    EXPECT_TRUE(b2[2] == 3);

    uint8_t ui = 24;
    b2.push_back(&ui, sizeof(uint8_t));
    EXPECT_TRUE(b2[3] == 24);
}
