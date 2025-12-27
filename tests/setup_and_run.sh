#!/bin/bash
# Setup and run CFL unit tests

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "=== CFL Test Suite Setup ==="

# Check if Unity exists
if [ ! -d "unity" ]; then
    echo "Unity test framework not found. Downloading..."
    git clone --depth 1 https://github.com/ThrowTheSwitch/Unity.git unity
    echo "✓ Unity downloaded successfully"
else
    echo "✓ Unity test framework found"
fi

# Create build directory
echo ""
echo "=== Building Tests ==="
mkdir -p build
cd build

# Run CMake
echo "Running CMake..."
cmake ..

# Build
echo "Building test executables..."
make

# Run tests
echo ""
echo "=== Running Tests ==="
make run_tests

echo ""
echo "=== Test Summary ==="
echo "All tests completed successfully!"
echo ""
echo "To run tests again:"
echo "  cd tests/build && make run_tests"
echo ""
echo "To run individual tests:"
echo "  cd tests/build && ./test_cfl_crc"
echo "  cd tests/build && ./test_cfl_header"
echo "  cd tests/build && ./test_cfl_serialization"
echo "  cd tests/build && ./test_cfl_validation"
echo "  cd tests/build && ./test_cfl_builders"
