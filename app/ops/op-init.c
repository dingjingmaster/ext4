//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"

void* extfs_op_init (struct fuse_conn_info* conn)
{
    ErrCode rc = 0;

    struct fuse_context* ctx = fuse_get_context();
    ExtFsData* data = ctx->private_data;

    rc = ext2fs_open(data->device, (data->readonly) ? 0 : EXT2_FLAG_RW, 0, 0, unix_io_manager, &data->e2fs);
    if (rc) {
        exit(1);
    }

    rc = ext2fs_read_bitmaps(data->e2fs);
    if (rc) {
        ext2fs_close(data->e2fs);
        exit(1);
    }

    return data;
}
