#pragma once
#include <cstdint>
#include <vector>
namespace imgc {
struct ImageRGBA {
    int width = 0;
    int height = 0;
    std::vector<uint8_t> pixels; // RGBA
};
}
