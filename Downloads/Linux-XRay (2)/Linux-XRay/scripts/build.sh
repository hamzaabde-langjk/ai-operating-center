#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"
BUILD_TYPE="${1:-Release}"

echo "Linux X-Ray Vision - Build Script"
echo "Build type: $BUILD_TYPE"
echo "=================================="

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
         -DXRAY_BUILD_TESTS=ON \
         -DXRAY_BUILD_BENCHMARKS=ON \
         -DXRAY_BUILD_GUI=ON \
         -DXRAY_ENABLE_EBPF=ON \
         -DXRAY_ENABLE_AI=ON

# Build
echo "Building..."
cmake --build . --parallel $(nproc)

# Run tests
echo "Running tests..."
ctest --output-on-failure

echo "Build complete!"
echo "Binaries are in: $BUILD_DIR"
