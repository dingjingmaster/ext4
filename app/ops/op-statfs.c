//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"

#define EXT2_BLOCKS_COUNT(s)    ((s)->s_blocks_count | ((__u64) (s)->s_blocks_count_hi << 32))
#define EXT2_RBLOCKS_COUNT(s)   ((s)->s_r_blocks_count | ((__u64) (s)->s_r_blocks_count_hi << 32))
#define EXT2_FBLOCKS_COUNT(s)   ((s)->s_free_blocks_count | ((__u64) (s)->s_free_blocks_hi << 32))

static int ext2_group_spare (int group);
static int test_root (unsigned int a, unsigned int b);
static int ext2_bg_num_gdb (ext2_filsys e2fs, int group);
static int ext2_bg_has_super (ext2_filsys e2fs, int group);


int extfs_op_statfs (const char* path, struct statvfs* buf)
{
    unsigned long long i = 0;
    unsigned long long sGdbCount = 0;
    unsigned long long sGroupsCount = 0;
    unsigned long long sItbPerGroup = 0;
    unsigned long long sOverheadLast = 0;
    unsigned long long sInodesPerBlock = 0;

    ext2_filsys e2fs = current_ext2fs();

    memset(buf, 0, sizeof(struct statvfs));
    if (e2fs->super->s_default_mount_opts & EXT2_MOUNT_MINIX_DF) {
        sOverheadLast = 0;
    }
    else {
        sOverheadLast = e2fs->super->s_first_data_block;
        sGroupsCount = ((EXT2_BLOCKS_COUNT(e2fs->super) - e2fs->super->s_first_data_block - 1) / e2fs->super->s_blocks_per_group) + 1;
        sGdbCount = (sGroupsCount + EXT2_DESC_PER_BLOCK(e2fs->super) - 1) / EXT2_DESC_PER_BLOCK(e2fs->super);
        for (i = 0; i < sGroupsCount; i++) {
            sOverheadLast += ext2_bg_has_super(e2fs, i) + ((ext2_bg_num_gdb(e2fs, i) == 0) ? 0 : sGdbCount);
        }
        sInodesPerBlock = EXT2_BLOCK_SIZE(e2fs->super) / EXT2_INODE_SIZE(e2fs->super);
        sItbPerGroup = e2fs->super->s_inodes_per_group / sInodesPerBlock;
        sOverheadLast += (sGroupsCount * (2 +  sItbPerGroup));
    }
    buf->f_bsize = EXT2_BLOCK_SIZE(e2fs->super);
    buf->f_frsize = EXT2_FRAG_SIZE(e2fs->super);
    buf->f_blocks = EXT2_BLOCKS_COUNT(e2fs->super) - sOverheadLast;
    buf->f_bfree = EXT2_FBLOCKS_COUNT(e2fs->super);
    if (EXT2_FBLOCKS_COUNT(e2fs->super) < EXT2_RBLOCKS_COUNT(e2fs->super)) {
        buf->f_bavail = 0;
    }
    else {
        buf->f_bavail = EXT2_FBLOCKS_COUNT(e2fs->super) - EXT2_RBLOCKS_COUNT(e2fs->super);
    }
    buf->f_files = e2fs->super->s_inodes_count;
    buf->f_ffree = e2fs->super->s_free_inodes_count;
    buf->f_favail = e2fs->super->s_free_inodes_count;
    buf->f_namemax = EXT2_NAME_LEN;

    return 0;
}

static int ext2_group_spare (int group)
{
    if (group <= 1) {
        return 1;
    }
    return (test_root(group, 3) || test_root(group, 5) || test_root(group, 7));
}


static int ext2_bg_has_super (ext2_filsys e2fs, int group)
{
    if (EXT2_HAS_RO_COMPAT_FEATURE(e2fs->super, EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER) && !ext2_group_spare(group)) {
        return 0;
    }

    return 1;
}


static int ext2_bg_num_gdb (ext2_filsys e2fs, int group)
{
    if (EXT2_HAS_RO_COMPAT_FEATURE(e2fs->super, EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER) && !ext2_group_spare(group)) {
        return 0;
    }

    return 1;
}


static int test_root (unsigned int a, unsigned int b)
{
    while (1) {
        if (a < b) {
            return 0;
        }
        if (a == b) {
            return 1;
        }
        if (a % b) {
            return 0;
        }
        a = a / b;
    }

    // IMPOSSIBLE
    return 1;
}
