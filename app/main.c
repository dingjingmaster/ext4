/*************************************************************************
> FileName: main.c
> Author  : DingJing
> Mail    : dingjing@live.cn
> Created Time: Tue Jan  7 10:10:41 2025
 ************************************************************************/
#include "defines.h"

#include <getopt.h>


static void     usage                   (void);
static int      strappend               (char** dest, const char* append);
static char*    parse_mount_options     (const char* origOpts, ExtFsData* opts);
static int      parse_options           (int argc, char* argv[], ExtFsData* opts);


#if __FreeBSD__ == 10
static char gsDefOpts[]   = "allow_other,default_permissions,local,";
static char gsDefOptsRd[] = "noappledouble,";
#else
static char gsDefOpts[]   = "allow_other,default_permissions,";
static char gsDefOptsRd[] = "";
#endif

static const char* gsUsageMsg =
"\n"
"%s %s %d - FUSE extX FS Driver\n"
"Copyright (C) 2024 DingJing <dingjing@live.cn>\n"
"\n"
"Usage:     %s <device|image_file> <mount_point> [-o option[,...]]\n"
"\n"
"Example: ./ext4 /dev/sda1 /mnt\n"
"%s\n"
"";

static const struct fuse_operations gsExtfsOps = {
    .getattr                = extfs_op_getattr,
    .readlink               = extfs_op_read_link,
    .mknod                  = extfs_op_mknod,
    .mkdir                  = extfs_op_mkdir,
    .unlink                 = extfs_op_unlink,
    .rmdir                  = extfs_op_rmdir,
    .symlink                = extfs_op_symlink,
    .rename                 = extfs_op_rename,
    .link                   = extfs_op_link,
    .chmod                  = extfs_op_chmod,
    .truncate               = extfs_op_truncate,
    .open                   = extfs_op_open,
    .read                   = extfs_op_read,
    .write                  = extfs_op_write,
    .statfs                 = extfs_op_statfs,
    .flush                  = extfs_op_flush,
    .release                = extfs_op_release,
    .fsync                  = extfs_op_fsync,
    .setxattr               = NULL,
    .getxattr               = extfs_op_getxattr,
    .listxattr              = NULL,
    .removexattr            = NULL,
    .opendir                = extfs_op_open,
    .readdir                = extfs_op_read_dir,
    .releasedir             = extfs_op_release,
    .fsyncdir               = extfs_op_fsync,
    .init                   = extfs_op_init,
    .destroy                = extfs_op_destroy,
    .access                 = extfs_op_access,
    .create                 = extfs_op_create,
    .ftruncate              = extfs_op_ftruncate,
    .fgetattr               = extfs_op_fgetattr,
    .lock                   = NULL,
    .utimens                = extfs_op_utimens,
    .bmap                   = NULL,
#if ((FUSE_VERSION) == 29)
    .flag_utime_omit_ok     = 1,
#endif
};

int main(int argc, char* argv[])
{
    int err = 0;
    struct stat sbuf;
    char* parsedOpts = NULL;
    struct fuse_args args = FUSE_ARGS_INIT(0, NULL);
    ExtFsData opts = {0};

    memset(&opts, 0, sizeof(opts));
    if (parse_options(argc, argv, &opts)) {
        usage();
        return -1;
    }

    if (stat((char*)opts.device, &sbuf)) {
        printf("Failed to access '%s'\n", (char*) opts.device);
        err = -3;
        goto errOut;
    }

    if (extfs_do_probe(&opts) != 0) {
        printf("Probe failed\n");
        err = -4;
        goto errOut;
    }

    parsedOpts = parse_mount_options(opts.options ? opts.options : "", &opts);
    if (!parsedOpts) {
        err = -2;
        goto errOut;
    }

    printf("opts.device: %s\n", (char*) opts.device);
    printf("opts.mntPoint: %s\n", (char*) opts.mntPoint);
    printf("opts.volname: %s\n", (opts.volName != NULL) ? opts.volName : "");
    printf("opts.options: %s\n", (char*) opts.options);
    printf("parsed_options: %s\n", (char*) parsedOpts);

    if (fuse_opt_add_arg(&args, "ext4") == -1
        || fuse_opt_add_arg(&args, "-s") == -1
        || fuse_opt_add_arg(&args, "-o") == -1
        || fuse_opt_add_arg(&args, parsedOpts) == -1
        || fuse_opt_add_arg(&args, (char*) opts.mntPoint) == -1) {
        printf("Failed to set FUSE options\n");
        fuse_opt_free_args(&args);
        err = -5;
        goto errOut;
    }

    fuse_main(args.argc, args.argv, &gsExtfsOps, &opts);

errOut:
    fuse_opt_free_args(&args);
    C_FREE(parsedOpts);
    C_FREE(opts.options);
    C_FREE(opts.device);
    C_FREE(opts.volName);

    return 0;
}

static void usage (void)
{
    printf(gsUsageMsg, "extfs", EXT_VERSION, fuse_version(), "extfs", "");
}

