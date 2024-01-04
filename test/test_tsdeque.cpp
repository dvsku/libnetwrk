#define LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#include "libnetwrk.hpp"
#include "utilities_assert.hpp"

using namespace libnetwrk;

void tsdeque_create() {
    tsdeque<int> deque;
    
    ASSERT(deque.count() == 0);
}

void tsdeque_push_front() {
    tsdeque<int> deque;
    deque.push_front(1);
    deque.push_front(0);
    deque.push_front(2);

    ASSERT(deque.pop_front() == 2);
    ASSERT(deque.pop_front() == 0);
    ASSERT(deque.pop_front() == 1);
}

void tsdeque_push_back() {
    tsdeque<int> deque;
    deque.push_back(1);
    deque.push_back(0);
    deque.push_back(2);

    ASSERT(deque.pop_front() == 1);
    ASSERT(deque.pop_front() == 0);
    ASSERT(deque.pop_front() == 2);
}

void tsdeque_front() {
    tsdeque<int> deque;
    deque.push_back(1);
    deque.push_back(0);
    deque.push_back(2);
    
    ASSERT(deque.front() == 1);
}

void tsdeque_back() {
    tsdeque<int> deque;
    deque.push_back(1);
    deque.push_back(0);
    deque.push_back(2);
    
    ASSERT(deque.back() == 2);
}

void tsdeque_pop_front() {
    tsdeque<int> deque;
    deque.push_back(1);
    deque.push_back(0);
    deque.push_back(2);
    int i = deque.pop_front();

    ASSERT(i == 1);
    ASSERT(deque.front() == 0);
}

void tsdeque_pop_back() {
    tsdeque<int> deque;
    deque.push_back(1);
    deque.push_back(0);
    deque.push_back(2);
    int i = deque.pop_back();

    ASSERT(i == 2);
    ASSERT(deque.back() == 0);
}

void tsdeque_clear() {
    tsdeque<int> deque;
    deque.push_back(1);
    deque.push_back(0);
    deque.push_back(2);
    deque.clear();

    ASSERT(deque.empty() == true);
}

void tsdeque_empty() {
    tsdeque<int> deque;
    deque.push_back(1);
    deque.push_back(0);
    deque.push_back(2);

    ASSERT(deque.empty() == false);

    deque.clear();

    ASSERT(deque.empty() == true);
}

void tsdeque_count() {
    tsdeque<int> deque;
    deque.push_back(1);
    deque.push_back(0);
    deque.push_back(2);

    ASSERT(deque.count() == 3);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
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
    else {
        switch (std::stoi(argv[1])) {
            case 0: tsdeque_create();     break;
            case 1: tsdeque_push_front(); break;
            case 2: tsdeque_push_back();  break;
            case 3: tsdeque_front();      break;
            case 4: tsdeque_back();       break;
            case 5: tsdeque_pop_front();  break;
            case 6: tsdeque_pop_back();   break;
            case 7: tsdeque_clear();      break;
            case 8: tsdeque_empty();      break;
            case 9: tsdeque_count();      break;
            default: break;
        }
    }

    return 0;
}