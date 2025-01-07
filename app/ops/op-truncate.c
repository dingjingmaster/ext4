//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"


int extfs_op_truncate (const char *path, cint64 len)
{
    int rt = 0;
    errcode_t rc = 0;
    ext2_ino_t ino = 0;
    struct ext2_inode inode;
    ext2_file_t efile;
    ext2_filsys e2fs = current_ext2fs();

    rt = extfs_do_check(path);
    if (rt != 0) {
        return rt;
    }

    efile = extfs_do_open(e2fs, path, O_WRONLY);
    if (efile == NULL) {
        return -ENOENT;
    }

    rc = ext2fs_file_set_size2(efile, len);
    if (rc) {
        extfs_do_release(efile);
        if (rc == EXT2_ET_FILE_TOO_BIG) {
            return -EFBIG;
        }
        return -EIO;
    }

    rt = extfs_do_read_inode(e2fs, path, &ino, &inode);
    if (rt) {
        extfs_do_release(efile);
        return rt;
    }
    inode.i_ctime = e2fs->now ? e2fs->now : time(NULL);
    inode.i_mtime = e2fs->now ? e2fs->now : time(NULL);
    rt = extfs_do_write_inode(e2fs, ino, &inode);
    if (rt) {
        extfs_do_release(efile);
        return -EIO;
    }

    rt = extfs_do_release(efile);
    if (rt != 0) {
        return rt;
    }

    return 0;
}

int extfs_op_ftruncate (const char *path, cint64 len, struct fuse_file_info *fi)
{
    (void) fi;

    return extfs_op_truncate(path, len);
}
