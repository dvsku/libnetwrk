#define LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#include "libnetwrk.hpp"
#include "utilities_assert.hpp"

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

static void serialize_deserialize_standard_layout() {
    buffer<bin_serialize> buffer;

    int i1 = 156, i2 = 0;
    buffer << i1 >> i2;
    ASSERT(i1 == i2);

    bool b1 = true, b2 = false;
    buffer << b1;
    buffer >> b2;
    ASSERT(b1 == b2);

    char c1 = 69, c2 = 0;
    buffer << c1 >> c2;
    ASSERT(c1 == c2);

    float f1 = 69.420f, f2 = 0.0f;
    buffer << f1 >> f2;
    ASSERT(f1 == f2);

    double d1 = 420.69, d2 = 0;
    buffer << d1 >> d2;
    ASSERT(d1 == d2);

    wchar_t w1 = 256, w2 = 0;
    buffer << w1 >> w2;
    ASSERT(w1 == w2);
}

static void serialize_deserialize_standard_layout_containers() {
    buffer<bin_serialize> buffer;
    std::vector<int> v1({ 123, 534, 346, 5432, 242, 735 });
    std::vector<int> v2;

    buffer << v1 >> v2;
    ASSERT(v1.size() == v2.size());
    ASSERT(v1 == v2);

    buffer.clear();

    std::deque<int> dq1({ 123, 534, 346, 5432, 242, 735 });
    std::deque<int> dq2;

    buffer << dq1 >> dq2;
    ASSERT(dq1.size() == dq2.size());
    ASSERT(dq1 == dq2);

    buffer.clear();

    std::forward_list<int> fl1({ 123, 534, 346, 5432, 242, 735 });
    std::forward_list<int> fl2;

    buffer << fl1 >> fl2;
    ASSERT(fl1 == fl2);

    buffer.clear();

    std::list<int> l1({ 123, 534, 346, 5432, 242, 735 });
    std::list<int> l2;

    buffer << l1 >> l2;
    ASSERT(l1.size() == l2.size());
    ASSERT(l1 == l2);

    std::array<int, 6> ar1({ 123, 534, 346, 5432, 242, 735 });
    std::array<int, 6> ar2{};

    buffer << ar1 >> ar2;
    ASSERT(ar1 == ar2);
}

static void serialize_deserialize_serializable() {
    simple_struct ss1(16, "test_1");
    simple_struct ss2(524, "test_2");
    simple_struct ss3, ss4, ss5;

    buffer<bin_serialize> buffer;
    buffer << ss1 >> ss3;
    ASSERT(ss1.equals(ss3));

    buffer.clear();

    buffer << ss1 << ss2 >> ss4 >> ss5;
    ASSERT(ss1.equals(ss4));
    ASSERT(ss2.equals(ss5));
}

static void serialize_deserialize_strings() {
    buffer<bin_serialize> buffer;

    std::string s1("SeRiaLiZE mE");
    std::string s2;

    buffer << s1 >> s2;
    ASSERT(s1 == s2);

    std::vector<std::string> vs1({ "nxnuNoeuLN", "XjTfSs5loB", "UWp8hsoW5s", "O0c7byqKfj", "CzAXjEObB0" });
    std::vector<std::string> vs2;

    buffer << vs1 >> vs2;
    for (size_t i = 0; i < vs1.size(); i++)
        ASSERT(vs1[i] == vs2[i]);
}

static void serialize_deserialize_unsupported() {
    buffer<bin_serialize> buffer;

    std::stack<int> stack;
    ASSERT_THROWS(buffer << stack);

    std::queue<int> queue;
    ASSERT_THROWS(buffer << queue);

    std::priority_queue<int> pqueue;
    ASSERT_THROWS(buffer << pqueue);

    std::set<int> set;
    ASSERT_THROWS(buffer << set);

    std::multiset<int> mset;
    ASSERT_THROWS(buffer << mset);

    std::unordered_set<int> uset;
    ASSERT_THROWS(buffer << uset);

    std::unordered_multiset<int> umset;
    ASSERT_THROWS(buffer << umset);

    std::map<int, int> map;
    ASSERT_THROWS(buffer << map);

    std::multimap<int, int> mmap;
    ASSERT_THROWS(buffer << mmap);

    std::unordered_map<int, int> umap;
    ASSERT_THROWS(buffer << umap);

    std::unordered_multimap<int, int> ummap;
    ASSERT_THROWS(buffer << ummap);
}

static void serialize_deserialize_multiple_containers() {
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

    ASSERT(cs1.a.size() == cs2.a.size());
    ASSERT(cs1.b.size() == cs2.b.size());
    ASSERT(cs1.c.size() == cs2.c.size());
    ASSERT(std::distance(cs1.d.begin(), cs1.d.end()) == std::distance(cs2.d.begin(), cs2.d.end()));

    ASSERT(std::equal(cs1.a.begin(), cs1.a.end(), cs2.a.begin()));
    ASSERT(std::equal(cs1.b.begin(), cs1.b.end(), cs2.b.begin()));
    ASSERT(std::equal(cs1.c.begin(), cs1.c.end(), cs2.c.begin()));
    ASSERT(std::equal(cs1.d.begin(), cs1.d.end(), cs2.d.begin()));
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        serialize_deserialize_standard_layout();
        serialize_deserialize_standard_layout_containers();
        serialize_deserialize_strings();
        serialize_deserialize_serializable();
        serialize_deserialize_unsupported();
        serialize_deserialize_multiple_containers();
    }
    else {
        switch (std::stoi(argv[1])) {
            case 0: serialize_deserialize_standard_layout();            break;
            case 1: serialize_deserialize_standard_layout_containers(); break;
            case 2: serialize_deserialize_strings();                    break;
            case 3: serialize_deserialize_serializable();               break;
            case 4: serialize_deserialize_unsupported();                break;
            case 5: serialize_deserialize_multiple_containers();        break;
            default: break;
        }
    }

    return 0;
}