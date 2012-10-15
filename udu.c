/*
 * $Id: udu.c,v 1.9 2012/10/15 21:27:16 urs Exp $
 *
 * Show disk usage and internal fragmentation needed by directories
 * with a given file system block size.
 */

#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ftw.h>
#include <sys/stat.h>
#include <sys/types.h>

static void usage(const char *name)
{
    fprintf(stderr, "Usage: %s [-x] [-b bsize] [dir ...]\n", name);
}

typedef long long int size;

static void print_head(void);
static void print(long count, size total_size, size frag);
static int  do_count(const char *file, const struct stat *st, int type,
		     struct FTW *ftw);

static int blocksize  = 1024;
static int one_fs     = 0;

static long count      = 0;
static size total_size = 0;
static size frag       = 0;

static long total_count      = 0;
static size total_total_size = 0;
static size total_frag       = 0;

int main(int argc, char **argv)
{
    int errflg = 0;
    int opt, nfds, flags;

    while ((opt = getopt(argc, argv, "b:x")) != -1) {
	switch (opt) {
	case 'b':
	    if ((blocksize = atoi(optarg)) <= 0) {
		fprintf(stderr, "Invalid block size %d\n", blocksize);
		errflg = 1;
	    }
	    break;
	case 'x':
	    one_fs = 1;
	    break;
	case '?':
	    errflg = 1;
	    break;
	}
    }

    if (errflg) {
	usage(argv[0]);
	exit(1);
    }

    nfds  = 1024;
    flags = FTW_PHYS | (one_fs ? FTW_MOUNT : 0);

    print_head();
    if (optind < argc) {
	int i;
	for (i = optind; i < argc; i++) {
	    nftw(argv[i], do_count, nfds, flags);
	    print(count, total_size, frag);
	    total_count      += count;
	    total_total_size += total_size;
	    total_frag       += frag;
	    count = total_size = frag = 0;
	}
	if (argc - optind >= 2)
	    print(total_count, total_total_size, total_frag);
    } else {
	nftw(".", do_count, nfds, flags);
	print(count, total_size, frag);
    }

    return 0;
}

static void print_head(void)
{
    printf(" count  total size  total frag  size/file  frag/file  %%frag\n");
}

static void print(long count, size total_size, size frag)
{
    printf("%6ld  %10lld  %10lld  %9.1f  %9.1f  %5.2f%%\n",
	count, total_size, frag,
	(double)total_size / count, (double)frag / count,
	(100.0 * frag) / (total_size + frag));
}

static int do_count(const char *file, const struct stat *st, int type,
		    struct FTW *ftw)
{
    if (type == FTW_F && S_ISREG(st->st_mode)) {
	count++;
	total_size += st->st_size;
	frag += (blocksize - st->st_size % blocksize) % blocksize;
    }
    return 0;
}
