file(GLOB EXT4_SRC
        ${CMAKE_SOURCE_DIR}/app/defines.h

        ${CMAKE_SOURCE_DIR}/app/ops/op-init.c
        ${CMAKE_SOURCE_DIR}/app/ops/op-link.c
        ${CMAKE_SOURCE_DIR}/app/ops/op-open.c
        ${CMAKE_SOURCE_DIR}/app/ops/op-read.c
        ${CMAKE_SOURCE_DIR}/app/ops/op-write.c
        ${CMAKE_SOURCE_DIR}/app/ops/op-rmdir.c
        ${CMAKE_SOURCE_DIR}/app/ops/op-mkdir.c
        ${CMAKE_SOURCE_DIR}/app/ops/op-mknod.c
        ${CMAKE_SOURCE_DIR}/app/ops/op-chmod.c
        ${CMAKE_SOURCE_DIR}/app/ops/op-flush.c
        ${CMAKE_SOURCE_DIR}/app/ops/op-fsync.c
        ${CMAKE_SOURCE_DIR}/app/ops/op-access.c
        ${CMAKE_SOURCE_DIR}/app/ops/op-create.c
        ${CMAKE_SOURCE_DIR}/app/ops/op-statfs.c
        ${CMAKE_SOURCE_DIR}/app/ops/op-unlink.c
        ${CMAKE_SOURCE_DIR}/app/ops/op-rename.c
        ${CMAKE_SOURCE_DIR}/app/ops/op-getattr.c
        ${CMAKE_SOURCE_DIR}/app/ops/op-release.c
        ${CMAKE_SOURCE_DIR}/app/ops/op-destroy.c
        ${CMAKE_SOURCE_DIR}/app/ops/op-symlink.c
        ${CMAKE_SOURCE_DIR}/app/ops/op-utimens.c
        ${CMAKE_SOURCE_DIR}/app/ops/op-truncate.c
        ${CMAKE_SOURCE_DIR}/app/ops/op-getxattr.c
        ${CMAKE_SOURCE_DIR}/app/ops/op-read-dir.c
        ${CMAKE_SOURCE_DIR}/app/ops/op-fget-attr.c
        ${CMAKE_SOURCE_DIR}/app/ops/op-read-link.c

        ${CMAKE_SOURCE_DIR}/app/ops/do-check.c
        ${CMAKE_SOURCE_DIR}/app/ops/do-probe.c
        ${CMAKE_SOURCE_DIR}/app/ops/do-read-inode.c
        ${CMAKE_SOURCE_DIR}/app/ops/do-write-inode.c
        ${CMAKE_SOURCE_DIR}/app/ops/do-fill-stat-buf.c
        ${CMAKE_SOURCE_DIR}/app/ops/do-kill-file-by-inode.c
)

add_executable(ext4 main.c ${EXT4_SRC})
target_link_libraries(ext4 PUBLIC
        ${FUSE_LIBRARIES}
        ${EXT2_LIBRARIES})
target_include_directories(ext4 PUBLIC
        ${CMAKE_SOURCE_DIR}/3thrd

        ${FUSE_INCLUDE_DIRS}
        ${EXT2_INCLUDE_DIRS})
