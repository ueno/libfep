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
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

const FepAttribute _fep_empty_attr;

void
_fep_sgr_params_to_attr (const char **params, int *attr_codes,
			 FepAttribute *r_attr)
{
  const char **p;
  for (p = params; *p; p++)
    {
      char *endptr;
      int param;

      errno = 0;
      param = strtoul (*p, &endptr, 10);
      assert (errno == 0 && *endptr == '\0');

      if (param == 0)
	memcpy (r_attr, &_fep_empty_attr, sizeof(FepAttribute));
      else if (param == attr_codes[FEP_ATTR_TYPE_ENTER_UNDERLINE])
	r_attr->underline = true;
      else if (param == attr_codes[FEP_ATTR_TYPE_EXIT_UNDERLINE])
	r_attr->underline = false;
      else if (param == attr_codes[FEP_ATTR_TYPE_ENTER_STANDOUT])
	r_attr->standout = true;
      else if (param == attr_codes[FEP_ATTR_TYPE_EXIT_STANDOUT])
	r_attr->standout = false;
      else if (param == attr_codes[FEP_ATTR_TYPE_ENTER_BOLD])
	r_attr->bold = true;
      else if (param == attr_codes[FEP_ATTR_TYPE_ENTER_BLINK])
	r_attr->blink = true;
      else if (param == attr_codes[FEP_ATTR_TYPE_ORIG_PAIR])
	r_attr->foreground = r_attr->background = 0;
      else if (param == attr_codes[FEP_ATTR_TYPE_ORIG_FORE])
	r_attr->foreground = 0;
      else if (param == attr_codes[FEP_ATTR_TYPE_ORIG_BACK])
	r_attr->background = 0;
      else if (param == 22)	/* normal color or intensity */
	r_attr->bold = false;
      else if (param == 25)	/* blink: off */
	r_attr->blink = false;
      else if (/* set foreground color */
	       (30 <= param && param <= 37)
	       /* set foreground color, high intensity */
	       || (90 <= param && param <= 97)) 
	r_attr->foreground = param;
      else if (/* set background color */
	       (40 <= param && param <= 47)
	       /* set background color, high intensity */
	       || (100 <= param && param <= 107))
	r_attr->background = param - 10;
    }
}

char **
_fep_sgr_params_from_attr (const FepAttribute *attr,
			   int *attr_codes)
{
#define MAX_PARAM_DIGITS 3
  char **params = calloc (7, sizeof(char *)), **p = params;
  if (memcmp (attr, &_fep_empty_attr, sizeof(FepAttribute)) != 0)
    {
      if (attr->underline
	  && attr_codes[FEP_ATTR_TYPE_ENTER_UNDERLINE] != 0)
	{
	  *p = malloc ((MAX_PARAM_DIGITS + 1)  * sizeof (char *));
	  snprintf (*p, MAX_PARAM_DIGITS,
		    "%d", attr_codes[FEP_ATTR_TYPE_ENTER_UNDERLINE]);
	  p++;
	}
      if (attr->standout
	  && attr_codes[FEP_ATTR_TYPE_ENTER_STANDOUT] != 0)
	{
	  *p = malloc ((MAX_PARAM_DIGITS + 1)  * sizeof (char *));
	  snprintf (*p, MAX_PARAM_DIGITS,
		    "%d", attr_codes[FEP_ATTR_TYPE_ENTER_STANDOUT]);
	  p++;
	}
      if (attr->bold
	  && attr_codes[FEP_ATTR_TYPE_ENTER_BOLD] != 0)
	{
	  *p = malloc ((MAX_PARAM_DIGITS + 1)  * sizeof (char *));
	  snprintf (*p, MAX_PARAM_DIGITS,
		    "%d", attr_codes[FEP_ATTR_TYPE_ENTER_BOLD]);
	  p++;
	}
      if (attr->blink
	  && attr_codes[FEP_ATTR_TYPE_ENTER_BLINK] != 0)
	{
	  *p = malloc ((MAX_PARAM_DIGITS + 1)  * sizeof (char *));
	  snprintf (*p, MAX_PARAM_DIGITS,
		    "%d", attr_codes[FEP_ATTR_TYPE_ENTER_BLINK]);
	  p++;
	}
      if (attr->foreground != 0)
	{
	  *p = malloc ((MAX_PARAM_DIGITS + 1)  * sizeof (char *));
	  snprintf (*p, MAX_PARAM_DIGITS,
		    "%d", attr->foreground);
	  p++;
	}
      if (attr->background != 0)
	{
	  *p = malloc ((MAX_PARAM_DIGITS + 1)  * sizeof (char *));
	  snprintf (*p, MAX_PARAM_DIGITS,
		    "%d", attr->background + 10);
	  p++;
	}
    }
  return params;
}

void
_fep_sgr_get_attr_codes (int *attr_codes)
{
  static const struct
  {
    char *name;
    int n_args;
    int index;
    FepAttrType code;
  } name_code[] =
      {
	{ "enter_underline_mode", 1, 0, FEP_ATTR_TYPE_ENTER_UNDERLINE },
	{ "exit_underline_mode", 1, 0, FEP_ATTR_TYPE_EXIT_UNDERLINE },
	{ "enter_standout_mode", 1, 0, FEP_ATTR_TYPE_ENTER_STANDOUT },
	{ "exit_standout_mode", 1, 0, FEP_ATTR_TYPE_EXIT_STANDOUT },
	{ "enter_bold_mode", 1, 0, FEP_ATTR_TYPE_ENTER_BOLD },
	{ "enter_blink_mode", 1, 0, FEP_ATTR_TYPE_ENTER_BLINK },
	{ "orig_pair", 1, 0, FEP_ATTR_TYPE_ORIG_PAIR },
	{ "orig_pair", 2, 0, FEP_ATTR_TYPE_ORIG_FORE },
	{ "orig_pair", 2, 1, FEP_ATTR_TYPE_ORIG_BACK }
      };
  int i;

  for (i = 0; i < SIZEOF (name_code); i++)
    {
      const char *str = _fep_cap_get_string (name_code[i].name);
      FepCSI *csi;

      if (str
	  && (csi = _fep_csi_parse (str, strlen (str), NULL)) != NULL)
	{
	  if (csi->final == 'm')
	    {
	      char **_params = _fep_strsplit (csi->params, ";", -1);
	      char *endptr;
	      int code;

	      if (_fep_strv_length (_params) == name_code[i].n_args)
		{
		  errno = 0;
		  code = strtoul (_params[name_code[i].index], &endptr, 10);
		  if (errno == 0 && endptr == '\0')
		    attr_codes[name_code[i].code] = code;
		}
	      else
		attr_codes[name_code[i].code] = -1;
	      _fep_strfreev (_params);
	    }
	  _fep_csi_free (csi);
	}
    }
}
