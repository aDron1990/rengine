# Build
```bash
cmake -S . -B build --preset=debug
cmake --build build
```

# Count of lines
```bash
cloc . --exclude-dir=build,.vs,third-party,images,rocket
```