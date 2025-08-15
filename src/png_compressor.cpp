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
#include "image_compress/png_compressor.h"
#include <cstring>
#include <png.h>
#include <vector>
#include <algorithm>
namespace imgc {
struct MemReaderState {
  const uint8_t *data;
  size_t size;
  size_t offset;
};
static void png_read_from_mem(png_structp png_ptr, png_bytep outBytes,
                              png_size_t byteCountToRead) {
  MemReaderState *st = (MemReaderState *)png_get_io_ptr(png_ptr);
  if (!st || st->offset + byteCountToRead > st->size) {
    png_error(png_ptr, "read past end");
    return;
  }
  std::memcpy(outBytes, st->data + st->offset, byteCountToRead);
  st->offset += byteCountToRead;
}
struct MemWriterState {
  std::vector<uint8_t> *out;
};
static void png_write_to_mem(png_structp png_ptr, png_bytep data,
                             png_size_t length) {
  MemWriterState *st = (MemWriterState *)png_get_io_ptr(png_ptr);
  size_t old = st->out->size();
  st->out->resize(old + length);
  std::memcpy(st->out->data() + old, data, length);
}
static void png_flush_noop(png_structp) {}
bool png_compressor::decodeToRGBA(const uint8_t *inputBuffer, size_t inputSize,
                                  ImageRGBA &outRGBA) {
  if (!inputBuffer || inputSize < 8)
    return false;
  png_structp r =
      png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if (!r)
    return false;
  png_infop info = png_create_info_struct(r);
  if (!info) {
    png_destroy_read_struct(&r, nullptr, nullptr);
    return false;
  }
  if (setjmp(png_jmpbuf(r))) {
    png_destroy_read_struct(&r, &info, nullptr);
    return false;
  }
  MemReaderState state{inputBuffer, inputSize, 0};
  png_set_read_fn(r, &state, png_read_from_mem);
  png_read_info(r, info);
  png_uint_32 w = png_get_image_width(r, info);
  png_uint_32 h = png_get_image_height(r, info);
  int bd = png_get_bit_depth(r, info);
  int ct = png_get_color_type(r, info);
  if (bd == 16)
    png_set_strip_16(r);
  if (ct == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(r);
  if (ct == PNG_COLOR_TYPE_GRAY && bd < 8)
    png_set_expand_gray_1_2_4_to_8(r);
  if (png_get_valid(r, info, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(r);
  if (ct == PNG_COLOR_TYPE_RGB || ct == PNG_COLOR_TYPE_GRAY)
    png_set_filler(r, 0xFF, PNG_FILLER_AFTER);
  if (ct == PNG_COLOR_TYPE_GRAY || ct == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(r);
  png_read_update_info(r, info);
  outRGBA.width = (int)w;
  outRGBA.height = (int)h;
  outRGBA.pixels.assign((size_t)w * h * 4, 0);
  std::vector<png_bytep> rows(h);
  for (size_t y = 0; y < h; ++y)
    rows[y] = &outRGBA.pixels[y * w * 4];
  png_read_image(r, rows.data());
  png_destroy_read_struct(&r, &info, nullptr);
  return true;
}
int png_compressor::encodeFromRGBA(const ImageRGBA &rgba,
                                   std::vector<uint8_t> &outputBuffer,
                                   const compress_params &params) {
  if (rgba.width <= 0 || rgba.height <= 0 ||
      rgba.pixels.size() < (size_t)rgba.width * rgba.height * 4)
    return -1;

  int w = rgba.width;
  int h = rgba.height;

  const uint8_t *pixelData = rgba.pixels.data();
  std::vector<uint8_t> scaledPixels;

  // --------------------
  // 缩放处理
  // --------------------
  if (params.output_width > 0 && params.output_height > 0 &&
      (params.output_width != w || params.output_height != h)) {
    int newW = params.output_width;
    int newH = params.output_height;
    scaledPixels.resize((size_t)newW * newH * 4);

    if (params.resize_algo == compress_params::ResizeAlgo::NEAREST) {
      // 最近邻
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
      // 双线性
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
  // 写 PNG
  // --------------------
  png_structp w_ptr =
      png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if (!w_ptr)
    return -1;
  png_infop info = png_create_info_struct(w_ptr);
  if (!info) {
    png_destroy_write_struct(&w_ptr, nullptr);
    return -1;
  }
  if (setjmp(png_jmpbuf(w_ptr))) {
    png_destroy_write_struct(&w_ptr, &info);
    return -1;
  }

  outputBuffer.clear();
  MemWriterState st{&outputBuffer};
  png_set_write_fn(w_ptr, &st, png_write_to_mem, png_flush_noop);
  png_set_IHDR(w_ptr, info, w, h, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  // 压缩级别
  int zlevel = params.quality >= 0 && params.quality <= 9 ? params.quality : 6;
  png_set_compression_level(w_ptr, zlevel);

  png_write_info(w_ptr, info);

  std::vector<png_bytep> rows(h);
  for (int y = 0; y < h; ++y)
    rows[y] = (png_bytep)&pixelData[y * w * 4];

  png_write_image(w_ptr, rows.data());
  png_write_end(w_ptr, nullptr);
  png_destroy_write_struct(&w_ptr, &info);

  return (int)outputBuffer.size();
}

int png_compressor::compressMemory(const uint8_t *inputBuffer, size_t inputSize,
                                   std::vector<uint8_t> &outputBuffer,
                                   const compress_params &params) {
  ImageRGBA rgba;
  if (!decodeToRGBA(inputBuffer, inputSize, rgba))
    return -1;
  return encodeFromRGBA(rgba, outputBuffer, params);
}
} // namespace imgc
