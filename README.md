# Zeroconf
C++11 Multicast DNS Client

[![Build Status](https://api.travis-ci.org/yvz/zeroconf.svg?branch=master)](https://travis-ci.org/yvz/zeroconf)

C++11 library intended to discover services and hosts on a local network using [MDNS protocol](https://en.wikipedia.org/wiki/Multicast_DNS).

### Overview

The idea behind MDSN discovery is to allow client applications to find IP addresses of their server counterparts by sending a query to broadcast address (255.255.255.255). MDNS client and servers talk to each other by UDP, using packet format similar to generic DNS. The approach is widely used with Apple and IoT devices.

The mainstream implementations of MDNS are [Avahi](https://en.wikipedia.org/wiki/Avahi_(software)) and [Bonjour](https://en.wikipedia.org/wiki/Bonjour_(software)).
While doing good job in most scenarios, they are not uniformly available, or might just be too heavy to bundle for the sake of one simple call.

This library comprises a cross-platform and very light MDNS client. It's implemented against both Posix sockets and Winsock using header-only approach.

### Content

src/zeroconf-detail.hpp -- data structures, domain logic, networking logic
src/zeroconf-util.hpp -- helpers
src/zeroconf.hpp -- client interface

test -- unit tests

samples/basic_demo/main.cpp -- console demo app that sends a query and displays the answers

![basic_demo](/samples/basic_demo/screenshot.png?raw=true)

### Quick Start

1. Import library sources from src directory to the project

2. Include zerconf.hpp and make a call to Zeroconf::Resolve. There's only a synchronous version of the API. 

  ```c++
  #include "zeroconf.hpp"
  
  std::vector<Zeroconf::mdns_responce> result;
  bool st = Zeroconf::Resolve("_http._tcp.local", /*scanTime*/ 3, &result);
  ```

3. Access the result as follows:

  ```c++
  result[i].peer                 // Address of the responded machine
  result[i].records              // Resource records of the answer
  result[i].records[j].type;     // The type of the RR
  result[i].records[j].pos;      // The offset of the RR, starting with 0xC0 
  result[i].records[j].len;      // Full length of the RR
  result[i].records[j].name;     // Name of the node to which the record belongs
  ```

4. In case of failure, Zeroconf::Resolve returns false and provides diagnostic output to the client's callback:

  ```c++
  Zeroconf::SetLogCallback([](Zeroconf::LogLevel level, const std::string& message) { ... });
  ```
