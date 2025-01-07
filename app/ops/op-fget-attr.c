//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"


int extfs_op_fgetattr (const char* path, struct stat* stBuf, struct fuse_file_info* fi)
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
    extfs_do_fill_statbuf(e2fs, ino, &inode, stBuf);

    return 0;
}