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
#include "image_compress/bmp_compressor.h"
#include <cstring>
#include <vector>
#include <algorithm>
namespace imgc {
#pragma pack(push, 1)
struct BMPFileHeader {
  uint16_t bfType;
  uint32_t bfSize;
  uint16_t r1;
  uint16_t r2;
  uint32_t bfOffBits;
};
struct BMPInfoHeader {
  uint32_t biSize;
  int32_t biWidth;
  int32_t biHeight;
  uint16_t biPlanes;
  uint16_t biBitCount;
  uint32_t biCompression;
  uint32_t biSizeImage;
  int32_t biXPelsPerMeter;
  int32_t biYPelsPerMeter;
  uint32_t biClrUsed;
  uint32_t biClrImportant;
};
#pragma pack(pop)
static bool read_u16(const uint8_t *p, size_t size, size_t off, uint16_t &v) {
  if (off + 2 > size)
    return false;
  v = p[off] | (uint16_t(p[off + 1]) << 8);
  return true;
}
static bool read_u32(const uint8_t *p, size_t size, size_t off, uint32_t &v) {
  if (off + 4 > size)
    return false;
  v = p[off] | (uint32_t(p[off + 1]) << 8) | (uint32_t(p[off + 2]) << 16) |
      (uint32_t(p[off + 3]) << 24);
  return true;
}
static bool read_i32(const uint8_t *p, size_t size, size_t off, int32_t &v) {
  uint32_t t;
  if (!read_u32(p, size, off, t))
    return false;
  v = (int32_t)t;
  return true;
}
bool bmp_compressor::decodeToRGBA(const uint8_t *inputBuffer, size_t inputSize,
                                  ImageRGBA &outRGBA) {
  if (!inputBuffer || inputSize < 54)
    return false;
  uint16_t bfType;
  if (!read_u16(inputBuffer, inputSize, 0, bfType))
    return false;
  if (bfType != 0x4D42)
    return false;
  uint32_t bfOffBits;
  if (!read_u32(inputBuffer, inputSize, 10, bfOffBits))
    return false;
  uint32_t biSize;
  if (!read_u32(inputBuffer, inputSize, 14, biSize))
    return false;
  int32_t bw, bh;
  if (!read_i32(inputBuffer, inputSize, 18, bw))
    return false;
  if (!read_i32(inputBuffer, inputSize, 22, bh))
    return false;
  uint16_t planes;
  if (!read_u16(inputBuffer, inputSize, 26, planes))
    return false;
  uint16_t bpp;
  if (!read_u16(inputBuffer, inputSize, 28, bpp))
    return false;
  uint32_t comp;
  if (!read_u32(inputBuffer, inputSize, 30, comp))
    return false;
  if (planes != 1 || comp != 0)
    return false;
  int width = bw;
  int height = bh >= 0 ? bh : -bh;
  bool bottom_up = bh >= 0;
  if (!(bpp == 24 || bpp == 32))
    return false;
  outRGBA.width = width;
  outRGBA.height = height;
  outRGBA.pixels.assign((size_t)width * height * 4, 255);
  const uint8_t *pix = inputBuffer + bfOffBits;
  if (bpp == 24) {
    size_t rowSrc = ((width * 3 + 3) / 4) * 4;
    for (int y = 0; y < height; ++y) {
      int sy = bottom_up ? (height - 1 - y) : y;
      const uint8_t *src = pix + (size_t)sy * rowSrc;
      uint8_t *dst = &outRGBA.pixels[(size_t)y * width * 4];
      for (int x = 0; x < width; ++x) {
        dst[x * 4 + 2] = src[x * 3 + 0];
        dst[x * 4 + 1] = src[x * 3 + 1];
        dst[x * 4 + 0] = src[x * 3 + 2];
        dst[x * 4 + 3] = 255;
      }
    }
  } else {
    size_t rowSrc = (size_t)width * 4;
    for (int y = 0; y < height; ++y) {
      int sy = bottom_up ? (height - 1 - y) : y;
      const uint8_t *src = pix + (size_t)sy * rowSrc;
      uint8_t *dst = &outRGBA.pixels[(size_t)y * width * 4];
      for (int x = 0; x < width; ++x) {
        dst[x * 4 + 2] = src[x * 4 + 0];
        dst[x * 4 + 1] = src[x * 4 + 1];
        dst[x * 4 + 0] = src[x * 4 + 2];
        dst[x * 4 + 3] = src[x * 4 + 3];
      }
    }
  }
  return true;
}
int bmp_compressor::encodeFromRGBA(const ImageRGBA &rgba,
                                   std::vector<uint8_t> &outputBuffer,
                                   const compress_params &params) {
  if (rgba.width <= 0 || rgba.height <= 0)
    return -1;

  int w = rgba.width;
  int h = rgba.height;

  const uint8_t *pixelData = rgba.pixels.data();
  std::vector<uint8_t> scaledPixels; // 如果缩放，这里会存缩放结果

  // 检查是否需要缩放
  if (params.output_width > 0 && params.output_height > 0 &&
      (params.output_width != w || params.output_height != h)) {
    int newW = params.output_width;
    int newH = params.output_height;
    scaledPixels.resize((size_t)newW * newH * 4);

    if (params.resize_algo == compress_params::ResizeAlgo::NEAREST) {
      // 最近邻缩放
      for (int y = 0; y < newH; ++y) {
        int srcY = y * h / newH;
        for (int x = 0; x < newW; ++x) {
          int srcX = x * w / newW;
          const uint8_t *srcPix = &rgba.pixels[(size_t)(srcY * w + srcX) * 4];
          uint8_t *dstPix = &scaledPixels[(size_t)(y * newW + x) * 4];
          dstPix[0] = srcPix[0];
          dstPix[1] = srcPix[1];
          dstPix[2] = srcPix[2];
          dstPix[3] = srcPix[3];
        }
      }
    } else if (params.resize_algo == compress_params::ResizeAlgo::BILINEAR) {
      // 双线性插值缩放
      for (int y = 0; y < newH; ++y) {
        float srcY = (y + 0.5f) * h / newH - 0.5f;
        int y0 = std::max(0, (int)floor(srcY));
        int y1 = std::min(h - 1, y0 + 1);
        float wy = srcY - y0;

        for (int x = 0; x < newW; ++x) {
          float srcX = (x + 0.5f) * w / newW - 0.5f;
          int x0 = std::max(0, (int)floor(srcX));
          int x1 = std::min(w - 1, x0 + 1);
          float wx = srcX - x0;

          const uint8_t *p00 = &rgba.pixels[(size_t)(y0 * w + x0) * 4];
          const uint8_t *p01 = &rgba.pixels[(size_t)(y0 * w + x1) * 4];
          const uint8_t *p10 = &rgba.pixels[(size_t)(y1 * w + x0) * 4];
          const uint8_t *p11 = &rgba.pixels[(size_t)(y1 * w + x1) * 4];

          uint8_t *dstPix = &scaledPixels[(size_t)(y * newW + x) * 4];
          for (int c = 0; c < 4; ++c) {
            float top = p00[c] * (1 - wx) + p01[c] * wx;
            float bottom = p10[c] * (1 - wx) + p11[c] * wx;
            dstPix[c] = (uint8_t)(top * (1 - wy) + bottom * wy + 0.5f);
          }
        }
      }
    }

    w = newW;
    h = newH;
    pixelData = scaledPixels.data();
  }

  // --------------------
  // 写 BMP 数据
  // --------------------
  size_t rowSize = ((w * 3 + 3) / 4) * 4; // 每行字节数对齐到4字节
  size_t pixelSize = rowSize * h;
  size_t fileSize = 14 + 40 + pixelSize;
  outputBuffer.resize(fileSize, 0);

  uint8_t *out = outputBuffer.data();
  out[0] = 'B';
  out[1] = 'M';
  uint32_t bfSize = (uint32_t)fileSize;
  std::memcpy(out + 2, &bfSize, 4);
  uint32_t bfOff = 14 + 40;
  std::memcpy(out + 10, &bfOff, 4);
  uint32_t biSize = 40;
  std::memcpy(out + 14, &biSize, 4);
  std::memcpy(out + 18, &w, 4);
  int32_t bh = h;
  std::memcpy(out + 22, &bh, 4);
  uint16_t planes = 1;
  std::memcpy(out + 26, &planes, 2);
  uint16_t bpp = 24;
  std::memcpy(out + 28, &bpp, 2);
  uint32_t comp = 0;
  std::memcpy(out + 30, &comp, 4);
  std::memcpy(out + 34, &pixelSize, 4);

  uint8_t *pix = out + bfOff;
  for (int y = 0; y < h; ++y) {
    uint8_t *row = pix + (size_t)y * rowSize;
    const uint8_t *src = &pixelData[(size_t)(h - 1 - y) * w * 4];
    for (int x = 0; x < w; ++x) {
      row[x * 3 + 0] = src[x * 4 + 2]; // B
      row[x * 3 + 1] = src[x * 4 + 1]; // G
      row[x * 3 + 2] = src[x * 4 + 0]; // R
    }
  }

  return (int)outputBuffer.size();
}


int bmp_compressor::compressMemory(const uint8_t *inputBuffer, size_t inputSize,
                                   std::vector<uint8_t> &outputBuffer,
                                   const compress_params &params) {
  ImageRGBA rgba;
  if (!decodeToRGBA(inputBuffer, inputSize, rgba))
    return -1;
  return encodeFromRGBA(rgba, outputBuffer, params);
}
} // namespace imgc
