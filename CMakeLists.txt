cmake_minimum_required(VERSION 3.15)
project(image_compress LANGUAGES CXX)

# 强制设置 vcpkg 工具链
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
    message(STATUS "Using vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
message(STATUS "Using C++11")

# MSVC 运行时库设置为动态 CRT
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()

option(BUILD_TESTS "Build tests" ON)
option(ENABLE_SHARED_LIB "Build dynamic library (DLL)" ON)

# 添加详细的构建信息输出
message(STATUS "------------------------------------")
message(STATUS "Build Configuration:")
message(STATUS "  Generator: ${CMAKE_GENERATOR}")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Architecture: ${CMAKE_SIZEOF_VOID_P} bits")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "------------------------------------")

# 设置 vcpkg 目标 triplet
if(NOT DEFINED VCPKG_TARGET_TRIPLET)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "Vcpkg target triplet")
    else()
        set(VCPKG_TARGET_TRIPLET "x86-windows-static" CACHE STRING "Vcpkg target triplet")
    endif()
endif()
message(STATUS "Vcpkg target triplet: ${VCPKG_TARGET_TRIPLET}")

# 使用传统方式查找 JPEG 和 PNG
find_path(JPEG_INCLUDE_DIR jpeglib.h
    PATHS "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/include"
    NO_DEFAULT_PATH
)
find_library(JPEG_LIBRARY_RELEASE
    NAMES jpeg-static turbojpeg-static jpeg libjpeg-static
    PATHS "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/lib"
    NO_DEFAULT_PATH
)
find_library(JPEG_LIBRARY_DEBUG
    NAMES jpeg-staticd turbojpeg-staticd jpegd libjpeg-staticd
    PATHS "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/debug/lib"
    NO_DEFAULT_PATH
)

find_path(PNG_INCLUDE_DIR png.h
    PATHS "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/include"
    NO_DEFAULT_PATH
)
find_library(PNG_LIBRARY_RELEASE
    NAMES png png16 libpng libpng16 png-static libpng-static
    PATHS "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/lib"
    NO_DEFAULT_PATH
)
find_library(PNG_LIBRARY_DEBUG
    NAMES pngd png16d libpngd libpng16d png-staticd libpng-staticd
    PATHS "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/debug/lib"
    NO_DEFAULT_PATH
)

find_path(ZLIB_INCLUDE_DIR zlib.h
    PATHS "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/include"
    NO_DEFAULT_PATH
)
find_library(ZLIB_LIBRARY_RELEASE
    NAMES zlib zlib-static
    PATHS "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/lib"
    NO_DEFAULT_PATH
)
find_library(ZLIB_LIBRARY_DEBUG
    NAMES zlibd zlib-staticd
    PATHS "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/debug/lib"
    NO_DEFAULT_PATH
)

# 设置变量
if(JPEG_INCLUDE_DIR AND (JPEG_LIBRARY_RELEASE OR JPEG_LIBRARY_DEBUG))
    set(JPEG_FOUND TRUE)
    set(JPEG_INCLUDE_DIRS ${JPEG_INCLUDE_DIR})
    
    if(NOT JPEG_LIBRARY_RELEASE)
        set(JPEG_LIBRARY_RELEASE ${JPEG_LIBRARY_DEBUG})
    endif()
    if(NOT JPEG_LIBRARY_DEBUG)
        set(JPEG_LIBRARY_DEBUG ${JPEG_LIBRARY_RELEASE})
    endif()
    
    set(JPEG_LIBRARIES
        optimized ${JPEG_LIBRARY_RELEASE}
        debug ${JPEG_LIBRARY_DEBUG}
    )
    message(STATUS "Found JPEG: ${JPEG_LIBRARIES}")
else()
    message(FATAL_ERROR "JPEG library not found. Please install via vcpkg")
endif()

if(PNG_INCLUDE_DIR AND (PNG_LIBRARY_RELEASE OR PNG_LIBRARY_DEBUG))
    set(PNG_FOUND TRUE)
    set(PNG_INCLUDE_DIRS ${PNG_INCLUDE_DIR})
    
    if(NOT PNG_LIBRARY_RELEASE)
        set(PNG_LIBRARY_RELEASE ${PNG_LIBRARY_DEBUG})
    endif()
    if(NOT PNG_LIBRARY_DEBUG)
        set(PNG_LIBRARY_DEBUG ${PNG_LIBRARY_RELEASE})
    endif()
    
    set(PNG_LIBRARIES
        optimized ${PNG_LIBRARY_RELEASE}
        debug ${PNG_LIBRARY_DEBUG}
    )
    message(STATUS "Found PNG: ${PNG_LIBRARIES}")
else()
    message(FATAL_ERROR "PNG library not found. Please install via vcpkg")
endif()

if(ZLIB_INCLUDE_DIR AND (ZLIB_LIBRARY_RELEASE OR ZLIB_LIBRARY_DEBUG))
    set(ZLIB_FOUND TRUE)
    set(ZLIB_INCLUDE_DIRS ${ZLIB_INCLUDE_DIR})
    
    if(NOT ZLIB_LIBRARY_RELEASE)
        set(ZLIB_LIBRARY_RELEASE ${ZLIB_LIBRARY_DEBUG})
    endif()
    if(NOT ZLIB_LIBRARY_DEBUG)
        set(ZLIB_LIBRARY_DEBUG ${ZLIB_LIBRARY_RELEASE})
    endif()
    
    set(ZLIB_LIBRARIES
        optimized ${ZLIB_LIBRARY_RELEASE}
        debug ${ZLIB_LIBRARY_DEBUG}
    )
    message(STATUS "Found ZLIB: ${ZLIB_LIBRARIES}")
