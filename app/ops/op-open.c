//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"


int extfs_op_open (const char* path, struct fuse_file_info* fi)
{
    ext2_file_t efile;
    ext2_filsys e2fs = current_ext2fs();

    efile = extfs_do_open(e2fs, path, fi->flags);
    if (efile == NULL) {
        return -ENOENT;
    }
    fi->fh = (uint64_t) efile;

    return 0;
}

ext2_file_t extfs_do_open (ext2_filsys e2fs, const char* path, int flags)
{
    int rt = 0;
    errcode_t rc = 0;
    ext2_ino_t ino = 0;
    ext2_file_t file = NULL;
    struct ext2_inode inode;
    struct fuse_context* ctx = fuse_get_context();
    ExtFsData* e2data = ctx->private_data;

    rt = extfs_do_check(path);
    if (rt != 0) {
        return NULL;
    }

    rt = extfs_do_read_inode(e2fs, path, &ino, &inode);
    if (rt) {
        return NULL;
    }

    rc = ext2fs_file_open2(e2fs, ino, &inode, (((flags & O_ACCMODE) != 0) ? EXT2_FILE_WRITE : 0) | EXT_FILE_SHARED_INODE, &file);
    if (rc) {
        return NULL;
    }

    if (e2data->readonly == 0) {
        inode.i_atime = e2fs->now ? e2fs->now : time(NULL);
        rt = extfs_do_write_inode(e2fs, ino, &inode);
        if (rt) {
            return NULL;
        }
    }

    return file;
}