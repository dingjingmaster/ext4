//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"


int extfs_do_read_inode (ext2_filsys e2fs, const char* path, ext2_ino_t* ino, struct ext2_inode* inode)
{
    errcode_t rc = ext2fs_namei(e2fs, EXT2_ROOT_INO, EXT2_ROOT_INO, path, ino);
    if (rc) {
        return -ENOENT;
    }

    rc = ext2fs_read_inode(e2fs, *ino, inode);
    if (rc) {
        return -EIO;
    }

    return 0;
}