static int strappend (char** dest, const char* append)
{
    char* p = NULL;
    cint64 size = 0;

    if (!dest) {
        return -1;
    }

    if (!append) {
        return 0;
    }

    size = strlen (append) + 1;
    if (*dest) {
        size += strlen(*dest);
    }

    p = realloc(*dest, size);
    if (!p) {
        return -1;
    }

    if (*dest) {
        strcat(p, append);
    }
    else {
        strcpy(p, append);
    }
    *dest = p;

    return 0;
}

static int parse_options (int argc, char* argv[], ExtFsData* opts)
{
    int c = 0;
    static const char* sOpt = "o:hv";
    static const struct option lOpt[] = {
        {"options",     1, NULL, 'o'},
        {"help",        0, NULL, 'h'},
        {"verbose",     0, NULL, 'v'},
        {NULL,          0, NULL, 0}
    };

    opterr = 0;

    while ((c = getopt_long(argc, argv, sOpt, lOpt, NULL)) != -1) {
        switch (c) {
            case 'o': {
                if (opts->options) {
                    if (strappend(&opts->options, ",")) {
                        return -1;
                    }
                }
                if (strappend(&opts->options, optarg)) {
                    return -1;
                }
                break;
            }
            case 'h': {
                usage();
                exit(0);
            }
            case 'v': {
                opts->debug = true;
                break;
            }
            default: {
                printf("Unknown option '%s'\n", argv[optind - 1]);
                return -1;
            }
        }
    }

    if (optind < argc) {
        optarg = argv[optind++];
        if (optarg[0] != '/') {
            char fullDevice[PATH_MAX + 1] = {0};
            if (!realpath(optarg, fullDevice)) {
                printf("Cannot mount %s\n", optarg);
                C_FREE(opts->device);
                return -1;
            }
            else {
                opts->device = strdup(fullDevice);
            }
        }
        else {
            opts->device = strdup(optarg);
        }
    }

    if (optind < argc) {
        opts->mntPoint = (cuchar*) argv[optind++];
    }

    if (optind < argc) {
        printf("You must specify exactly one device and exactly one mount point\n");
        return -1;
    }

    if (!opts->device) {
        printf("No device specified\n");
        return -1;
    }

    if (!opts->mntPoint) {
        printf("No mount point specified\n");
        return -1;
    }

    return 0;
}

static char* parse_mount_options (const char* origOpts, ExtFsData* opts)
{
    char* options = NULL, *s = NULL, *opt = NULL, *val = NULL, *ret = NULL;

    ret = malloc(strlen(gsDefOpts) + strlen(gsDefOptsRd) + strlen(origOpts) + 256 + PATH_MAX);
    if (!ret) {
        return NULL;
    }

    *ret = 0;
    options = strdup(origOpts);
    if (!options) {
        printf("strdup failed\n");
        return NULL;
    }

    s = options;
    while (s && *s && (val = strsep(&s, ","))) {
        opt = strsep(&val, "=");
        if (!strcmp(opt, "ro")) {
            if (val) {
                printf("'ro' option should not have value\n");
                goto errExit;
            }
            opts->readonly = true;
            strcat(ret, "ro,");
        }
        else if (!strcmp(opt, "rw")) {
            if (val) {
                printf("'rw' option should not have value\n");
                goto errExit;
            }
            opts->readonly = false;
            strcat(ret, "rw,");
        }
        else if (!strcmp(opt, "rw+")) {
            if (val) {
                printf("'rw+' option should not have value\n");
                goto errExit;
            }
            opts->readonly = false;
            opts->force = 1;
            strcat(ret, "rw,");
        }
        else if (!strcmp(opt, "debug")) {
            if (val) {
                printf("'debug' option should not have value\n");
                goto errExit;
            }
            opts->debug = 1;
            strcat(ret, "debug,");
        }
        else if (!strcmp(opt, "silent")) {
            if (val) {
                printf("'silent' option should not have value\n");
                goto errExit;
            }
            opts->silent = 1;
        }
        else if (!strcmp(opt, "force")) {
            if (val) {
                printf("'force' option should not have value\n");
                goto errExit;
            }
            opts->force = 1;
#if __FreeBSD__ == 10
            strcat(ret, "force,");
#endif
        }
        else {
            strcat(ret, opt);
            if (val) {
                strcat(ret, "=");
                strcat(ret, val);
            }
            strcat(ret, ",");
        }
    }

    if (!opts->readonly && !opts->force) {
        fprintf(stderr, "Mounting %s Read-Only.\nUse 'force' or 'rw+' options to enable Read-Write mode\n", (const char*) opts->device);
        opts->readonly = true;
    }

    strcat(ret, gsDefOpts);
    if (opts->readonly) {
        strcat(ret, gsDefOptsRd);
        strcat(ret, "ro,");
    }

    strcat(ret, "fsname=");
    strcat(ret, (const char*) opts->device);

#if __FreeBSD__ == 10
    strcat(ret, ",fstypename=");
    strcat(ret, "ext2");
    strcat(ret, ",volname=");
    if (opts->volName == NULL || opts->volName[0] == '\0') {
        s = strrchr((const char*)opts->device, '/');
        if (s != NULL) {
            strcat(ret, s + 1);
        }
        else {
            strcat(ret, (char*) opts->device);
        }
    }
    else {
        strcat(ret, (char*) opts->volName);
    }
#endif

exit:
    free (options);
    return ret;
errExit:
    free(ret);
    ret = NULL;
    goto exit;
}
