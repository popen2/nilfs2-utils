/*
 * code borrowed from util-linux-2.12r/mount/mntent.h
 *
 * modified by Ryusuke Konishi <ryusuke@osrg.net>
 */
#ifndef MY_MNTENT_H
#define MY_MNTENT_H

/* General filesystem types.  */
#ifndef MNTTYPE_IGNORE
#define MNTTYPE_IGNORE  "ignore"        /* Ignore this entry.  */
#define MNTTYPE_NFS     "nfs"           /* Network file system.  */
#define MNTTYPE_SWAP    "swap"          /* Swap device.  */
#endif

struct my_mntent {
	const char *mnt_fsname;
	const char *mnt_dir;
	const char *mnt_type;
	const char *mnt_opts;
	int mnt_freq;
	int mnt_passno;
};

#define ERR_MAX 5

typedef struct mntFILEstruct {
	FILE *mntent_fp;
	char *mntent_file;
	int mntent_lineno;
	int mntent_errs;
	int mntent_softerrs;
} mntFILE;

mntFILE *my_setmntent (const char *file, char *mode);
void my_endmntent (mntFILE *mfp);
int my_addmntent (mntFILE *mfp, struct my_mntent *mnt);
struct my_mntent *my_getmntent (mntFILE *mfp);

#endif
