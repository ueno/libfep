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

#include "fep.h"
#include <libfep/private.h>	/* _fep_strsplit_set */
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <locale.h>

static void
usage (FILE *out, const char *program_name)
{
  fprintf (out,
	   "Usage: %s OPTIONS...\n"
	   "where OPTIONS are:\n"
	   "  -e, --executable=COMMAND\tCommand to run (default: $SHELL)\n"
	   "  -h, --help\tShow this help\n",
	   program_name);
}

int
main (int argc, char **argv)
{
  Fep *fep;
  int c;
  char **command = NULL;

  setlocale (LC_ALL, "");

  while (1)
    {
      int option_index = 0;
      static struct option long_options[] =
	{
	  { "execute", required_argument, 0, 'e' },
	  { "help", no_argument, 0, 'h' },
	  { NULL, 0, 0, 0 }
	};
      c = getopt_long (argc, argv, "e:h",
		       long_options, &option_index);
      if (c == -1)
	break;

      switch (c)
	{
	case 'e':
	  command = _fep_strsplit_set (optarg, " \f\t\n\r\v", -1);
	  break;
	case 'h':
	  usage (stdout, argv[0]);
	  exit (0);
	  break;
	default:
	  usage (stderr, argv[0]);
	  exit (1);
	  break;
	}
    }

  if (optind < argc)
    {
      usage (stderr, argv[0]);
      exit (1);
    }

  fep = fep_new ();
  if (command == NULL)
    {
      static char *shell[2];
      shell[0] = getenv ("SHELL");
      shell[1] = NULL;
      command = shell;
    }
  if (fep_run (fep, (const char **) command) < 0)
    {
      fprintf (stderr, "Can't run FEP command\n");
      exit (2);
    }
  fep_free (fep);

  return 0;
}
