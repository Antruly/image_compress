#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include "compress_params.h"
#include "image_types.h"

namespace imgc {
class i_image_compressor {
public:
    virtual ~i_image_compressor() = default;
    virtual int compressMemory(const uint8_t* inputBuffer, size_t inputSize,
                               std::vector<uint8_t>& outputBuffer,
                               const compress_params& params) = 0;
    virtual bool decodeToRGBA(const uint8_t* inputBuffer, size_t inputSize,
                              ImageRGBA& outRGBA) = 0;
    virtual int encodeFromRGBA(const ImageRGBA& rgba,
                               std::vector<uint8_t>& outputBuffer,
                               const compress_params& params) = 0;
};
}
