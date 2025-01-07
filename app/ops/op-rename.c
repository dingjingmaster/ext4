//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"


static int do_fix_dot_dot (ext2_filsys e2fs, ext2_ino_t ino, ext2_ino_t dotDot);
static int fix_dot_dot_proc (ext2_ino_t dir EXT2FS_ATTR((unused)), int entry EXT2FS_ATTR((unused)), struct ext2_dir_entry* dirent, int offset EXT2FS_ATTR((unused)), int blockSize EXT2FS_ATTR((unused)), char* buf EXT2FS_ATTR((unused)), void *private);

int extfs_op_rename (const char* source, const char* dest)
{
    int rt = 0;
    ErrCode rc = 0;

    char* pSrc = NULL;
    char* rSrc = NULL;
    char* pDest = NULL;
    char* rDest = NULL;

    ext2_ino_t srcIno;
    ext2_ino_t destIno;
    ext2_ino_t dSrcIno;
    ext2_ino_t dDestIno;
    struct ext2_inode srcInode;
    struct ext2_inode destInode;
    struct ext2_inode dSrcInode;
    struct ext2_inode dDestInode;
    ext2_filsys e2fs = current_ext2fs();

    rt = extfs_do_check_split(source, &pSrc, &rSrc);
    if (rt != 0) {
        return rt;
    }

    rt = extfs_do_check_split(dest, &pDest, &rDest);
    if (rt != 0) {
        return rt;
    }

    rt = extfs_do_read_inode(e2fs, pSrc, &dSrcIno, &dSrcInode);
    if (rt != 0) {
        goto out;
    }

    rt = extfs_do_read_inode(e2fs, pDest, &dDestIno, &dDestInode);
    if (rt != 0) {
        goto out;
    }

    rt = extfs_do_read_inode(e2fs, source, &srcIno, &srcInode);
    if (rt != 0) {
        goto out;
    }

    rt = extfs_do_read_inode(e2fs, dest, &destIno, &destInode);
    if (rt != 0 && rt != -ENOENT) {
        goto out;
    }

    /* If oldpath  and  newpath are existing hard links referring to the same
     * file, then rename() does nothing, and returns a success status.
     */
    if (rt == 0 && srcIno == destIno) {
        goto out;
    }

    if (rt == 0) {
        if (LINUX_S_ISDIR(destInode.i_mode)) {
            if (!(LINUX_S_ISDIR(srcInode.i_mode))) {
                rt = -EISDIR;
                goto out;
            }
            rt = extfs_do_check_empty_dir(e2fs, destIno);
            if (rt != 0) {
                goto out;
            }
        }
        if (LINUX_S_ISDIR(srcInode.i_mode) && !(LINUX_S_ISDIR(destInode.i_mode))) {
            rt = -ENOTDIR;
            goto out;
        }
        if (LINUX_S_ISDIR(destInode.i_mode)) {
            rc = extfs_op_rmdir(dest);
        }
        else {
            rc = extfs_op_unlink(dest);
        }

        if (rc) {
            goto out;
        }
        rt = extfs_do_read_inode(e2fs, pDest, &dDestIno, &dDestInode);
        if (rt != 0) {
            goto out;
        }
    }
    /* Step 2: add the link */
    do {
        rc = ext2fs_link(e2fs, dDestIno, rDest, srcIno, extfs_do_mode_to_ext2_lag(srcInode.i_mode));
        if (rc == EXT2_ET_DIR_NO_SPACE) {
            if (ext2fs_expand_dir(e2fs, dDestIno)) {
                rt = -ENOSPC;
                goto out;
            }
            /* ext2fs_expand_dir changes d_dest_inode */
            rt = extfs_do_read_inode(e2fs, pDest, &dDestIno, &dDestInode);
            if (rt != 0) {
                goto out;
            }
        }
    } while (rc == EXT2_ET_DIR_NO_SPACE);

    if (rc != 0) {
        rt = -EIO;
        goto out;
    }

    /* Special case: if moving dir across different parents fix counters and '..' */
    if (LINUX_S_ISDIR(srcInode.i_mode) && dSrcIno != dDestIno) {
        dDestInode.i_links_count++;
        if (dSrcInode.i_links_count > 1) {
            dSrcInode.i_links_count--;
        }
        rc = extfs_do_write_inode(e2fs, dSrcIno, &dSrcInode);
        if (rc != 0) {
            rt = -EIO;
            goto out;
        }
        rt = do_fix_dot_dot(e2fs, srcIno, dDestIno);
        if (rt != 0) {
            goto out;
        }
    }

    /* utimes and inodes update */
    dDestInode.i_mtime = dDestInode.i_ctime = srcInode.i_ctime = e2fs->now ? e2fs->now : time(NULL);
    rt = extfs_do_write_inode(e2fs, dDestIno, &dDestInode);
    if (rt != 0) {
        goto out;
    }
    rt = extfs_do_write_inode(e2fs, srcIno, &srcInode);
    if (rt != 0) {
        goto out;
    }

    /* Step 3: delete the source */
    rc = ext2fs_unlink(e2fs, dSrcIno, rSrc, srcIno, 0);
    if (rc) {
        rt = -EIO;
        goto out;
    }

out:
    extfs_free_split(pSrc, rSrc);
    extfs_free_split(pDest, rDest);

    return rt;
}


static int fix_dot_dot_proc (ext2_ino_t dir, int entry, struct ext2_dir_entry* dirent, int offset, int blockSize, char* buf, void *private)
{
    ext2_ino_t *pDotDot = (ext2_ino_t*) private;

    if ((dirent->name_len & 0xFF) == 2 && strncmp(dirent->name, "..", 2) == 0) {
        dirent->inode = *pDotDot;
        return DIRENT_ABORT | DIRENT_CHANGED;
    }

    return 0;
}

static int do_fix_dot_dot (ext2_filsys e2fs, ext2_ino_t ino, ext2_ino_t dotDot)
{
    errcode_t rc = 0;

    rc = ext2fs_dir_iterate2(e2fs, ino, DIRENT_FLAG_INCLUDE_EMPTY, 0, fix_dot_dot_proc, &dotDot);
    if (rc) {
        return -EIO;
    }

    return 0;
}
