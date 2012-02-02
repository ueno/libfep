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
