//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"


cint32 extfs_do_write_inode (ext2_filsys e2fs, ext2_ino_t ino, struct ext2_inode* inode)
{
    int rt = 0;
    errcode_t rc = 0;
    if (inode->i_links_count < 1) {
        rt = extfs_do_kill_file_by_inode(e2fs, ino, inode);
        if (rt) {
            return rt;
        }
    }
    else {
        rc = ext2fs_write_inode(e2fs, ino, inode);
        if (rc) {
            return -EIO;
        }
    }

    return 0;
}
