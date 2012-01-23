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

void
_fep_string_append (FepString *buf, const char *str, size_t count)
{
  if (buf->len + count > buf->cap)
    {
      buf->cap = MAX(buf->cap * 2, buf->len + count);
      buf->str = realloc (buf->str, buf->cap);
    }
  memcpy (buf->str + buf->len, str, count);
  buf->len += count;
}

void
_fep_string_clear (FepString *buf)
{
  buf->len = 0;
}

char **
_fep_strsplit (const char *str, const char *delimiter, int max_tokens)
{
  size_t len = 0;
  const char *p, *q;
  char **strv, **r;

  for (p = strstr (str, delimiter); p && *p != '\0'; p = strstr (p, delimiter))
    len++;

  strv = calloc (len + 1, sizeof (char *));
  for (p = str, q = strstr (p, delimiter), r = strv;
       q && *q != '\0';
       p = q, q = strstr (p, delimiter), r++)
    *r = strndup (p, q - p);

  return strv;
}

char *
_fep_strjoinv (char **strv, const char *delimiter)
{
  char **p, *str;
  size_t len = 0, delimiter_len = strlen (delimiter);

  for (p = strv; *p; p++)
    {
      len += strlen (*p);
      if (*(p + 1))
	len += delimiter_len;
    }
  str = calloc (len + 1, sizeof(char));
  for (p = strv; *p; p++)
    {
      strcat (str, *p);
      if (*(p + 1))
	strcat (str, delimiter);
    }
  return str;
}

void
_fep_strfreev (char **strv)
{
  char **p;
  for (p = strv; *p; p++)
    free (*p);
  free (strv);
}
