#pragma once

namespace libnetwrk {
    // Forward declare buffer
    template<typename Tserialize>
    class buffer;

    // Base struct for every user made serializable object
    template <typename Tserialize>
    struct serializable {
        using buffer_t = buffer<Tserialize>;

        // Serializes struct to buffer
        virtual void serialize(buffer_t& buffer) const = 0;

        // Deserializes a struct from a buffer
        virtual void deserialize(buffer_t& serialized) = 0;
    };
}
