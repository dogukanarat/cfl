# CFL Unit Tests

Comprehensive unit test suite for the CFL (Control Freak Lite) library using the Unity test framework.

## Test Coverage

The test suite includes:

- **test_cfl_crc.c** - CRC-16-CCITT implementation tests
  - NULL pointer handling
  - Known test vectors (validates against CRC-16-CCITT standard)
  - Determinism and error detection
  - Incremental CRC computation
  - Single-bit error detection

- **test_cfl_header.c** - Header manipulation functions
  - Initialization
  - Sequence and length setting
  - CRC computation
  - Flag manipulation (set, clear, has_flag)
  - All flag combinations

- **test_cfl_serialization.c** - Serialization/deserialization
  - NULL pointer and buffer size validation
  - Header-only and with-payload serialization
  - Round-trip testing
  - Maximum payload handling
  - Buffer overflow protection

- **test_cfl_validation.c** - Validation functions
  - Header validation (sync, version, length)
  - Message validation (CRC, payload consistency)
  - Error detection (corruption, mismatches)
  - Error string functions

- **test_cfl_builders.c** - Message builder functions
  - Request, reply, ACK, NACK, push builders
  - ID and sequence preservation
  - Integration flows (request-reply, request-ack)

## Prerequisites

### Unity Test Framework

The tests use the Unity test framework. You need to download it first:

```bash
cd tests
git clone https://github.com/ThrowTheSwitch/Unity.git unity
```

Alternatively, you can add Unity as a git submodule:

```bash
cd /path/to/cfl
git submodule add https://github.com/ThrowTheSwitch/Unity.git tests/unity
git submodule update --init --recursive
```

### Build Tools

- CMake 3.10 or later
- GCC, Clang, or compatible C compiler
- Make or Ninja build system

## Building Tests

```bash
cd tests
mkdir build
cd build
cmake ..
make
```

## Running Tests

### Run all tests:
```bash
make run_tests
```

### Run all tests with verbose output:
```bash
make run_tests_verbose
```

### Run individual test:
```bash
./test_cfl_crc
./test_cfl_header
./test_cfl_serialization
./test_cfl_validation
./test_cfl_builders
```

### Using CTest directly:
```bash
ctest --output-on-failure
ctest --verbose
```

## Code Coverage (Optional)

To build with code coverage enabled:

```bash
cd tests
mkdir build-coverage
cd build-coverage
cmake -DENABLE_COVERAGE=ON ..
make
make run_tests

# Generate coverage report (GCC)
gcov ../unit/*.c
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
```

## Test Results

All tests should pass with output similar to:

```
test_cfl_crc.c:180:test_cfl_crc16_test_vector:PASS
...
-----------------------
50 Tests 0 Failures 0 Ignored
OK
```

## Adding New Tests

1. Create test file in `unit/` directory
2. Include Unity and CFL headers:
   ```c
   #include "unity.h"
   #include "cfl/cfl.h"
   ```
3. Implement `setUp()` and `tearDown()` functions
4. Write test functions with `test_` prefix
5. Add test runner in `main()`:
   ```c
   int main(void) {
       UNITY_BEGIN();
       RUN_TEST(test_your_function);
       return UNITY_END();
   }
   ```
6. Add to `CMakeLists.txt`:
   ```cmake
   add_cfl_test(test_your_module unit/test_your_module.c)
   ```

## Continuous Integration

These tests can be integrated into CI/CD pipelines:

### GitHub Actions Example:
```yaml
name: CFL Tests
on: [push, pull_request]
jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
        with:
          submodules: recursive
      - name: Build and Test
        run: |
          cd tests
          mkdir build && cd build
          cmake ..
          make
          make run_tests
```

## Troubleshooting

### Unity not found
```
CMake Error: Could not find Unity
```
**Solution:** Clone Unity into tests/unity directory

### Compiler warnings
Enable strict compilation with:
```bash
cmake -DCMAKE_C_FLAGS="-Wall -Wextra -Werror" ..
```

### Test failures
- Check that CFL library implementation matches expected behavior
- Verify endianness on target platform
- Ensure no undefined behavior (run with sanitizers)

### Running with sanitizers:
```bash
cmake -DCMAKE_C_FLAGS="-fsanitize=address -fsanitize=undefined" ..
make
make run_tests
```

## Test Statistics

- **Total Tests:** 100+
- **Lines of Test Code:** ~1500
- **Coverage:** Targets >95% code coverage of core CFL functions
- **Execution Time:** < 1 second on modern hardware

## License

Tests follow the same license as the CFL library.
