#!/bin/bash

# VitaNS macOS M1 Build Script
# This script builds VitaNS for Nintendo Switch using Docker on macOS M1

set -e  # Exit on any error

echo "🚀 VitaNS macOS M1 Build Script"
echo "=================================="

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    echo "❌ Docker is not installed. Please install Docker Desktop for Mac."
    echo "   Download from: https://www.docker.com/products/docker-desktop"
    exit 1
fi

# Check if Docker is running
if ! docker info &> /dev/null; then
    echo "❌ Docker is not running. Please start Docker Desktop."
    exit 1
fi

echo "✅ Docker is available and running"

# Build the Docker image
echo "🔨 Building Docker image..."
docker build -t vitans-build .

# Create build directory if it doesn't exist
mkdir -p build

# Run the build in Docker
echo "🏗️  Building VitaNS in Docker container..."
docker run --rm -v "$(pwd)/build:/workspace/build" vitans-build bash -c "
    cd /workspace
    mkdir -p build
    cd build
    cmake .. -DCMAKE_TOOLCHAIN_FILE=\$DEVKITPRO/cmake/Switch.cmake
    make -j\$(nproc)
"

echo "✅ Build completed successfully!"
echo ""
echo "📁 Build artifacts are in the 'build' directory"
echo "🎮 Copy VitaNS.nro to your Switch SD card at: /switch/VitaNS/"
echo ""
echo "🔧 To clean the build:"
echo "   rm -rf build/"
echo ""
echo "🐳 To clean Docker image:"
echo "   docker rmi vitans-build" 