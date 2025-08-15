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

# 库路径
if(MSVC)
  if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(DEFAULT_LIBPNG_ROOT "C:/Program Files/libpng")
    set(DEFAULT_ZLIB_ROOT "C:/Program Files/zlib")
  else()
    set(DEFAULT_LIBPNG_ROOT "C:/Program Files (x86)/libpng")
    set(DEFAULT_ZLIB_ROOT "C:/Program Files (x86)/zlib")
  endif()
  set(DEFAULT_LIBJPEG_ROOT "C:/libjpeg-turbo")
else()
  set(DEFAULT_LIBPNG_ROOT "/usr")
  set(DEFAULT_ZLIB_ROOT "/usr")
  set(DEFAULT_LIBJPEG_ROOT "/usr")
endif()

set(LIBJPEG_ROOT_DIR "${DEFAULT_LIBJPEG_ROOT}" CACHE PATH "Path to libjpeg(-turbo) root")
set(LIBPNG_ROOT_DIR  "${DEFAULT_LIBPNG_ROOT}" CACHE PATH "Path to libpng root")
set(ZLIB_ROOT_DIR     "${DEFAULT_ZLIB_ROOT}" CACHE PATH "Path to zlib root")

set(JPEG_INCLUDE_DIR "${LIBJPEG_ROOT_DIR}/include")
set(PNG_INCLUDE_DIR  "${LIBPNG_ROOT_DIR}/include")
set(ZLIB_INCLUDE_DIR "${ZLIB_ROOT_DIR}/include")

if (MSVC)
    set(JPEG_LIB_RELEASE "${LIBJPEG_ROOT_DIR}/lib/jpeg-static.lib")
    set(JPEG_LIB_DEBUG   "${LIBJPEG_ROOT_DIR}/lib/jpeg-static.lib")

    set(PNG_LIB_RELEASE  "${LIBPNG_ROOT_DIR}/lib/libpng16_static.lib")
    set(PNG_LIB_DEBUG    "${LIBPNG_ROOT_DIR}/lib/libpng16_staticd.lib")

    set(ZLIB_LIB_RELEASE "${ZLIB_ROOT_DIR}/lib/zlibstatic.lib")
    set(ZLIB_LIB_DEBUG   "${ZLIB_ROOT_DIR}/lib/zlibstaticd.lib")
else()
    find_library(JPEG_LIB_RELEASE NAMES jpeg turbojpeg libjpeg)
    set(JPEG_LIB_DEBUG ${JPEG_LIB_RELEASE})
    find_library(PNG_LIB_RELEASE  NAMES png png16 libpng libpng16)
    set(PNG_LIB_DEBUG ${PNG_LIB_RELEASE})
    find_library(ZLIB_LIB_RELEASE NAMES z zlib zlibstatic zlib1)
    set(ZLIB_LIB_DEBUG ${ZLIB_LIB_RELEASE})
endif()

set(SRC_FILES
    src/image_converter.cpp
    src/jpeg_compressor.cpp
    src/png_compressor.cpp
    src/bmp_compressor.cpp
)
set(HDR_FILES
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
add_definitions(-DPNG_STATIC)
target_include_directories(image_compress_static
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${JPEG_INCLUDE_DIR}
        ${PNG_INCLUDE_DIR}
        ${ZLIB_INCLUDE_DIR}
)
target_compile_definitions(image_compress_static PRIVATE USE_LIBJPEG USE_LIBPNG)
set(JPEG_LINK "$<$<CONFIG:Debug>:${JPEG_LIB_DEBUG}>$<$<NOT:$<CONFIG:Debug>>:${JPEG_LIB_RELEASE}>")
set(PNG_LINK  "$<$<CONFIG:Debug>:${PNG_LIB_DEBUG}>$<$<NOT:$<CONFIG:Debug>>:${PNG_LIB_RELEASE}>")
set(ZLIB_LINK "$<$<CONFIG:Debug>:${ZLIB_LIB_DEBUG}>$<$<NOT:$<CONFIG:Debug>>:${ZLIB_LIB_RELEASE}>")
target_link_libraries(image_compress_static PRIVATE ${JPEG_LINK} ${PNG_LINK} ${ZLIB_LINK})
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
            ${JPEG_INCLUDE_DIR}
            ${PNG_INCLUDE_DIR}
            ${ZLIB_INCLUDE_DIR}
    )
    # 定义 DLL 导出宏
    target_compile_definitions(image_compress PRIVATE USE_LIBJPEG USE_LIBPNG IMAGE_COMPRESS_EXPORTS)
    target_link_libraries(image_compress PRIVATE ${JPEG_LINK} ${PNG_LINK} ${ZLIB_LINK})
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
    add_executable(image_compress_test tests/main.cpp)
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
