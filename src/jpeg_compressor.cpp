#include "image_compress/jpeg_compressor.h"
#include <jpeglib.h>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
namespace imgc {
bool jpeg_compressor::decodeToRGBA(const uint8_t* inputBuffer, size_t inputSize, ImageRGBA& outRGBA){
    if (!inputBuffer || inputSize < 3) return false;
    jpeg_decompress_struct cinfo; jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr); jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, const_cast<unsigned char*>(inputBuffer), inputSize);
    if (jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK){ jpeg_destroy_decompress(&cinfo); return false; }
    cinfo.out_color_space = JCS_RGB;
    jpeg_start_decompress(&cinfo);
    int width = (int)cinfo.output_width, height = (int)cinfo.output_height;
    int channels = (int)cinfo.output_components;
    outRGBA.width = width; outRGBA.height = height;
    outRGBA.pixels.assign((size_t)width*height*4, 255);
    std::vector<uint8_t> row((size_t)width*channels);
    while (cinfo.output_scanline < cinfo.output_height){
        JSAMPROW rowptr = row.data();
        jpeg_read_scanlines(&cinfo, &rowptr, 1);
        size_t y = cinfo.output_scanline - 1;
        uint8_t* dst = &outRGBA.pixels[y*(size_t)width*4];
        for (int x=0;x<width;++x){ dst[x*4+0]=row[x*channels+0]; dst[x*4+1]=row[x*channels+1]; dst[x*4+2]=row[x*channels+2]; dst[x*4+3]=255; }
    }
    jpeg_finish_decompress(&cinfo); jpeg_destroy_decompress(&cinfo); return true;
}
int jpeg_compressor::encodeFromRGBA(const ImageRGBA& rgba, std::vector<uint8_t>& outputBuffer, const compress_params& params){
    if (rgba.width<=0||rgba.height<=0||rgba.pixels.size()< (size_t)rgba.width*rgba.height*4) return -1;
    jpeg_compress_struct ccomp; jpeg_error_mgr jerr2;
    ccomp.err = jpeg_std_error(&jerr2); jpeg_create_compress(&ccomp);
    unsigned char* outbuf=nullptr; unsigned long outsize=0; jpeg_mem_dest(&ccomp,&outbuf,&outsize);
    ccomp.image_width = rgba.width; ccomp.image_height = rgba.height; ccomp.input_components=3; ccomp.in_color_space=JCS_RGB;
    jpeg_set_defaults(&ccomp);
    int q=params.quality; if(q<1)q=1; if(q>100)q=100; jpeg_set_quality(&ccomp,q,TRUE);
    jpeg_start_compress(&ccomp, TRUE);
    std::vector<uint8_t> row((size_t)rgba.width*3);
    while (ccomp.next_scanline < ccomp.image_height){
        const uint8_t* src = &rgba.pixels[(size_t)ccomp.next_scanline*rgba.width*4];
        for (int x=0;x<rgba.width;++x){ row[x*3+0]=src[x*4+0]; row[x*3+1]=src[x*4+1]; row[x*3+2]=src[x*4+2]; }
        JSAMPROW rowptr=row.data(); jpeg_write_scanlines(&ccomp,&rowptr,1);
    }
    jpeg_finish_compress(&ccomp);
    outputBuffer.assign(outbuf, outbuf+outsize); free(outbuf); jpeg_destroy_compress(&ccomp);
    return (int)outputBuffer.size();
}
int jpeg_compressor::compressMemory(const uint8_t* inputBuffer, size_t inputSize, std::vector<uint8_t>& outputBuffer, const compress_params& params){
    ImageRGBA rgba; if(!decodeToRGBA(inputBuffer,inputSize,rgba)) return -1; return encodeFromRGBA(rgba,outputBuffer,params);
}
}
