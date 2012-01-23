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

static char *
search_strstr (const char *str, const char *delimiter, const char **next)
{
  char *p = strstr (str, delimiter);
  if (p)
    *next = p + strlen (delimiter);
  return p;
}

static char *
search_strpbrk (const char *str, const char *delimiter, const char **next)
{
  char *p = strpbrk (str, delimiter);
  if (p)
    *next = p + strspn (str, delimiter);
  return p;
}

static char **
_fep_strsplit_full (const char *str, const char *delimiter, int max_tokens,
		    char *(*search) (const char *, const char *, const char **))
{
  size_t len = 0;
  const char *p, *q;
  char *next, **strv, **r;

  p = search (str, delimiter, &next);
  if (!p && str[0] != '\0')
    len++;
  else
    {
      for (; p && *p != '\0'; p = search (p, delimiter, &next))
	{
	  len++;
	  p = next;
	}
    }

  strv = calloc (len + 1, sizeof (char *));
  for (p = str, q = search (p, delimiter, &next), r = strv;
       q && *q != '\0';
       q = search (p, delimiter, &next), r++)
    {
      *r = strndup (p, q - p);
      p = next;
    }
  if (p - str < strlen (str))
    *r = strdup (p);

  return strv;
}

char **
_fep_strsplit (const char *str, const char *delimiter, int max_tokens)
{
  return _fep_strsplit_full (str, delimiter, max_tokens, search_strstr);
}

char **
_fep_strsplit_set (const char *str, const char *delimiter, int max_tokens)
{
  return _fep_strsplit_full (str, delimiter, max_tokens, search_strpbrk);
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
