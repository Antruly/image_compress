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
#include "image_compress/image_converter.h"
#include "image_compress/bmp_compressor.h"
#include "image_compress/jpeg_compressor.h"
#include "image_compress/png_compressor.h"
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
namespace imgc {
ImageFormat detectImageFormat(const uint8_t *data, size_t size) {
  if (!data || size < 3)
    return ImageFormat::UNKNOWN;
  if (size >= 3 && data[0] == 0xFF && data[1] == 0xD8 && data[2] == 0xFF)
    return ImageFormat::JPEG;
  if (size >= 8 && data[0] == 0x89 && data[1] == 'P' && data[2] == 'N' &&
      data[3] == 'G' && data[4] == 0x0D && data[5] == 0x0A && data[6] == 0x1A &&
      data[7] == 0x0A)
    return ImageFormat::PNG;
  if (size >= 2 && data[0] == 'B' && data[1] == 'M')
    return ImageFormat::BMP;
  return ImageFormat::UNKNOWN;
}
static std::unique_ptr<i_image_compressor> makeComp(compress_params::Format f) {
  switch (f) {
  case compress_params::Format::JPEG:
    return std::make_unique<jpeg_compressor>();
  case compress_params::Format::PNG:
    return std::make_unique<png_compressor>();
  case compress_params::Format::BMP:
    return std::make_unique<bmp_compressor>();
  default:
    return nullptr;
  }
}
int image_converter::convertMemory(const uint8_t *inputBuffer, size_t inputSize,
                                   std::vector<uint8_t> &outputBuffer,
                                   const compress_params &params) {
  if (!inputBuffer || inputSize == 0)
    return -1;
  ImageFormat inFmt = detectImageFormat(inputBuffer, inputSize);
  compress_params::Format outFmt = params.format;
  if (outFmt == compress_params::Format::AUTO) {
    if (inFmt == ImageFormat::JPEG)
      outFmt = compress_params::Format::JPEG;
    else if (inFmt == ImageFormat::PNG)
      outFmt = compress_params::Format::PNG;
    else if (inFmt == ImageFormat::BMP)
      outFmt = compress_params::Format::BMP;
    else
      return -1;
  }
  bool needConvert =
      (inFmt == ImageFormat::JPEG && outFmt != compress_params::Format::JPEG) ||
      (inFmt == ImageFormat::PNG && outFmt != compress_params::Format::PNG) ||
      (inFmt == ImageFormat::BMP && outFmt != compress_params::Format::BMP);
  if (needConvert) {
    std::unique_ptr<i_image_compressor> inComp;
    if (inFmt == ImageFormat::JPEG)
      inComp = std::make_unique<jpeg_compressor>();
    else if (inFmt == ImageFormat::PNG)
      inComp = std::make_unique<png_compressor>();
    else if (inFmt == ImageFormat::BMP)
      inComp = std::make_unique<bmp_compressor>();
    else
      return -1;
    ImageRGBA rgba;
    if (!inComp->decodeToRGBA(inputBuffer, inputSize, rgba))
      return -1;
    auto outComp = makeComp(outFmt);
    if (!outComp)
      return -1;
    return outComp->encodeFromRGBA(rgba, outputBuffer, params);
  } else {
    auto comp = makeComp(outFmt);
    if (!comp)
      return -1;
    return comp->compressMemory(inputBuffer, inputSize, outputBuffer, params);
  }
}
int image_converter::convertFileToFile(const std::string &inputPath,
                                       const std::string &outputPath,
                                       const compress_params &params) {
  std::ifstream ifs(inputPath, std::ios::binary);
  if (!ifs) {
    std::cerr << "Open input failed: " << inputPath << std::endl;
    return -1;
  }
  std::vector<uint8_t> in((std::istreambuf_iterator<char>(ifs)),
                          std::istreambuf_iterator<char>());
  ifs.close();
  std::vector<uint8_t> out;
  int s = convertMemory(in.data(), in.size(), out, params);
  if (s < 0)
    return s;
  std::ofstream ofs(outputPath, std::ios::binary);
  if (!ofs) {
    std::cerr << "Open output failed: " << outputPath << std::endl;
    return -1;
  }
  ofs.write((const char *)out.data(), (std::streamsize)out.size());
  ofs.close();
  return s;
}
int image_converter::convertFileToMemory(const std::string &inputPath,
                                         std::vector<uint8_t> &outputBuffer,
                                         const compress_params &params) {
  std::ifstream ifs(inputPath, std::ios::binary);
  if (!ifs) {
    std::cerr << "Open input failed: " << inputPath << std::endl;
    return -1;
  }
  std::vector<uint8_t> in((std::istreambuf_iterator<char>(ifs)),
                          std::istreambuf_iterator<char>());
  ifs.close();
  return convertMemory(in.data(), in.size(), outputBuffer, params);
}
int image_converter::convertMemoryToFile(const uint8_t *inputBuffer,
                                         size_t inputSize,
                                         const std::string &outputPath,
                                         const compress_params &params) {
  std::vector<uint8_t> out;
  int s = convertMemory(inputBuffer, inputSize, out, params);
  if (s < 0)
    return s;
  std::ofstream ofs(outputPath, std::ios::binary);
  if (!ofs) {
    std::cerr << "Open output failed: " << outputPath << std::endl;
    return -1;
  }
  ofs.write((const char *)out.data(), (std::streamsize)out.size());
  ofs.close();
  return s;
}
} // namespace imgc
