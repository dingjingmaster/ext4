//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"


void extfs_op_destroy (void* userData)
{
    ext2_filsys e2fs = current_ext2fs();
    ext2fs_close(e2fs);
    e2fs = NULL;
}
