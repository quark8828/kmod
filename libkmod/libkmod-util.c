/*
 * libkmod - interface to kernel module operations
 *
 * Copyright (C) 2011  ProFUSION embedded systems
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation version 2.1.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include "libkmod.h"
#include "libkmod-private.h"

/*
 * Read one logical line from a configuration file.
 *
 * Line endings may be escaped with backslashes, to form one logical line from
 * several physical lines.  No end of line character(s) are included in the
 * result.
 *
 * If linenum is not NULL, it is incremented by the number of physical lines
 * which have been read.
 */
char *getline_wrapped(FILE *fp, unsigned int *linenum)
{
	int size = 256;
	int i = 0;
	char *buf = malloc(size);
	for(;;) {
		int ch = getc_unlocked(fp);

		switch(ch) {
		case EOF:
			if (i == 0) {
				free(buf);
				return NULL;
			}
			/* else fall through */

		case '\n':
			if (linenum)
				(*linenum)++;
			if (i == size)
				buf = realloc(buf, size + 1);
			buf[i] = '\0';
			return buf;

		case '\\':
			ch = getc_unlocked(fp);

			if (ch == '\n') {
				if (linenum)
					(*linenum)++;
				continue;
			}
			/* else fall through */

		default:
			buf[i++] = ch;

			if (i == size) {
				size *= 2;
				buf = realloc(buf, size);
			}
		}
	}
}

/*
 * Replace dashes with underscores.
 * Dashes inside character range patterns (e.g. [0-9]) are left unchanged.
 */
char *underscores(struct kmod_ctx *ctx, char *s)
{
	unsigned int i;

	if (!s)
		return NULL;

	for (i = 0; s[i]; i++) {
		switch (s[i]) {
		case '-':
			s[i] = '_';
			break;

		case ']':
			INFO(ctx, "Unmatched bracket in %s\n", s);
			break;

		case '[':
			i += strcspn(&s[i], "]");
			if (!s[i])
				INFO(ctx, "Unmatched bracket in %s\n", s);
			break;
		}
	}
	return s;
}

bool startswith(const char *s, const char *prefix) {
        size_t sl, pl;

        assert(s);
        assert(prefix);

        sl = strlen(s);
        pl = strlen(prefix);

        if (pl == 0)
                return true;

        if (sl < pl)
                return false;

        return memcmp(s, prefix, pl) == 0;
}
