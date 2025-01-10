# Build Windows
Not supported by lithium

# Build Linux
## This is tested for Debian 12, maybe is working elsewhere
git submodule update --init --recursive
apt install cmake build-essential libcurl4-openssl-dev libssl-dev libboost-context-dev

mkdir build
cd build 
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j${nproc} GdtCache
