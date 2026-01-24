cmake_minimum_required(VERSION 3.15)
project(image_compress LANGUAGES CXX)

message(STATUS "Reading CMakeLists.txt")

# 基础配置
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()

option(ENABLE_STATIC_LIB "Build static library" ON)
option(ENABLE_SHARED_LIB "Build shared library" ON)
option(BUILD_TESTS "Build tests" ON)

# vcpkg配置
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
    message(STATUS "Using vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
endif()

if(NOT DEFINED VCPKG_TARGET_TRIPLET)
    if(WIN32)
        # Prefer explicit generator platform (e.g., Win32 or x64) when provided by toolchain/CI
        if(DEFINED CMAKE_GENERATOR_PLATFORM)
            if(CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
                set(VCPKG_TARGET_TRIPLET "x86-windows-static" CACHE STRING "Vcpkg target triplet")
                set(CMAKE_PREFIX_PATH "$ENV{VCPKG_ROOT}/installed/x86-windows-static" CACHE PATH "Vcpkg install path")
            else()
                set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "Vcpkg target triplet")
                set(CMAKE_PREFIX_PATH "$ENV{VCPKG_ROOT}/installed/x64-windows-static" CACHE PATH "Vcpkg install path")
            endif()
        else()
            # Fallback to pointer size detection
            if(CMAKE_SIZEOF_VOID_P EQUAL 8)
                set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "Vcpkg target triplet")
                set(CMAKE_PREFIX_PATH "$ENV{VCPKG_ROOT}/installed/x64-windows-static" CACHE PATH "Vcpkg install path")
            else()
                set(VCPKG_TARGET_TRIPLET "x86-windows-static" CACHE STRING "Vcpkg target triplet")
                set(CMAKE_PREFIX_PATH "$ENV{VCPKG_ROOT}/installed/x86-windows-static" CACHE PATH "Vcpkg install path")
            endif()
        endif()
    elseif(APPLE)
        set(VCPKG_TARGET_TRIPLET "x64-osx" CACHE STRING "Vcpkg target triplet")
        set(CMAKE_PREFIX_PATH "$ENV{VCPKG_ROOT}/installed/x64-osx" CACHE PATH "Vcpkg install path")
    elseif(UNIX)
        # Detect 64 vs 32 bit on Unix via pointer size
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "Vcpkg target triplet")
            set(CMAKE_PREFIX_PATH "$ENV{VCPKG_ROOT}/installed/x64-linux" CACHE PATH "Vcpkg install path")
        else()
            set(VCPKG_TARGET_TRIPLET "x86-linux" CACHE STRING "Vcpkg target triplet")
            set(CMAKE_PREFIX_PATH "$ENV{VCPKG_ROOT}/installed/x86-linux" CACHE PATH "Vcpkg install path")
        endif()
    endif()
    message(STATUS "Vcpkg target triplet: ${VCPKG_TARGET_TRIPLET}")
    if(DEFINED CMAKE_PREFIX_PATH)
        message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
    endif()
endif()

# 查找依赖
if(WIN32)
    # Windows: 使用vcpkg
    find_package(libjpeg-turbo CONFIG REQUIRED)
    find_package(PNG CONFIG REQUIRED)
else()
    # Linux/macOS: 使用系统包
    find_package(JPEG REQUIRED)
    find_package(PNG REQUIRED)
endif()
find_package(ZLIB REQUIRED)

message(STATUS "Dependencies found")

# 源码文件
set(SRC_FILES src/image_converter.cpp src/jpeg_compressor.cpp src/png_compressor.cpp src/bmp_compressor.cpp)
set(HDR_FILES 
    include/image_compress/image_compress.h
    include/image_compress/image_compress_version.h
    include/image_compress/compress_params.h
    include/image_compress/i_image_compressor.h
    include/image_compress/image_types.h
    include/image_compress/image_converter.h
    include/image_compress/jpeg_compressor.h
    include/image_compress/png_compressor.h
    include/image_compress/bmp_compressor.h
)

# 静态库
if(ENABLE_STATIC_LIB)
    add_library(image_compress_static STATIC ${SRC_FILES} ${HDR_FILES})
    target_include_directories(image_compress_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(image_compress_static PRIVATE
        $<IF:$<BOOL:${WIN32}>,libjpeg-turbo::jpeg-static,JPEG::JPEG>
        $<IF:$<BOOL:${WIN32}>,PNG::png_static,PNG::PNG>
        ZLIB::ZLIB
    )
    target_compile_definitions(image_compress_static PRIVATE USE_LIBJPEG USE_LIBPNG PNG_STATIC IMAGE_COMPRESS_STATIC IMAGE_COMPRESS_STATIC_BUILD)
    set_target_properties(image_compress_static PROPERTIES OUTPUT_NAME "image_compress_static" ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../lib")
endif()

# 动态库
if(ENABLE_SHARED_LIB)
    add_library(image_compress SHARED ${SRC_FILES} ${HDR_FILES})
    target_include_directories(image_compress PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(image_compress PRIVATE
        $<IF:$<BOOL:${WIN32}>,libjpeg-turbo::jpeg-static,JPEG::JPEG>
        $<IF:$<BOOL:${WIN32}>,PNG::png_static,PNG::PNG>
        ZLIB::ZLIB
    )
    target_compile_definitions(image_compress PRIVATE USE_LIBJPEG USE_LIBPNG IMAGE_COMPRESS_EXPORTS)
    set_target_properties(image_compress PROPERTIES
        OUTPUT_NAME "image_compress"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../bin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../bin"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../lib"
    )
endif()

# 测试
if(BUILD_TESTS)
    add_executable(image_compress_test tests/test_image.cpp)
    target_include_directories(image_compress_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    if(ENABLE_SHARED_LIB)
        target_link_libraries(image_compress_test PRIVATE image_compress)
    elseif(ENABLE_STATIC_LIB)
        target_link_libraries(image_compress_test PRIVATE image_compress_static)
    endif()
    set_target_properties(image_compress_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../bin")
    enable_testing()
    add_test(NAME image_mem_test COMMAND image_compress_test)
endif()

message(STATUS "Configuration completed")
