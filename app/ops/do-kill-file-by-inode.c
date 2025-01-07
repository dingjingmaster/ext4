//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"


static int release_blocks_proc (ext2_filsys fs, blk_t* blocknr, int blockcnt, void *private);


cint32 extfs_do_kill_file_by_inode (ext2_filsys e2fs, ext2_ino_t ino, struct ext2_inode *inode)
{
    errcode_t rc = 0;
    char scratchBuf[3 * e2fs->blocksize];

    inode->i_links_count = 0;
    inode->i_dtime = time(NULL);

    rc = ext2fs_write_inode(e2fs, ino, inode);
    if (rc) {
        return -EIO;
    }

    if (ext2fs_inode_has_valid_blocks(inode)) {
#ifdef CLEAN_UNUSED_BLOCKS
        ext2fs_block_iterate(e2fs, ino, BLOCK_FLAG_DEPTH_TRAVERSE, scratchbuf, release_blocks_proc, NULL);
#else
        ext2fs_block_iterate(e2fs, ino, 0, scratchBuf, release_blocks_proc, NULL);
#endif
    }

    ext2fs_inode_alloc_stats2(e2fs, ino, -1, LINUX_S_ISDIR(inode->i_mode));

    return 0;
}

static int release_blocks_proc (ext2_filsys fs, blk_t* blockNr, int blockCnt, void *private)
{
    blk_t block = *blockNr;
    ext2fs_block_alloc_stats(fs, block, -1);

#ifdef CLEAN_UNUSED_BLOCKS
    *blocknr = 0;
    return BLOCK_CHANGED;
#else
    return 0;
#endif
}
