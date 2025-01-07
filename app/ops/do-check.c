//
// Created by dingjing on 1/7/25.
//
#include "../defines.h"


int extfs_do_check (const char *path)
{
    char *baseNamePath = strrchr(path, '/');
    if (baseNamePath == NULL) {
        return -ENOENT;
    }
    baseNamePath++;
    if (strlen(baseNamePath) > 255) {
        return -ENAMETOOLONG;
    }

    return 0;
}

int extfs_do_check_split (const char *path, char **dirName, char **baseName)
{
    char* tmp = NULL;
    char* cPath = strdup(path);
    tmp = strrchr(cPath, '/');
    if (tmp == NULL) {
        free(cPath);
        return -ENOENT;
    }
    *tmp='\0';
    tmp++;
    if (strlen(tmp) > 255) {
        free(cPath);
        return -ENAMETOOLONG;
    }
    *dirName = cPath;
    *baseName = tmp;

    return 0;
}

void extfs_free_split (char *dirName, char *baseName)
{
    free(dirName);
}
