#pragma once

namespace libnetwrk {
    class non_moveable {
    public:
        non_moveable()                    = default;
        non_moveable(non_moveable&&)    = delete;
        virtual ~non_moveable()            = default;
        
        non_moveable& operator= (non_moveable&&) = delete;
    };
}