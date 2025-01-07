//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"


int extfs_op_read (const char *path, char *buf, cuint64 size, cint64 offset, struct fuse_file_info *fi)
{
    __u64 pos = 0;
    errcode_t rc = 0;
    unsigned int bytes;
    ext2_file_t file = EXT_FS_FILE(fi->fh);
    ext2_filsys e2fs = current_ext2fs();

    file = extfs_do_open(e2fs, path, O_RDONLY);
    rc = ext2fs_file_llseek(file, offset, SEEK_SET, &pos);
    if (rc) {
        extfs_do_release(file);
        return -EINVAL;
    }

    rc = ext2fs_file_read(file, buf, size, &bytes);
    if (rc) {
        extfs_do_release(file);
        return -EIO;
    }
    extfs_do_release(file);

    return bytes;
}
