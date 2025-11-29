# townbuilder

## Getting Started
1. Install conan from pip or Conda
2. conan profile detect
3. conan build . --build=missing


## Notes
 * Needs C++ 20
 * Only tested on MSVC on Windows
 * Must be executed from a place where Python is available - e.g. a conda prompt
   * This is required due to the ability to call Python code from the C++


## Setup

### Conan

### Clang
`conda install clang clangdev clangxx lld`

### Conan Profiles

MSVC
```
[settings]
arch=x86_64
build_type=Release
compiler=msvc
compiler.cppstd=20
compiler.runtime=dynamic
compiler.version=194
os=Windows
```

Clang on Windows (via Conda)
```
[settings]
arch=x86_64
build_type=Release
compiler=clang
compiler.cppstd=20
compiler.runtime=dynamic
compiler.runtime_version=v144
compiler.version=20
os=Windows

[conf]
tools.cmake.cmaketoolchain:generator=Ninja
```
