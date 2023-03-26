#define LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#include "libnetwrk.hpp"
#include "utilities_assert.hpp"

using namespace libnetwrk::net::common;

void tslist_create() {
	tslist<int> list;

	ASSERT(list.count() == 0);
}

void tslist_push_front() {
	tslist<int> list;
	list.push_front(1);
	list.push_front(0);
	list.push_front(2);

	ASSERT(list.pop_front() == 2);
	ASSERT(list.pop_front() == 0);
	ASSERT(list.pop_front() == 1);
}

void tslist_push_back() {
	tslist<int> list;
	list.push_back(1);
	list.push_back(0);
	list.push_back(2);

	ASSERT(list.pop_front() == 1);
	ASSERT(list.pop_front() == 0);
	ASSERT(list.pop_front() == 2);
}

void tslist_front() {
	tslist<int> list;
	list.push_back(1);
	list.push_back(0);
	list.push_back(2);

	ASSERT(list.front() == 1);
}

void tslist_back() {
	tslist<int> list;
	list.push_back(1);
	list.push_back(0);
	list.push_back(2);

	ASSERT(list.back() == 2);
}

void tslist_pop_front() {
	tslist<int> list;
	list.push_back(1);
	list.push_back(0);
	list.push_back(2);
	int i = list.pop_front();

	ASSERT(i == 1);
	ASSERT(list.front() == 0);
}

void tslist_pop_back() {
	tslist<int> list;
	list.push_back(1);
	list.push_back(0);
	list.push_back(2);
	int i = list.pop_back();

	ASSERT(i == 2);
	ASSERT(list.back() == 0);
}

void tslist_clear() {
	tslist<int> list;
	list.push_back(1);
	list.push_back(0);
	list.push_back(2);
	list.clear();

	ASSERT(list.empty() == true);
}

void tslist_empty() {
	tslist<int> list;
	list.push_back(1);
	list.push_back(0);
	list.push_back(2);

	ASSERT(list.empty() == false);

	list.clear();

	ASSERT(list.empty() == true);
}

void tslist_count() {
	tslist<int> list;
	list.push_back(1);
	list.push_back(0);
	list.push_back(2);

	ASSERT(list.count() == 3);
}

void tslist_foreach() {
	tslist<int> list;
	list.push_back(1);
	list.push_back(0);
	list.push_back(2);

	ASSERT_THROWS(list.foreach([](double d) {}));
	ASSERT_THROWS(list.foreach([](int i) {}));
	ASSERT_THROWS(list.foreach([](const int i) {}));
	ASSERT_THROWS(list.foreach([](const int& i) {}));
	//ASSERT_THROWS(list.foreach([](int* i) {}));				// correctly doesn't compile
	//ASSERT_THROWS(list.foreach([](int&& i) {}));				// correctly doesn't compile
	//ASSERT_THROWS(list.foreach([](int** i) {}));				// correctly doesn't compile
	//ASSERT_THROWS(list.foreach([](const int* i) {}));			// correctly doesn't compile
	//ASSERT_THROWS(list.foreach([](const int&& i) {}));		// correctly doesn't compile
	//ASSERT_THROWS(list.foreach([](const int** i) {}));		// correctly doesn't compile
	ASSERT_NOT_THROWS(list.foreach([](int& i) {}));

	list.foreach([](int& i) {
		i++;
	});

	ASSERT(list.pop_front() == 2);
	ASSERT(list.pop_front() == 1);
	ASSERT(list.pop_front() == 3);

	return;
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		tslist_create();
		tslist_push_front();
		tslist_push_back();
		tslist_front();
		tslist_back();
		tslist_pop_front();
		tslist_pop_back();
		tslist_clear();
		tslist_empty();
		tslist_count();
		tslist_foreach();
	}
	else {
		switch (std::stoi(argv[1])) {
			case 0: tslist_create();			break;
			case 1: tslist_push_front();		break;
			case 2: tslist_push_back();			break;
			case 3: tslist_front();				break;
			case 4: tslist_back();				break;
			case 5: tslist_pop_front();			break;
			case 6: tslist_pop_back();			break;
			case 7: tslist_clear();				break;
			case 8: tslist_empty();				break;
			case 9: tslist_count();				break;
			case 10: tslist_foreach();			break;
			default:							break;
		}
	}

	return 0;
}