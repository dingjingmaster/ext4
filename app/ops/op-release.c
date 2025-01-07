//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"


int extfs_do_release (ext2_file_t file)
{
    errcode_t rc = 0;

    if (file == NULL) {
        return -ENOENT;
    }
    rc = ext2fs_file_close(file);
    if (rc) {
        return -EIO;
    }

    return 0;
}

int extfs_op_release (const char* path, struct fuse_file_info* fi)
{
    int rt = 0;
    ext2_file_t file = (ext2_file_t) (unsigned long) fi->fh;

    rt = extfs_do_release(file);
    if (rt != 0) {
        return rt;
    }

    return 0;
}
