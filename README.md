<div align="center">
  <h1>libnetwrk</h1>
    <p>
    <a href="#">
      <img src="https://img.shields.io/github/actions/workflow/status/dvsku/libnetwrk/build.yml?branch=main&label=build%20and%20tests"/>
    </a>
    <a href="#">
      <img src="https://img.shields.io/github/downloads/dvsku/libnetwrk/total"/>
    </a>
     <a href="#">
      <img src="https://img.shields.io/github/license/dvsku/libnetwrk"/>
    </a>
  </p>
  <p>
    Cross-platform networking library.
  </p>
</div></br>

## About
Based heavily on <a href="https://www.youtube.com/@javidx9">javidx9</a>'s work with networking. Relies on embedded ASIO 1.14.0 library. <br/> Recommended for use on small personal projects as it's not heavily tested.

## Requirements
- c++20 or later compiler

## Usage
- Copy libnetwrk directory to your project
- Include ``libnetwrk`` directory in your project
- Include ``libnetwrk.hpp``

## Limitations
- currently only supports TCP
- libnetwrk uses a simple binary serializer by default, so cross-platform compatibility is not guaranteed. You can implement your own serializer by looking at ``bin_serialize.hpp`` as an example.

## Usage examples
### Making a custom object serializeable

```
template<typename T>
void serialize(libnetwrk::buffer<T>& buffer) const {
    ...
}

template<typename T>
void deserialize(libnetwrk::buffer<T>& buffer) {
    ...
}
```
To make an object serializable you need to add and implement these functions. </br>

```
#include "libnetwrk.hpp"

struct object {
    std::string string_1;
  
    template<typename T>
    void serialize(libnetwrk::buffer<T>& buffer) const {
        buffer << string_1;
    }
  
    template<typename T>
    void deserialize(libnetwrk::buffer<T>& buffer) {
        buffer >> string_1;
    }
}

struct derived_object : object {
    std::string string_2;
  
    template<typename T>
    void serialize(libnetwrk::buffer<T>& buffer) const {
        object::serialize(buffer);
        buffer << string_2;
    }
  
    template<typename T>
    void deserialize(libnetwrk::buffer<T>& buffer) {
        object::deserialize(buffer);
        buffer >> string_2;
    }
}
```
## Changes

- 05 Jan 2024
    - Changed custom object serialization. Previous serialization requried a separate object for each serializer type.
