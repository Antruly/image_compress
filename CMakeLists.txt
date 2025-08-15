cmake_minimum_required(VERSION 3.15)
project(image_compress LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
message(STATUS "Using C++17")

# MSVC 运行时库设置为动态 CRT (/MD / /MDd)
if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()

option(BUILD_TESTS "Build tests" ON)
option(ENABLE_SHARED_LIB "Build dynamic library (DLL)" OFF)

message(STATUS "Configuring image_compress (libjpeg-turbo + libpng + zlib + BMP builtin)")

# 添加详细的构建信息输出
message(STATUS "------------------------------------")
message(STATUS "Build Configuration:")
message(STATUS "  Generator: ${CMAKE_GENERATOR}")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Architecture: ${CMAKE_SIZEOF_VOID_P} bits")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "------------------------------------")

# 处理 Windows 的 vcpkg 工具链
if(WIN32)
    # 优先使用环境变量中的 vcpkg 路径
    if(DEFINED ENV{VCPKG_ROOT})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
            CACHE STRING "Vcpkg toolchain file")
        message(STATUS "Using vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
    endif()
    
    # 设置库查找后缀 - 仅静态库
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib" ".a")
    message(STATUS "Library search suffixes: ${CMAKE_FIND_LIBRARY_SUFFIXES}")
endif()

# 查找 PNG 库 (跨平台)
find_package(PNG REQUIRED)
message(STATUS "PNG found: ${PNG_FOUND}")
message(STATUS "PNG includes: ${PNG_INCLUDE_DIRS}")
message(STATUS "PNG libraries: ${PNG_LIBRARIES}")

# 打印 PNG 的链接库信息
if(TARGET PNG::PNG)
    get_property(png_libs TARGET PNG::PNG PROPERTY INTERFACE_LINK_LIBRARIES)
    message(STATUS "PNG link libraries: ${png_libs}")
endif()

# 增强 JPEG 查找逻辑 (解决找不到的问题)
if(WIN32)
    # Windows 平台需要更明确的查找逻辑
    find_path(JPEG_INCLUDE_DIR jpeglib.h
        PATHS
        "$ENV{VCPKG_ROOT}/installed/x64-windows-static/include"
        "$ENV{VCPKG_ROOT}/installed/x86-windows-static/include"
        "$ENV{VCPKG_ROOT}/installed/x64-windows/include"
        "$ENV{VCPKG_ROOT}/installed/x86-windows/include"
        "C:/libjpeg-turbo/include"
        "C:/Program Files/libjpeg-turbo/include"
        "C:/Program Files (x86)/libjpeg-turbo/include"
        DOC "Path to JPEG include directory"
    )
    
    find_library(JPEG_LIBRARY_RELEASE
        NAMES jpeg-static turbojpeg-static jpeg libjpeg-static
        PATHS
        "$ENV{VCPKG_ROOT}/installed/x64-windows-static/lib"
        "$ENV{VCPKG_ROOT}/installed/x86-windows-static/lib"
        "$ENV{VCPKG_ROOT}/installed/x64-windows/lib"
        "$ENV{VCPKG_ROOT}/installed/x86-windows/lib"
        "C:/libjpeg-turbo/lib"
        "C:/Program Files/libjpeg-turbo/lib"
        "C:/Program Files (x86)/libjpeg-turbo/lib"
        DOC "Path to JPEG release library"
    )
    
    find_library(JPEG_LIBRARY_DEBUG
        NAMES jpeg-staticd turbojpeg-staticd jpegd libjpeg-staticd
        PATHS
        "$ENV{VCPKG_ROOT}/installed/x64-windows-static/debug/lib"
        "$ENV{VCPKG_ROOT}/installed/x86-windows-static/debug/lib"
        "$ENV{VCPKG_ROOT}/installed/x64-windows/debug/lib"
        "$ENV{VCPKG_ROOT}/installed/x86-windows/debug/lib"
        "C:/libjpeg-turbo/lib"
        "C:/Program Files/libjpeg-turbo/lib"
        "C:/Program Files (x86)/libjpeg-turbo/lib"
        DOC "Path to JPEG debug library"
    )
    
    if(JPEG_INCLUDE_DIR AND (JPEG_LIBRARY_RELEASE OR JPEG_LIBRARY_DEBUG))
        set(JPEG_FOUND TRUE)
        set(JPEG_INCLUDE_DIR ${JPEG_INCLUDE_DIR})
        
        # 如果只找到一种配置的库，使用相同的库作为两种配置
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
        message(FATAL_ERROR "JPEG library not found on Windows. Please install libjpeg-turbo via vcpkg")
    endif()
else()
    # Linux/macOS 使用标准查找
    find_package(JPEG REQUIRED)
    message(STATUS "JPEG found: ${JPEG_FOUND}")
    message(STATUS "JPEG includes: ${JPEG_INCLUDE_DIR}")
    message(STATUS "JPEG libraries: ${JPEG_LIBRARIES}")
endif()

# 添加详细输出
if(WIN32)
    message(STATUS "JPEG include dir: ${JPEG_INCLUDE_DIR}")
    message(STATUS "JPEG release lib: ${JPEG_LIBRARY_RELEASE}")
    message(STATUS "JPEG debug lib: ${JPEG_LIBRARY_DEBUG}")
endif()

# 处理 Windows 的特殊依赖 (zlib)
if(WIN32)
    # 确保只查找静态库
    set(ZLIB_USE_STATIC_LIBS ON)
    find_package(ZLIB REQUIRED)
    message(STATUS "ZLIB found: ${ZLIB_FOUND}")
    message(STATUS "ZLIB includes: ${ZLIB_INCLUDE_DIRS}")
    message(STATUS "ZLIB libraries: ${ZLIB_LIBRARIES}")
    
    # 将 zlib 添加到 PNG 依赖中
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
        ${PNG_INCLUDE_DIRS}
        ${JPEG_INCLUDE_DIR}
)
target_compile_definitions(image_compress_static 
    PRIVATE 
        USE_LIBJPEG 
        USE_LIBPNG 
        PNG_STATIC
)

# 添加详细的链接信息
message(STATUS "Linking static library with:")
message(STATUS "  PNG libraries: ${PNG_LIBRARIES}")
message(STATUS "  JPEG libraries: ${JPEG_LIBRARIES}")
if(WIN32)
    message(STATUS "  ZLIB libraries: ${ZLIB_LIBRARIES}")
endif()

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
            ${PNG_INCLUDE_DIRS}
            ${JPEG_INCLUDE_DIR}
    )
    # 定义 DLL 导出宏
    target_compile_definitions(image_compress 
        PRIVATE 
            USE_LIBJPEG 
            USE_LIBPNG 
            IMAGE_COMPRESS_EXPORTS
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