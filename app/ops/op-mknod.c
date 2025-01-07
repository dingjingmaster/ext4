//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"


cint32 extfs_op_mknod (const char* path, mode_t mode, dev_t dev)
{
    int rt = 0;
    ext2_filsys e2fs = current_ext2fs();

    rt = extfs_do_create(e2fs, path, mode, dev, NULL);

    return rt;
}

