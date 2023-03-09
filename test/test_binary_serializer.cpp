#include "libnetwrk.hpp"

#include <cassert>
#include <vector>
#include <string>

using namespace libnetwrk::net::common;

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
	for (size_t i = 0; i < v1.size(); i++)
		assert(v1[i] == v2[i]);
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

int main(int argc, char* argv[]) {
	if (argc != 2) return -1;

	switch (std::stoi(argv[1])) {
		case 0: serialize_deserialize_standard_layout();				break;
		case 1: serialize_deserialize_standard_layout_containers();		break;
		case 2: serialize_deserialize_strings();						break;
		default: break;
	}

	return 0;
}