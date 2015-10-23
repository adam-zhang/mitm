# MITM

Copyright © 2015 INRA

Gauthier Quesnel <gauthier.quesnel@toulouse.inra.fr>

The software is released under the MIT license. See the COPYING file.

**mitm** is a free implementation of the Wedelin Heuristic for Integer
Programming. It implements several algorithms: simple algorithm, heuristics
using C++11, Eigen3 or Cuda.

## Requirements

* eigen3 (≥ 3)
* cmake (≥ 2.8)
* make (≥ 1.8)
* c++ compiler (gcc ≥ 4.8, clang ≥ 3.3, intel icc (≥ 11.0).

For recent Debian and Ubuntu derivatives (remove clang to only use gcc):

```bash
apt-get install build-essential cmake clang libboost-dev libeigen3-dev
```

## Compilation

Compiling and installing:

```bash
cd mitm
mkdir build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release ..
make
make install
```

To use clang replace the previous `cmake` command:

```
CXX=clang++-libc++ cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release ..
```
