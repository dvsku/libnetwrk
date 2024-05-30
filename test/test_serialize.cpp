#include "libnetwrk/net/serialize/serialize.hpp"

#include <gtest/gtest.h>
#include <cstring>
#include <vector>
#include <deque>
#include <list>
#include <stack>
#include <queue>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <forward_list>
#include <stack>

#ifdef LIBNETWRK_SERIALIZE_TEST_BUFFER_DYNAMIC
    #define __BUFFER libnetwrk::dynamic_buffer
#else
    #define __BUFFER libnetwrk::fixed_buffer<1024>
#endif

using namespace libnetwrk;

struct base_struct {
    uint32_t    a = 420;
    std::string b = "abcde";

    void serialize(__BUFFER& buffer) const {
        buffer << a << b;
    }

    void deserialize(__BUFFER& buffer) {
        buffer >> a >> b;
    }

    bool equals(const base_struct& obj) {
        return a == obj.a && b == obj.b;
    }
};

struct derived_struct : base_struct {
    std::vector<int> c = { 123, 534, 346, 5432, 242, 735 };
    bool             d = true;

    void serialize(__BUFFER& buffer) const {
        base_struct::serialize(buffer);
        buffer << c << d;
    }

    void deserialize(__BUFFER& buffer) {
        base_struct::deserialize(buffer);
        buffer >> c >> d;
    }

    bool equals(const derived_struct& obj) const {
        return a == obj.a                                    &&
               b == obj.b                                    &&
               std::equal(c.begin(), c.end(), obj.c.begin()) &&
               d == obj.d;
    }

    bool operator==(const derived_struct& rhs) const {
        return equals(rhs);
    }
};

struct architecture_struct {
    bool                       m1 = true;
    int8_t                     m2 = 23;
    int16_t                    m3 = 876;
    uint32_t                   m4 = 77;
    int64_t                    m5 = 89777;
    std::string                m6 = "gfdsoi'gj984fsdg'[][fsd4fds'f][sdf]dfg84df9g9fds48gdf";
    std::vector<int>           m7 = { 478, 64, 7892 };
    std::map<int, std::string> m8 = { {778, "gfdsg"}, {88, "vxcvw"}, {654, "qwepote"} };

    architecture_struct() = delete;
    architecture_struct(bool clean) {
        if (!clean) return;

        m1 = false;
        m2 = 0;
        m3 = 0;
        m4 = 0;
        m5 = 0;
        m6 = "";
        m7 = {};
        m8 = {};
    }

    void serialize(__BUFFER& buffer) const {
        buffer << m1 << m2 << m3 << m4 << m5 << m6 << m7 << m8;
    }

    void deserialize(__BUFFER& buffer) {
        buffer >> m1 >> m2 >> m3 >> m4 >> m5 >> m6 >> m7 >> m8;
    }

    bool operator==(const architecture_struct& rhs) const {
        return m1 == rhs.m1 &&
               m2 == rhs.m2 &&
               m3 == rhs.m3 &&
               m4 == rhs.m4 &&
               m5 == rhs.m5 &&
               m6 == rhs.m6 &&
               m7 == rhs.m7 &&
               m8 == rhs.m8;
    }
};

