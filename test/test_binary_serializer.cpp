#define LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#include <libnetwrk.hpp>
#include <gtest/gtest.h>

using namespace libnetwrk;

struct simple_struct {
    uint32_t a = 420;
    std::string b = "";

    simple_struct() {}

    simple_struct(uint32_t a, std::string b) {
        this->a = a;
        this->b = b;
    }

    template<typename Tserialize>
    void serialize(buffer<Tserialize>& buffer) const {
        buffer << a;
        buffer << b;
    }

    template<typename Tserialize>
    void deserialize(buffer<Tserialize>& buffer) {
        buffer >> a >> b;
    }

    bool equals(const simple_struct& obj) {
        return a == obj.a && b == obj.b;
    }
};

struct container_struct {
    std::vector<int> a;
    std::deque<int> b;
    std::list<int> c;
    std::forward_list<int> d;

    template<typename Tserialize>
    void serialize(buffer<Tserialize>& buffer) const {
        buffer << a << b << c << d;
    }

    template<typename Tserialize>
    void deserialize(buffer<Tserialize>& buffer) {
        buffer >> a >> b >> c >> d;
    }

    bool equals(const container_struct& obj) {
        return a == obj.a && b == obj.b && c == obj.c && d == obj.d;
    }
};

struct string_struct {
    std::string a;

    template<typename Tserialize>
    void serialize(buffer<Tserialize>& buffer) const {
        buffer << a;
    }

    template<typename Tserialize>
    void deserialize(buffer<Tserialize>& buffer) {
        buffer >> a;
    }

    bool equals(const string_struct& obj) {
        return a == obj.a;
    }
};

TEST(bin_serializer, serialize_deserialize_standard_layout) {
    buffer<bin_serialize> buffer;

    int i1 = 156, i2 = 0;
    buffer << i1 >> i2;
    EXPECT_TRUE(i1 == i2);

    bool b1 = true, b2 = false;
    buffer << b1;
    buffer >> b2;
    EXPECT_TRUE(b1 == b2);

    char c1 = 69, c2 = 0;
    buffer << c1 >> c2;
    EXPECT_TRUE(c1 == c2);

    float f1 = 69.420f, f2 = 0.0f;
    buffer << f1 >> f2;
    EXPECT_TRUE(f1 == f2);

    double d1 = 420.69, d2 = 0;
    buffer << d1 >> d2;
    EXPECT_TRUE(d1 == d2);

    wchar_t w1 = 256, w2 = 0;
    buffer << w1 >> w2;
    EXPECT_TRUE(w1 == w2);
}

TEST(bin_serializer, serialize_deserialize_standard_layout_containers) {
    buffer<bin_serialize> buffer;
    std::vector<int> v1({ 123, 534, 346, 5432, 242, 735 });
    std::vector<int> v2;

    buffer << v1 >> v2;
    EXPECT_TRUE(v1.size() == v2.size());
    EXPECT_TRUE(v1 == v2);

    buffer.clear();

    std::deque<int> dq1({ 123, 534, 346, 5432, 242, 735 });
    std::deque<int> dq2;

    buffer << dq1 >> dq2;
    EXPECT_TRUE(dq1.size() == dq2.size());
    EXPECT_TRUE(dq1 == dq2);

    buffer.clear();

    std::forward_list<int> fl1({ 123, 534, 346, 5432, 242, 735 });
    std::forward_list<int> fl2;

    buffer << fl1 >> fl2;
    EXPECT_TRUE(fl1 == fl2);

    buffer.clear();

    std::list<int> l1({ 123, 534, 346, 5432, 242, 735 });
    std::list<int> l2;

    buffer << l1 >> l2;
    EXPECT_TRUE(l1.size() == l2.size());
    EXPECT_TRUE(l1 == l2);

    std::array<int, 6> ar1({ 123, 534, 346, 5432, 242, 735 });
    std::array<int, 6> ar2{};

    buffer << ar1 >> ar2;
    EXPECT_TRUE(ar1 == ar2);
}

