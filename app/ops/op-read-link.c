//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"


cint32 extfs_op_read_link (const char* path, char* buf, cuint64 size)
{
    int rt = 0;
    cuint64 s = 0;
    ErrCode rc = 0;
    ext2_ino_t ino = 0;
    char* b = NULL;
    char* pathName = NULL;
    struct ext2_inode inode;
    ext2_filsys e2fs = current_ext2fs();

    rt = extfs_do_read_inode(e2fs, path, &ino, &inode);
    if (rt) {
        return rt;
    }

    if (!LINUX_S_ISLNK(inode.i_mode)) {
        return -EINVAL;
    }

    if (ext2fs_inode_data_blocks(e2fs, &inode)) {
        rc = ext2fs_get_mem(EXT2_BLOCK_SIZE(e2fs->super), &b);
        if (rc) {
            return -ENOMEM;
        }
        rc = io_channel_read_blk(e2fs->io, inode.i_block[0], 1, b);
        if (rc) {
            ext2fs_free_mem(&b);
            return -EIO;
        }
        pathName = b;
    }
    else {
        pathName = (char*) &(inode.i_block[0]);
    }

    s = (size < strlen(pathName) + 1) ? size : strlen(pathName) + 1;
    snprintf(buf, s, "%s", pathName);

    if (b) {
        ext2fs_free_mem(&b);
    }

    return 0;
}