else()
    message(FATAL_ERROR "ZLIB library not found. Please install via vcpkg")
endif()

# 添加头文件存在性检查
find_file(JPEGLIB_H jpeglib.h PATHS ${JPEG_INCLUDE_DIRS} REQUIRED)
message(STATUS "jpeglib.h found at: ${JPEGLIB_H}")

find_file(PNG_H png.h PATHS ${PNG_INCLUDE_DIRS} REQUIRED)
message(STATUS "png.h found at: ${PNG_H}")

find_file(PNG_H png.h PATHS ${PNG_INCLUDE_DIRS} REQUIRED)
message(STATUS "zlib.h found at: ${PNG_H}")

message(STATUS "JPEG found: ${JPEG_FOUND}")
message(STATUS "JPEG includes: ${JPEG_INCLUDE_DIRS}")
message(STATUS "JPEG libraries: ${JPEG_LIBRARIES}")

message(STATUS "PNG found: ${PNG_FOUND}")
message(STATUS "PNG includes: ${PNG_INCLUDE_DIRS}")
message(STATUS "PNG libraries: ${PNG_LIBRARIES}")

message(STATUS "ZLIB found: ${ZLIB_FOUND}")
message(STATUS "ZLIB includes: ${ZLIB_INCLUDE_DIRS}")
message(STATUS "ZLIB libraries: ${ZLIB_LIBRARIES}")
# 将 zlib 添加到 PNG 依赖中
if(WIN32)
    list(APPEND PNG_LIBRARIES ${ZLIB_LIBRARIES})
    
    # 确保我们链接的是静态库而不是 DLL
    foreach(lib IN LISTS PNG_LIBRARIES JPEG_LIBRARIES ZLIB_LIBRARIES)
        if(lib MATCHES "\\.dll$")
            message(WARNING "Found DLL in libraries: ${lib} - this may cause linking issues")
        endif()
    endforeach()
endif()

set(SRC_FILES
    src/image_converter.cpp
    src/jpeg_compressor.cpp
    src/png_compressor.cpp
    src/bmp_compressor.cpp
)
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

# -----------------------------
# 静态库 target
# -----------------------------
add_library(image_compress_static STATIC ${SRC_FILES} ${HDR_FILES})
target_include_directories(image_compress_static
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE  # 依赖头文件设为私有
        "${PNG_INCLUDE_DIRS}"
        "${JPEG_INCLUDE_DIR}"
        "${ZLIB_INCLUDE_DIRS}"
)
target_compile_definitions(image_compress_static 
    PRIVATE 
        USE_LIBJPEG 
        USE_LIBPNG 
        PNG_STATIC
        IMAGE_COMPRESS_STATIC  # 静态库定义
        IMAGE_COMPRESS_STATIC_BUILD  # 额外的静态构建定义
)

# 添加详细的链接信息
message(STATUS "Linking static library with:")
message(STATUS "  PNG libraries: ${PNG_LIBRARIES}")
message(STATUS "  JPEG libraries: ${JPEG_LIBRARIES}")
message(STATUS "  ZLIB libraries: ${ZLIB_LIBRARIES}")

# 打印最终包含路径
get_target_property(INCL_DIRS image_compress_static INCLUDE_DIRECTORIES)
message(STATUS "Final include directories for image_compress_static: ${INCL_DIRS}")

target_link_libraries(image_compress_static PRIVATE 
    ${PNG_LIBRARIES}
    ${JPEG_LIBRARIES}
)
set_target_properties(image_compress_static PROPERTIES
    OUTPUT_NAME "image_compress_static"
    DEBUG_POSTFIX "d"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# -----------------------------
# 动态库 target（可选）
# -----------------------------
if(ENABLE_SHARED_LIB)
    add_library(image_compress SHARED ${SRC_FILES} ${HDR_FILES})
    target_include_directories(image_compress
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE  # 依赖头文件设为私有
            "${PNG_INCLUDE_DIRS}"
            "${JPEG_INCLUDE_DIR}"
            "${ZLIB_INCLUDE_DIRS}"
    )
    # 定义 DLL 导出宏
    target_compile_definitions(image_compress 
        PRIVATE 
            USE_LIBJPEG 
            USE_LIBPNG 
            IMAGE_COMPRESS_EXPORTS  # 动态库导出定义
    )
    target_link_libraries(image_compress PRIVATE 
        ${PNG_LIBRARIES}
        ${JPEG_LIBRARIES}
    )
    set_target_properties(image_compress PROPERTIES
        OUTPUT_NAME "image_compress"
        DEBUG_POSTFIX "d"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()

# -----------------------------
# 测试
# -----------------------------
if(BUILD_TESTS)
    add_executable(image_compress_test tests/test_image.cpp)
    # 默认链接静态库
    if(ENABLE_SHARED_LIB)
        target_link_libraries(image_compress_test PRIVATE image_compress)
    else()
        target_link_libraries(image_compress_test PRIVATE image_compress_static)
    endif()
    target_include_directories(image_compress_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    set_target_properties(image_compress_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    enable_testing()
    add_test(NAME image_mem_test COMMAND image_compress_test)
endif()

# -----------------------------
# 安装
# -----------------------------
install(TARGETS image_compress_static
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)

if(ENABLE_SHARED_LIB)
    install(TARGETS image_compress
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
endif()

message(STATUS "image_compress configuration completed.")