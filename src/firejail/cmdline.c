/*
 * Copyright (C) 2014-2016 Firejail Authors
 *
 * This file is part of firejail project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "firejail.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <linux/limits.h>
#include <assert.h>
#include <errno.h>

int cmdline_length(int argc, char **argv, int index) {
	unsigned i,j;
	int len = 0;
	unsigned argcnt = argc - index;
	bool in_quotes = false;

	for (i = 0; i < argcnt; i++) {
		in_quotes = false;
		for (j = 0; j < strlen(argv[i + index]); j++) {
			if (argv[i + index][j] == '\'') {
				if (in_quotes)
					len++;
				if (j > 0 && argv[i + index][j-1] == '\'')
					len++;
				else
					len += 3;
				in_quotes = false;
			} else {
				if (!in_quotes)
					len++;
				len++;
				in_quotes = true;
			}
		}
		if (in_quotes) {
			len++;
		}
		if (strlen(argv[i + index]) == 0) {
			len += 2;
		}
		len++;
	}

	return len;
}

void quote_cmdline(char *command_line, char *window_title, int len, int argc, char **argv, int index) {
	unsigned i,j;
	unsigned argcnt = argc - index;
	bool in_quotes = false;
	char *ptr1 = command_line;
	char *ptr2 = window_title;

	for (i = 0; i < argcnt; i++) {

		// enclose args by single quotes,
		// and since single quote can't be represented in single quoted text
		// each occurence of it should be enclosed by double quotes
		in_quotes = false;
		for (j = 0; j < strlen(argv[i + index]); j++) {
			// single quote
			if (argv[i + index][j] == '\'') {
				if (in_quotes) {
					// close quotes
					ptr1[0] = '\'';
					ptr1++;
				}
				// previous char was single quote too
				if (j > 0 && argv[i + index][j-1] == '\'') {
					ptr1--;
					sprintf(ptr1, "\'\"");
				} 
				// this first in series
				else
				{
					sprintf(ptr1, "\"\'\"");
				}
				ptr1 += strlen(ptr1);
				in_quotes = false;
			}
			// anything other
			else
			{
				if (!in_quotes) {
					// open quotes
					ptr1[0] = '\'';
					ptr1++;
				}
				ptr1[0] = argv[i + index][j];
				ptr1++;
				in_quotes = true;
			}
		}
		// close quotes
		if (in_quotes) {
			ptr1[0] = '\'';
			ptr1++;
		}
		// handle empty argument case
		if (strlen(argv[i + index]) == 0) {
			sprintf(ptr1, "\'\'");
			ptr1 += strlen(ptr1);
		}
		// add space
		sprintf(ptr1, " ");
		ptr1 += strlen(ptr1);

		sprintf(ptr2, "%s ", argv[i + index]);
		ptr2 += strlen(ptr2);
	}

	assert((unsigned) len == strlen(command_line));
}

void build_cmdline(char **command_line, char **window_title, int argc, char **argv, int index) {
	int len = cmdline_length(argc, argv, index);
	if (len > ARG_MAX) {
		errno = E2BIG;
		errExit("cmdline_length");
	}

	*command_line = malloc(len + 1);
	if (!*command_line)
			errExit("malloc");
	*window_title = malloc(len + 1);
	if (!*window_title)
			errExit("malloc");
	
	quote_cmdline(*command_line, *window_title, len, argc, argv, index);

	assert(*command_line);
	assert(*window_title);
}
