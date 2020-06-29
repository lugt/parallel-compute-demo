# Final for Parallel Computing

Author: Guanting Lu, id = 2017152003

## Features

* Quick sort using OpenMP sections
* Merge sort using OpenMP
* Rank sort using OpenMP

## Usage

Executable <config.yml> <random.txt>

## Dependencies

* Simple Web Server (Including LibBoost etc.,)
* Boost.Asio or standalone Asio
* Boost is required to compile
* OpenSSL libraries
* HunterGate
* Jaeger Tracing
* GCC/LLVM/Open64/ICC/... supporting c++14 at least

Installation instructions for the dependencies needed to compile the examples on a selection of platforms can be seen below.
Default build with Boost.Asio is assumed. Turn on CMake option `USE_STANDALONE_ASIO` to instead use standalone Asio.

### Debian based distributions

```sh
sudo apt-get install libssl-dev libboost-filesystem-dev libboost-thread-dev
```