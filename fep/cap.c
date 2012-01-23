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

#include "private.h"
#include <string.h>

struct FepCapEntry
{
  char *name;
  char *capname;
};

static const struct FepCapEntry caps[] =
  {
    { "enter_underline_mode", "smul" },
    { "exit_underline_mode", "rmul" },
    { "enter_standout_mode", "smso" },
    { "exit_standout_mode", "rmso" },
    { "exit_attribute_mode", "sgr0" },
    { "enter_bold_mode", "bold" },
    { "enter_blink_mode", "blink" },
    { "orig_pair", "op" },
    { "cursor_address", "cup" },
    { "clear_screen", "clear" },
    { "clr_eol", "el" },
    { "clr_eos", "ed" },
    { "change_scroll_region", "csr" },
    { "cursor_up", "cuu1" },
    { "save_cursor", "sc" },
    { "restore_cursor", "rc" },
    { "cursor_left", "cub1" },
    { "cursor_right", "cuf1" },
    { "parm_ich", "ich" },
    { "parm_dch", "dch" },
    { "key_backspace", "kbs" },
    { "key_dc", "kdch1" },
    { "key_left", "kcub1" },
    { "key_up", "kcuu1" },
    { "key_right", "kcuf1" },
    { "key_down", "kcud1" },
    { "key_ppage", "kpp" },
    { "key_npage", "knp" },
    { "key_home", "khome" },
    { "key_end", "kend" },
    { "key_ic", "kich1" },
    { "key_f1", "kf1" },
    { "key_f2", "kf2" },
    { "key_f3", "kf3" },
    { "key_f4", "kf4" },
    { "key_f5", "kf5" },
    { "key_f6", "kf6" },
    { "key_f7", "kf7" },
    { "key_f8", "kf8" },
    { "key_f9", "kf9" },
    { "key_f10", "kf10" },
    { "key_f11", "kf11" },
    { "key_f12", "kf12" },
  };

const char *
_fep_cap_get_string (const char *name)
{
  int i;
  for (i = 0; i < SIZEOF (caps); i++)
    {
      if (strcmp (name, caps[i].name) == 0)
	return tigetstr (caps[i].capname);
    }
  fprintf (stderr, "unknown terminfo variable %s\n", name);
  return NULL;
}
