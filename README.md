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
    Cross-platform header only networking library.
  </p>
</div></br></br>

## About
Based heavily on <a href="https://www.youtube.com/@javidx9">javidx9</a>'s work with networking. Relies on embedded ASIO 1.14.0 library. <br/> Recommended for use on small personal projects as it's not heavily tested.

## Requirements
- c++17 or later compiler

## Usage
- Copy libnetwrk folder to your project
- Include ``libnetwrk`` directories in your project
- Include ``libnetwrk.hpp``

## Limitations
- currently only supports TCP
- libnetwrk uses a simple binary serializer by default, so cross-platform compatibility is not guaranteed. You can implement your own serializer by looking at ``binary_serializer.hpp`` as an example.
