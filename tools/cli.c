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

#include <unistd.h>
#include <libfep/libfep.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>

static void
usage (FILE *out, const char *program_name)
{
  fprintf (out,
	   "Usage: %s OPTIONS...\n"
	   "where OPTIONS are:\n"
	   "  -c, --cursor-text=TEXT\tRender text at the cursor position\n"
	   "  -s, --status-text=TEXT\tRender text at the bottom\n"
	   "  -d, --send-data=DATA\tSend text to the child process\n"
	   "  -k, --listen-key-event\tListen to a new key event\n"
	   "  -l, --log-file=FILE\tLog file\n"
	   "  -h, --help\tShow this help\n",
	   program_name);
}

static int
key_event_filter (unsigned int keyval, FepModifierType modifiers)
{
  printf ("keyval = %u, modifiers = %u\n",
	  keyval, modifiers);
  return 1;
}

int
main (int argc, char **argv)
{
  FepClient *client;
  int c;
  char *cursor_text = NULL, *status_text = NULL, *send_data = NULL, *log_file = NULL;
  bool key_event = false;

  while (1)
    {
      int option_index = 0;
      static struct option long_options[] =
	{
	  { "cursor-text", required_argument, 0, 'c' },
	  { "status-text", required_argument, 0, 's' },
	  { "send-data", required_argument, 0, 'd' },
	  { "listen-key-event", no_argument, 0, 'k' },
	  { "log-file", no_argument, 0, 'l' },
	  { "help", no_argument, 0, 'h' },
	  { NULL, 0, 0, 0 }
	};
      c = getopt_long (argc, argv, "c:s:d:kl:h",
		       long_options, &option_index);
      if (c == -1)
	break;

      switch (c)
	{
	case 'c':
	  cursor_text = optarg;
	  break;
	case 's':
	  status_text = optarg;
	  break;
	case 'd':
	  send_data = optarg;
	  break;
	case 'k':
	  key_event = true;
	  break;
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
    {
      usage (stderr, argv[0]);
      exit (1);
    }

  if (!cursor_text && !status_text && !send_data && !key_event)
    {
      fprintf (stderr, "No action specified\n");
      exit (1);
    }

  if (log_file != NULL)
    {
      fep_set_log_file (log_file);
      fep_set_log_level (FEP_LOG_LEVEL_DEBUG);
    }

  client = fep_client_open (NULL);
  if (!client)
    {
      fprintf (stderr, "Can't open FEP control socket\n");
      exit (1);
    }

  if (cursor_text)
    {
      fep_client_set_cursor_text (client, cursor_text);
      exit (0);
    }
  else if (status_text)
    {
      fep_client_set_status_text (client, status_text);
      exit (0);
    }
  else if (send_data)
    {
      fep_client_send_data (client, send_data, strlen (send_data));
      exit (0);
    }
  else if (key_event)
    {
      fep_client_set_key_event_filter (client,
				       (FepKeyEventFilter) key_event_filter,
				       NULL);
      printf ("# type any key\n");
      if (fep_client_dispatch_key_event (client) < 0)
	{
	  fprintf (stderr, "Can't dispatch key event\n");
	  exit (1);
	}
    }

  fep_client_close (client);

  return 0;
}
