# AGENTS.md — MMBasic for Linux (MMB4L)

## Project Overview

MMB4L is a port of Geoff Graham's [MMBasic](https://mmbasic.com/) interpreter to Linux, written
in C. It supports audio, graphics (SDL2), gamepads, and a full BASIC runtime. The project targets
x86_64, arm64, and armv6l.

## Repository Layout

```
src/
  main.c              # Entry point
  Version.h           # Version constants
  Configuration.h     # Build-time feature flags
  commands/           # BASIC command implementations (cmd_*.c)
    gtest/            # GoogleTest unit tests (*_test.cxx)
  functions/          # BASIC function implementations (fun_*.c)
    gtest/            # GoogleTest unit tests (*_test.cxx)
  operators/          # Operator implementations (op_*.c)
    gtest/            # GoogleTest unit tests (*_test.cxx)
  common/             # Shared utilities (cmdline, console, parse, path, …)
    gtest/            # GoogleTest unit tests (*_test.cxx)
  core/               # MMBasic interpreter core
    gtest/            # GoogleTest unit tests (*_test.cxx)
  fonts/              # Bitmap fonts
  third_party/        # Vendored libraries
tests/                # Integration tests
  tst_*.bas           # Individual test suites (one per feature area)
  run_tests.bas       # Master test runner
  audio/              # Audio-specific tests
  gamepad/            # Gamepad-specific tests
  graphics/           # Graphics-specific tests
  keyboard/           # Keyboard-specific tests
  manual-tests/       # Tests requiring manual interaction
  resources/          # Test resource files
sptools/              # sptest framework used for integration tests
examples/             # Example MMBasic programs
resources/            # Project resources (syntax files, codepage data, …)
tools/                # Developer scripts (copyright templates, utilities)
build/                # Generated; do not commit
dist/                 # Generated tarballs; do not commit
docker/               # Dockerfiles for cross-compilation
```

## Building

See [BUILDING.md](BUILDING.md) for full instructions. Quick reference:

**Linux** — install deps then run the wrapper script:
```bash
sudo apt-get install build-essential cmake libsdl2-dev git
./build.sh              # release build
./build.sh --type debug # debug build
./build.sh --clean      # clean then build
```
The executable lands at `build/build-<type>-<arch>-<os>-<compiler>-<ver>/mmbasic`.

**Windows** — requires Visual Studio 2022 and SDL2 dev libs; see `BUILDING.md` for setup details:
```powershell
cmake -B build -S . -G "Visual Studio 17 2022"
cmake --build build --config Release
```
Executables land in `build\Release\` or `build\Debug\`.

## Running Tests

See [BUILDING.md](BUILDING.md) for full details. Quick reference:

Unit tests run automatically via CTest as part of `./build.sh`. To run separately:
```bash
cd build/build-release-*/
ctest
```

Integration tests (requires a completed build):
```bash
( cd tests; ../build/build-release-$(uname -m)*/mmbasic ../sptools/sptest )
( cd sptools; ../build/build-release-$(uname -m)*/mmbasic sptest )
```

## Code Style

- C99/C11, 4-space indentation, no tabs.
- Source files begin with the standard MMB4L copyright header block.
- Functions and variables use `snake_case`; macros use `UPPER_SNAKE_CASE`.
- Keep lines reasonably short (≤ 100 chars where practical).
- New source files must follow the naming conventions already established: `cmd_*.c` (commands),
  `fun_*.c` (functions), `op_*.c` (operators), and descriptive names for `src/common/` utilities.
- Unit tests live in a `gtest/` subdirectory under the relevant module and are named `*_test.cxx`.

## Commit Message Convention

Follow [Conventional Commits](https://www.conventionalcommits.org/):
```
<type>(<scope>): <description>
```

Common types: `feat`, `fix`, `refactor`, `test`, `docs`, `chore`, `style`.  
Common scopes: `core`, `cli`, `options`, `memory`, `runtime`, `logging`, `graphics`.

Example:
```
fix(core): tighten typed result propagation in expression evaluation
```

Do **not** prefix commits with `*WIP*` unless the change is genuinely work-in-progress on a
development branch.

## Workflow Notes

- The active development branch follows the pattern `develop-vX.Y-N`.
- Run `./sanitise.sh` to strip trailing whitespace and normalise tabs to spaces across the source
  tree before committing.
- All tests must pass (`./build.sh` exits non-zero on failure) before committing.
- Build and `dist/` directories are `.gitignore`d — never commit generated artefacts.
- Cross-compilation via Docker is available for arm64 and armv6l; see `BUILDING.md`.
- When adding a new BASIC command or function, add a corresponding unit test under the module's
  `gtest/` subdirectory (GoogleTest, `*_test.cxx`) and, where practical, an integration test
  `.bas` file under `tests/` following the `tst_*.bas` naming convention.
