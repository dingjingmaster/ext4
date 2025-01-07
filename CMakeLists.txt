cmake_minimum_required(VERSION 3.29)
project(ext4 C)

set(CMAKE_C_STANDARD 11)

add_definitions(-DFUSE_USE_VERSION=27)
add_definitions(-D_FILE_OFFSET_BITS=64)
add_definitions(-D EXT_VERSION=\\"0.0.0\\")

include_directories(${CMAKE_SOURCE_DIR})

find_package(PkgConfig)

pkg_check_modules(FUSE REQUIRED fuse)
pkg_check_modules(EXT2 REQUIRED ext2fs)

add_subdirectory(app)
#add_subdirectory(test)
