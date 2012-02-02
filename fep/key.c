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

struct EscKeyvalEntry
{
  char *code;
  uint32_t keyval;
} esc_keyvals[] =
  {
    { "\033[A", FEP_Up },
    { "\033[B", FEP_Down },
    { "\033[C", FEP_Right },
    { "\033[D", FEP_Left },
  };

struct CapKeyvalEntry
{
  char *name;
  uint32_t keyval;
} cap_keyvals[] =
  {
    { "key_backspace", FEP_BackSpace },
    { "key_dc", FEP_Delete },
    { "key_left", FEP_Left },
    { "key_up", FEP_Up },
    { "key_right", FEP_Right },
    { "key_down", FEP_Down },
    { "key_ppage", FEP_Prior },
    { "key_npage", FEP_Next },
    { "key_home", FEP_Home },
    { "key_end", FEP_End },
    { "key_ic", FEP_Insert },
    { "key_f1", FEP_F1 },
    { "key_f2", FEP_F2 },
    { "key_f3", FEP_F3 },
    { "key_f4", FEP_F4 },
    { "key_f5", FEP_F5 },
    { "key_f6", FEP_F6 },
    { "key_f7", FEP_F7 },
    { "key_f8", FEP_F8 },
    { "key_f9", FEP_F9 },
    { "key_f10", FEP_F10 },
    { "key_f11", FEP_F11 },
    { "key_f12", FEP_F12 },
  };

bool
_fep_char_to_key (char      tty,
                  uint32_t *r_key,
                  uint32_t *r_state)
{
  uint32_t key;
  uint32_t state;

  tty &= 0x7f;
  state = (tty & 0x80) ? FEP_META_MASK : 0;
  switch (tty)
    {
      /* C-space */
    case 0:
      key = ' ';
      break;
    case '\t':
      key = FEP_Tab;
      break;
    case '\r':
      key = FEP_Return;
      break;
      /* C-[ */
    case '\033':
      key = FEP_Escape;
      break;
      /* C-? */
    case 0x7f:
      key = FEP_Delete;
      break;
    default:
      /* C-a .. C-z */
      if (tty >= 1 && tty <= 26)
	{
	  key = tty + ('a' - 1);
	  state |= FEP_CONTROL_MASK;
	}
      /* C-\ C-] C-^ C-_ */
      else if (tty >= 28 && tty <= 31)
	{
	  key = tty + ('A' - 1);
	  state |= FEP_CONTROL_MASK;
	}
      /* A .. Z */
      else if (tty >= 'A' && tty <= 'Z')
	{
	  key = tty;
	  state |= FEP_SHIFT_MASK;
	}
      else
	{
	  key = tty;
	  state = 0;
	}
    }

  *r_key = key;
  *r_state = state;

  return key != 0;
}

FepReadKeyResult
_fep_read_key_from_string (const char *str,
			   int         str_len,
			   uint32_t   *r_key,
			   size_t     *r_key_len)
{
  FepReadKeyResult retval = FEP_READ_KEY_ERROR;
  int i;

  for (i = 0; i < SIZEOF (esc_keyvals); i++)
    {
      size_t len = strlen (esc_keyvals[i].code);
      if (strncmp (str, esc_keyvals[i].code, len) == 0)
	{
	  *r_key = esc_keyvals[i].keyval;
	  *r_key_len = len;
	  return FEP_READ_KEY_OK;
	}
      else if (strncmp (str, esc_keyvals[i].code, str_len) == 0)
	retval = FEP_READ_KEY_NOT_ENOUGH;
    }

  for (i = 0; i < SIZEOF (cap_keyvals); i++)
    {
      const char *esc = _fep_cap_get_string (cap_keyvals[i].name);
      if (esc != NULL)
	{
	  size_t len = strlen (esc);
	  if (strncmp (str, esc, len) == 0)
	    {
	      *r_key = cap_keyvals[i].keyval;
	      *r_key_len = len;
	      return FEP_READ_KEY_OK;
	    }
	  else if (strncmp (str, esc, str_len) == 0)
	    retval = FEP_READ_KEY_NOT_ENOUGH;
	}
    }

  *r_key = 0;
  *r_key_len = 0;
  return retval;
}
