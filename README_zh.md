
---

### **2️⃣ README_zh.md（中文）**

```markdown
# ImageCompress 🎨🖼️

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

**ImageCompress** 是一个轻量级 C++ 库，用于图片压缩和转换。支持 **JPEG、PNG、BMP** 格式，并提供方便的内存和文件操作接口。  

---

## 🌐 Language / 语言
- [English](README.md)  
- [中文](README_zh.md)  

---

## 🌟 功能

- 压缩 **JPEG**、**PNG**、**BMP** 图片  
- 内存缓冲区与文件之间的图片转换  
- 跨平台支持（Windows / Linux）  
- 支持静态库和动态库  
- 内置对 **libjpeg-turbo**、**libpng** 和 **zlib** 的支持  

---

## ⚙️ 依赖

- **libjpeg-turbo**（JPEG 压缩）  
- **libpng** + **zlib**（PNG 压缩）  

请确保已安装依赖库，并在 CMake 中提供对应路径。  

---

## 🛠️ 构建

```bash
mkdir build
cd build
cmake .. -DENABLE_SHARED_LIB=ON
cmake --build .
```

## 📦 安装
```bash
cmake --install .
```

## 🧪 使用示例

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

    std::vector<uint8_t> inputData; // 加载图片数据
    std::vector<uint8_t> outputData;

    converter.convertMemory(inputData.data(), inputData.size(), outputData, params);

    std::ofstream out("output.jpg", std::ios::binary);
    out.write(reinterpret_cast<char*>(outputData.data()), outputData.size());
}

```
