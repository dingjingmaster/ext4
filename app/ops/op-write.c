//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"


int extfs_op_write (const char *path, const char *buf, cuint64 size, cint64 offset, struct fuse_file_info *fi)
{
    size_t rt = 0;
    ext2_file_t file = EXT_FS_FILE(fi->fh);
    ext2_filsys e2fs = current_ext2fs();

    file = extfs_do_open(e2fs, path, O_WRONLY);
    rt = extfs_do_write(file, buf, size, offset);
    extfs_do_release(file);

    return rt;
}


size_t do_write (ext2_file_t file, const char *buf, cuint64 size, cint64 offset)
{
    int rt = 0;
    const char* tmp = NULL;
    unsigned int wr = 0;
    unsigned long long nPos = 0;
    unsigned long long fSize = 0;
    unsigned long long wSize = 0;

    rt = ext2fs_file_get_lsize(file, &fSize);
    if (rt != 0) {
        return -EIO;
    }

    rt = ext2fs_file_llseek(file, offset, SEEK_SET, &nPos);
    if (rt) {
        return rt;
    }

    for (rt = 0, wr = 0, tmp = buf, wSize = 0; size > 0 && rt == 0; size -= wr, wSize += wr, tmp += wr) {
        rt = ext2fs_file_write(file, tmp, size, &wr);
    }
    if (rt != 0 && rt != EXT2_ET_BLOCK_ALLOC_FAIL) {
        return -EIO;
    }

    if (offset + wSize > fSize) {
        rt = ext2fs_file_set_size2(file, offset + wSize);
        if (rt) {
            return -EIO;
        }
    }

    rt = ext2fs_file_flush(file);
    if (rt != 0 && rt != EXT2_ET_BLOCK_ALLOC_FAIL) {
        return -EIO;
    }

    return wSize;
}


