# Codex agent instructions for /cncvis

When modifying code in this folder:

1. Format all `.c` and `.h` files using `clang-format -i` before committing.
2. Ensure the project still builds using CMake:
   ```bash
   cmake -S . -B build
   cmake --build build
   ```
3. Update cncvis/README.md` if the public API or build steps change.
