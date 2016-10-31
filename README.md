# Zeroconf
C++11 Multicast DNS Client

[![Build Status](https://api.travis-ci.org/yvz/zeroconf.svg?branch=master)](https://travis-ci.org/yvz/zeroconf)

C++11 library intended to discover services and hosts on a local network using [MDNS protocol](https://en.wikipedia.org/wiki/Multicast_DNS).

Briefly the the idea behind MDSN discovery is to empower client applications to find IP addresses of their server counterparts by sending a query to broadcast address (255.255.255.255). MDNS client and servers talk to each other by UDP using packet format similar to generic DNS. The approach is widely used with Apple and IoT devices.

The mainstream implementations of MDNS are [Avahi](https://en.wikipedia.org/wiki/Avahi_(software)) and [Bonjour](https://en.wikipedia.org/wiki/Bonjour_(software)).
While doing good job in most scenarios, they are not uniformly available, or might just be too heavy to bundle for the sake of one simple call.

The library is implemented against both Posix sockets and Winsock using header-only approach. This way it is cross-platform and very light in use.
