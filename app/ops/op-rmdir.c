//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"

typedef struct _RmdirST RmdirST;

struct _RmdirST
{
    ext2_ino_t parent;
    int empty;
};

static int rmdir_proc (ext2_ino_t dir EXT2FS_ATTR((unused)), int entry EXT2FS_ATTR((unused)), struct ext2_dir_entry* dirent, int offset EXT2FS_ATTR((unused)), int blockSize EXT2FS_ATTR((unused)), char* buf EXT2FS_ATTR((unused)), void* private);


cint32 extfs_do_check_empty_dir (ext2_filsys e2fs, ext2_ino_t ino)
{
    ErrCode rc = 0;
    int empty = 1;

    rc = ext2fs_dir_iterate2(e2fs, ino, 0, 0, rmdir_proc, &empty);
    if (rc) {
        return -EIO;
    }

    if (empty == 0) {
        return -ENOTEMPTY;
    }

    return 0;
}

cint32 extfs_op_rmdir (const char* path)
{
    int rt = 0;
    ErrCode rc = 0;
    char* pPath = NULL;
    char* rPath = NULL;
    ext2_ino_t pIno = 0;
    struct ext2_inode pInode;
    ext2_ino_t rIno = 0;
    struct ext2_inode rInode;

    ext2_filsys e2fs = current_ext2fs();

    rt = extfs_do_check_split(path, &pPath, &rPath);
    if (0 != rt) {
        return rt;
    }

    rt = extfs_do_read_inode(e2fs, pPath, &pIno, &pInode);
    if (rt) {
        extfs_free_split(pPath, rPath);
        return rt;
    }

    rt = extfs_do_read_inode(e2fs, path, &rIno, &rInode);
    if (rt) {
        extfs_free_split(pPath, rPath);
        return rt;
    }

    if (!LINUX_S_ISDIR(rInode.i_mode)) {
        extfs_free_split(pPath, rPath);
        return -ENOTDIR;
    }

    if (rIno == EXT2_ROOT_INO) {
        extfs_free_split(pPath, rPath);
        return -EIO;
    }

    rt = extfs_do_check_empty_dir(e2fs, rIno);
    if (rt) {
        extfs_free_split(pPath, rPath);
        return rt;
    }

    rc = ext2fs_unlink(e2fs, pIno, rPath, rIno, 0);
    if (rc) {
        extfs_free_split(pPath, rPath);
        return -EIO;
    }

    rt = extfs_do_kill_file_by_inode(e2fs, rIno, &rInode);
    if (rt) {
        extfs_free_split(pPath, rPath);
        return rt;
    }

    rt = extfs_do_read_inode(e2fs, pPath, &pIno, &pInode);
    if (rt) {
        extfs_free_split(pPath, rPath);
        return rt;
    }

    if (pInode.i_links_count > 1) {
        pInode.i_links_count--;
    }

    pInode.i_mtime = e2fs->now ? e2fs->now : time(NULL);
    pInode.i_ctime = e2fs->now ? e2fs->now : time(NULL);
    rc = extfs_do_write_inode(e2fs, pIno, &pInode);
    if (rc) {
        extfs_free_split(pPath, rPath);
        return -EIO;
    }

    extfs_free_split(pPath, rPath);

    return 0;
}


static int rmdir_proc (ext2_ino_t dir, int entry, struct ext2_dir_entry* dirent, int offset, int blockSize, char* buf, void* private)
{
    int* pEmpty = (int*) private;

    if (dirent->inode == 0
        || (((dirent->name_len && 0xFF) == 1) && (dirent->name[0] == '.'))
        || (((dirent->name_len && 0xFF) == 2) && (dirent->name[0] == '.') && (dirent->name[1] == '.'))) {
        return 0;
    }

    *pEmpty = 0;

    return 0;
}
