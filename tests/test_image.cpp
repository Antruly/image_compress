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
#include "image_compress/compress_params.h"
#include "image_compress/image_compress.h"
#include <cstdint>
#include <fstream>
#include <image_compress/jpeg_compressor.h>
#include <image_compress/png_compressor.h>
#include <iostream>
#include <vector>

using namespace imgc;

// ----------------- 文件写入辅助 -----------------
static bool write_file(const std::string &path,
                       const std::vector<uint8_t> &bytes) {
  std::ofstream f(path, std::ios::binary);
  if (!f)
    return false;
  f.write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
  return true;
}

// ----------------- 生成测试 RGBA 图像 -----------------
static void generate_test_image(ImageRGBA &out) {
  out.pixels.resize(out.width * out.height * 4);
  for (int y = 0; y < out.height; ++y) {
    for (int x = 0; x < out.width; ++x) {
      int idx = (y * out.width + x) * 4;
      out.pixels[idx + 0] = static_cast<uint8_t>(x * 4); // R
      out.pixels[idx + 1] = static_cast<uint8_t>(y * 4); // G
      out.pixels[idx + 2] = 128;                         // B
      out.pixels[idx + 3] = 255;                         // A
    }
  }
}

int main() {
  image_converter converter;
  jpeg_compressor jpeg_csr;
  png_compressor png_csr;
  bool all_pass = true;

  // ----------------- 生成输入文件 -----------------
  ImageRGBA test_rgb;
  test_rgb.width = 1024;
  test_rgb.height = 1024;
  generate_test_image(test_rgb);

  compress_params p;
  p.format = compress_params::Format::JPEG;
  p.quality = 80;

  std::vector<uint8_t> jpeg_buffer;
  if (jpeg_csr.encodeFromRGBA(test_rgb, jpeg_buffer, p) <= 0 ||
      !write_file("input.jpg", jpeg_buffer)) {
    std::cerr << "Failed to create input.jpg" << std::endl;
    return 1;
  }

  p.format = compress_params::Format::PNG;
  std::vector<uint8_t> png_buffer;
  if (png_csr.encodeFromRGBA(test_rgb, png_buffer, p) <= 0 ||
      !write_file("input.png", png_buffer)) {
    std::cerr << "Failed to create input.png" << std::endl;
    return 1;
  }

  // ----------------- File->File -----------------
  {
    compress_params p;
    p.format = compress_params::Format::JPEG;
    p.quality = 85;
    int s = converter.convertFileToFile("input.jpg", "out_ff.jpg", p);
    std::cout << "[File->File JPEG] size=" << s
              << (s > 0 ? " [PASS]" : " [FAIL]") << std::endl;
    all_pass &= (s > 0);
  }

  {
    compress_params p;
    p.format = compress_params::Format::PNG;
    int s = converter.convertFileToFile("input.png", "out_ff.png", p);
    std::cout << "[File->File PNG] size=" << s
              << (s > 0 ? " [PASS]" : " [FAIL]") << std::endl;
    all_pass &= (s > 0);
  }

  {
    compress_params p;
    p.format = compress_params::Format::JPEG;
    int s = converter.convertFileToFile("input.png", "out_png_to_jpg.jpg", p);
    std::cout << "[File->File PNG->JPEG] size=" << s
              << (s > 0 ? " [PASS]" : " [FAIL]") << std::endl;
    all_pass &= (s > 0);
  }

  {
    compress_params p;
    p.format = compress_params::Format::PNG;
    int s = converter.convertFileToFile("input.jpg", "out_jpg_to_png.png", p);
    std::cout << "[File->File JPEG->PNG] size=" << s
              << (s > 0 ? " [PASS]" : " [FAIL]") << std::endl;
    all_pass &= (s > 0);
  }

  {
    compress_params p;
    p.format = compress_params::Format::BMP;
    int s = converter.convertFileToFile("input.jpg", "out_jpg_to_bmp.bmp", p);
    std::cout << "[File->File JPEG->BMP] size=" << s
              << (s > 0 ? " [PASS]" : " [FAIL]") << std::endl;
    all_pass &= (s > 0);
  }

  // ----------------- File->Memory -----------------
  {
    compress_params p;
    p.format = compress_params::Format::JPEG;
    p.quality = 90;
    std::vector<uint8_t> out;
    int s = converter.convertFileToMemory("input.jpg", out,
                                          p); // <-- 正确传文件路径
    bool ok = (s > 0) && write_file("out_fm.jpg", out);
    std::cout << "[File->Memory JPEG] size=" << s
              << (ok ? " [PASS]" : " [FAIL]") << std::endl;
    all_pass &= ok;
  }

  // ----------------- Memory->Memory -----------------
  {
    std::vector<uint8_t> buf;
    std::ifstream f("input.png", std::ios::binary);
    if (f) {
      buf = std::vector<uint8_t>((std::istreambuf_iterator<char>(f)),
                                 std::istreambuf_iterator<char>());
      f.close();
    }
    compress_params p;
    p.format = compress_params::Format::PNG;
    std::vector<uint8_t> out;
    int s = converter.convertMemory(buf.data(), buf.size(), out, p);
    std::cout << "[Memory->Memory PNG] size=" << s
              << (s > 0 ? " [PASS]" : " [FAIL]") << std::endl;
    all_pass &= (s > 0);
  }

  // ----------------- Memory->File -----------------
  {
    std::vector<uint8_t> buf;
    std::ifstream f("input.jpg", std::ios::binary);
    if (f) {
      buf = std::vector<uint8_t>((std::istreambuf_iterator<char>(f)),
                                 std::istreambuf_iterator<char>());
      f.close();
    }
    compress_params p;
    p.format = compress_params::Format::BMP;
    int s =
        converter.convertMemoryToFile(buf.data(), buf.size(), "out_mf.bmp", p);
    std::cout << "[Memory->File JPEG->BMP] size=" << s
              << (s > 0 ? " [PASS]" : " [FAIL]") << std::endl;
    all_pass &= (s > 0);
  }

  // ----------------- 缩放测试 -----------------
  struct TestResize {
    std::string src;
    std::string label;
    compress_params::Format fmt;
    int out_w;
    int out_h;
    compress_params::ResizeAlgo algo;
  };
  std::vector<TestResize> resize_tests = {
      {"input.jpg", "jpg_half_nn", compress_params::Format::JPEG, 512, 512,
       compress_params::ResizeAlgo::NEAREST},
      {"input.png", "png_half_bl", compress_params::Format::PNG, 512, 512,
       compress_params::ResizeAlgo::BILINEAR}};

  for (auto &t : resize_tests) {
    compress_params p;
    p.format = t.fmt;
    p.output_width = t.out_w;
    p.output_height = t.out_h;
    p.resize_algo = t.algo;
    std::vector<uint8_t> out;
    int s = converter.convertFileToMemory(t.src, out, p);
    bool ok = (s > 0) && write_file("out_resize_" + t.label + ".bin", out);
    std::cout << "[Resize " << t.label << "] size=" << s
              << (ok ? " [PASS]" : " [FAIL]") << std::endl;
    all_pass &= ok;
  }

  std::cout << (all_pass ? ">>> ALL TESTS PASSED <<<"
                         : ">>> SOME TESTS FAILED <<<")
            << std::endl;
  return all_pass ? 0 : 1;
}
