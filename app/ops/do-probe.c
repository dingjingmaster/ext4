//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"


#define VOLNAME_SIZE_MAX 16


int extfs_do_probe (ExtFsData* opts)
{
    errcode_t rc = 0;
    ext2_filsys e2fs;

    rc = ext2fs_open(opts->device, EXT2_FLAG_RW, 0, 0, unix_io_manager, &e2fs);
    if (rc) {
        return -1;
    }
#if 0
    rc = ext2fs_read_bitmaps(e2fs);
    if (rc) {
        debugf_main("Error while reading bitmaps (rc=%d)", rc);
        ext2fs_close(e2fs);
        return -2;
    }
#endif
    if (e2fs->super != NULL) {
        opts->volName = (char *) malloc(sizeof(char) * (VOLNAME_SIZE_MAX + 1));
        if (opts->volName != NULL) {
            memset(opts->volName, 0, sizeof(char) * (VOLNAME_SIZE_MAX + 1));
            strncpy(opts->volName, e2fs->super->s_volume_name, VOLNAME_SIZE_MAX);
            opts->volName[VOLNAME_SIZE_MAX] = '\0';
        }
    }
    ext2fs_close(e2fs);

    return 0;
}