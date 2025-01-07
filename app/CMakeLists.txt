file(GLOB EXT4_SRC
        ${CMAKE_SOURCE_DIR}/app/defines.h
)

add_executable(ext4 main.c ${EXT4_SRC})
target_link_libraries(ext4 PUBLIC
        ${FUSE_LIBRARIES}
        ${EXT2_LIBRARIES})
target_include_directories(ext4 PUBLIC
        ${CMAKE_SOURCE_DIR}/3thrd

        ${FUSE_INCLUDE_DIRS}
        ${EXT2_INCLUDE_DIRS})
