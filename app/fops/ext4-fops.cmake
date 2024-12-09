file(GLOB EXT4_FOPS_SRC
    ${CMAKE_SOURCE_DIR}/app/fops/ops.h
    ${CMAKE_SOURCE_DIR}/app/fops/op-init.c
    ${CMAKE_SOURCE_DIR}/app/fops/op-open.c
    ${CMAKE_SOURCE_DIR}/app/fops/op-read.c
    ${CMAKE_SOURCE_DIR}/app/fops/op-readdir.c
    ${CMAKE_SOURCE_DIR}/app/fops/op-getattr.c
    ${CMAKE_SOURCE_DIR}/app/fops/op-readlink.c
)