/*

  Copyright (c) 2003-2011 uim Project http://code.google.com/p/uim/

  All rights reserved.

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

#include "private.h"
#include <string.h>
#include <stdlib.h>

static int tty_out;

static int
_putchar (int c)
{
  char _c = c;
  write (tty_out, &_c, 1);
  return c;
}

void
_fep_putp (Fep *fep, const char *str)
{
  tty_out = fep->tty_out;
  tputs (str, 1, _putchar);
}

void
_fep_output_set_attributes (Fep *fep, const FepAttribute *attr)
{
  char **params, *params_str, *str;

  params = _fep_sgr_params_from_attr (attr, fep->attr_codes);
  params_str = _fep_strjoinv (params, ";");
  _fep_strfreev (params);

  str = _fep_csi_format (params_str, "", 'm');
  free (params_str);

  _fep_putp (fep, str);
  free (str);

  memcpy (&fep->attr, attr, sizeof(FepAttribute));
}

void
_fep_output_change_scroll_region (Fep *fep, int start, int end)
{
  const char *csr = tparm (change_scroll_region, start, end);
  _fep_putp (fep, csr);
  fep->cursor.row = fep->cursor.col = 0;
}

static void
apply_attr (Fep *fep, const FepAttribute *attr)
{
  if ((fep->attr.underline && !attr->underline)
      || (fep->attr.standout && !attr->standout)
      || (fep->attr.bold && !attr->bold)
      || (fep->attr.blink && !attr->blink)
      || (fep->attr.foreground != 0 && attr->foreground == 0)
      || (fep->attr.background != 0 && attr->background == 0))
    _fep_output_set_attributes (fep, &_fep_empty_attr);
  else
    {
      FepAttribute _attr;

      memset (&_attr, 0, sizeof(FepAttribute));
      if (fep->attr.underline == attr->underline)
	_attr.underline = FALSE;
      if (fep->attr.standout == attr->standout)
	_attr.standout = FALSE;
      if (fep->attr.bold == attr->bold)
	_attr.bold = FALSE;
      if (fep->attr.blink == attr->blink)
	_attr.blink = FALSE;
      if (fep->attr.foreground == attr->foreground)
	_attr.foreground = FALSE;
      if (fep->attr.background == attr->background)
	_attr.background = FALSE;
      _fep_output_set_attributes (fep, &_attr);
    }
}

void
_fep_output_string_from_pty (Fep *fep, const char *str, int str_len)
{
  if (str_len > 0)
    {
      const char *p, *sgr;
      size_t sgr_len;

      apply_attr (fep, &fep->attr_pty);
      write (fep->tty_out, str, str_len);

      p = str;
      while (_fep_csi_scan (p, str_len, 'm', &sgr, &sgr_len))
	{
	  const char *params_str, *intermediate_str;
	  char final;
	  if (_fep_csi_parse (sgr, sgr_len,
			      &params_str, &intermediate_str, &final))
	    {
	      char **params = _fep_strsplit (params_str, ";", -1);
	      FepAttribute attr;
	      _fep_sgr_params_to_attr ((const char **) params,
				       fep->attr_codes,
				       &attr);
	      _fep_strfreev (params);
	      memcpy (&fep->attr, &attr, sizeof(FepAttribute));
	    }
	  p = sgr + sgr_len;
	}
      if (sgr != NULL)
	_fep_string_append (&fep->ptybuf, sgr, sgr_len);
      fep->cursor.row = fep->cursor.col = -1;
    }
}

void
_fep_output_string (Fep *fep, const char *str)
{
  if (*str != '\0')
    {
      const char *p = str;
      wchar_t *dest;
      size_t len;

      if (fep->ptybuf.len > 0)
	{
	  free (fep->ptybuf.str);
	  fep->ptybuf.str = NULL;
	}

      apply_attr (fep, &fep->attr_tty);

      len = mbsrtowcs (NULL, &p, strlen (str), NULL);
      dest = malloc (len * sizeof(wchar_t));
      mbsrtowcs (dest, &p, strlen (str), NULL);
      fep->cursor.col += wcswidth (dest, len);
      free (dest);
      write (fep->tty_out, str, strlen (str));
    }
}

static void
_fep_output_goto_statusline (Fep *fep, int col)
{
  int row = fep->winsize.ws_row;
  if (row != fep->cursor.row || col != fep->cursor.col)
    {
      char *str = tparm (cursor_address, row, col);
      _fep_putp (fep, str);
      fep->cursor.row = row;
      fep->cursor.col = col;
    }
}

void
_fep_output_draw_statusline (Fep *fep, const char *statusline)
{

  if (fep->statusline != statusline)
    {
      free (fep->statusline);
      fep->statusline = strdup (statusline);
    }
  _fep_putp (fep, save_cursor);

  _fep_output_goto_statusline (fep, 0);
  _fep_putp (fep, clr_eol);

  _fep_output_string (fep, fep->statusline);
  _fep_putp (fep, restore_cursor);
}

void
_fep_output_set_screen_size (Fep *fep, int col, int row)
{
  _fep_putp (fep, save_cursor);
  _fep_output_change_scroll_region (fep, 0, row - 1);
  _fep_output_draw_statusline (fep, fep->statusline);
  _fep_putp (fep, restore_cursor);
}

void
_fep_output_init_screen (Fep *fep)
{
  char *str;

  _fep_output_change_scroll_region (fep, 0, fep->winsize.ws_row - 1);

  str = tparm (clear_screen, 2);
  _fep_putp (fep, str);

  _fep_output_draw_statusline (fep, "");
}
