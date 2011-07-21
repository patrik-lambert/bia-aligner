#!/bin/bash
#mkdir -p bin include
install -d build
pushd build
cmake -DCMAKE_INSTALL_PREFIX=. -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
# cmake -DCMAKE_INSTALL_PREFIX=$1  ..
make -j4 && \
 make install
popd
