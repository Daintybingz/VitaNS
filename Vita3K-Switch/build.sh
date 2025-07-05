#!/bin/bash

# Check if DEVKITPRO is set
if [ -z "$DEVKITPRO" ]; then
    echo "Error: DEVKITPRO environment variable is not set"
    echo "Please install devkitPro and set the DEVKITPRO environment variable"
    exit 1
fi

# Create build directory
mkdir -p build
cd build

# Configure with CMake (Switch only)
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/Switch.cmake \
    -DCMAKE_BUILD_TYPE=Release

# Build
make -j$(nproc)

# Create NRO
make VitaNS.nro

# Check if build was successful
if [ -f "VitaNS.nro" ]; then
    echo "Build successful! NRO file created at build/VitaNS.nro"
    
    # Create release directory
    mkdir -p ../release
    cp VitaNS.nro ../release/
    cp -r ../resources ../release/
    
    echo "Release package created in release directory"
    echo "You can now copy the contents of the release directory to your Switch's SD card"
else
    echo "Build failed! Check the error messages above"
    exit 1
fi 