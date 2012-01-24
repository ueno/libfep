/*
 * Copyright (C) 2012 Daiki Ueno <ueno@unixuser.org>
 * Copyright (C) 2012 Red Hat, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __LIBFEP_PRIVATE_H__
#define __LIBFEP_PRIVATE_H__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <libfep/libfep.h>

#ifndef BUFSIZ
#define BUFSIZ 4096
#endif

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#define SIZEOF(array) (sizeof((array)) / sizeof((array)[0]))

/* string.c */
struct _FepString {
  char *str;
  size_t len;
  size_t cap;
};
typedef struct _FepString FepString;

void    _fep_string_append (FepString  *buf,
                            const char *str,
                            size_t      count);
void    _fep_string_clear  (FepString  *buf);
char ** _fep_strsplit      (const char *str,
                            const char *delimiter,
                            int         max_tokens);
char ** _fep_strsplit_set  (const char *str,
                            const char *delimiter,
                            int         max_tokens);
char *  _fep_strjoinv      (char      **strv,
                            const char *delimiter);
void    _fep_strfreev      (char      **strv);
int     _fep_strwidth      (const char *str);
char *  _fep_strtrunc      (const char *str,
                            int         width);

/* control.c */
typedef enum {
  FEP_CONTROL_SET_STATUS = 1,
  FEP_CONTROL_SET_CURSOR_TEXT = 2,
  FEP_CONTROL_SET_STATUS_TEXT = 3,
  FEP_CONTROL_KEY_EVENT = 4,
  FEP_CONTROL_KEY_EVENT_RESPONSE = 5,
} FepControlCommand;

struct _FepControlMessage
{
  FepControlCommand command;
  char **args;
};
typedef struct _FepControlMessage FepControlMessage;

int
_fep_read_control_message (int fd,
			   FepControlMessage *message);
int
_fep_write_control_message (int fd,
			    FepControlMessage *message);

#endif	/* __LIBFEP_PRIVATE_H__ */
