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
#include <locale.h>
#include "striconv.h"
#include <langinfo.h>
#include <assert.h>
#include <errno.h>

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

static void
_fep_output_status_text_string (Fep *fep, const char *str)
{
  if (*str != '\0')
    {
      char *local, *mbs;

      if (fep->ptybuf.len > 0)
	_fep_string_clear (&fep->ptybuf);

      apply_attr (fep, &fep->attr_tty);

      local = str_iconv (str, "UTF-8", nl_langinfo (CODESET));
      mbs = _fep_strtrunc (local, fep->winsize.ws_col);
      free (local);
      if (mbs)
	write (fep->tty_out, mbs, strlen (mbs));
      free (mbs);
    }
}

static void
_fep_output_goto_status_text (Fep *fep, int col)
{
  _fep_output_cursor_address (fep, fep->winsize.ws_row, col);
}

void
_fep_output_cursor_address (Fep *fep, int row, int col)
{
  char *str = tparm (cursor_address, row, col);
  _fep_putp (fep, str);
  fep->cursor.row = row;
  fep->cursor.col = col;
}

void
_fep_output_save_cursor (Fep *fep)
{
  if (fep->has_cpr)
    _fep_output_get_cursor_position (fep, &fep->cursor_save);
  else
    {
      _fep_putp (fep, save_cursor);
      memcpy (&fep->cursor_save, &fep->cursor, sizeof(FepPoint));
    }
}

void
_fep_output_restore_cursor (Fep *fep)
{
  if (fep->has_cpr)
    _fep_output_cursor_address (fep, fep->cursor_save.row, fep->cursor_save.col);
  else
    {
      _fep_putp (fep, restore_cursor);
      memcpy (&fep->cursor, &fep->cursor_save, sizeof(FepPoint));
    }
}

void
_fep_output_status_text (Fep *fep, const char *text)
{

  if (fep->status_text != text)
    {
      free (fep->status_text);
      fep->status_text = strdup (text);
    }
  _fep_putp (fep, save_cursor);

  _fep_output_goto_status_text (fep, 0);
  _fep_putp (fep, clr_eol);

  _fep_output_status_text_string (fep, fep->status_text);
  _fep_putp (fep, restore_cursor);
}

void
_fep_output_cursor_text (Fep *fep, const char *text)
{
  char *str;

  if (fep->cursor_text && *fep->cursor_text != '\0')
    {
      int width = _fep_strwidth (fep->cursor_text);

      _fep_output_save_cursor (fep);
      if (fep->has_cpr)
	_fep_output_cursor_address (fep, fep->cursor.row, fep->cursor.col);

      str = malloc (width * sizeof(char));
      memset (str, ' ', width * sizeof(char));
      write (fep->tty_out, str, width * sizeof(char));
      free  (str);

      free (fep->cursor_text);
      fep->cursor_text = NULL;
      _fep_output_restore_cursor (fep);
    }

  if (*text != '\0')
    {
      _fep_output_save_cursor (fep);

      if (_fep_output_get_cursor_position (fep, &fep->cursor))
	_fep_output_cursor_address (fep, fep->cursor.row, fep->cursor.col);

      if (fep->ptybuf.len > 0)
	_fep_string_clear (&fep->ptybuf);

      apply_attr (fep, &fep->attr_tty);

      char *local = str_iconv (text, "UTF-8", nl_langinfo (CODESET));
      fep->cursor_text = _fep_strtrunc (local,
					fep->winsize.ws_col - fep->cursor.col);
      free (local);

      if (fep->cursor_text)
	write (fep->tty_out, fep->cursor_text, strlen (fep->cursor_text));
      _fep_output_restore_cursor (fep);
    }
}

void
_fep_output_send_text (Fep *fep, const char *text)
{
  char *local = str_iconv (text,
			   "UTF-8",
			   nl_langinfo (CODESET));
  if (local)
    {
      ssize_t total = 0;
      size_t length = strlen (local);

      while (total < length)
	{
	  ssize_t bytes_sent = write (fep->pty, local + total, length - total);
	  if (bytes_sent < 0)
	    break;
	  total += bytes_sent;
	}
    }
  free (local);
}

