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
#if defined(IMAGE_COMPRESS_STATIC) || defined(IMAGE_COMPRESS_STATIC_BUILD)
#define IMAGE_COMPRESS_API
#elif defined(IMAGE_COMPRESS_EXPORTS)
#define IMAGE_COMPRESS_API __declspec(dllexport)
#else
#define IMAGE_COMPRESS_API __declspec(dllimport)
#endif

namespace imgc {
struct compress_params {
  int output_width = 0;
  int output_height = 0;
  int quality = 75;
  int target_size = 0;
  enum class Format { AUTO, JPEG, PNG, BMP } format = Format::AUTO;
  enum class ResizeAlgo { NEAREST, BILINEAR } resize_algo = ResizeAlgo::NEAREST;
};
} // namespace imgc
