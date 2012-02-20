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
_fep_csi_scan (const char *str,
               size_t      len,
               const char  final,
               char      **r_csi,
               size_t     *r_csi_len)
{
  const char *p;
  char *start;

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
	  if ((final < 0 || *p == final) &&
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

FepCSI *
_fep_csi_parse (const char *str,
                size_t      len,
                char      **r_endptr)
{
  const char *p, *start;
  FepCSI csi, *r_csi;

  if (len < 3 || strncmp (str, "\033\133", 2) != 0)
    return NULL;

  /* P */
  start = str + 2;
  for (p = start; p - str < len && '\060'<= *p && *p <= '\077'; p++)
    ;
  if (p - str == len)
    {
      fep_log (FEP_LOG_LEVEL_DEBUG, "premature CSI at P");
      return NULL;
    }
  csi.params = xcharalloc (p - start + 1);
  memcpy (csi.params, start, p - start);
  csi.params[p - start] = '\0';
  
  /* I */
  start = p;
  for (; p - str < len && '\040'<= *p && *p <= '\057'; p++)
    ;
  if (p - str == len)
    {
      free (csi.params);
      fep_log (FEP_LOG_LEVEL_DEBUG, "premature CSI at I");
      return NULL;
    }
  csi.intermediate = xcharalloc (p - start + 1);
  memcpy (csi.intermediate, start, p - start);
  csi.intermediate[p - start] = '\0';

  /* F */
  if (p - str == len)
    {
      free (csi.params);
      free (csi.intermediate);
      fep_log (FEP_LOG_LEVEL_DEBUG, "premature CSI at F \"%s\" \"%s\"", str, p);
      return NULL;
    }
  csi.final = *p++;

  r_csi = xmemdup (&csi, sizeof(FepCSI));
  if (r_endptr)
    *r_endptr = (char *) p;
  return r_csi;
}

char *
_fep_csi_format (FepCSI *csi)
{
  return xasprintf ("\033\133%s%s%c",
		    csi->params,
		    csi->intermediate,
		    csi->final);
}

void
_fep_csi_free (FepCSI *csi)
{
  free (csi->params);
  free (csi->intermediate);
  free (csi);
}