TEST(bin_serializer, serialize_deserialize_serializable) {
    simple_struct ss1(16, "test_1");
    simple_struct ss2(524, "test_2");
    simple_struct ss3, ss4, ss5;

    buffer<bin_serialize> buffer;
    buffer << ss1 >> ss3;
    EXPECT_TRUE(ss1.equals(ss3));

    buffer.clear();

    buffer << ss1 << ss2 >> ss4 >> ss5;
    EXPECT_TRUE(ss1.equals(ss4));
    EXPECT_TRUE(ss2.equals(ss5));
}

TEST(bin_serializer, serialize_deserialize_strings) {
    buffer<bin_serialize> buffer;

    std::string s1("SeRiaLiZE mE");
    std::string s2;

    buffer << s1 >> s2;
    EXPECT_TRUE(s1 == s2);

    std::vector<std::string> vs1({ "nxnuNoeuLN", "XjTfSs5loB", "UWp8hsoW5s", "O0c7byqKfj", "CzAXjEObB0" });
    std::vector<std::string> vs2;

    buffer << vs1 >> vs2;
    for (size_t i = 0; i < vs1.size(); i++)
        EXPECT_TRUE(vs1[i] == vs2[i]);
}

TEST(bin_serializer, serialize_deserialize_unsupported) {
    buffer<bin_serialize> buffer;

    std::stack<int> stack;
    EXPECT_ANY_THROW(buffer << stack);

    std::queue<int> queue;
    EXPECT_ANY_THROW(buffer << queue);

    std::priority_queue<int> pqueue;
    EXPECT_ANY_THROW(buffer << pqueue);

    std::set<int> set;
    EXPECT_ANY_THROW(buffer << set);

    std::multiset<int> mset;
    EXPECT_ANY_THROW(buffer << mset);

    std::unordered_set<int> uset;
    EXPECT_ANY_THROW(buffer << uset);

    std::unordered_multiset<int> umset;
    EXPECT_ANY_THROW(buffer << umset);

    std::map<int, int> map;
    EXPECT_ANY_THROW(buffer << map);

    std::multimap<int, int> mmap;
    EXPECT_ANY_THROW(buffer << mmap);

    std::unordered_map<int, int> umap;
    EXPECT_ANY_THROW(buffer << umap);

    std::unordered_multimap<int, int> ummap;
    EXPECT_ANY_THROW(buffer << ummap);
}

TEST(bin_serializer, serialize_deserialize_multiple_containers) {
    buffer<bin_serialize> buffer;
    container_struct cs1, cs2;

    cs1.a.push_back(435921472);
    cs1.a.push_back(123);
    cs1.a.push_back(85447);

    cs1.b.push_back(476516);
    cs1.b.push_back(176541697);

    cs1.c.push_back(778987562);

    cs1.d.push_front(778151561);
    cs1.d.push_front(111474);
    cs1.d.push_front(87916);

    buffer << cs1;
    buffer >> cs2;

    EXPECT_TRUE(cs1.a.size() == cs2.a.size());
    EXPECT_TRUE(cs1.b.size() == cs2.b.size());
    EXPECT_TRUE(cs1.c.size() == cs2.c.size());
    EXPECT_TRUE(std::distance(cs1.d.begin(), cs1.d.end()) == std::distance(cs2.d.begin(), cs2.d.end()));

    EXPECT_TRUE(std::equal(cs1.a.begin(), cs1.a.end(), cs2.a.begin()));
    EXPECT_TRUE(std::equal(cs1.b.begin(), cs1.b.end(), cs2.b.begin()));
    EXPECT_TRUE(std::equal(cs1.c.begin(), cs1.c.end(), cs2.c.begin()));
    EXPECT_TRUE(std::equal(cs1.d.begin(), cs1.d.end(), cs2.d.begin()));
}
