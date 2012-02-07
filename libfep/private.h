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
#include <stdint.h>
#include <unistd.h>
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
void    _fep_string_copy   (FepString  *dst,
                            FepString  *src);
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

/* list.c */
struct _FepList
{
  struct _FepList *prev, *next;
  void *data;
};
typedef struct _FepList FepList;

FepList *_fep_list_append (FepList *head, void *data);

/* control.c */
typedef enum
  {
    /* client to server */
    FEP_CONTROL_SET_CURSOR_TEXT = 1,
    FEP_CONTROL_SET_STATUS_TEXT = 2,
    FEP_CONTROL_SEND_DATA = 3,
    /* server to client */
    FEP_CONTROL_KEY_EVENT = 4,
    FEP_CONTROL_RESIZE_EVENT = 5,
    FEP_CONTROL_SET_OPTION = 6,
    /* response from client */
    FEP_CONTROL_RESPONSE = 7
  } FepControlCommand;

struct _FepControlMessage
{
  FepControlCommand command;
  FepString *args;
  size_t n_args;
};
typedef struct _FepControlMessage FepControlMessage;

int      _fep_read_control_message             (int                fd,
                                                FepControlMessage *message);
int      _fep_write_control_message            (int                fd,
                                                FepControlMessage *message);
int      _fep_transceive_control_message       (int                fd,
                                                FepControlMessage *request,
                                                FepControlMessage *response);
FepList *_fep_append_control_message           (FepList           *head,
                                                FepControlMessage *message);
void     _fep_control_message_alloc_args       (FepControlMessage *message,
                                                size_t             n_args);
void     _fep_control_message_free_args        (FepControlMessage *message);
void     _fep_control_message_free             (FepControlMessage *message);
int      _fep_control_message_read_int_arg     (FepControlMessage *message,
                                                off_t              index,
                                                int32_t           *r_val);
int      _fep_control_message_write_int_arg    (FepControlMessage *message,
                                                off_t              index,
                                                int32_t            val);
int      _fep_control_message_write_byte_arg   (FepControlMessage *message,
                                                off_t              index,
                                                uint8_t            val);
int      _fep_control_message_write_string_arg (FepControlMessage *message,
                                                off_t              index,
                                                const char        *str,
                                                size_t             length);

#endif	/* __LIBFEP_PRIVATE_H__ */
