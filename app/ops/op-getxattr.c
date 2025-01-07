//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"

#if !defined(ENOATTR)
#define ENOATTR ENODATA
#endif


static int parse_name(const char *name, int *nameIndex, char **attrName);
static int do_getxattr(ext2_filsys e2fs, struct ext2_inode *node, const char *name, char *value, size_t size);


cint32 extfs_op_getxattr(const char *path, const char *name, char *value, cuint64 size)
{
    int rt = 0;
    ext2_ino_t ino = 0;
    struct ext2_inode inode;
    ext2_filsys e2fs = current_ext2fs();

    rt = extfs_do_check(path);
    if (rt != 0) {
        return rt;
    }

    rt = extfs_do_read_inode(e2fs, path, &ino, &inode);
    if (rt) {
        return rt;
    }

    rt = do_getxattr(e2fs, &inode, name, value, size);
    if (rt < 0) {
        return rt;
    }

    return rt;
}


static int parse_name(const char *name, int *nameIndex, char **attrName)
{
    char namespace[16];
    char* attrNameStr = NULL;

    memcpy(namespace, name, sizeof namespace);

    attrNameStr = strchr(namespace, '.');
    if (!attrName) {
        return -ENOTSUP;
    }
    else {
        *attrNameStr = 0;
        *attrName = ++attrNameStr;
    }

    if (!strcmp(namespace, "user")) {
        *nameIndex = 1;
        return 0;
    }

    return -ENOTSUP;
}

static int do_getxattr(ext2_filsys e2fs, struct ext2_inode* node, const char* name, char* value, cuint64 size)
{
    int res = 0;
    int nameIndex = 0;
    char *buf = NULL, *attrStart = NULL;
    struct ext2_ext_attr_entry *entry = NULL;
    char *entryName = NULL, *valueName = NULL;

    res = parse_name(name, &nameIndex, &valueName);
    if (res < 0) {
        return res;
    }

    buf = malloc(e2fs->blocksize);
    if (!buf) {
        return -ENOMEM;
    }
    ext2fs_read_ext_attr(e2fs, node->i_file_acl, buf);

    attrStart = buf + sizeof(struct ext2_ext_attr_header);
    entry = (struct ext2_ext_attr_entry *) attrStart;
    res = -ENOATTR;

    while (!EXT2_EXT_IS_LAST_ENTRY(entry)) {
        entryName = (char*)entry + sizeof(struct ext2_ext_attr_entry);
        if (nameIndex == entry->e_name_index && entry->e_name_len == strlen(valueName)) {
            if (!strncmp(entryName, valueName, entry->e_name_len)) {
                if (size > 0) {
                    memcpy(value, buf + entry->e_value_offs, entry->e_value_size);
                }
                res = entry->e_value_size;
                break;
            }
        }
        entry = EXT2_EXT_ATTR_NEXT(entry);
    }

    free(buf);

    return res;
}
