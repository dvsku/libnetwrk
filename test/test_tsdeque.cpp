#include "net/common/containers/tsdeque.hpp"

#include <cassert>

using namespace libnetwrk::net::common;

void tsdeque_create() {
	tsdeque<int> deque;
	
	assert(deque.count() == 0);
}

void tsdeque_push_front() {
	tsdeque<int> deque;
	deque.push_front(1);
	deque.push_front(0);
	deque.push_front(2);

	assert(deque.pop_front() == 2);
	assert(deque.pop_front() == 0);
	assert(deque.pop_front() == 1);
}

void tsdeque_push_back() {
	tsdeque<int> deque;
	deque.push_back(1);
	deque.push_back(0);
	deque.push_back(2);

	assert(deque.pop_front() == 1);
	assert(deque.pop_front() == 0);
	assert(deque.pop_front() == 2);
}

void tsdeque_front() {
	tsdeque<int> deque;
	deque.push_back(1);
	deque.push_back(0);
	deque.push_back(2);
	
	assert(deque.front() == 1);
}

void tsdeque_back() {
	tsdeque<int> deque;
	deque.push_back(1);
	deque.push_back(0);
	deque.push_back(2);
	
	assert(deque.back() == 2);
}

void tsdeque_pop_front() {
	tsdeque<int> deque;
	deque.push_back(1);
	deque.push_back(0);
	deque.push_back(2);
	int i = deque.pop_front();

	assert(i == 1);
	assert(deque.front() == 0);
}

void tsdeque_pop_back() {
	tsdeque<int> deque;
	deque.push_back(1);
	deque.push_back(0);
	deque.push_back(2);
	int i = deque.pop_back();

	assert(i == 2);
	assert(deque.back() == 0);
}

void tsdeque_clear() {
	tsdeque<int> deque;
	deque.push_back(1);
	deque.push_back(0);
	deque.push_back(2);
	deque.clear();

	assert(deque.empty() == true);
}

void tsdeque_empty() {
	tsdeque<int> deque;
	deque.push_back(1);
	deque.push_back(0);
	deque.push_back(2);

	assert(deque.empty() == false);

	deque.clear();

	assert(deque.empty() == true);
}

void tsdeque_count() {
	tsdeque<int> deque;
	deque.push_back(1);
	deque.push_back(0);
	deque.push_back(2);

	assert(deque.count() == 3);
}

void run_all_tests() {
	tsdeque_create();
	tsdeque_push_front();
	tsdeque_push_back();
	tsdeque_front();
	tsdeque_back();
	tsdeque_pop_front();
	tsdeque_pop_back();
	tsdeque_clear();
	tsdeque_empty();
	tsdeque_count();
}

int main(int argc, char* argv[]) {
	run_all_tests();
	return 0;
}