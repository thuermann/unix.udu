/*
 * $Id: udu.c,v 1.2 2002/03/24 19:11:42 urs Exp $
 *
 * Show disk usage and internal fragmentation needed by directories
 * with a given file system block size.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

void usage(char *name)
{
    fprintf(stderr, "Usage: %s [-x] [-b bsize] [dir ...]\n", name);
}

typedef long long int size;

int do_dir(char *name);

int blocksize  = 1024;
int one_fs     = 0;

long count      = 0;
size total_size = 0;
size frag       = 0;

int main(int argc, char **argv)
{
    int opt;

    while ((opt = getopt(argc, argv, "b:x")) != -1) {
	switch (opt) {
	case 'b':
	    blocksize = atoi(optarg);
	    break;
	case 'x':
	    one_fs = 1;
	    break;
	case '?':
	    usage(argv[0]);
	    exit(1);
	}
    }

    if (optind < argc) {
	int i;
	for (i = optind; i < argc; i++)
	    do_dir(argv[i]);
    } else {
	do_dir(".");
    }

    printf("%ld %lld %lld %.1f %.1f %.2f%%\n",
	   count, total_size, frag,
	   (double)total_size / count, (double)frag / count,
	   (100.0 * frag) / (total_size + frag));

    return 0;
}

int do_dir(char *name)
{
    DIR *dir;
    struct dirent *ent;
    struct stat st;
    dev_t dev;

    if (!(dir = opendir(name)))
	return -1;
    if (chdir(name) < 0)
	return -1;
    if (stat(".", &st) < 0)
	return -1;
    dev = st.st_dev;

    while (ent = readdir(dir)) {
	if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
	    continue;

	if (lstat(ent->d_name, &st) < 0)
	    continue;
	if (S_ISREG(st.st_mode)) {
	    count++;
	    total_size += st.st_size;
	    frag += blocksize - st.st_size % blocksize;
	} else if (S_ISDIR(st.st_mode)) {
	    if (st.st_dev == dev || !one_fs)
		do_dir(ent->d_name);
	}
    }
    chdir("..");
    closedir(dir);

    return 0;
}
