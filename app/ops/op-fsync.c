//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"


int extfs_op_fsync (const char* path, int dataSync, struct fuse_file_info* fi)
{
    errcode_t rc = 0;
    ext2_filsys efs = current_ext2fs();

    rc = ext2fs_flush(efs);
    if (rc) {
        return -EIO;
    }

    return 0;
}
