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

#include "fep.h"
#include <libfep/private.h>    /* _fep_strsplit_set */
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <locale.h>

#ifndef _LIBC
char *program_name = "fep";
#endif

static void
usage (FILE *out, const char *program_name)
{
  fprintf (out,
	   "Usage: %s OPTIONS COMMAND...\n"
	   "where OPTIONS are:\n"
	   "  -l, --log-file=FILE\tLog file\n"
	   "  -h, --help\tShow this help\n",
	   program_name);
}

int
main (int argc, char **argv)
{
  Fep *fep;
  int c;
  char **command = NULL, *log_file = NULL;

  setlocale (LC_ALL, "");

  while (1)
    {
      int option_index = 0;
      static struct option long_options[] =
	{
	  { "log-file", required_argument, 0, 'l' },
	  { "help", no_argument, 0, 'h' },
	  { NULL, 0, 0, 0 }
	};
      c = getopt_long (argc, argv, "e:l:h",
		       long_options, &option_index);
      if (c == -1)
	break;

      switch (c)
	{
	case 'l':
	  log_file = optarg;
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
    command = &argv[optind];
  else
    {
      static char *shell[2];
      shell[0] = getenv ("SHELL");
      shell[1] = NULL;
      command = shell;
    }

  if (log_file != NULL)
    {
      fep_set_log_file (log_file);
      fep_set_log_level (FEP_LOG_LEVEL_DEBUG);
    }

  fep = fep_new ();
  if (fep_run (fep, (const char **) command) < 0)
    {
      fprintf (stderr, "Can't run FEP command\n");
      exit (2);
    }
  fep_free (fep);

  return 0;
}
