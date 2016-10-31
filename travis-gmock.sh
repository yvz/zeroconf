#!/bin/bash
mkdir google-mock
wget https://github.com/google/googletest/archive/release-1.8.0.tar.gz
tar xf release-1.8.0.tar.gz
pushd googletest-release-1.8.0
cmake -DCMAKE_INSTALL_PREFIX:PATH=./ .
make && make install
popd
