//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"

cint32 extfs_op_mkdir (const char* path, mode_t mode)
{
    int rt = 0;
    time_t tm = 0;
    ErrCode rc = 0;

    char* pPath = NULL;
    char* rPath = NULL;

    ext2_ino_t ino = 0;
    struct ext2_inode inode;
    struct fuse_context* ctx = NULL;
    ext2_filsys efs = current_ext2fs();

    rt = extfs_do_check_split(path, &pPath, &rPath);
    if (0 != rt) {
        return rt;
    }

    rt = extfs_do_read_inode(efs, pPath, &ino, &inode);
    if (rt) {
        extfs_free_split(pPath, rPath);
        return rt;
    }

    do {
        rc = ext2fs_mkdir(efs, ino, 0, rPath);
        if (rc == EXT2_ET_DIR_NO_SPACE) {
            if (ext2fs_expand_dir(efs, ino)) {
                extfs_free_split(pPath, rPath);
                return -ENOSPC;
            }
        }
    } while (rc == EXT2_ET_DIR_NO_SPACE);

    if (rc) {
        extfs_free_split(pPath, rPath);
        return -EIO;
    }

    rt = extfs_do_read_inode(efs, path, &ino, &inode);
    if (rt) {
        extfs_free_split(pPath, rPath);
        return -EIO;
    }

    tm = efs->now ? efs->now : time(NULL);
    inode.i_mode = LINUX_S_IFDIR | mode;
    inode.i_ctime = inode.i_atime = inode.i_mtime = tm;
    ctx = fuse_get_context();
    if (ctx) {
        extfs_write_uid(&inode, ctx->uid);
        extfs_write_gid(&inode, ctx->gid);
    }

    rc = extfs_do_write_inode(efs, ino, &inode);
    if (rc) {
        extfs_free_split(pPath, rPath);
        return -EIO;
    }

    rt = extfs_do_read_inode(efs, pPath, &ino, &inode);
    if (rt) {
        extfs_free_split(pPath, rPath);
        return -EIO;
    }

    inode.i_ctime = inode.i_mtime = tm;
    rc = extfs_do_write_inode(efs, ino, &inode);
    if (rc) {
        extfs_free_split(pPath, rPath);
        return -EIO;
    }

    extfs_free_split(pPath, rPath);

    return 0;
}
