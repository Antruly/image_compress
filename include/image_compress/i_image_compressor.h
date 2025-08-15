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
#include "compress_params.h"
#include "image_types.h"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace imgc {
class IMAGE_COMPRESS_API i_image_compressor {
public:
  virtual ~i_image_compressor() = default;
  virtual int compressMemory(const uint8_t *inputBuffer, size_t inputSize,
                             std::vector<uint8_t> &outputBuffer,
                             const compress_params &params) = 0;
  virtual bool decodeToRGBA(const uint8_t *inputBuffer, size_t inputSize,
                            ImageRGBA &outRGBA) = 0;
  virtual int encodeFromRGBA(const ImageRGBA &rgba,
                             std::vector<uint8_t> &outputBuffer,
                             const compress_params &params) = 0;
};
} // namespace imgc
