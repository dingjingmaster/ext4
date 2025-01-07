//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"


int extfs_op_link (const char* source, const char* dest)
{
    int rc = 0;
    char* pPath = NULL;
    char* rPath = NULL;

    ext2_ino_t sIno;
    ext2_ino_t dIno;
    struct ext2_inode sInode;
    struct ext2_inode dInode;
    ext2_filsys e2fs = current_ext2fs();

    rc = extfs_do_check(source);
    if (rc != 0) {
        return rc;
    }

    rc = extfs_do_check_split(dest, &pPath, &rPath);
    if (rc != 0) {
        return rc;
    }

    rc = extfs_do_read_inode(e2fs, pPath, &dIno, &dInode);
    if (rc) {
        extfs_free_split(pPath, rPath);
        return rc;
    }

    rc = extfs_do_read_inode(e2fs, source, &sIno, &sInode);
    if (rc) {
        extfs_free_split(pPath, rPath);
        return rc;
    }

    do {
        rc = ext2fs_link(e2fs, dIno, rPath, sIno, extfs_do_mode_to_ext2_lag(sInode.i_mode));
        if (rc == EXT2_ET_DIR_NO_SPACE) {
            if (ext2fs_expand_dir(e2fs, dIno)) {
                extfs_free_split(pPath, rPath);
                return -ENOSPC;
            }
        }
    } while (rc == EXT2_ET_DIR_NO_SPACE);
    if (rc) {
        extfs_free_split(pPath, rPath);
        return -EIO;
    }

    dInode.i_mtime = dInode.i_ctime = sInode.i_ctime = e2fs->now ? e2fs->now : time(NULL);
    sInode.i_links_count += 1;

    rc = extfs_do_write_inode(e2fs, sIno, &sInode);
    if (rc) {
        extfs_free_split(pPath, rPath);
        return -EIO;
    }

    rc = extfs_do_write_inode(e2fs, dIno, &dInode);
    if (rc) {
        extfs_free_split(pPath, rPath);
        return -EIO;
    }

    extfs_free_split(pPath, rPath);

    return 0;
}
