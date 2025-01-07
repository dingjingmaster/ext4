//
// Created by dingjing on 1/7/25.
//
/**
 * @note 此文件不包含除 macros.h 外的任何第三方头文件
 */
#ifndef ext4_GLOBAL_H
#define ext4_GLOBAL_H
#include <fuse.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <ext2fs/ext2fs.h>

#include "macros/macros.h"

C_BEGIN_EXTERN_C

#if !defined(FUSE_VERSION) || (FUSE_VERSION < 27)
#error " Compilation requires at least FUSE version 2.6.0! "
#endif

#ifndef PATH_MAX
#define PATH_MAX                            4096
#endif
#define EXT_FLUSH_BITMAPS_TIMEOUT           10
#define EXT_FILE_SHARED_INODE               0x8000
#define EXT_FS_FILE(file)					((void*)(unsigned long) (file))

typedef long								ErrCode;
typedef struct _ExtFsData                   ExtFsData;

struct _ExtFsData
{
    bool                debug;
    bool                silent;
    bool                force;
    bool                readonly;
    cuint64             lastFlush;
    cuchar*             mntPoint;
    cchar*              options;
    cchar*              device;
    cchar*              volName;
    ext2_filsys         e2fs;
};

ErrCode extfs_file_close2(ext2_file_t file, void(*CloseCallback)(struct ext2_inode* inode, int flags));

static inline ext2_filsys current_ext2fs(void)
{
    struct fuse_context* ctx = fuse_get_context();
    ExtFsData* data = ctx->private_data;
    cuint64 now = time(NULL);

    if ((now - data->lastFlush) > EXT_FLUSH_BITMAPS_TIMEOUT) {
        ext2fs_write_bitmaps(data->e2fs);
        data->lastFlush = now;
    }

    return (ext2_filsys) data->e2fs;
}

static inline uid_t extfs_read_uid (struct ext2_inode* inode)
{
    return ((uid_t) inode->osd2.linux2.l_i_uid_high << 16) | inode->i_uid;
}

static inline void extfs_write_uid (struct ext2_inode* inode, uid_t uid)
{
    inode->i_uid = uid & 0xffff;
    inode->osd2.linux2.l_i_uid_high = (uid >> 16) & 0xffff;
}

static inline gid_t extfs_read_gid (struct ext2_inode* inode)
{
    return ((gid_t)inode->osd2.linux2.l_i_gid_high << 16) | inode->i_gid;
}

static inline void extfs_write_gid (struct ext2_inode* inode, gid_t gid)
{
    inode->i_gid = gid & 0xffff;
    inode->osd2.hurd2.h_i_gid_high = (gid >> 16) & 0xffff;
}

void*       extfs_op_init                       (struct fuse_conn_info* conn);
void        extfs_op_destroy                    (void* userData);
cint32      extfs_op_access                     (const char* path, int mask);
cint32      extfs_op_getattr                    (const char* path, struct stat* statBuf);
cint32      extfs_op_fgetattr                   (const char* path, struct stat* statBuf, struct fuse_file_info* fi);
cint32      extfs_op_getxattr                   (const char* path, const char* name, char* value, cuint64 size);
cint32      extfs_op_open                       (const char* path, struct fuse_file_info* fi);
cint32      extfs_op_read                       (const char* path, char* buf, cuint64 size, cint64 offset, struct fuse_file_info* fi);
cint32      extfs_op_read_dir                   (const char* path, void* buf, fuse_fill_dir_t filler, cint64 offset, struct fuse_file_info* fi);
cint32      extfs_op_read_link                  (const char* path, char* buf, cuint64 size);
cint32      extfs_op_release                    (const char* path, struct fuse_file_info* fi);
cint32      extfs_op_statfs                     (const char* path, struct statvfs* buf);
cint32      extfs_op_chmod                      (const char* path, mode_t mode);
cint32      extfs_op_chown                      (const char* path, uid_t uid, gid_t gid);
cint32      extfs_op_create                     (const char* path, mode_t mode, struct fuse_file_info* fi);
cint32      extfs_op_flush                      (const char* path, struct fuse_file_info* fi);
cint32      extfs_op_fsync                      (const char* path, int dataSync, struct fuse_file_info* fi);
cint32      extfs_op_mkdir                      (const char* path, mode_t mode);
cint32      extfs_op_rmdir                      (const char* path);
cint32      extfs_op_unlink                     (const char* path);
cint32      extfs_op_utimens                    (const char* path, const struct timespec tv[2]);
cint32      extfs_op_write                      (const char* path, const char* buf, cuint64 size, cint64 offset, struct fuse_file_info* fi);
cint32      extfs_op_mknod                      (const char* path, mode_t mode, dev_t dev);
cint32      extfs_op_symlink                    (const char* srcName, const char* dstName);
cint32      extfs_op_link                       (const char* srcPath, const char* dstPath);
cint32      extfs_op_rename                     (const char* srcPath, const char* dstPath);
cint32      extfs_op_truncate                   (const char* path, cint64 len);
cint32      extfs_op_ftruncate                  (const char* path, cint64 len, struct fuse_file_info* fi);

cint32      extfs_do_probe                      (ExtFsData* opts);
cint32      extfs_do_label                      (void);
cint32      extfs_do_check                      (const char* path);
cint32      extfs_do_check_split                (const char* path, char** dirName, char** baseName);
void        extfs_do_fill_statbuf               (ext2_filsys e2fs, ext2_ino_t ino, struct ext2_inode* inode, struct stat* st);
cint32      extfs_do_read_inode                 (ext2_filsys e2fs, const char* path, ext2_ino_t* ino, struct ext2_inode* inode);
cint32      extfs_do_write_inode                (ext2_filsys e2fs, ext2_ino_t ino, struct ext2_inode* inode);
cint32      extfs_do_kill_file_by_inode         (ext2_filsys e2fs, ext2_ino_t ino, struct ext2_inode* inode);
ext2_file_t extfs_do_open                       (ext2_filsys e2fs, const char* path, int flags);
cint32      extfs_do_release                    (ext2_file_t file);
cint32      extfs_do_mode_to_ext2_lag           (mode_t mode);
cint32      extfs_do_create                     (ext2_filsys e2fs, const char* path, mode_t mode, dev_t dev, const char* fastSymlink);
cint32      extfs_do_check_empty_dir            (ext2_filsys e2fs, ext2_ino_t ino);
cint64      extfs_do_write                      (ext2_file_t file, const char* buf, cuint64 size, cuint64 offset);

void        extfs_free_split                    (char* dirName, char* baseName);

C_END_EXTERN_C

#endif // ext4_DEFINES_H
