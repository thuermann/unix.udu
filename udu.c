/*
 * $Id: udu.c,v 1.6 2004/04/20 21:28:30 urs Exp $
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

void print_head(void);
void print(long count, size total_size, size frag);
void walk(struct stat *parent, char *name, void (*func)(struct stat *st));
void do_count(struct stat *st);

int blocksize  = 1024;
int one_fs     = 0;

long count      = 0;
size total_size = 0;
size frag       = 0;

long total_count      = 0;
size total_total_size = 0;
size total_frag       = 0;

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

    print_head();
    if (optind < argc) {
	int i;
	for (i = optind; i < argc; i++) {
	    walk(NULL, argv[i], do_count);
	    print(count, total_size, frag);
	    total_count      += count;
	    total_total_size += total_size;
	    total_frag       += frag;
	    count = total_size = frag = 0;
	}
	if (argc - optind >= 2)
	    print(total_count, total_total_size, total_frag);
    } else {
	walk(NULL, ".", do_count);
	print(count, total_size, frag);
    }

    return 0;
}

void print_head(void)
{
    printf(" count  total size  total frag  size/file  frag/file  %%frag\n");
}

void print(long count, size total_size, size frag)
{
    printf("%6ld  %10lld  %10lld  %9.1f  %9.1f  %5.2f%%\n",
	count, total_size, frag,
	(double)total_size / count, (double)frag / count,
	(100.0 * frag) / (total_size + frag));
}

void walk(struct stat *parent, char *name, void (*func)(struct stat *st))
{
    struct stat st;

    if (lstat(name, &st) < 0)
	return;
    else if (S_ISREG(st.st_mode))
	func(&st);
    else if (S_ISDIR(st.st_mode)) {
	DIR *dir;
	struct dirent *ent;

	if (one_fs && parent && parent->st_dev != st.st_dev)
	    return;

	if (chdir(name) < 0)
	    return;
	if (!(dir = opendir(".")))
	    goto leave;

	while (ent = readdir(dir)) {
	    if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
		continue;
	    walk(&st, ent->d_name, func);
	}

	closedir(dir);
    leave:
	chdir("..");
    }
}

void do_count(struct stat *st)
{
    count++;
    total_size += st->st_size;
    frag += (blocksize - st->st_size % blocksize) % blocksize;
}
