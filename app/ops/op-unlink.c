//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"


cint32 extfs_op_unlink (const char* path)
{
    int rt = 0;
    ErrCode rc = 0;
    char* pPath = NULL;
    char* rPath = NULL;
    ext2_ino_t pIno = 0;
    ext2_ino_t rIno = 0;
    struct ext2_inode pInode;
    struct ext2_inode rInode;
    ext2_filsys efs = current_ext2fs();

    rt = extfs_do_check(path);
    if (0 != rt) {
        return rt;
    }

    rt = extfs_do_check_split(path, &pPath, &rPath);
    if (0 != rt) {
        return rt;
    }

    rt = extfs_do_read_inode(efs, pPath, &pIno, &pInode);
    if (rt) {
        extfs_free_split(pPath, rPath);
        return rt;
    }

    rt = extfs_do_read_inode(efs, path, &rIno, &rInode);
    if (rt) {
        extfs_free_split(pPath, rPath);
        return rt;
    }

    if (LINUX_S_ISDIR(rInode.i_mode)) {
        extfs_free_split(pPath, rPath);
        return -EISDIR;
    }

    rc = ext2fs_unlink(efs, pIno, rPath, rIno, 0);
    if (rc) {
        extfs_free_split(pPath, rPath);
        return -EIO;
    }

    pInode.i_ctime = pInode.i_mtime = efs->now ? efs->now : time(NULL);
    rt = extfs_do_write_inode(efs, pIno, &pInode);
    if (rt) {
        extfs_free_split(pPath, rPath);
        return -EIO;
    }

    if (rInode.i_links_count > 0) {
        rInode.i_links_count -= 1;
    }

    rInode.i_ctime = efs->now ? efs->now : time(NULL);
    rc = extfs_do_write_inode(efs, rIno, &rInode);
    if (rc) {
        extfs_free_split(pPath, rPath);
        return -EIO;
    }

    extfs_free_split(pPath, rPath);

    return 0;
}
