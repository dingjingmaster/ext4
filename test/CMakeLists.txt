include(../app/ext4-mount.cmake)

add_executable(ext4-mount fuse-main.c ${EXT4_FS_SRC} ${EXT4_FOPS_SRC} ${EXT4_TYPES_SRC})
target_include_directories(ext4-mount PUBLIC
    ${CMAKE_SOURCE_DIR}/app
    ${CMAKE_SOURCE_DIR}/app
    ${FUSE_INCLUDE_DIRS}
)

target_link_libraries(ext4-mount PUBLIC
    ${FUSE_LIBRARIES}
)
