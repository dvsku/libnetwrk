#pragma once

namespace libnetwrk {
    class non_copyable {
    public:
        non_copyable()                        = default;
        non_copyable(const non_copyable&)    = delete;
        virtual ~non_copyable()                = default;

        non_copyable& operator= (const non_copyable&) = delete;
    };
}