ssize_t
_fep_output_send_data (Fep *fep, const char *data, size_t length)
{
  return write (fep->pty, data, length);
}

void
_fep_output_set_screen_size (Fep *fep, int col, int row)
{
  _fep_output_save_cursor (fep);
  _fep_output_change_scroll_region (fep, 0, row - 1);
  _fep_output_status_text (fep, fep->status_text);
  _fep_output_restore_cursor (fep);
}

/* http://www.vt100.net/docs/vt510-rm/DSR-CPR */
static bool
_fep_output_dsr_cpr (Fep *fep, FepPoint *point)
{
  FepString csibuf;
  char buf[16];
#define RETRY 2;
  int retry = RETRY

  _fep_putp (fep, "\033\1336n"); /* DSR-CPR */
  memset (&csibuf, 0, sizeof(FepString));
  while (--retry > 0)
    {
      ssize_t bytes_read;
      char *csi;
      size_t csi_len;

      bytes_read = _fep_read (fep, buf, sizeof(buf));
      if (bytes_read < 0)
	{
	  free (csibuf.str);
	  return false;
	}
      _fep_string_append (&csibuf, buf, bytes_read);
      if (_fep_csi_scan (csibuf.str, csibuf.len, 'R',
			 (const char **) &csi, &csi_len))
	{
	  const char *csi_end;
	  char **strv, *endptr;

	  csi[csi_len - 1] = '\0';
	  strv = _fep_strsplit (csi + 2, ";", 2);
	  errno = 0;
	  point->row = strtoul (strv[0], &endptr, 10);
	  if (errno != 0 || *endptr != '\0')
	    {
	      _fep_strfreev (strv);
	      free (csibuf.str);
	      return false;
	    }
	  point->col = strtoul (strv[1], &endptr, 10);
	  if (errno != 0 || *endptr != '\0')
	    {
	      _fep_strfreev (strv);
	      free (csibuf.str);
	      return false;
	    }
	  _fep_strfreev (strv);

	  /* rewind data around CPR */
	  _fep_string_append (&fep->ttybuf,
			      csibuf.str,
			      csi - csibuf.str);
	  csi_end = csi + csi_len;
	  _fep_string_append (&fep->ttybuf,
			      csi_end,
			      bytes_read - (csi_end - csibuf.str));
	  free (csibuf.str);
	  return true;
	}
      if (csi == NULL)
	_fep_string_append (&csibuf, buf, bytes_read);
    }
  free (csibuf.str);
  return false;
}

void
_fep_output_init_screen (Fep *fep)
{
  char *str;

  _fep_output_change_scroll_region (fep, 0, fep->winsize.ws_row - 1);

  str = tparm (clear_screen, 2);
  _fep_putp (fep, str);

  _fep_putp (fep, cursor_invisible);
  fep->cursor.row = fep->cursor.col = -1;
  /* FIXME: DSR-CPR is currently unused for some reason */
  /* fep->has_cpr = _fep_output_dsr_cpr (fep, &fep->cursor); */
  fep->has_cpr = 0;
  if (fep->has_cpr)
    {
      FepPoint cursor0, cursor1;

      str = tparm (cursor_address, 1, 1);
      _fep_putp (fep, str);
      _fep_output_dsr_cpr (fep, &cursor0);

      str = tparm (cursor_address, cursor0.row, cursor0.col);
      _fep_putp (fep, str);
      _fep_output_dsr_cpr (fep, &cursor1);

      fep->cursor_diff.row = cursor1.row - cursor0.row;
      fep->cursor_diff.col = cursor1.col - cursor0.col;

      fep->cursor.row -= fep->cursor_diff.row;
      fep->cursor.col -= fep->cursor_diff.col;

      str = tparm (cursor_address, fep->cursor.row, fep->cursor.col);
      _fep_putp (fep, str);
    }
  _fep_putp (fep, cursor_normal);

  _fep_output_status_text (fep, "");
}

bool
_fep_output_get_cursor_position (Fep *fep, FepPoint *cursor)
{
  if (fep->has_cpr && _fep_output_dsr_cpr (fep, cursor))
    {
      cursor->row -= fep->cursor_diff.row;
      cursor->col -= fep->cursor_diff.col;
      return true;
    }
  return false;
}
