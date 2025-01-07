//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"

cint32 extfs_op_access (const char* path, int mask)
{
    cint32 rt = 0;
    ext2_filsys e2fs = current_ext2fs();

    rt = extfs_do_check(path);
    if (rt != 0) {
        return rt;
    }

    if ((mask & W_OK) && !(e2fs->flags & EXT2_FLAG_RW)) {
        return -EACCES;
    }

    return 0;
}
