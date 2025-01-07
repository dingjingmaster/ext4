//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"

typedef struct _DirWalkData
{
    char*           buf;
    fuse_fill_dir_t filler;
} DirWalkData;

#if defined(_USE_DIR_ITERATE2) && (_USE_DIR_ITERATE2 == 1)
static int walk_dir2 (ext2_ino_t dir, int   entry, struct ext2_dir_entry *dirent, int offset, int blocksize, char *buf, void *vpsid)
{
    int res = 0;
    int len = 0;
    struct stat st;
    unsigned char type = 0;

    if (dirent->name_len <= 0) {
        return 0;
    }
    DirWalkData* psid = (DirWalkData*) vpsid;
    memset(&st, 0, sizeof(st));

    len = dirent->name_len & 0xff;
    dirent->name[len] = 0; // bug wraparound

    switch  (dirent->name_len >> 8) {
        case EXT2_FT_UNKNOWN:   type = DT_UNKNOWN;      break;
        case EXT2_FT_REG_FILE:  type = DT_REG;          break;
        case EXT2_FT_DIR:       type = DT_DIR;          break;
        case EXT2_FT_CHRDEV:    type = DT_CHR;          break;
        case EXT2_FT_BLKDEV:    type = DT_BLK;          break;
        case EXT2_FT_FIFO:      type = DT_FIFO;         break;
        case EXT2_FT_SOCK:      type = DT_SOCK;         break;
        case EXT2_FT_SYMLINK:   type = DT_LNK;          break;
        default:                type = DT_UNKNOWN;      break;
    }
    if (type == DT_UNKNOWN) {
        return 0;
    }
    {
        int rc = 0;
        struct ext2_inode inode;
        ext2_filsys efs = current_ext2fs();
        rc = (int) ext2fs_read_inode(efs, dirent->inode, &inode);
        if (rc) {
            return 0;
        }
    }
    st.st_ino = dirent->inode;
    st.st_mode = type << 12;
    res = psid->filler(psid->buf, dirent->name, &st, 0);
    if (res != 0) {
        return BLOCK_ABORT;
    }

    return 0;
}
#else
static int walk_dir (struct ext2_dir_entry *de, int offset, int blocksize, char* buf, void* priv_data)
{
    int ret = 0;
    size_t fLen = 0;
    char* fName = NULL;
    DirWalkData* b = priv_data;

    fLen = de->name_len & 0xff;
    fName = (char *) malloc(sizeof(char) * (fLen + 1));
    if (fName == NULL) {
        return -ENOMEM;
    }
    snprintf(fName, fLen + 1, "%s", de->name);
    ret = b->filler(b->buf, fName, NULL, 0);
    free(fName);

    return ret;
}
#endif

int extfs_op_read_dir (const char* path, void* buf, fuse_fill_dir_t filler, cint64 offset, struct fuse_file_info* fi)
{
    int rt = 0;
    errcode_t rc = 0;
    ext2_ino_t ino = 0;
    struct ext2_inode inode;
    DirWalkData dwd = {
        .buf = buf,
        .filler = filler
    };
    ext2_filsys e2fs = current_ext2fs();

    rt = extfs_do_read_inode(e2fs, path, &ino, &inode);
    if (rt) {
        return rt;
    }

#if defined(_USE_DIR_ITERATE2) && (_USE_DIR_ITERATE2 == 1)
    rc = ext2fs_dir_iterate2(e2fs,ino, DIRENT_FLAG_INCLUDE_EMPTY, NULL, walk_dir2, &dwd);
#else
    rc = ext2fs_dir_iterate(e2fs, ino, 0, NULL, walk_dir, &dwd);
#endif

    if (rc) {
        return -EIO;
    }

    return 0;
}
