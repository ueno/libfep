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
#include <stdlib.h>

/* http://vt100.net/docs/vt102-ug/appendixd.html */
bool
_fep_csi_scan (const char *str, size_t len, const char final,
	       const char **r_csi, size_t *r_csi_len)
{
  const char *p, *start;

  for (p = str; p - str < len; )
    {
      /* CSI */
      p = start = memchr (p, '\033', len - (p - str));
      if (p == NULL)
	break;

      p++;
      if (p - str == len)
	{
	  *r_csi = start;
	  *r_csi_len = p - start;
	  return false;
	}

      /* CSI */
      if (*p == '\133')
	{
	  p++;
	  if (p - str == len)
	    {
	      *r_csi = start;
	      *r_csi_len = p - start;
	      return false;
	    }
	  /* P */
	  for (; p - str < len && '\060'<= *p && *p <= '\077'; p++)
	    ;
	  /* I */
	  for (; p - str < len && '\040'<= *p && *p <= '\057'; p++)
	    ;
	  if (p - str == len)
	    {
	      *r_csi = start;
	      *r_csi_len = p - start;
	      return false;
	    }
	  /* F */
	  if ((final > 0 && *p == final) ||
	      ('\100' <= *p && *p <= '\176'))
	    {
	      *r_csi = start;
	      *r_csi_len = p - start + 1;
	      return true;
	    }
	}
    }
  *r_csi = NULL;
  *r_csi_len = 0;
  return false;
}

bool
_fep_csi_parse (const char *str, size_t len,
		const char **r_params,
		const char **r_intermediate,
		char *r_final)
{
  const char *p;
  const char *params, *intermediate;

  if (len <= 3 || strncmp (str, "\033\133", 2) != 0)
    return false;

  /* P */
  params = str + 2;
  for (p = params; p - str < len && '\060'<= *p && *p <= '\077'; p++)
    ;
  if (p - str == len)
    return false;

  /* I */
  intermediate = p;
  for (; p - str < len && '\040'<= *p && *p <= '\057'; p++)
    ;
  if (p - str == len)
    return false;

  /* F */
  p++;
  if (p - str == len)
    return false;

  *r_params = params;
  *r_intermediate = intermediate;
  *r_final = *p;
  return true;
}

char *
_fep_csi_format (const char *params,
                 const char *intermediate,
                 char        final)
{
  char *str;
  size_t len;

  len = 3 + strlen (params) + strlen (intermediate);
  str = calloc (len + 1, sizeof(char));
  snprintf (str, len + 1, "\033\133%s%s%c", params, intermediate, final);

  return str;
}
