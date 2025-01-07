//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"

int extfs_op_chmod (const char* path, mode_t mode)
{
    int rt = 0;
    int mask = 0;
    time_t tm = 0;
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

    tm = e2fs->now ? e2fs->now : time(NULL);
    mask = S_IRWXU | S_IRWXG | S_IRWXO | S_ISUID | S_ISGID | S_ISVTX;
    inode.i_mode = (inode.i_mode & ~mask) | (mode & mask);
    inode.i_ctime = tm;

    rt = extfs_do_write_inode(e2fs, ino, &inode);
    if (rt) {
        return -EIO;
    }

    return 0;
}
