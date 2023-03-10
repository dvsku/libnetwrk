<div align="center">
  <h1>libnetwrk</h1>
    <p>
    <a href="#">
      <img src="https://img.shields.io/github/actions/workflow/status/dvsku/libnetwrk/build.yml?branch=main"/>
    </a>
    <a href="#">
      <img src="https://img.shields.io/github/actions/workflow/status/dvsku/libnetwrk/tests.yml?branch=main"/>
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
Based heavily on javidx9's work with networking. Relies on included ASIO library for all IO operations. <br/> Recommended for use on small personal projects as it's not heavily tested.

## Usage
- Copy libnetwrk folder to your project
- Include ``libnetwrk`` and ``libnetwrk/lib/asio`` directories in your project
- Include ``libnetwrk.hpp``

## Limitations
- currently only supports TCP
- libnetwrk uses a simple binary serializer by default, so cross-platform compatibility is not guaranteed. You can implement your own serializer by looking at ``binary_serializer.hpp`` as an example.
