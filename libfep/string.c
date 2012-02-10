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
#include <wchar.h>
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

void
_fep_string_copy (FepString *dst, FepString *src)
{
  dst->cap = src->cap;
  dst->len = src->len;
  dst->str = malloc (src->cap);
  memcpy (dst->str, src->str, src->len);
}

static char *
search_strstr (const char *str, const char *delimiter, char **next)
{
  char *p = strstr (str, delimiter);
  if (p)
    *next = p + strlen (delimiter);
  return p;
}

static char *
search_strpbrk (const char *str, const char *delimiter, char **next)
{
  char *p = strpbrk (str, delimiter);
  if (p)
    *next = p + strspn (str, delimiter);
  return p;
}

static char **
_fep_strsplit_full (const char *str, const char *delimiter, int max_tokens,
		    char *(*search) (const char *, const char *, char **))
{
  size_t n_delimiters = 0;
  const char *p, *q;
  char *next, **strv, **r;

  p = search (str, delimiter, &next);
  for (; p && *p != '\0'; p = search (p, delimiter, &next))
    {
      n_delimiters++;
      p = next;
    }

  p = str;
  q = search (p, delimiter, &next);
  r = strv = calloc (n_delimiters + 2, sizeof (char *));
  for (; q && *q != '\0'; q = search (p, delimiter, &next))
    {
      *r++ = strndup (p, q - p);
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

size_t
_fep_strv_length (char **strv)
{
  size_t length = 0;
  while (*strv++)
    length++;
  return length;
}

void
_fep_strfreev (char **strv)
{
  char **p;
  for (p = strv; *p; p++)
    free (*p);
  free (strv);
}

int
_fep_strwidth (const char *str)
{
  const char *p;
  wchar_t *wcs;
  size_t wcs_len;
  int width;

  p = str;
  wcs = calloc (strlen (str) + 1, sizeof(wchar_t));
  wcs_len = mbsrtowcs (wcs, &p, strlen (str), NULL);
  width = wcswidth (wcs, wcs_len);
  free (wcs);
  return width;
}

char *
_fep_strtrunc (const char *str, int width)
{
  const char *p;
  char *mbs;
  const wchar_t *q;
  wchar_t *wcs;
  size_t wcs_len;
  int total, i;

  p = str;
  wcs = calloc (strlen (str) + 1, sizeof(wchar_t));
  wcs_len = mbsrtowcs (wcs, &p, strlen (str), NULL);
  if (wcs_len == (size_t) -1)
    {
      free (wcs);
      fep_log (FEP_LOG_LEVEL_WARNING,
	       "can't convert string to wchar string");
      return NULL;
    }

  for (i = 0, total = 0; i < wcs_len; i++)
    {
      int w = wcwidth (wcs[i]);
      if (total + w > width)
	break;
      total += w;
    }
  wcs[i] = L'\0';
  q = wcs;
  mbs = calloc (strlen (str) + 1, sizeof(char));
  wcsrtombs (mbs, &q, strlen (str), NULL);
  free (wcs);

  return mbs;
}

int
_fep_charcount (const char *str)
{
  const char *p;
  size_t retval;
  
  p = str;
  retval = mbsrtowcs (NULL, &p, strlen (str), NULL);
  if (retval == (size_t) -1)
    {
      fep_log (FEP_LOG_LEVEL_WARNING,
	       "can't convert string to wchar string");
      return -1;
    }
  return (int) retval;
}

char *
_fep_substring (const char *str, int from, int to)
{
  const char *p;
  char *mbs;
  const wchar_t *q;
  wchar_t *wcs;
  size_t wcs_len;
  int index;

  p = str;
  wcs = calloc (strlen (str) + 1, sizeof(wchar_t));
  wcs_len = mbsrtowcs (wcs, &p, strlen (str), NULL);
  if (wcs_len == (size_t) -1)
    {
      free (wcs);
      fep_log (FEP_LOG_LEVEL_WARNING,
	       "can't convert string to wchar string");
      return NULL;
    }

  if (from > to)
    {
      index = from;
      from = to;
      to = index;
    }

  if (from > wcs_len || to > wcs_len)
    {
      free (wcs);
      fep_log (FEP_LOG_LEVEL_WARNING,
	       "invalid range");
      return NULL;
    }

  wcs[to] = L'\0';
  q = &wcs[from];
  mbs = calloc (strlen (str) + 1, sizeof(char));
  wcsrtombs (mbs, &q, strlen (str), NULL);
  free (wcs);

  return mbs;
}
