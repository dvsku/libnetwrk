<div align="center">
  <h1>libnetwrk</h1>
    <p>
      <span>
        <img src="https://img.shields.io/github/actions/workflow/status/dvsku/libnetwrk/build-windows.yml?branch=main&label=windows"/>
      </span>
      <span>
        <img src="https://img.shields.io/github/actions/workflow/status/dvsku/libnetwrk/build-linux.yml?branch=main&label=linux"/>
      </span>
      <span>
        <img src="https://img.shields.io/github/downloads/dvsku/libnetwrk/total"/>
      </span>
      <span>
        <img src="https://img.shields.io/github/license/dvsku/libnetwrk"/>
      </span>
  </p>
  <p>
    Header-only cross-platform networking library.
  </p>
</div></br>

## About
Based on <a href="https://www.youtube.com/@javidx9">javidx9</a>'s work with networking.<br/> Recommended for use on small personal projects as it's not heavily tested.

## Dependencies
- ASIO 1.30.2 (embedded)

## Requirements
- c++20
- compiler support for concepts and coroutines

## Tested compiler support
- MSVC 14.30 / Visual Studio 2022 17.0
- GCC 10.5 with -fconcepts and -fcoroutines

## Usage
### CMake
libnetwrk TARGET is provided as an INTERFACE that sets up compile flags, definitions and include directories.

- Add libnetwrk via ``ADD_SUBDIRECTORY`` or ``FetchContent``
- Use ``LINK_LIBRARIES(libnetwrk)`` or ``TARGET_LINK_LIBRARIES(your_target PRIVATE|PUBLIC libnetwrk)``
- Include ``libnetwrk.hpp``

### Manual
...

## Limitations
- currently only supports TCP
- ``std::size_t`` clamped to ``uint32_t`` to enable architecture independent serialization/deserialization of STL containers

## Usage examples
### Making a custom object serializeable

```
void serialize(libnetwrk::dynamic_buffer& buffer) const {
    ...
}

void deserialize(libnetwrk::dynamic_buffer& buffer) {
    ...
}
```
To make an object serializable you need to add and implement these functions. </br>

```
#include "libnetwrk.hpp"

struct object {
    std::string string_1;
  
    void serialize(libnetwrk::dynamic_buffer& buffer) const {
        buffer << string_1;
    }
  
    void deserialize(libnetwrk::dynamic_buffer& buffer) {
        buffer >> string_1;
    }
}

struct derived_object : object {
    std::string string_2;
  
    void serialize(libnetwrk::dynamic_buffer& buffer) const {
        object::serialize(buffer);
        buffer << string_2;
    }
  
    void deserialize(libnetwrk::dynamic_buffer& buffer) {
        object::deserialize(buffer);
        buffer >> string_2;
    }
}
```
