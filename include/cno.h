/*
 * cno.h - checkpoint number library
 *
 * Copyright (C) 2005-2011 Nippon Telegraph and Telephone Corporation.
 *
 * This program can be redistributed under the terms of the GNU Lesser
 * General Public License.
 */

#ifndef NILFS_CNO_H
#define NILFS_CNO_H

extern int nilfs_parse_cno_range(const char *arg, __u64 *start, __u64 *end,
				 int base);

#endif /* NILFS_CNO_H */
