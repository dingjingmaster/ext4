//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"

int extfs_op_flush (const char* path, struct fuse_file_info* fi)
{
    errcode_t rc = 0;
    ext2_file_t file = EXT_FS_FILE(fi->fh);

    if (file == NULL) {
        return -ENOENT;
    }

    rc = ext2fs_file_flush(file);
    if (rc) {
        return -EIO;
    }

    return 0;
}