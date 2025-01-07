//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"

cint32 extfs_op_symlink (const char* srcName, const char* dstName)
{
    int rt = 0;
    cuint64 wr = 0;
    ext2_file_t file;
    ext2_filsys e2fs = current_ext2fs();
    int srcLen = (int) strlen(srcName);

    if (srcLen < (EXT2_N_BLOCKS * sizeof(__u32))) {
        rt = extfs_do_create(e2fs, dstName, LINUX_S_IFLNK | 0777, 0, srcName);
        if (0 != rt) {
            return rt;
        }
    }
    else {
        rt = extfs_do_create(e2fs, dstName, LINUX_S_IFLNK | 0777, 0, NULL);
        if (0 != rt) {
            return rt;
        }

        file = extfs_do_open(e2fs, dstName, O_WRONLY);
        if (NULL == file) {
            return -EIO;
        }
        wr = extfs_do_write(file, srcName, srcLen, 0);
        if (wr != strlen(srcName)) {
            return -EIO;
        }

        rt = extfs_do_release(file);
        if (rt != 0) {
            return rt;
        }
    }

    return 0;
}