TEST(serialize, supported) {
    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_supported<__BUFFER, bool>));
    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_supported<__BUFFER, char>));
    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_supported<__BUFFER, int8_t>));
    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_supported<__BUFFER, uint8_t>));
    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_supported<__BUFFER, int16_t>));
    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_supported<__BUFFER, uint16_t>));
    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_supported<__BUFFER, int32_t>));
    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_supported<__BUFFER, uint32_t>));
    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_supported<__BUFFER, int64_t>));
    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_supported<__BUFFER, uint64_t>));
    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_supported<__BUFFER, float>));
    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_supported<__BUFFER, double>));

    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_supported<__BUFFER, std::string>));
    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_supported<__BUFFER, char*>));

    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_supported<__BUFFER, std::array<int, 5>>));
    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_supported<__BUFFER, std::vector<int>>));
    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_supported<__BUFFER, std::deque<int>>));
    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_supported<__BUFFER, std::list<int>>));
    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_supported<__BUFFER, std::set<int>>));
    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_supported<__BUFFER, std::unordered_set<int>>));
    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_supported<__BUFFER, std::map<int,int>>));
    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_supported<__BUFFER, std::unordered_map<int, int>>));

    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_unsupported<__BUFFER, std::vector<bool>>));
    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_unsupported<__BUFFER, std::stack<int>>));
    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_unsupported<__BUFFER, std::queue<int>>));
    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_unsupported<__BUFFER, std::priority_queue<int>>));
    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_unsupported<__BUFFER, std::forward_list<int>>));
    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_unsupported<__BUFFER, std::multiset<int>>));
    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_unsupported<__BUFFER, std::multimap<int, int>>));
    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_unsupported<__BUFFER, std::unordered_multiset<int>>));
    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_unsupported<__BUFFER, std::unordered_multimap<int, int>>));

    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_supported<__BUFFER, base_struct>));
    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_supported<__BUFFER, derived_struct>));

    struct failing_struct {};

    EXPECT_TRUE((libnetwrk::serialize::internal::serialize_unsupported<__BUFFER, failing_struct>));
}

TEST(serialize, endian) {
    {
        int16_t value = (int16_t)0xAABB;
        libnetwrk::serialize::internal::byte_swap(value);
        ASSERT_TRUE(value == (int16_t)0xBBAA);
    }

    {
        uint16_t value = (uint16_t)0xAABB;
        libnetwrk::serialize::internal::byte_swap(value);
        ASSERT_TRUE(value == (uint16_t)0xBBAA);
    }

    {
        int32_t value = (int32_t)0xAABBCCDD;
        libnetwrk::serialize::internal::byte_swap(value);
        ASSERT_TRUE(value == (int32_t)0xDDCCBBAA);
    }

    {
        uint32_t value = (uint32_t)0xAABBCCDD;
        libnetwrk::serialize::internal::byte_swap(value);
        ASSERT_TRUE(value == (uint32_t)0xDDCCBBAA);
    }

    {
        int64_t value = (int64_t)0xAAAABBBBCCCCDDDD;
        libnetwrk::serialize::internal::byte_swap(value);
        ASSERT_TRUE(value == (int64_t)0xDDDDCCCCBBBBAAAA);
    }

    {
        uint64_t value = (uint64_t)0xAAAABBBBCCCCDDDD;
        libnetwrk::serialize::internal::byte_swap(value);
        ASSERT_TRUE(value == (uint64_t)0xDDDDCCCCBBBBAAAA);
    }

    {
        uint32_t val = (uint32_t)0xAABBCCDD;

        float value;
        std::memcpy(&value, &val, 4);

        libnetwrk::serialize::internal::byte_swap(value);
        ASSERT_TRUE(*((uint32_t*)&value) == (uint32_t)0xDDCCBBAA);
    }

    {
        uint64_t val = (uint64_t)0xAAAABBBBCCCCDDDD;

        double value;
        std::memcpy(&value, &val, 8);

        libnetwrk::serialize::internal::byte_swap(value);
        ASSERT_TRUE(*((uint64_t*)&value) == (uint64_t)0xDDDDCCCCBBBBAAAA);
    }
}

#ifdef LIBNETWRK_SERIALIZE_TEST_BUFFER_DYNAMIC

#include <fstream>
#include <filesystem>

