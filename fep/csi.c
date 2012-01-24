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
