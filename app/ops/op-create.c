//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"

#include <sys/sysmacros.h>


static inline int old_valid_dev(dev_t dev)
{
    return major(dev) < 256 && minor(dev) < 256;
}

static inline __u16 old_encode_dev(dev_t dev)
{
    return (major(dev) << 8) | minor(dev);
}

static inline __u32 new_encode_dev(dev_t dev)
{
    unsigned major_v = major(dev);
    unsigned minor_v = minor(dev);
    return (minor_v & 0xff) | (major_v << 8) | ((minor_v & ~0xff) << 12);
}


cint32 extfs_do_mode_to_ext2_lag (mode_t mode)
{
    if (S_ISREG(mode)) {
        return EXT2_FT_REG_FILE;
    }
    else if (S_ISDIR(mode)) {
        return EXT2_FT_DIR;
    }
    else if (S_ISCHR(mode)) {
        return EXT2_FT_CHRDEV;
    }
    else if (S_ISBLK(mode)) {
        return EXT2_FT_BLKDEV;
    }
    else if (S_ISFIFO(mode)) {
        return EXT2_FT_FIFO;
    }
    else if (S_ISSOCK(mode)) {
        return EXT2_FT_SOCK;
    }
    else if (S_ISLNK(mode)) {
        return EXT2_FT_SYMLINK;
    }

    return EXT2_FT_UNKNOWN;
}

int extfs_do_create (ext2_filsys e2fs, const char *path, mode_t mode, dev_t dev, const char* fastsymlink)
{
    int rt = 0;
    time_t tm = 0;
    errcode_t rc = 0;

    char* pPath = NULL;
    char* rPath = NULL;

    ext2_ino_t ino = 0;
    ext2_ino_t n_ino = 0;
    struct ext2_inode inode;

    struct fuse_context* ctx = NULL;

    rt = extfs_do_check_split(path, &pPath, &rPath);
    rt = extfs_do_read_inode(e2fs, pPath, &ino, &inode);
    if (rt) {
        extfs_free_split(pPath, rPath);
        return rt;
    }

    rc = ext2fs_new_inode(e2fs, ino, mode, 0, &n_ino);
    if (rc) {
        extfs_free_split(pPath, rPath);
        return -ENOMEM;
    }

    do {
        rc = ext2fs_link(e2fs, ino, rPath, n_ino, extfs_do_mode_to_ext2_lag(mode));
        if (rc == EXT2_ET_DIR_NO_SPACE) {
            if (ext2fs_expand_dir(e2fs, ino)) {
                extfs_free_split(pPath, rPath);
                return -ENOSPC;
            }
        }
    } while (rc == EXT2_ET_DIR_NO_SPACE);

    if (rc) {
        extfs_free_split(pPath, rPath);
        return -EIO;
    }

    if (ext2fs_test_inode_bitmap(e2fs->inode_map, n_ino)) {
    }

    ext2fs_inode_alloc_stats2(e2fs, n_ino, +1, 0);
    memset(&inode, 0, sizeof(inode));
    tm = e2fs->now ? e2fs->now : time(NULL);
    inode.i_mode = mode;
    inode.i_atime = inode.i_ctime = inode.i_mtime = tm;
    inode.i_links_count = 1;
    inode.i_size = 0;
    ctx = fuse_get_context();
    if (ctx) {
        extfs_write_uid(&inode, ctx->uid);
        extfs_write_gid(&inode, ctx->gid);
    }
    if (e2fs->super->s_feature_incompat & EXT3_FEATURE_INCOMPAT_EXTENTS) {
        int i = 0;
        struct ext3_extent_header* eh = NULL;

        eh = (struct ext3_extent_header *) &inode.i_block[0];
        eh->eh_depth = 0;
        eh->eh_entries = 0;
        eh->eh_magic = ext2fs_cpu_to_le16(EXT3_EXT_MAGIC);
        i = (sizeof(inode.i_block) - sizeof(*eh)) / sizeof(struct ext3_extent);
        eh->eh_max = ext2fs_cpu_to_le16(i);
        inode.i_flags |= EXT4_EXTENTS_FL;
    }

    if (S_ISCHR(mode) || S_ISBLK(mode)) {
        if (old_valid_dev(dev)) {
            inode.i_block[0]= ext2fs_cpu_to_le32(old_encode_dev(dev));
        }
        else {
            inode.i_block[1]= ext2fs_cpu_to_le32(new_encode_dev(dev));
        }
    }

    if (S_ISLNK(mode) && fastsymlink != NULL) {
        inode.i_size = strlen(fastsymlink);
        strncpy((char *)&(inode.i_block[0]),fastsymlink, (EXT2_N_BLOCKS * sizeof(inode.i_block[0])));
    }

    rc = ext2fs_write_new_inode(e2fs, n_ino, &inode);
    if (rc) {
        extfs_free_split(pPath, rPath);
        return -EIO;
    }

    /* update parent dir */
    rt = extfs_do_read_inode(e2fs, pPath, &ino, &inode);
    if (rt) {
        extfs_free_split(pPath, rPath);
        return -EIO;
    }
    inode.i_ctime = inode.i_mtime = tm;
    rc = extfs_do_write_inode(e2fs, ino, &inode);
    if (rc) {
        extfs_free_split(pPath, rPath);
        return -EIO;
    }

    extfs_free_split(pPath, rPath);

    return 0;
}




cint32 extfs_op_create (const char *path, mode_t mode, struct fuse_file_info *fi)
{
    int rt = 0;
    ext2_filsys e2fs = current_ext2fs();

    if (extfs_op_open(path, fi) == 0) {
        return 0;
    }

    rt = extfs_do_create(e2fs, path, mode, 0, NULL);
    if (rt != 0) {
        return rt;
    }

    if (extfs_op_open(path, fi)) {
        return -EIO;
    }

    return 0;
}