static void read_to_buffer(const std::filesystem::path& filename, __BUFFER& buffer) {
    ASSERT_TRUE(std::filesystem::exists(filename));

    std::ifstream stream(filename, std::ios::binary);
    ASSERT_TRUE(stream.is_open());

    auto& underlying = buffer.underlying();
    underlying = { (std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>() };

    stream.close();
};

TEST(serialize, architecture) {
    {
        std::filesystem::path filename = LIBNETWRK_ARCHITECTURE_DIR;
        filename.append("architecture_x86.data");

        __BUFFER buffer;
        read_to_buffer(filename, buffer);

        architecture_struct as1(true);
        architecture_struct as2(false);

        ASSERT_FALSE(as1 == as2);
        buffer >> as1;
        ASSERT_TRUE(as1 == as2);
    }

    {
        std::filesystem::path filename = LIBNETWRK_ARCHITECTURE_DIR;
        filename.append("architecture_x64.data");

        __BUFFER buffer;
        read_to_buffer(filename, buffer);

        architecture_struct as1(true);
        architecture_struct as2(false);

        ASSERT_FALSE(as1 == as2);
        buffer >> as1;
        ASSERT_TRUE(as1 == as2);
    }
}

#endif

TEST(serialize, primitives) {
    __BUFFER buffer;

    {
        bool v1 = true;
        bool v2 = false;

        buffer << v1 >> v2;
        ASSERT_TRUE(v1 == v2);
    }

    buffer.clear();

    {
        int8_t v1 = (int8_t)0xFF;
        int8_t v2 = 0;

        buffer << v1 >> v2;
        ASSERT_TRUE(v1 == v2);
    }

    buffer.clear();

    {
        uint8_t v1 = (uint8_t)0xFF;
        uint8_t v2 = 0;

        buffer << v1 >> v2;
        ASSERT_TRUE(v1 == v2);
    }

    buffer.clear();

    {
        int16_t v1 = (int16_t)0xFFFF;
        int16_t v2 = 0;

        buffer << v1 >> v2;
        ASSERT_TRUE(v1 == v2);
    }

    buffer.clear();

    {
        uint16_t v1 = (uint16_t)0xFFFF;
        uint16_t v2 = 0;

        buffer << v1 >> v2;
        ASSERT_TRUE(v1 == v2);
    }

    buffer.clear();

    {
        int32_t v1 = (int32_t)0xFFFFFFFF;
        int32_t v2 = 0;

        buffer << v1 >> v2;
        ASSERT_TRUE(v1 == v2);
    }

    buffer.clear();

    {
        uint32_t v1 = (uint32_t)0xFFFFFFFF;
        uint32_t v2 = 0;

        buffer << v1 >> v2;
        ASSERT_TRUE(v1 == v2);
    }

    buffer.clear();

    {
        int64_t v1 = (int64_t)0xFFFFFFFFFFFFFFFF;
        int64_t v2 = 0;

        buffer << v1 >> v2;
        ASSERT_TRUE(v1 == v2);
    }

    buffer.clear();

    {
        uint64_t v1 = (uint64_t)0xFFFFFFFFFFFFFFFF;
        uint64_t v2 = 0;

        buffer << v1 >> v2;
        ASSERT_TRUE(v1 == v2);
    }

    {
        float v1 = 420.69f;
        float v2 = 0.0f;

        buffer << v1 >> v2;
        ASSERT_TRUE(v1 == v2);
    }

    {
        double v1 = 420.69;
        double v2 = 0.0;

        buffer << v1 >> v2;
        ASSERT_TRUE(v1 == v2);
    }
}

TEST(serialize, serializable) {
    __BUFFER buffer;

    {
        base_struct s1{};
        base_struct s2{ 69, "" };

        ASSERT_FALSE(s1.equals(s2));

        buffer << s1 >> s2;
        ASSERT_TRUE(s1.equals(s2));
    }

    buffer.clear();

    {
        derived_struct s1{};
        derived_struct s2{ 69, "", { 567, 8910 }, false };

        ASSERT_FALSE(s1.equals(s2));

        buffer << s1 >> s2;
        ASSERT_TRUE(s1.equals(s2));
    }
}

TEST(serialize, out_of_bounds_fail) {
#ifdef LIBNETWRK_SERIALIZE_TEST_BUFFER_DYNAMIC

    /*
        Try to read out of bounds
    */

    {
        libnetwrk::dynamic_buffer buffer;

        int  a1 = 15;
        int  a2 = 0;
        char b = 0;

        buffer << a1;
        ASSERT_NO_THROW(buffer >> a2);
        ASSERT_TRUE(a1 == a2);
        ASSERT_THROW(buffer >> b, libnetwrk::libnetwrk_exception);
    }

#else
    /*
        Try to write to a 0 size buffer
    */

    {
        libnetwrk::fixed_buffer<0> buffer;
        char a = 1;

        ASSERT_THROW(buffer << a, libnetwrk::libnetwrk_exception);
    }

    /*
        Try to write out of bounds
    */

    {
        libnetwrk::fixed_buffer<2> buffer;
        char a = 1;

        ASSERT_NO_THROW(buffer << a << a);
        ASSERT_THROW(buffer << a, libnetwrk::libnetwrk_exception);
    }

    /*
        Try to read from a 0 size buffer
    */

    {
        libnetwrk::fixed_buffer<0> buffer;
        char a = 1;

        ASSERT_THROW(buffer >> a, libnetwrk::libnetwrk_exception);
    }

    /*
        Try to read out of bounds
    */

    {
        libnetwrk::fixed_buffer<2> buffer;
        char a = 1;

        buffer << a << a;

        ASSERT_NO_THROW(buffer >> a >> a);
        ASSERT_THROW(buffer >> a, libnetwrk::libnetwrk_exception);
    }
    
#endif
}

TEST(serialize, strings) {
    __BUFFER buffer;

    {
        std::string v1("SeRiaLiZE mE");
        std::string v2("abcd");
        ASSERT_FALSE(v1 == v2);

        buffer << v1 >> v2;
        ASSERT_TRUE(v1 == v2);
    }

    buffer.clear();

    {
        std::vector<std::string> v1({ "nxnuNoeuLN", "XjTfSs5loB", "UWp8hsoW5s", "O0c7byqKfj", "CzAXjEObB0" });
        std::vector<std::string> v2({ "ads" });
        ASSERT_FALSE(v1 == v2);

        buffer << v1 >> v2;
        ASSERT_TRUE(v1 == v2);
    }

    buffer.clear();

    {
        std::string v2 = "";

        buffer << "testing" >> v2;
        ASSERT_TRUE(v2 == "testing");
    }
}

TEST(serialize, arrays) {
    __BUFFER buffer;

    /*
        Primitive
    */

    {
        std::array<uint16_t, 6> v1({ 123, 534, 346, 5432, 242, 735 });
        std::array<uint16_t, 6> v2({ 555, 666 });
        ASSERT_FALSE(v1 == v2);

        buffer << v1 >> v2;
        ASSERT_TRUE(v1 == v2);
    }

    buffer.clear();

    /*
        Nested
    */

    {
        std::array<std::array<int, 2>, 3> v1{ { {123, 534}, {346, 5432}, {242, 735} } };
        std::array<std::array<int, 2>, 3> v2({ {555, 666} });
        ASSERT_FALSE(v1 == v2);

        buffer << v1 >> v2;
        ASSERT_TRUE(v1 == v2);
    }

    buffer.clear();

    /*
        User defined serialize
    */

    {
        std::array<derived_struct, 3> v1{ { {69, "a", { 567, 8910 }, false}, {70, "b", { 568, 8911 }, true} } };
        std::array<derived_struct, 3> v2{ { {}, { 71, "c", { 569, 8912 }, false } } };
        ASSERT_FALSE(v1 == v2);

        buffer << v1 >> v2;
        ASSERT_TRUE(v1 == v2);
    }

    buffer.clear();

    /*
        Size doesn't match
    */

    {
        std::array<uint64_t, 6> v1({ 123, 534, 346, 5432, 242, 735 });
        std::array<uint64_t, 5> v2{};
        std::array<uint64_t, 7> v3{};

        ASSERT_NO_THROW(buffer << v1);
        ASSERT_THROW(buffer >> v2, libnetwrk_exception);
        ASSERT_THROW(buffer >> v3, libnetwrk_exception);
    }
}

TEST(serialize, vectors) {
    __BUFFER buffer;

    /*
        Primitive
    */

    {
        std::vector<uint16_t> v1({ 123, 534, 346, 5432, 242, 735 });
        std::vector<uint16_t> v2({ 555, 666 });
        ASSERT_FALSE(v1 == v2);

        buffer << v1 >> v2;
        ASSERT_TRUE(v1 == v2);
    }

    buffer.clear();

    /*
        Nested
    */

    {
        std::vector<std::vector<int>> v1({ {123, 534}, {346, 5432}, {242, 735} });
        std::vector<std::vector<int>> v2({ {555, 666} });
        ASSERT_FALSE(v1 == v2);

        buffer << v1 >> v2;
        ASSERT_TRUE(v1 == v2);
    }

    buffer.clear();

    /*
        User defined serialize
    */

    {
        std::vector<derived_struct> v1({ {69, "a", { 567, 8910 }, false}, {70, "b", { 568, 8911 }, true} });
        std::vector<derived_struct> v2({ {}, { 71, "c", { 569, 8912 }, false } });
        ASSERT_FALSE(v1 == v2);

        buffer << v1 >> v2;
        ASSERT_TRUE(v1 == v2);
    }
}

TEST(serialize, dequeues) {
    __BUFFER buffer;

    /*
        Primitive
    */

    {
        std::deque<uint16_t> v1({ 123, 534, 346, 5432, 242, 735 });
        std::deque<uint16_t> v2({ 555, 666 });
        ASSERT_FALSE(v1 == v2);

        buffer << v1 >> v2;
        ASSERT_TRUE(v1 == v2);
    }

    buffer.clear();

    /*
        Nested
    */

    {
        std::deque<std::deque<int>> v1({ {123, 534}, {346, 5432}, {242, 735} });
        std::deque<std::deque<int>> v2({ {555, 666} });
        ASSERT_FALSE(v1 == v2);

        buffer << v1 >> v2;
        ASSERT_TRUE(v1 == v2);
    }

    buffer.clear();

    /*
        User defined serialize
    */

    {
        std::deque<derived_struct> v1({ {69, "a", { 567, 8910 }, false}, {70, "b", { 568, 8911 }, true} });
        std::deque<derived_struct> v2({ {}, { 71, "c", { 569, 8912 }, false } });
        ASSERT_FALSE(v1 == v2);

        buffer << v1 >> v2;
        ASSERT_TRUE(v1 == v2);
    }
}

TEST(serialize, lists) {
    __BUFFER buffer;

    /*
        Primitive
    */

    {
        std::list<uint16_t> v1({ 123, 534, 346, 5432, 242, 735 });
        std::list<uint16_t> v2({ 555, 666 });
        ASSERT_FALSE(v1 == v2);

        buffer << v1 >> v2;
        ASSERT_TRUE(v1 == v2);
    }

    buffer.clear();

    /*
        Nested
    */

    {
        std::list<std::list<int>> v1({ {123, 534}, {346, 5432}, {242, 735} });
        std::list<std::list<int>> v2({ {555, 666} });
        ASSERT_FALSE(v1 == v2);

        buffer << v1 >> v2;
        ASSERT_TRUE(v1 == v2);
    }

    buffer.clear();

    /*
        User defined serialize
    */

    {
        std::list<derived_struct> v1({ {69, "a", { 567, 8910 }, false}, {70, "b", { 568, 8911 }, true} });
        std::list<derived_struct> v2({ {}, { 71, "c", { 569, 8912 }, false } });
        ASSERT_FALSE(v1 == v2);

        buffer << v1 >> v2;
        ASSERT_TRUE(v1 == v2);
    }
}

TEST(serialize, sets) {
    __BUFFER buffer;

    /*
        Primitive
    */

    {
        std::set<uint16_t> v1({ 123, 534, 346, 5432, 242, 735 });
        std::set<uint16_t> v2({ 555, 666 });
        ASSERT_FALSE(v1 == v2);
        
        buffer << v1 >> v2;
        ASSERT_TRUE(v1 == v2);
    }

    buffer.clear();
}

TEST(serialize, unordered_sets) {
    __BUFFER buffer;

    /*
        Primitive
    */

    {
        std::unordered_set<uint16_t> v1({ 123, 534, 346, 5432, 242, 735 });
        std::unordered_set<uint16_t> v2({ 555, 666 });

        buffer << v1 >> v2;
        ASSERT_TRUE(v2.contains(123));
        ASSERT_TRUE(v2.contains(534));
        ASSERT_TRUE(v2.contains(346));
        ASSERT_TRUE(v2.contains(5432));
        ASSERT_TRUE(v2.contains(242));
        ASSERT_TRUE(v2.contains(735));
        ASSERT_FALSE(v2.contains(555));
        ASSERT_FALSE(v2.contains(666));
    }

    buffer.clear();
}

TEST(serialize, maps) {
    __BUFFER buffer;

    /*
        Primitive
    */

    {
        std::map<int, uint16_t> v1({ {123, 534}, {346, 5432}, {242, 735} });
        std::map<int, uint16_t> v2({ {555, 666} });
        ASSERT_FALSE(v1 == v2);

        buffer << v1 >> v2;
        ASSERT_TRUE(v1 == v2);
    }

    buffer.clear();

    /*
        Nested
    */

    {
        std::map<int, std::map<int, int>> v1;
        v1[123][536]  = 346;
        v1[5432][242] = 735;
        std::map<int, std::map<int, int>> v2;
        v2[567][873] = 1248;

        buffer << v1 >> v2;
        ASSERT_TRUE(v2[123][536]  == 346);
        ASSERT_TRUE(v2[5432][242] == 735);
        ASSERT_FALSE(v2[567][873] == 735);
    }

    buffer.clear();

    /*
        User defined serialize
    */

    {
        std::map<int, derived_struct> v1({ { 32, {69, "a", { 567, 8910 }, false} }, { 557, {70, "b", { 568, 8911 }, true} } });
        std::map<int, derived_struct> v2({ { 987, {32, "a", { 11, 78 }, false} } });
        ASSERT_FALSE(v1 == v2);

        buffer << v1 >> v2;
        ASSERT_TRUE(v1 == v2);
    }
}

TEST(serialize, unordered_maps) {
    __BUFFER buffer;

    /*
        Primitive
    */

    {
        std::unordered_map<int, uint16_t> v1({ {123, 534}, {346, 5432}, {242, 735} });
        std::unordered_map<int, uint16_t> v2({ {555, 666} });
        
        buffer << v1 >> v2;
        ASSERT_TRUE(v1[123]  == 534);
        ASSERT_TRUE(v1[346]  == 5432);
        ASSERT_TRUE(v1[242]  == 735);
        ASSERT_FALSE(v1[555] == 666);
    }

    buffer.clear();

    /*
        Nested
    */

    {
        std::unordered_map<int, std::unordered_map<int, int>> v1;
        v1[123][536]  = 346;
        v1[5432][242] = 735;
        std::unordered_map<int, std::unordered_map<int, int>> v2;
        v2[567][873] = 1248;

        buffer << v1 >> v2;
        ASSERT_TRUE(v2[123][536]  == 346);
        ASSERT_TRUE(v2[5432][242] == 735);
        ASSERT_FALSE(v2[567][873] == 735);
    }

    buffer.clear();

    /*
        User defined serialize
    */

    {
        std::unordered_map<int, derived_struct> v1({ { 32, {69, "a", { 567, 8910 }, false} }, { 557, {70, "b", { 568, 8911 }, true} } });
        std::unordered_map<int, derived_struct> v2({ { 987, {32, "a", { 11, 78 }, false} } });

        buffer << v1 >> v2;
        ASSERT_TRUE(v2.contains(32));
        ASSERT_TRUE(v2.contains(557));
        ASSERT_FALSE(v2.contains(987));
    }
}

TEST(serialize, multiple_containers) {
    __BUFFER buffer;
    
    std::vector<int> v1{ 435921472, 123, 85447 };
    std::vector<int> v2{ 435921474 };
    std::set<int>    s1{ 778151561, 111474, 87916 };
    std::set<int>    s2{ 778151568 };

    buffer << v1 << s1;
    buffer >> v2 >> s2;

    ASSERT_TRUE(v1 == v2);
    ASSERT_TRUE(s1 == s2);
}
