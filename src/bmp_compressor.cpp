#include "image_compress/bmp_compressor.h"
#include <vector>
#include <cstring>
namespace imgc {
#pragma pack(push,1)
struct BMPFileHeader{ uint16_t bfType; uint32_t bfSize; uint16_t r1; uint16_t r2; uint32_t bfOffBits; };
struct BMPInfoHeader{ uint32_t biSize; int32_t biWidth; int32_t biHeight; uint16_t biPlanes; uint16_t biBitCount; uint32_t biCompression; uint32_t biSizeImage; int32_t biXPelsPerMeter; int32_t biYPelsPerMeter; uint32_t biClrUsed; uint32_t biClrImportant; };
#pragma pack(pop)
static bool read_u16(const uint8_t* p, size_t size, size_t off, uint16_t& v){ if(off+2>size) return false; v=p[off]|(uint16_t(p[off+1])<<8); return true; }
static bool read_u32(const uint8_t* p, size_t size, size_t off, uint32_t& v){ if(off+4>size) return false; v=p[off]|(uint32_t(p[off+1])<<8)|(uint32_t(p[off+2])<<16)|(uint32_t(p[off+3])<<24); return true; }
static bool read_i32(const uint8_t* p, size_t size, size_t off, int32_t& v){ uint32_t t; if(!read_u32(p,size,off,t)) return false; v=(int32_t)t; return true; }
bool bmp_compressor::decodeToRGBA(const uint8_t* inputBuffer, size_t inputSize, ImageRGBA& outRGBA){
    if(!inputBuffer||inputSize<54) return false;
    uint16_t bfType; if(!read_u16(inputBuffer,inputSize,0,bfType)) return false; if(bfType!=0x4D42) return false;
    uint32_t bfOffBits; if(!read_u32(inputBuffer,inputSize,10,bfOffBits)) return false;
    uint32_t biSize; if(!read_u32(inputBuffer,inputSize,14,biSize)) return false;
    int32_t bw,bh; if(!read_i32(inputBuffer,inputSize,18,bw)) return false; if(!read_i32(inputBuffer,inputSize,22,bh)) return false;
    uint16_t planes; if(!read_u16(inputBuffer,inputSize,26,planes)) return false;
    uint16_t bpp; if(!read_u16(inputBuffer,inputSize,28,bpp)) return false;
    uint32_t comp; if(!read_u32(inputBuffer,inputSize,30,comp)) return false;
    if(planes!=1 || comp!=0) return false;
    int width=bw; int height= bh>=0? bh : -bh; bool bottom_up = bh>=0;
    if(!(bpp==24 || bpp==32)) return false;
    outRGBA.width=width; outRGBA.height=height; outRGBA.pixels.assign((size_t)width*height*4,255);
    const uint8_t* pix = inputBuffer + bfOffBits;
    if (bpp==24){
        size_t rowSrc=((width*3+3)/4)*4;
        for(int y=0;y<height;++y){
            int sy = bottom_up? (height-1-y): y;
            const uint8_t* src = pix + (size_t)sy*rowSrc;
            uint8_t* dst = &outRGBA.pixels[(size_t)y*width*4];
            for(int x=0;x<width;++x){ dst[x*4+2]=src[x*3+0]; dst[x*4+1]=src[x*3+1]; dst[x*4+0]=src[x*3+2]; dst[x*4+3]=255; }
        }
    }else{
        size_t rowSrc=(size_t)width*4;
        for(int y=0;y<height;++y){
            int sy = bottom_up? (height-1-y): y;
            const uint8_t* src = pix + (size_t)sy*rowSrc;
            uint8_t* dst = &outRGBA.pixels[(size_t)y*width*4];
            for(int x=0;x<width;++x){ dst[x*4+2]=src[x*4+0]; dst[x*4+1]=src[x*4+1]; dst[x*4+0]=src[x*4+2]; dst[x*4+3]=src[x*4+3]; }
        }
    }
    return true;
}
int bmp_compressor::encodeFromRGBA(const ImageRGBA& rgba, std::vector<uint8_t>& outputBuffer, const compress_params& params){
    if(rgba.width<=0||rgba.height<=0) return -1;
    int w=rgba.width, h=rgba.height;
    size_t rowSize=((w*3+3)/4)*4, pixelSize=rowSize*h;
    size_t fileSize=14+40+pixelSize;
    outputBuffer.resize(fileSize,0); uint8_t* out=outputBuffer.data();
    out[0]='B'; out[1]='M'; uint32_t bfSize=(uint32_t)fileSize; std::memcpy(out+2,&bfSize,4); uint32_t bfOff=14+40; std::memcpy(out+10,&bfOff,4);
    uint32_t biSize=40; std::memcpy(out+14,&biSize,4); std::memcpy(out+18,&w,4); int32_t bh=h; std::memcpy(out+22,&bh,4);
    uint16_t planes=1; std::memcpy(out+26,&planes,2); uint16_t bpp=24; std::memcpy(out+28,&bpp,2); uint32_t comp=0; std::memcpy(out+30,&comp,4);
    std::memcpy(out+34,&pixelSize,4);
    uint8_t* pix = out + bfOff;
    for(int y=0;y<h;++y){
        uint8_t* row = pix + (size_t)y*rowSize;
        const uint8_t* src = &rgba.pixels[(size_t)(h-1-y)*w*4];
        for(int x=0;x<w;++x){ row[x*3+0]=src[x*4+2]; row[x*3+1]=src[x*4+1]; row[x*3+2]=src[x*4+0]; }
    }
    return (int)outputBuffer.size();
}
int bmp_compressor::compressMemory(const uint8_t* inputBuffer, size_t inputSize, std::vector<uint8_t>& outputBuffer, const compress_params& params){
    ImageRGBA rgba; if(!decodeToRGBA(inputBuffer,inputSize,rgba)) return -1; return encodeFromRGBA(rgba,outputBuffer,params);
}
}
