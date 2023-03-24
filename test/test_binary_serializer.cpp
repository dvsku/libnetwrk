#define LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#include "libnetwrk.hpp"

#include "utilities_assert.hpp"

using namespace libnetwrk::net::common;

struct simple_struct : public serializable<binary_serializer> {
	uint32_t a = 420;
	std::string b = "";

	simple_struct() {}

	simple_struct(uint32_t a, std::string b) {
		this->a = a;
		this->b = b;
	}
	
	buffer_t serialize() const override {
		buffer_t buffer;
		buffer << a;
		buffer << b;
		return buffer;
	}

	void deserialize(buffer_t serialized) override {
		serialized >> a >> b;
	}

	bool equals(const simple_struct& obj) {
		return a == obj.a && b == obj.b;
	}
};

void serialize_deserialize_standard_layout() {
	buffer buffer;

	int i1 = 156, i2 = 0;
	buffer << i1 >> i2;
	assert(i1 == i2);

	bool b1 = true, b2 = false;
	buffer << b1; 
	buffer >> b2;
	assert(b1 == b2);

	char c1 = 69, c2 = 0;
	buffer << c1 >> c2;
	assert(c1 == c2);

	float f1 = 69.420f, f2 = 0.0f;
	buffer << f1 >> f2;
	assert(f1 == f2);

	double d1 = 420.69, d2 = 0;
	buffer << d1 >> d2;
	assert(d1 == d2);

	wchar_t w1 = 256, w2 = 0;
	buffer << w1 >> w2;
	assert(w1 == w2);
}

void serialize_deserialize_standard_layout_containers() {
	buffer buffer;
	std::vector<int> v1({ 123, 534, 346, 5432, 242, 735 });
	std::vector<int> v2;

	buffer << v1 >> v2;
	assert(v1.size() == v2.size());
	assert(v1 == v2);

	buffer.clear();

	std::deque<int> dq1({ 123, 534, 346, 5432, 242, 735 });
	std::deque<int> dq2;

	buffer << dq1 >> dq2;
	assert(dq1.size() == dq2.size());
	assert(dq1 == dq2);

	buffer.clear();

	std::forward_list<int> fl1({ 123, 534, 346, 5432, 242, 735 });
	std::forward_list<int> fl2;

	buffer << fl1 >> fl2;
	assert(fl1 == fl2);

	buffer.clear();

	std::list<int> l1({ 123, 534, 346, 5432, 242, 735 });
	std::list<int> l2;

	buffer << l1 >> l2;
	assert(l1.size() == l2.size());
	assert(l1 == l2);

	std::array<int, 6> ar1({ 123, 534, 346, 5432, 242, 735 });
	std::array<int, 6> ar2{};

	buffer << ar1 >> ar2;
	assert(ar1 == ar2);
}

void serialize_deserialize_serializable() {
	simple_struct ss1(16, "test_1");
	simple_struct ss2(524, "test_2");
	simple_struct ss3, ss4, ss5;

	buffer buffer;
	buffer << ss1 >> ss3;
	assert(ss1.equals(ss3));

	buffer.clear();

	buffer << ss1 << ss2 >> ss4 >> ss5;
	assert(ss1.equals(ss4));
	assert(ss2.equals(ss5));
}

void serialize_deserialize_strings() {
	buffer buffer;

	std::string s1("SeRiaLiZE mE");
	std::string s2;

	buffer << s1 >> s2;
	assert(s1 == s2);

	std::vector<std::string> vs1({ "nxnuNoeuLN", "XjTfSs5loB", "UWp8hsoW5s", "O0c7byqKfj", "CzAXjEObB0" });
	std::vector<std::string> vs2;

	buffer << vs1 >> vs2;
	for (size_t i = 0; i < vs1.size(); i++)
		assert(vs1[i] == vs2[i]);
}

void serialize_deserialize_unsupported() {
	buffer buffer;
	
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

int main(int argc, char* argv[]) {
	if (argc != 2) {
		serialize_deserialize_standard_layout();
		serialize_deserialize_standard_layout_containers();
		serialize_deserialize_strings();
		serialize_deserialize_serializable();
		serialize_deserialize_unsupported();
	}
	else {
		switch (std::stoi(argv[1])) {
			case 0: serialize_deserialize_standard_layout();				break;
			case 1: serialize_deserialize_standard_layout_containers();		break;
			case 2: serialize_deserialize_strings();						break;
			case 3: serialize_deserialize_serializable();					break;
			case 4: serialize_deserialize_unsupported();					break;
			default: break;
		}
	}

	return 0;
}