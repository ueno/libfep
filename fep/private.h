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

#ifndef __FEP_PRIVATE_H__
#define __FEP_PRIVATE_H__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdbool.h>
#include <stdint.h>
#include <curses.h>
#include <term.h>
#include <sys/select.h>
#include <unistd.h>
#include <pty.h>
#include <signal.h>
#include "fep.h"
#include <libfep/private.h>

typedef enum {
  FEP_ATTR_TYPE_ENTER_UNDERLINE,
  FEP_ATTR_TYPE_EXIT_UNDERLINE,
  FEP_ATTR_TYPE_ENTER_STANDOUT,
  FEP_ATTR_TYPE_EXIT_STANDOUT,
  FEP_ATTR_TYPE_ENTER_BOLD,
  FEP_ATTR_TYPE_ENTER_BLINK,
  FEP_ATTR_TYPE_ORIG_PAIR,
  FEP_ATTR_TYPE_ORIG_FORE,
  FEP_ATTR_TYPE_ORIG_BACK,
  FEP_ATTR_TYPE_LAST
} FepAttrType;

typedef enum {
  FEP_READ_KEY_OK,
  FEP_READ_KEY_ERROR,
  FEP_READ_KEY_NOT_ENOUGH
} FepReadKeyResult;

typedef enum {
  FEP_SIG_FLAG_TERM = 1,
  FEP_SIG_FLAG_WINCH = 1 << 2,
  FEP_SIG_FLAG_TSTP = 1 << 5
} FepSigFlag;

struct _FepPoint {
  int row;
  int col;
};
typedef struct _FepPoint FepPoint;

struct _FepAttribute {
  bool underline;
  bool standout;
  bool bold;
  bool blink;
  int foreground;
  int background;
};
typedef struct _FepAttribute FepAttribute;

struct _Fep
{
  /* input/output via tty */
  int tty_in;
  int tty_out;

  /* input/output via pty */
  int pty;

  /* input/output via control socket */
  int server;
  char *control_socket_path;
#define FEP_MAX_CLIENTS 10
  int clients[FEP_MAX_CLIENTS];
  size_t n_clients;

  /* input buffer for tty (used by ungetc) */
  FepString ttybuf;

  /* input buffer for pty (to keep incomplete escape sequences from pty) */
  FepString ptybuf;

  bool has_cpr;

  FepAttribute attr;
  FepAttribute attr_tty;
  FepAttribute attr_pty;
  int attr_codes[FEP_ATTR_TYPE_LAST];

  char *cursor_text;
  FepPoint cursor;
  FepPoint cursor_save;
  FepPoint cursor_diff;
  char *status_text;

  struct winsize winsize;
  struct termios orig_termios;
};

struct _FepCSI
{
  char *params;
  char *intermediate;
  char final;
};

typedef struct _FepCSI FepCSI;

extern const FepAttribute _fep_empty_attr;

/* csi.c */
bool             _fep_csi_scan             (const char         *str,
                                            size_t              len,
                                            const char          final,
                                            const char        **r_csi,
                                            size_t             *r_csi_len);
FepCSI          *_fep_csi_parse            (const char         *str,
                                            size_t              len,
					    char              **endptr);
char *           _fep_csi_format           (FepCSI             *csi);
void             _fep_csi_free             (FepCSI             *csi);

/* sgr.c */
void             _fep_sgr_params_to_attr   (const char        **params,
                                            int                *attr_codes,
                                            FepAttribute       *r_attr);
char **          _fep_sgr_params_from_attr (const FepAttribute *attr,
                                            int                *attr_codes);
void             _fep_sgr_get_attr_codes   (int                *attr_codes);

/* cap.c */
const char *     _fep_cap_get_string       (const char         *name);

/* key.c */
bool             _fep_csi_to_key           (FepCSI             *csi,
                                            uint32_t           *r_key,
					    uint32_t           *r_state);
bool             _fep_char_to_key          (char                tty,
                                            uint32_t           *r_key,
                                            uint32_t           *r_state);

/* input.c */
ssize_t          _fep_read                 (Fep                *fep,
                                            void               *buf,
                                            size_t              count);
int              _fep_pselect              (Fep                *fep,
                                            fd_set             *fds,
                                            sigset_t           *sigmask);

/* output.c */
void             _fep_putp                 (Fep                *fep,
                                            const char         *str);
void             _fep_output_set_attributes (Fep                *fep,
                                             const FepAttribute *attr);
void             _fep_output_change_scroll_region
                                           (Fep                *fep,
                                            int                 start,
                                            int                 end);
void             _fep_output_cursor_address
                                           (Fep                *fep,
					    int                 row,
					    int                 col);
void             _fep_output_string_from_pty
                                           (Fep                *fep,
                                            const char         *str,
                                            int                 str_len);
void             _fep_output_cursor_text   (Fep                *fep,
                                            const char         *text);
void             _fep_output_status_text   (Fep                *fep,
                                            const char         *text);
void             _fep_output_send_text     (Fep                *fep,
					    const char         *text);
ssize_t          _fep_output_send_data     (Fep                *fep,
                                            const char         *data,
					    size_t              length);
void             _fep_output_set_screen_size
                                           (Fep                *fep,
                                            int                 col,
                                            int                 row);
void             _fep_output_init_screen   (Fep                *fep);
bool             _fep_output_get_cursor_position
                                           (Fep                *fep,
                                            FepPoint           *point);

/* control.c */
int              _fep_open_control_socket  (const char         *template,
                                            char              **r_path);
int              _fep_dispatch_control_message
                                           (Fep                *fep,
                                            int                 fd);

#endif	/* __FEP_PRIVATE_H__ */
