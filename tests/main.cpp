#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <cstdio>
#include "image_compress/image_converter.h"
#include "image_compress/compress_params.h"
using namespace imgc;
static bool write_file(const std::string& path, const std::vector<uint8_t>& bytes){
    std::FILE* f = std::fopen(path.c_str(), "wb"); if(!f) return false;
    std::fwrite(bytes.data(),1,bytes.size(),f); std::fclose(f); return true;
}
int main(){
    image_converter converter; bool all_pass=true;
    { compress_params p; p.format=compress_params::Format::JPEG; p.quality=85;
      int s=converter.convertFileToFile("input.jpg","out_ff.jpg",p);
      std::cout<<"[File->File JPEG] size="<<s<<(s>0?" [PASS]":" [FAIL]")<<std::endl; all_pass&=(s>0); }
    { compress_params p; p.format=compress_params::Format::PNG;
      int s=converter.convertFileToFile("input.png","out_ff.png",p);
      std::cout<<"[File->File PNG] size="<<s<<(s>0?" [PASS]":" [FAIL]")<<std::endl; all_pass&=(s>0); }
    { compress_params p; p.format=compress_params::Format::JPEG;
      int s=converter.convertFileToFile("input.png","out_png_to_jpg.jpg",p);
      std::cout<<"[File->File PNG->JPEG] size="<<s<<(s>0?" [PASS]":" [FAIL]")<<std::endl; all_pass&=(s>0); }
    { compress_params p; p.format=compress_params::Format::PNG;
      int s=converter.convertFileToFile("input.jpg","out_jpg_to_png.png",p);
      std::cout<<"[File->File JPEG->PNG] size="<<s<<(s>0?" [PASS]":" [FAIL]")<<std::endl; all_pass&=(s>0); }
    { compress_params p; p.format=compress_params::Format::BMP;
      int s=converter.convertFileToFile("input.jpg","out_jpg_to_bmp.bmp",p);
      std::cout<<"[File->File JPEG->BMP] size="<<s<<(s>0?" [PASS]":" [FAIL]")<<std::endl; all_pass&=(s>0); }
    { compress_params p; p.format=compress_params::Format::JPEG; std::vector<uint8_t> out;
      int s=converter.convertFileToMemory("input.jpg",out,p); bool ok=(s>0)&&write_file("out_fm.jpg",out);
      std::cout<<"[File->Memory JPEG] size="<<s<<(ok?" [PASS]":" [FAIL]")<<std::endl; all_pass&=ok; }
    { std::FILE* f=std::fopen("input.png","rb"); std::vector<uint8_t> buf; if(f){ std::fseek(f,0,SEEK_END); long n=std::ftell(f); std::fseek(f,0,SEEK_SET); buf.resize(n); std::fread(buf.data(),1,buf.size(),f); std::fclose(f); }
      compress_params p; p.format=compress_params::Format::PNG; std::vector<uint8_t> out;
      int s=converter.convertMemory(buf.data(),buf.size(),out,p);
      std::cout<<"[Memory->Memory PNG] size="<<s<<(s>0?" [PASS]":" [FAIL]")<<std::endl; all_pass&=(s>0); }
    { std::FILE* f=std::fopen("input.jpg","rb"); std::vector<uint8_t> buf; if(f){ std::fseek(f,0,SEEK_END); long n=std::ftell(f); std::fseek(f,0,SEEK_SET); buf.resize(n); std::fread(buf.data(),1,buf.size(),f); std::fclose(f); }
      compress_params p; p.format=compress_params::Format::BMP;
      int s=converter.convertMemoryToFile(buf.data(),buf.size(),"out_mf.bmp",p);
      std::cout<<"[Memory->File JPEG->BMP] size="<<s<<(s>0?" [PASS]":" [FAIL]")<<std::endl; all_pass&=(s>0); }
    std::cout<<(all_pass?">>> ALL TESTS PASSED <<<":">>> SOME TESTS FAILED <<<")<<std::endl; return all_pass?0:1;
}
