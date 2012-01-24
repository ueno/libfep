/*

  Copyright (c) 2012 Daiki Ueno <ueno@unixuser.org>
  Copyright (c) 2012 Red Hat, Inc.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  3. Neither the name of authors nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS'' AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  SUCH DAMAGE.

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

struct _Fep {
  /* input/output via tty */
  int tty_in;
  int tty_out;

  /* input/output via pty */
  int pty;

  /* input/output via control socket */
  int server;
#define FEP_MAX_CLIENTS 10
  int clients[FEP_MAX_CLIENTS];
  size_t n_clients;

  /* input buffer for tty (used by ungetc) */
  FepString ttybuf;

  /* input buffer for pty (to keep incomplete escape sequences from pty) */
  FepString ptybuf;

  FepPoint cursor;

  FepAttribute attr;
  FepAttribute attr_tty;
  FepAttribute attr_pty;
  int attr_codes[FEP_ATTR_TYPE_LAST];

  char *statusline;

  struct winsize winsize;
  struct termios orig_termios;
};

extern const FepAttribute _fep_empty_attr;

/* csi.c */
bool             _fep_csi_scan             (const char         *str,
                                            size_t              len,
                                            const char          final,
                                            const char        **r_csi,
                                            size_t             *r_csi_len);
bool             _fep_csi_parse            (const char         *str,
                                            size_t              len,
                                            const char        **r_params,
                                            const char        **r_intermediate,
                                            char               *r_final);
char *           _fep_csi_format           (const char         *params,
                                            const char         *intermediate,
                                            char                final);

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
bool             _fep_char_to_key          (char                tty,
                                            uint32_t           *r_key,
                                            uint32_t           *r_state);
FepReadKeyResult _fep_read_key_from_string (const char         *str,
                                            int                 str_len,
                                            uint32_t           *r_key,
                                            size_t             *r_key_len);

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
void             _fep_output_string_from_pty
                                           (Fep                *fep,
                                            const char         *str,
                                            int                 str_len);
void             _fep_output_string        (Fep                *fep,
                                            const char         *str);
void             _fep_output_draw_statusline
                                           (Fep                *fep,
                                            const char         *statusline);
void             _fep_output_set_screen_size
                                           (Fep                *fep,
                                            int                 col,
                                            int                 row);
void             _fep_output_init_screen   (Fep                *fep);

/* control.c */
int              _fep_open_control_socket  (const char         *template,
                                            char              **r_path);
int              _fep_dispatch_control_message
                                           (Fep                *fep,
                                            int                 fd);

#endif	/* __FEP_PRIVATE_H__ */
