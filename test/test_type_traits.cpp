#define LIBNETWRK_THROW_INSTEAD_OF_STATIC_ASSERT
#include "libnetwrk.hpp"

#include <cassert>

#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace libnetwrk::net::common;

void type_trait_is_serializable() {

}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        type_trait_is_serializable();
    }
    else {
        switch (std::stoi(argv[1])) {
            case 0: type_trait_is_serializable();            break;
            default: break;
        }
    }

    return 0;
}