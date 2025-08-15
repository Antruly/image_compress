# ImageCompress 🎨🖼️

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

**ImageCompress** is a lightweight C++ library for compressing and converting images. It supports **JPEG, PNG, BMP** formats and provides easy-to-use interfaces for memory and file operations.  

---

## 🌐 Language / 语言
- [English](README.md)  
- [中文](README_zh.md)  

---

## 🌟 Features

- Compress images in **JPEG**, **PNG**, and **BMP** formats  
- Convert images between memory buffers and files  
- Cross-platform support (Windows/Linux)  
- Static and dynamic library options  
- Built-in handling for libjpeg-turbo, libpng, and zlib  

---

## ⚙️ Dependencies

- **libjpeg-turbo** (JPEG compression)  
- **libpng** + **zlib** (PNG compression)  

Make sure these libraries are installed and their paths are provided in CMake.  

---

## 🛠️ Build

```bash
mkdir build
cd build
cmake .. -DENABLE_SHARED_LIB=ON
cmake --build .
```

## 📦 Installation
```bash
cmake --install .
```

## 🧪 Usage Example

```cpp
#include "image_compress/image_converter.h"
#include "image_compress/compress_params.h"
#include <vector>
#include <fstream>

int main() {
    imgc::image_converter converter;
    imgc::compress_params params;
    params.format = imgc::compress_params::Format::JPEG;
    params.quality = 80;

    std::vector<uint8_t> inputData; // load image data
    std::vector<uint8_t> outputData;

    converter.convertMemory(inputData.data(), inputData.size(), outputData, params);

    std::ofstream out("output.jpg", std::ios::binary);
    out.write(reinterpret_cast<char*>(outputData.data()), outputData.size());
}
```

