//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"

int extfs_op_utimens (const char* path, const struct timespec tv[2])
{
    int rt = 0;
    ext2_ino_t ino = 0;
    struct ext2_inode inode;
    ext2_filsys e2fs = current_ext2fs();

    rt = extfs_do_check(path);
    if (rt != 0) {
        return rt;
    }

    rt = extfs_do_read_inode(e2fs, path, &ino, &inode);
    if (rt) {
        return rt;
    }

    if (tv[0].tv_nsec != UTIME_OMIT) {
        inode.i_atime = tv[0].tv_sec;
    }

    if (tv[1].tv_nsec != UTIME_OMIT) {
        inode.i_mtime = tv[1].tv_sec;
    }

    rt = extfs_do_write_inode(e2fs, ino, &inode);
    if (rt) {
        return -EIO;
    }

    return 0;
}
