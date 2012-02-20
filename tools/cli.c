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

#ifndef _LIBC
char *program_name = "fepcli";
#endif

static void
usage (FILE *out, const char *program_name)
{
  fprintf (out,
	   "Usage: %s OPTIONS...\n"
	   "where OPTIONS are:\n"
	   "  -c, --cursor-text=TEXT\tRender text at the cursor position\n"
	   "  -s, --status-text=TEXT\tRender text at the bottom\n"
	   "  -d, --send-text=TEXT\tSend text to the child process\n"
	   "  -e, --listen-event\tListen to an event from server\n"
	   "  -l, --log-file=FILE\tLog file\n"
	   "  -h, --help\tShow this help\n",
	   program_name);
}

static int
event_filter (FepEvent *event)
{
  if (event->type == FEP_KEY_PRESS)
    {
      FepEventKey *_event = (FepEventKey *)event;
      printf ("keyval = %u, modifiers = %u\n",
	      _event->keyval, _event->modifiers);
    }
  else
    printf ("unknown event %u\n", event->type);
  return 1;
}

int
main (int argc, char **argv)
{
  FepClient *client;
  int c;
  char *cursor_text = NULL, *status_text = NULL, *send_text = NULL;
  bool listen_event = false;
  char *log_file = NULL;

  while (1)
    {
      int option_index = 0;
      static struct option long_options[] =
	{
	  { "cursor-text", required_argument, 0, 'c' },
	  { "status-text", required_argument, 0, 's' },
	  { "send-text", required_argument, 0, 'd' },
	  { "listen-event", no_argument, 0, 'e' },
	  { "log-file", no_argument, 0, 'l' },
	  { "help", no_argument, 0, 'h' },
	  { NULL, 0, 0, 0 }
	};
      c = getopt_long (argc, argv, "c:s:d:el:h",
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
	  send_text = optarg;
	  break;
	case 'e':
	  listen_event = true;
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

  if (!cursor_text && !status_text && !send_text && !listen_event)
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
      fep_client_set_cursor_text (client, cursor_text, NULL);
      exit (0);
    }
  else if (status_text)
    {
      fep_client_set_status_text (client, status_text, NULL);
      exit (0);
    }
  else if (send_text)
    {
      fep_client_send_text (client, send_text);
      exit (0);
    }
  else if (listen_event)
    {
      fep_client_set_event_filter (client,
				   (FepEventFilter) event_filter,
				   NULL);
      printf ("# waiting for an event\n");
      if (fep_client_dispatch (client) < 0)
	{
	  fprintf (stderr, "Can't dispatch event\n");
	  exit (1);
	}
    }

  fep_client_close (client);

  return 0;
}
