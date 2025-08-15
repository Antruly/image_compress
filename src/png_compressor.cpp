#include "image_compress/png_compressor.h"
#include <png.h>
#include <vector>
#include <cstring>
namespace imgc {
struct MemReaderState{ const uint8_t* data; size_t size; size_t offset; };
static void png_read_from_mem(png_structp png_ptr, png_bytep outBytes, png_size_t byteCountToRead){
    MemReaderState* st = (MemReaderState*)png_get_io_ptr(png_ptr);
    if(!st || st->offset + byteCountToRead > st->size) { png_error(png_ptr, "read past end"); return; }
    std::memcpy(outBytes, st->data + st->offset, byteCountToRead); st->offset += byteCountToRead;
}
struct MemWriterState{ std::vector<uint8_t>* out; };
static void png_write_to_mem(png_structp png_ptr, png_bytep data, png_size_t length){
    MemWriterState* st = (MemWriterState*)png_get_io_ptr(png_ptr);
    size_t old=st->out->size(); st->out->resize(old+length); std::memcpy(st->out->data()+old, data, length);
}
static void png_flush_noop(png_structp){}
bool png_compressor::decodeToRGBA(const uint8_t* inputBuffer, size_t inputSize, ImageRGBA& outRGBA){
    if(!inputBuffer||inputSize<8) return false;
    png_structp r = png_create_read_struct(PNG_LIBPNG_VER_STRING,nullptr,nullptr,nullptr); if(!r) return false;
    png_infop info = png_create_info_struct(r); if(!info){ png_destroy_read_struct(&r,nullptr,nullptr); return false; }
    if(setjmp(png_jmpbuf(r))){ png_destroy_read_struct(&r,&info,nullptr); return false; }
    MemReaderState state{inputBuffer,inputSize,0}; png_set_read_fn(r,&state,png_read_from_mem); png_read_info(r,info);
    png_uint_32 w = png_get_image_width(r,info); png_uint_32 h = png_get_image_height(r,info);
    int bd = png_get_bit_depth(r,info); int ct = png_get_color_type(r,info);
    if (bd==16) png_set_strip_16(r);
    if (ct==PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(r);
    if (ct==PNG_COLOR_TYPE_GRAY && bd<8) png_set_expand_gray_1_2_4_to_8(r);
    if (png_get_valid(r,info,PNG_INFO_tRNS)) png_set_tRNS_to_alpha(r);
    if (ct==PNG_COLOR_TYPE_RGB || ct==PNG_COLOR_TYPE_GRAY) png_set_filler(r,0xFF,PNG_FILLER_AFTER);
    if (ct==PNG_COLOR_TYPE_GRAY || ct==PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(r);
    png_read_update_info(r,info);
    outRGBA.width=(int)w; outRGBA.height=(int)h; outRGBA.pixels.assign((size_t)w*h*4,0);
    std::vector<png_bytep> rows(h);
    for(size_t y=0;y<h;++y) rows[y]=&outRGBA.pixels[y*w*4];
    png_read_image(r, rows.data()); png_destroy_read_struct(&r,&info,nullptr); return true;
}
int png_compressor::encodeFromRGBA(const ImageRGBA& rgba, std::vector<uint8_t>& outputBuffer, const compress_params& params){
    if (rgba.width<=0||rgba.height<=0||rgba.pixels.size()< (size_t)rgba.width*rgba.height*4) return -1;
    png_structp w = png_create_write_struct(PNG_LIBPNG_VER_STRING,nullptr,nullptr,nullptr); if(!w) return -1;
    png_infop info = png_create_info_struct(w); if(!info){ png_destroy_write_struct(&w,nullptr); return -1; }
    if(setjmp(png_jmpbuf(w))){ png_destroy_write_struct(&w,&info); return -1; }
    outputBuffer.clear(); MemWriterState st{ &outputBuffer };
    png_set_write_fn(w,&st,png_write_to_mem,png_flush_noop);
    png_set_IHDR(w,info,rgba.width,rgba.height,8,PNG_COLOR_TYPE_RGBA,PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_BASE,PNG_FILTER_TYPE_BASE);
    png_set_compression_level(w,6);
    png_write_info(w,info);
    std::vector<png_bytep> rows(rgba.height);
    for(int y=0;y<rgba.height;++y) rows[y]=(png_bytep)&rgba.pixels[(size_t)y*rgba.width*4];
    png_write_image(w, rows.data()); png_write_end(w,nullptr); png_destroy_write_struct(&w,&info);
    return (int)outputBuffer.size();
}
int png_compressor::compressMemory(const uint8_t* inputBuffer, size_t inputSize, std::vector<uint8_t>& outputBuffer, const compress_params& params){
    ImageRGBA rgba; if(!decodeToRGBA(inputBuffer,inputSize,rgba)) return -1; return encodeFromRGBA(rgba,outputBuffer,params);
}
}
