#pragma once
#ifdef _WIN32
#ifdef IMAGE_COMPRESS_EXPORTS
#define IMAGE_COMPRESS_API __declspec(dllexport)
#else
#define IMAGE_COMPRESS_API __declspec(dllimport)
#endif
#else
#define IMAGE_COMPRESS_API
#endif

namespace imgc {
struct compress_params {
    int output_width  = 0;
    int output_height = 0;
    int quality       = 75;
    int target_size   = 0;
    enum class Format { AUTO, JPEG, PNG, BMP } format = Format::AUTO;
};
}
