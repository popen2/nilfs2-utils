## Makefile.am

sbin_PROGRAMS = nilfs_cleanerd
LDADD = $(top_builddir)/lib/libnilfs.la $(top_builddir)/lib/libnilfsgc.la \
	$(LIB_POSIX_MQ) -luuid

nilfs_cleanerd_SOURCES = cleanerd.c cldconfig.c \
	cldconfig.h
nilfs_cleanerd_CFLAGS = -Wall
nilfs_cleanerd_CPPFLAGS = -I$(top_srcdir)/include \
	-DSYSCONFDIR=\"$(sysconfdir)\"

dist_sysconf_DATA = nilfs_cleanerd.conf
