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
#include "image_compress/jpeg_compressor.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <jpeglib.h>
#include <vector>
#include <algorithm>
namespace imgc {
bool jpeg_compressor::decodeToRGBA(const uint8_t *inputBuffer, size_t inputSize,
                                   ImageRGBA &outRGBA) {
  if (!inputBuffer || inputSize < 3)
    return false;
  jpeg_decompress_struct cinfo;
  jpeg_error_mgr jerr;
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);
  jpeg_mem_src(&cinfo, const_cast<unsigned char *>(inputBuffer), inputSize);
  if (jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK) {
    jpeg_destroy_decompress(&cinfo);
    return false;
  }
  cinfo.out_color_space = JCS_RGB;
  jpeg_start_decompress(&cinfo);
  int width = (int)cinfo.output_width, height = (int)cinfo.output_height;
  int channels = (int)cinfo.output_components;
  outRGBA.width = width;
  outRGBA.height = height;
  outRGBA.pixels.assign((size_t)width * height * 4, 255);
  std::vector<uint8_t> row((size_t)width * channels);
  while (cinfo.output_scanline < cinfo.output_height) {
    JSAMPROW rowptr = row.data();
    jpeg_read_scanlines(&cinfo, &rowptr, 1);
    size_t y = cinfo.output_scanline - 1;
    uint8_t *dst = &outRGBA.pixels[y * (size_t)width * 4];
    for (int x = 0; x < width; ++x) {
      dst[x * 4 + 0] = row[x * channels + 0];
      dst[x * 4 + 1] = row[x * channels + 1];
      dst[x * 4 + 2] = row[x * channels + 2];
      dst[x * 4 + 3] = 255;
    }
  }
  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  return true;
}
int jpeg_compressor::encodeFromRGBA(const ImageRGBA &rgba,
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
  // JPEG 写入
  // --------------------
  jpeg_compress_struct ccomp;
  jpeg_error_mgr jerr2;
  ccomp.err = jpeg_std_error(&jerr2);
  jpeg_create_compress(&ccomp);

  unsigned char *outbuf = nullptr;
  unsigned long outsize = 0;
  jpeg_mem_dest(&ccomp, &outbuf, &outsize);

  ccomp.image_width = w;
  ccomp.image_height = h;
  ccomp.input_components = 3;
  ccomp.in_color_space = JCS_RGB;

  jpeg_set_defaults(&ccomp);

  int q = params.quality;
  if (q < 1)
    q = 1;
  if (q > 100)
    q = 100;
  jpeg_set_quality(&ccomp, q, TRUE);

  jpeg_start_compress(&ccomp, TRUE);

  std::vector<uint8_t> row((size_t)w * 3);
  while (ccomp.next_scanline < ccomp.image_height) {
    const uint8_t *src = &pixelData[(size_t)ccomp.next_scanline * w * 4];
    for (int x = 0; x < w; ++x) {
      row[x * 3 + 0] = src[x * 4 + 0];
      row[x * 3 + 1] = src[x * 4 + 1];
      row[x * 3 + 2] = src[x * 4 + 2];
    }
    JSAMPROW rowptr = row.data();
    jpeg_write_scanlines(&ccomp, &rowptr, 1);
  }

  jpeg_finish_compress(&ccomp);
  outputBuffer.assign(outbuf, outbuf + outsize);
  free(outbuf);
  jpeg_destroy_compress(&ccomp);

  return (int)outputBuffer.size();
}

int jpeg_compressor::compressMemory(const uint8_t *inputBuffer,
                                    size_t inputSize,
                                    std::vector<uint8_t> &outputBuffer,
                                    const compress_params &params) {
  ImageRGBA rgba;
  if (!decodeToRGBA(inputBuffer, inputSize, rgba))
    return -1;
  return encodeFromRGBA(rgba, outputBuffer, params);
}
} // namespace imgc
