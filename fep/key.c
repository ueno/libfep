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
#include <ctype.h>
#include <errno.h>
#include <assert.h>

struct CursorStateEntry
{
  int param;
  FepModifierType state;
} cursor_states[] = {
  { 2, FEP_SHIFT_MASK },
  { 5, FEP_CONTROL_MASK },
};

struct CursorKeyvalEntry
{
  char final;
  uint32_t keyval;
} cursor_keyvals[] =
  {
    { 'A', FEP_Up },
    { 'B', FEP_Down },
    { 'C', FEP_Right },
    { 'D', FEP_Left },
  };

struct CapKeyvalEntry
{
  char *name;
  uint32_t keyval;
} cap_keyvals[] =
  {
    { "key_backspace", FEP_BackSpace },
    { "key_dc", FEP_Delete },
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
      state |= FEP_CONTROL_MASK;
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
      key = FEP_BackSpace;
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

static uint32_t
_fep_get_state_from_cursor (FepCSI *csi)
{
  char **params = _fep_strsplit (csi->params, ";", -1);
  uint32_t state = 0;
  if (_fep_strv_length (params) == 2)
    {
      int param, i;
      char *endptr;

      errno = 0;
      param = strtoul (params[1], &endptr, 10);
      assert (errno == 0 && *endptr == '\0');

      for (i = 0; i < SIZEOF (cursor_states); i++)
	if (cursor_states[i].param == param)
	  {
	    state = cursor_states[i].state;
	    break;
	  }
    }
  _fep_strfreev (params);
  return state;
}

static bool
_fep_csi_to_cursor_key (FepCSI *csi,
			uint32_t *r_key,
			uint32_t *r_state)
{
  int i;

  for (i = 0; i < SIZEOF (cursor_keyvals); i++)
    {
      if (cursor_keyvals[i].final == csi->final)
	{
	  *r_key = cursor_keyvals[i].keyval;
	  *r_state = _fep_get_state_from_cursor (csi);
	  return true;
	}
    }
  return false;
}

static bool
_fep_csi_to_key (FepCSI   *csi,
                 uint32_t *r_key,
                 uint32_t *r_state)
{
  char *csi_str;
  int i;

  if (_fep_csi_to_cursor_key (csi, r_key, r_state))
    return true;

  csi_str = _fep_csi_format (csi);
  for (i = 0; i < SIZEOF (cap_keyvals); i++)
    {
      const char *cap_str = _fep_cap_get_string (cap_keyvals[i].name);
      if (cap_str)
	{
	  size_t len = strlen (cap_str);
	  if (strncmp (csi_str, cap_str, len) == 0)
	    {
	      *r_key = cap_keyvals[i].keyval;
	      *r_state = 0;
	      return true;
	    }
	}
    }

  *r_key = 0;
  *r_state = 0;
  return false;
}

bool
_fep_esc_to_key (const char *str,
		 size_t len,
		 uint32_t *r_key,
                 uint32_t *r_state,
		 char **r_endptr)
{
  FepCSI *csi;
  bool retval = false;
  uint32_t key, state;
  char *endptr;

  csi = _fep_csi_parse (str, len, 'O', &endptr);
  if (csi)
    {
      switch (csi->final)
	{
	case 'A': /* Esc O A == Up on VT100/VT320/xterm. */
	case 'B': /* Esc O B == Down on
		   * VT100/VT320/xterm. */
	case 'C': /* Esc O C == Right on
		   * VT100/VT320/xterm. */
	case 'D': /* Esc O D == Left on
		   * VT100/VT320/xterm. */
	  retval = _fep_csi_to_cursor_key (csi, &key, &state);
	  if (retval)
	    {
	      *r_key = key;
	      *r_state = state;
	      *r_endptr = endptr;
	    }
	default:
	  break;
	}
      _fep_csi_free (csi);
      return retval;
    }

  csi = _fep_csi_parse (str, len, 'o', &endptr);
  if (csi)
    {
      switch (csi->final)
	{
	case 'a': /* Esc o a == Ctrl-Up on Eterm. */
	case 'b': /* Esc o b == Ctrl-Down on Eterm. */
	case 'c': /* Esc o c == Ctrl-Right on Eterm. */
	case 'd': /* Esc o d == Ctrl-Left on Eterm. */
	  csi->final = toupper (csi->final);
	  retval = _fep_csi_to_cursor_key (csi, &key, &state);
	  if (retval)
	    {
	      *r_key = key;
	      *r_state = state;
	      *r_endptr = endptr;
	    }
	  break;
	default:
	  break;
	}
      _fep_csi_free (csi);
      return retval;
    }

  csi = _fep_csi_parse (str, len, '\133', &endptr);
  if (csi)
    {
      retval = _fep_csi_to_key (csi, &key, &state);
      if (retval)
	{
	  *r_key = key;
	  *r_state = state;
	  *r_endptr = endptr;
	}
      _fep_csi_free (csi);
      return retval;
    }

  return false;
}
