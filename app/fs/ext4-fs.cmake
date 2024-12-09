file(GLOB EXT4_FS_SRC
    ${CMAKE_SOURCE_DIR}/app/fs/common.h

    ${CMAKE_SOURCE_DIR}/app/fs/dcache.h
    ${CMAKE_SOURCE_DIR}/app/fs/dcache.c

    ${CMAKE_SOURCE_DIR}/app/fs/disk.h
    ${CMAKE_SOURCE_DIR}/app/fs/disk.c

    ${CMAKE_SOURCE_DIR}/app/fs/extents.h
    ${CMAKE_SOURCE_DIR}/app/fs/extents.c

    ${CMAKE_SOURCE_DIR}/app/fs/inode.h
    ${CMAKE_SOURCE_DIR}/app/fs/inode.c

    ${CMAKE_SOURCE_DIR}/app/fs/logging.h
    ${CMAKE_SOURCE_DIR}/app/fs/logging.c

    ${CMAKE_SOURCE_DIR}/app/fs/super.h
    ${CMAKE_SOURCE_DIR}/app/fs/super.c
)