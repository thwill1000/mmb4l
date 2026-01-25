# Building MMBasic for Linux (MMB4L)

This guide explains how to build MMB4L from source on Linux systems.

## Prerequisites

### Required Software

- **CMake** (version 3.14 or later)
- **C/C++ Compiler**: Either GCC or Clang
- **Make** (build automation)
- **SDL2** development libraries
- **Git** (for fetching GoogleTest)

### Installing Dependencies

On Debian/Ubuntu-based systems:
```bash
sudo apt-get update
sudo apt-get install build-essential cmake libsdl2-dev git
```

On Fedora/RHEL-based systems:
```bash
sudo dnf install gcc gcc-c++ cmake SDL2-devel git make
```

### Clone Repository

```
git clone --recursive https://github.com/thwill1000/mmb4l.git
```

## Build Script Overview

The project includes a `build.sh` script that handles both native and cross-platform builds using Docker. The script supports three build types:

- **release** - Optimized production build (default)
- **debug** - Debug build with symbols
- **coverage** - Build with code coverage instrumentation

## Native Build

### Basic Build

To build MMB4L with default settings (release build, GCC compiler):

```bash
./build.sh
```

### Build Options

**Clean build** (removes existing build artifacts):
```bash
./build.sh --clean
```

**Debug build**:
```bash
./build.sh --type debug
```

**Coverage build** (for code coverage analysis):
```bash
./build.sh --type coverage
```

**Using Clang instead of GCC**:
```bash
./build.sh --compiler clang
```

**Combined options**:
```bash
./build.sh --clean --type debug --compiler clang
```

### Build Output

The build process will:
1. Create a build directory: `build/build-{type}-{arch}-{os}-{compiler}-{version}/`
2. Configure the project using CMake
3. Compile the source code with parallel jobs (`make -j4`)
4. Run all unit tests via CTest
5. Create a distribution tarball in the `dist/` directory

The executable will be located at: `build/build-*/mmbasic`

### Running the Executable

After building, you can run MMB4L directly:

```bash
./build.sh --run
```

Or navigate to the build directory and run it manually:
```bash
cd build/build-release-*/
./mmbasic
```

## Cross-Platform Build with Docker

The build script supports cross-compilation for different architectures using Docker.

### Supported Platforms

- `armv6l` - ARM v6 (32-bit)
- `arm64` / `aarch64` - ARM 64-bit
- `amd64` / `x86_64` - x86 64-bit

### Creating Docker Images

Before cross-compiling, create the Docker image for your target platform:

```bash
./build.sh --create-image amd64
./build.sh --create-image arm64
./build.sh --create-image armv6l
```

This sets up a multi-platform builder and bootstraps the necessary QEMU emulation.

### Cross-Platform Build

Once the Docker image is created, build for the target platform:

```bash
./build.sh --make amd64
./build.sh --make arm64 --type debug
./build.sh --make armv6l --compiler clang
```

### Running in Docker

To run the compiled binary within the Docker container:

```bash
./build.sh --run arm64
```

### Interactive Docker Shell

To start an interactive bash session in the Docker container:

```bash
./build.sh --start-bash amd64
```

This is useful for debugging build issues or exploring the container environment.

## Build Directory Structure

The build system creates architecture and compiler-specific directories:

```
build/
├── build-release-x86_64-ubuntu-22.04-gcc-11.4.0/
│   ├── mmbasic                    # Main executable
│   ├── test_*                     # Unit test executables
│   └── mmb4l-{version}.tgz        # Distribution tarball
└── build-debug-x86_64-ubuntu-22.04-clang-14.0.0/
    └── ...
```

## Distribution Package

The build process automatically creates a distribution tarball containing:

- `mmbasic` - The executable
- `mmbasic.nanorc` - Nano editor syntax highlighting
- `mmbasic.syntax.nanorc` - Syntax definitions
- `ChangeLog` - Project changelog
- `LICENSE*` - License files
- `README.md` - Project documentation

The tarball is named: `mmb4l-{version}-{arch}-glibc-{version}.tgz`

Version format follows semantic versioning with special handling:
- `X.Y.Z` - Stable release (micro >= 300)
- `X.Y-rc.N` - Release candidate (200 <= micro < 300)
- `X.Y-beta.N` - Beta release (100 <= micro < 200)
- `X.Y-alpha.N` - Alpha release (micro < 100)

## Testing

The build system uses GoogleTest for unit testing. Tests are automatically run after compilation via CTest.

### Running Tests Manually

```bash
cd build/build-release-*/
ctest
```

### Running Specific Tests

```bash
cd build/build-release-*/
./test_cmdline
./test_parse
./test_graphics
```

### Available Test Suites

The project includes extensive test coverage:
- Core tests: `test_mmbasic_core`, `test_tokentbl`, `test_vartbl`, `test_funtbl`
- Command tests: `test_cmd_do`, `test_cmd_run`
- Function tests: `test_fun_sprite`
- Common module tests: `test_cmdline`, `test_parse`, `test_file`, `test_path`, etc.
- Graphics tests: `test_display`, `test_graphics`, `test_sprite`
- Third-party library tests: `test_spbmp`

## Integration Tests

### MMBasic Tests

```
( cd tests; ../build/build-release-<arch>/mmbasic ../sptools/sptest )
```

### "sptools" Tests

```
( cd sptools; ../build/build-release-<arch>/mmbasic sptest )
```

## Troubleshooting

### SDL2 Not Found

If CMake cannot find SDL2:
```bash
sudo apt-get install libsdl2-dev
# or
sudo dnf install SDL2-devel
```

### Build Fails with "Unknown option"

Ensure you're using the correct syntax:
```bash
./build.sh --type release  # Correct
./build.sh -type release   # Incorrect (single dash for short options)
```

### Docker Build Issues

If Docker builds fail, ensure Docker is properly installed and you have permissions:
```bash
sudo usermod -aG docker $USER
# Log out and back in for group changes to take effect
```

### Clean Build Required

If you encounter unexpected build errors, try a clean build:
```bash
./build.sh --clean
```

## Advanced Usage

### Custom Build Directory

The build directory is automatically determined based on:
- Build type (release/debug/coverage)
- Architecture (detected via `uname -m`)
- OS distribution and version (from `/etc/os-release`)
- Compiler and version

This allows multiple build configurations to coexist.

### Code Coverage

For coverage builds, the system uses `gcov` instrumentation:

```bash
./build.sh --type coverage
cd build/build-coverage-*/
# Run tests and generate coverage reports
```

### Compiler-Specific Builds

Compare builds between GCC and Clang:
```bash
./build.sh --compiler gcc --type release
./build.sh --compiler clang --type release
```

## Contributing

When submitting patches or pull requests:
1. Ensure all tests pass: `./build.sh && cd build/build-*/ && ctest`
2. Run both GCC and Clang builds to check compatibility
3. Consider running the coverage build to verify test coverage

## Additional Resources

For more information about MMB4L, see:
- [README.md](README.md) - Project overview and usage
- [ChangeLog](ChangeLog) - Release history and changes
- [LICENSE](LICENSE) - Licensing information
