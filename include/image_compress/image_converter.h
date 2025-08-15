#pragma once
#include "compress_params.h"
#include "image_types.h"
#include <cstdint>
#include <string>
#include <vector>

namespace imgc {
enum class ImageFormat { UNKNOWN, JPEG, PNG, BMP };
IMAGE_COMPRESS_API ImageFormat detectImageFormat(const uint8_t *data,
                                                 size_t size);
class IMAGE_COMPRESS_API image_converter {
public:
  image_converter() = default;
  ~image_converter() = default;
  int convertMemory(const uint8_t *inputBuffer, size_t inputSize,
                    std::vector<uint8_t> &outputBuffer,
                    const compress_params &params);
  int convertFileToFile(const std::string &inputPath,
                        const std::string &outputPath,
                        const compress_params &params);
  int convertFileToMemory(const std::string &inputPath,
                          std::vector<uint8_t> &outputBuffer,
                          const compress_params &params);
  int convertMemoryToFile(const uint8_t *inputBuffer, size_t inputSize,
                          const std::string &outputPath,
                          const compress_params &params);
};
} // namespace imgc
