#pragma once
#include "i_image_compressor.h"
namespace imgc {
class IMAGE_COMPRESS_API jpeg_compressor : public i_image_compressor {
public:
  jpeg_compressor() = default;
  ~jpeg_compressor() = default;
  int compressMemory(const uint8_t *inputBuffer, size_t inputSize,
                     std::vector<uint8_t> &outputBuffer,
                     const compress_params &params) override;
  bool decodeToRGBA(const uint8_t *inputBuffer, size_t inputSize,
                    ImageRGBA &outRGBA) override;
  int encodeFromRGBA(const ImageRGBA &rgba, std::vector<uint8_t> &outputBuffer,
                     const compress_params &params) override;
};
} // namespace imgc
