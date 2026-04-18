# RarLib

A lightweight C++ library, written in C++17, to compress files and directories into RAR archives, and decompress them.

![CI](https://github.com/DostLeFan/RarLib/actions/workflows/ci.yml/badge.svg)
![Version](https://img.shields.io/github/v/release/DostLeFan/RarLib)
![License](https://img.shields.io/github/license/DostLeFan/RarLib)
![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Platforms](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey)

---

## Table of Contents

- [Overview](#overview)
- [Requirements](#requirements)
- [Building from Source](#building-from-source)
- [Integration via CMake FetchContent](#integration-via-cmake-fetchcontent)
- [Usage](#usage)
- [Roadmap](#roadmap)
- [Contributing](#contributing)
- [License](#license)

---

## Overview

RarLib is a cross-platform C++17 wrapper around the `rar` / `WinRAR` command-line executable.  
It automatically detects the RAR binary on the host system and exposes a simple API to compress and decompress files and directories.

> **Note:** RarLib does **not** bundle a RAR binary. The `rar` or `WinRAR` executable must be installed separately on the target machine.

---

## Requirements

### Compiler & Standard

| Requirement | Minimum version |
|---|---|
| C++ Standard | C++17 |
| GCC | 8+ |
| Clang | 7+ |
| MSVC | 19.14+ (Visual Studio 2017 15.7+) |

### Build System

- [CMake](https://cmake.org/) 3.16+

### Runtime Dependency

RarLib requires either `rar` or `WinRAR` to be installed and accessible on the target system:

| Platform | Install |
|---|---|
| **Linux** | `sudo apt install rar` / `sudo dnf install rar` |
| **macOS** | `brew install rar` |
| **Windows** | [WinRAR](https://www.win-rar.com/download.html) — must be installed in the default path or registered in the Windows registry |

---

## Building from Source

```bash
# Clone the repository
git clone https://github.com/DostLeFan/RarLib.git
cd RarLib

# Configure
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DRАРLIB_INSTALL=ON

# Build
cmake --build build --config Release --parallel

# Install (optional — requires appropriate permissions)
cmake --install build
```

### Available CMake Options

| Option | Default | Description |
|---|---|---|
| `RARLIB_BUILD_SHARED` | `OFF` | Build as a shared library instead of static |
| `RARLIB_BUILD_TESTS` | `OFF` | Build the test suite |
| `RARLIB_HEADER_ONLY` | `ON` | Expose the `RarLib::HeaderOnly` interface target |
| `RARLIB_INSTALL` | `ON` | Generate install rules |

---

## Integration via CMake FetchContent

The easiest way to consume RarLib in your own CMake project is via `FetchContent`:

```cmake
include(FetchContent)

FetchContent_Declare(
    RarLib
    GIT_REPOSITORY https://github.com/DostLeFan/RarLib.git
    GIT_TAG        v1.0.0
)

FetchContent_MakeAvailable(RarLib)

target_link_libraries(your_target PRIVATE RarLib::RarLib)
```

Alternatively, if you installed RarLib system-wide via `cmake --install`, you can use `find_package`:

```cmake
find_package(RarLib 1.0.0 REQUIRED)
target_link_libraries(your_target PRIVATE RarLib::RarLib)
```

### Header-only variant

If you prefer to compile the sources yourself, use the `INTERFACE` target instead:

```cmake
target_link_libraries(your_target PRIVATE RarLib::HeaderOnly)
```

Then add `src/Rar.cpp` to your own build.

---

## Usage

### Check if RAR is available

```cpp
#include <RarLib/Rar.hpp>
#include <iostream>

int main()
{
    Rar rar;

    if (!rar.isRarInstalled())
    {
        std::cerr << "RAR executable not found. Please install rar or WinRAR.\n";
        return 1;
    }

    // Prints type (rar / WinRAR) and path
    std::cout << rar << "\n";
}
```

### Compress a single file

```cpp
#include <RarLib/Rar.hpp>
#include <iostream>

int main()
{
    Rar rar;

    bool ok = rar.compressOneFile("path/to/file.txt", "archive.rar");

    if (!ok)
        std::cerr << "Compression failed.\n";
    else
        std::cout << "archive.rar created successfully.\n";
}
```

### Compress multiple files

```cpp
#include <RarLib/Rar.hpp>
#include <iostream>
#include <vector>
#include <string>

int main()
{
    Rar rar;

    std::vector<std::string> files = {
        "path/to/file1.txt",
        "path/to/file2.cpp",
        "path/to/image.png"
    };

    bool ok = rar.compressMultipleFiles(files, "archive.rar");

    if (!ok)
        std::cerr << "Compression failed.\n";
    else
        std::cout << "archive.rar created successfully.\n";
}
```

### Compress an entire directory

```cpp
#include <RarLib/Rar.hpp>
#include <iostream>

int main()
{
    Rar rar;

    bool ok = rar.compressDirectory("path/to/my_folder", "backup.rar");

    if (!ok)
        std::cerr << "Compression failed.\n";
    else
        std::cout << "backup.rar created successfully.\n";
}
```

---

## Roadmap

- [ ] Extraction / decompression support (`unrar`)
- [ ] Password-protected archives
- [ ] Compression level control (`-m0` to `-m5`)
- [ ] Split archives (`-v` flag)
- [ ] Progress callback API
- [ ] `std::expected` / exception-based error reporting

---

## Contributing

Contributions are welcome! Here is how to get started:

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/my-feature`
3. Commit your changes: `git commit -m "feat: add my feature"`
4. Push to your branch: `git push origin feature/my-feature`
5. Open a Pull Request against `main`

Please make sure your code:
- Compiles cleanly on all three platforms (the CI will check)
- Follows the existing code style
- Includes a test if the change affects behaviour

---

## License

This project is licensed under the **MIT License**.  
See the [LICENSE](LICENSE) file for details.