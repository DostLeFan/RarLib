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
- [Architecture](#architecture)
- [Requirements](#requirements)
- [Building from Source](#building-from-source)
- [Integration via CMake FetchContent](#integration-via-cmake-fetchcontent)
- [Usage](#usage)
  - [Compression](#compression)
  - [Extraction](#extraction)
  - [Inspection](#inspection)
- [Roadmap](#roadmap)
- [Contributing](#contributing)
- [License](#license)

---

## Overview

RarLib is a cross-platform C++17 wrapper around the `rar` / `WinRAR` and `unrar` command-line executables.  
It automatically detects the available RAR binaries on the host system and exposes a simple API to compress, extract, and inspect RAR archives.

> **Note:** RarLib does **not** bundle any RAR binary. The `rar`, `WinRAR`, or `unrar` executable must be installed separately on the target machine.

---

## Architecture

RarLib is organized around three classes:

```
Wrapper        (base class — detection, command execution, shared helpers)
├── Rar        (compression)
└── Unrar      (extraction & inspection)
```

`Wrapper` centralizes binary detection (`rar`, `WinRAR`, `unrar`), path resolution, and safe command execution. `Rar` and `Unrar` each expose only the operations relevant to their role.

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

### Runtime Dependencies

| Class | Executable needed | Platform |
|---|---|---|
| `Rar` | `rar` or `WinRAR` | All |
| `Unrar` | `unrar` (preferred) or `rar` / `WinRAR` as fallback | All |

| Platform | Install |
|---|---|
| **Linux** | `sudo apt install rar unrar` / `sudo dnf install rar unrar` |
| **macOS** | `brew install rar` / `brew install unar` |
| **Windows** | [WinRAR](https://www.win-rar.com/download.html) — `UnRAR.exe` is included in the WinRAR installation |

---

## Building from Source

```bash
# Clone the repository
git clone https://github.com/DostLeFan/RarLib.git
cd RarLib

# Configure
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DRARLIB_INSTALL=ON

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
    GIT_TAG        v2.0.0
)

FetchContent_MakeAvailable(RarLib)

target_link_libraries(your_target PRIVATE RarLib::RarLib)
```

Alternatively, if you installed RarLib system-wide via `cmake --install`, you can use `find_package`:

```cmake
find_package(RarLib 2.0.0 REQUIRED)
target_link_libraries(your_target PRIVATE RarLib::RarLib)
```

### Header-only variant

If you prefer to compile the sources yourself, use the `INTERFACE` target instead:

```cmake
target_link_libraries(your_target PRIVATE RarLib::HeaderOnly)
```

Then add `src/RarLib/Rar.cpp`, `src/RarLib/Unrar.cpp` and `src/RarLib/Wrapper.cpp` to your own build.

---

## Usage

### Compression

The `Rar` class wraps the `rar` / `WinRAR` executable and exposes three compression methods.

#### Check if RAR is available

```cpp
#include <RarLib/Rar.hpp>
#include <iostream>

int main()
{
    Rar rar;

    if (!rar.isRarInstalled())
    {
        std::cerr << "rar / WinRAR not found. Please install it.\n";
        return 1;
    }

    std::cout << rar << "\n"; // Prints detected type and path
}
```

#### Compress a single file

```cpp
#include <RarLib/Rar.hpp>
#include <iostream>

int main()
{
    Rar rar;

    if (!rar.compressOneFile("path/to/file.txt", "archive.rar"))
        std::cerr << "Compression failed.\n";
    else
        std::cout << "archive.rar created successfully.\n";
}
```

#### Compress multiple files

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

    if (!rar.compressMultipleFiles(files, "archive.rar"))
        std::cerr << "Compression failed.\n";
    else
        std::cout << "archive.rar created successfully.\n";
}
```

#### Compress an entire directory

```cpp
#include <RarLib/Rar.hpp>
#include <iostream>

int main()
{
    Rar rar;

    if (!rar.compressDirectory("path/to/my_folder", "backup.rar"))
        std::cerr << "Compression failed.\n";
    else
        std::cout << "backup.rar created successfully.\n";
}
```

---

### Extraction

The `Unrar` class wraps `unrar` when available, and falls back to `rar` / `WinRAR` automatically.

#### Check if unrar is available

```cpp
#include <RarLib/Unrar.hpp>
#include <iostream>

int main()
{
    Unrar unrar;

    if (!unrar.isUnrarInstalled())
        std::cerr << "unrar not found. Falling back to rar / WinRAR if available.\n";

    std::cout << unrar << "\n"; // Prints detected executables and their paths
}
```

#### Extract an entire archive

```cpp
#include <RarLib/Unrar.hpp>
#include <iostream>

int main()
{
    Unrar unrar;

    // Extract to a specific directory
    if (!unrar.extractArchive("archive.rar", "path/to/output"))
        std::cerr << "Extraction failed.\n";
    else
        std::cout << "Extracted successfully.\n";

    // Extract to the current working directory
    if (!unrar.extractArchive("archive.rar"))
        std::cerr << "Extraction failed.\n";
}
```

#### Extract a single file

```cpp
#include <RarLib/Unrar.hpp>
#include <iostream>

int main()
{
    Unrar unrar;

    // The second argument is the path of the file inside the archive
    if (!unrar.extractOneFile("archive.rar", "docs/readme.txt", "path/to/output"))
        std::cerr << "Extraction failed.\n";
    else
        std::cout << "File extracted successfully.\n";
}
```

---

### Inspection

#### List archive contents

Prints the archive content (filenames, sizes, dates) directly to stdout.

```cpp
#include <RarLib/Unrar.hpp>
#include <iostream>

int main()
{
    Unrar unrar;

    if (!unrar.listArchive("archive.rar"))
        std::cerr << "Failed to list archive contents.\n";
}
```

#### Test archive integrity

Verifies the CRC checksums of every file in the archive. Returns `true` if the archive is intact.

```cpp
#include <RarLib/Unrar.hpp>
#include <iostream>

int main()
{
    Unrar unrar;

    if (!unrar.testArchive("archive.rar"))
        std::cerr << "Archive is corrupted or could not be tested.\n";
    else
        std::cout << "Archive integrity OK.\n";
}
```

---

## Roadmap

- [x] Extraction / decompression support (`unrar`)
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