/*
 * cno.c - NILFS checkpoint number parser
 *
 * Copyright (C) 2009-2010 Nippon Telegraph and Telephone Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Written by Ryusuke Konishi <ryusuke@osrg.net>.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif	/* HAVE_CONFIG_H */

#include <stdio.h>

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif	/* HAVE_STDLIB_H */

#if HAVE_STRING_H
#include <string.h>
#endif	/* HAVE_STRING_H */

#include <assert.h>
#include "nilfs.h"

int nilfs_parse_cno_range(const char *arg, __u64 *start, __u64 *end, int base)
{
	const char *delim;
	char *endptr;
	nilfs_cno_t cno, cno2;

	assert(arg && *arg != '\0');

	delim = strstr(arg, "..");
	if (delim && delim == arg) {
		if (arg[2] != '\0') {
			/* ..yyy */
			cno = strtoull(arg + 2, &endptr, base);
			if (*endptr == '\0') {
				/* ..CNO */
				*start = NILFS_CNO_MIN;
				*end = cno;
				return 0;
			}
		}
	} else if (!delim) {
		/* xxx */
		cno = strtoull(arg, &endptr, base);
		if (*endptr == '\0') {
			/* CNO */
			*start = *end = cno;
			return 0;
		}
	} else {
		/* xxx..yyy */
		cno = strtoull(arg, &endptr, base);
		if (endptr == delim) {
			if (delim[2] == '\0') {
				/* CNO.. */
				*start = cno;
				*end = NILFS_CNO_MAX;
				return 0;
			}
			cno2 = strtoull(delim + 2, &endptr, base);
			if (*endptr == '\0') {
				/* CNO..CNO */
				*start = cno;
				*end = cno2;
				return 0;
			}
		}
	}
	return -1; /* parse error */
}
