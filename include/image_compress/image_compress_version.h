#pragma once
/*
MIT License

Copyright (c) 2025 ZHUWEIYE

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#define IMAGE_COMPRESS_VERSION_MAJOR 1
#define IMAGE_COMPRESS_VERSION_MINOR 0
#define IMAGE_COMPRESS_VERSION_PATCH 0
#define IMAGE_COMPRESS_VERSION_IS_RELEASE 0 // 0=非发布版，1=发布版
#define IMAGE_COMPRESS_VERSION_SUFFIX ""

// 辅助宏：双展开字符串化
#define IMAGE_COMPRESS_STRINGIFY_HELPER(x) #x
#define IMAGE_COMPRESS_STRINGIFY(x) IMAGE_COMPRESS_STRINGIFY_HELPER(x)

// 根据是否为发布版定义 RELEASE_TAG
#if IMAGE_COMPRESS_VERSION_IS_RELEASE
#define IMAGE_COMPRESS_RELEASE_TAG
#else
#define IMAGE_COMPRESS_RELEASE_TAG -dev
#endif

// 最终版本字符串
#define IMAGE_COMPRESS_VERSION_STRING                                          \
  IMAGE_COMPRESS_STRINGIFY(                                                    \
      IMAGE_COMPRESS_VERSION_MAJOR.IMAGE_COMPRESS_VERSION_MINOR                \
          .IMAGE_COMPRESS_VERSION_PATCH IMAGE_COMPRESS_RELEASE_TAG)

// 版本号十六进制表示
#define IMAGE_COMPRESS_VERSION_HEX                                             \
  ((IMAGE_COMPRESS_VERSION_MAJOR << 16) |                                      \
   (IMAGE_COMPRESS_VERSION_MINOR << 8) | (IMAGE_COMPRESS_VERSION_PATCH))
