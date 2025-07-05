#!/bin/bash
# Script to download and build Dynarmic for VitaNS

set -e

# Create externals directory if it doesn't exist
EXTERNALS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)/externals"
mkdir -p "$EXTERNALS_DIR"
cd "$EXTERNALS_DIR"

# Clone Dynarmic if it doesn't exist
if [ ! -d "dynarmic" ]; then
    echo "Cloning Dynarmic repository..."
    git clone https://github.com/merryhime/dynarmic.git --recursive
    cd dynarmic
else
    echo "Updating Dynarmic repository..."
    cd dynarmic
    git pull
    git submodule update --init --recursive
fi

# Create build directory
mkdir -p build
cd build

# Configure and build
echo "Configuring Dynarmic..."
cmake .. -DCMAKE_BUILD_TYPE=Release -DDYNARMIC_TESTS=OFF -DDYNARMIC_WARNINGS_AS_ERRORS=OFF

echo "Building Dynarmic..."
cmake --build . -j$(nproc 2>/dev/null || sysctl -n hw.ncpu)

echo "Dynarmic build complete!"
