/*
 * cleaner_exec.c - old cleaner control routines
 *
 * Licensed under LGPLv2: the complete text of the GNU Lesser General
 * Public License can be found in COPYING file of the nilfs-utils
 * package.
 *
 * Copyright (C) 2007-2011 Nippon Telegraph and Telephone Corporation.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif	/* HAVE_CONFIG_H */

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif	/* HAVE_SYS_TYPES_H */

#include <stdio.h>

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif	/* HAVE_STDLIB_H */

#if HAVE_UNISTD_H
#include <unistd.h>
#endif	/* HAVE_UNISTD_H */

#if HAVE_FCNTL_H
#include <fcntl.h>
#endif	/* HAVE_FCNTL_H */

#if HAVE_STRING_H
#include <string.h>
#endif	/* HAVE_STRING_H */

#if HAVE_LIMITS_H
#include <limits.h>	/* ULONG_MAX */
#endif	/* HAVE_LIMITS_H */

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif	/* HAVE_SYS_STAT_H */

#if HAVE_TIME_H
#include <time.h>
#endif	/* HAVE_TIME_H */

#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif	/* HAVE_SYS_WAIT_H */

#if HAVE_SYSLOG_H
#include <syslog.h>
#endif	/* HAVE_SYSLOG_H */

#include <signal.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>

#include "cleaner_exec.h"
#include "nls.h"

#define CLEANERD_WAIT_RETRY_COUNT	3
#define CLEANERD_WAIT_RETRY_INTERVAL	2  /* in seconds */

static const char cleanerd[] = "/sbin/" NILFS_CLEANERD_NAME;
static const char cleanerd_nofork_opt[] = "-n";
static const char cleanerd_protperiod_opt[] = "-p";

static void default_logger(int priority, const char *fmt, ...)
{
	va_list args;

	if (priority >= LOG_INFO)
		return;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	fputs(_("\n"), stderr);
	va_end(args);
}

static void default_printf(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}

static void default_flush(void)
{
	fflush(stdout);
}

void (*nilfs_cleaner_logger)(int priority, const char *fmt, ...)
	= default_logger;
void (*nilfs_cleaner_printf)(const char *fmt, ...) = default_printf;
void (*nilfs_cleaner_flush)(void) = default_flush;


static inline int process_is_alive(pid_t pid)
{
	return (kill(pid, 0) == 0);
}

int nilfs_launch_cleanerd(const char *device, const char *mntdir,
			  unsigned long protperiod, pid_t *ppid)
{
	const char *dargs[7];
	struct stat statbuf;
	sigset_t sigs;
	int i = 0;
	int ret;
	char buf[256];

	if (stat(cleanerd, &statbuf) != 0) {
		nilfs_cleaner_logger(LOG_ERR,  _("Error: %s not found"),
				     NILFS_CLEANERD_NAME);
		return -1;
	}

	ret = fork();
	if (ret == 0) {
		if (setgid(getgid()) < 0) {
			nilfs_cleaner_logger(
				LOG_ERR,
				_("Error: failed to drop setgid privileges"));
			exit(1);
		}
		if (setuid(getuid()) < 0) {
			nilfs_cleaner_logger(
				LOG_ERR,
				_("Error: failed to drop setuid privileges"));
			exit(1);
		}
		dargs[i++] = cleanerd;
		dargs[i++] = cleanerd_nofork_opt;
		if (protperiod != ULONG_MAX) {
			dargs[i++] = cleanerd_protperiod_opt;
			snprintf(buf, sizeof(buf), "%lu", protperiod);
			dargs[i++] = buf;
		}
		dargs[i++] = device;
		dargs[i++] = mntdir;
		dargs[i] = NULL;

		sigfillset(&sigs);
		sigdelset(&sigs, SIGTRAP);
		sigdelset(&sigs, SIGSEGV);
		sigprocmask(SIG_UNBLOCK, &sigs, NULL);

		execv(cleanerd, (char **)dargs);
		exit(1);   /* reach only if failed */
	} else if (ret != -1) {
		*ppid = ret;
		return 0; /* cleanerd started */
	} else {
		int errsv = errno;
		nilfs_cleaner_logger(LOG_ERR,
				     _("Error: could not fork: %s"),
				     strerror(errsv));
	}
	return -1;
}

int nilfs_ping_cleanerd(pid_t pid)
{
	return process_is_alive(pid);
}

static int nilfs_wait_cleanerd(const char *device, pid_t pid)
{
	int cnt = CLEANERD_WAIT_RETRY_COUNT;
	int ret;

	sleep(0);
	if (!process_is_alive(pid))
		return 0;
	sleep(1);
	if (!process_is_alive(pid))
		return 0;

	nilfs_cleaner_printf(_("cleanerd (pid=%ld) still exists on %d. "
			       "waiting."),
			     (long)pid, device);
	nilfs_cleaner_flush();

	for (;;) {
		if (cnt-- < 0) {
			nilfs_cleaner_printf(_("failed\n"));
			nilfs_cleaner_flush();
			ret = -1; /* wait failed */
			break;
		}
		sleep(CLEANERD_WAIT_RETRY_INTERVAL);
		if (!process_is_alive(pid)) {
			nilfs_cleaner_printf(_("done\n"));
			nilfs_cleaner_flush();
			ret = 0;
			break;
		}
		nilfs_cleaner_printf(_("."));
		nilfs_cleaner_flush();
	}
	return ret;
}

int nilfs_shutdown_cleanerd(const char *device, pid_t pid)
{
	int ret;

	nilfs_cleaner_logger(LOG_INFO, _("kill cleanerd (pid=%ld) on %s"),
			     (long)pid, device);

	if (kill(pid, SIGTERM) < 0) {
		int errsv = errno;

		ret = 0;
		if (errsv == ESRCH) {
			goto out;
		} else {
			nilfs_cleaner_logger(
				LOG_ERR, _("Error: cannot kill cleanerd: %s"),
				strerror(errsv));
			ret = -1;
			goto out;
		}
	}
	ret = nilfs_wait_cleanerd(device, pid);
	if (ret < 0)
		nilfs_cleaner_logger(LOG_INFO, _("wait timeout"));
	else
		nilfs_cleaner_logger(LOG_INFO,
				     _("cleanerd (pid=%ld) stopped"),
				     pid);
out:
	return ret;
}
