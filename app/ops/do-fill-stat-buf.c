//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"

#include <sys/sysmacros.h>


static inline dev_t old_decode_dev (__u16 val)
{
    return makedev((val >> 8) & 255, val & 255);
}

static inline dev_t new_decode_dev (__u32 dev)
{
    unsigned major = (dev & 0xfff00) >> 8;
    unsigned minor = (dev & 0xff) | ((dev >> 12) & 0xfff00);
    return makedev(major, minor);
}

void extfs_do_fill_statbuf (ext2_filsys e2fs, ext2_ino_t ino, struct ext2_inode *inode, struct stat *st)
{
    memset(st, 0, sizeof(*st));
    /* XXX workaround
     * should be unique and != existing devices */
    st->st_dev = (dev_t) ((long) e2fs);
    st->st_ino = ino;
    st->st_mode = inode->i_mode;
    st->st_nlink = inode->i_links_count;
    st->st_uid = extfs_read_uid(inode);
    st->st_gid = extfs_read_gid(inode);
    if (S_ISCHR(inode->i_mode) || S_ISBLK(inode->i_mode)) {
        if (inode->i_block[0]) {
            st->st_rdev = old_decode_dev(ext2fs_le32_to_cpu(inode->i_block[0]));
        }
        else {
            st->st_rdev = new_decode_dev(ext2fs_le32_to_cpu(inode->i_block[1]));
        }
    }
    else {
        st->st_rdev = 0;
    }
    st->st_size = EXT2_I_SIZE(inode);
    st->st_blksize = EXT2_BLOCK_SIZE(e2fs->super);
    st->st_blocks = inode->i_blocks;
    st->st_atime = inode->i_atime;
    st->st_mtime = inode->i_mtime;
    st->st_ctime = inode->i_ctime;
#if __FreeBSD__ == 10
    st->st_gen = inode->i_generation;
#endif
}