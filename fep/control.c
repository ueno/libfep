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
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdlib.h>

static char *
create_socket_name (const char *template)
{
  char *name, *p;
  size_t len;

  /* Prepend the tmp directory to the template.  */
  p = getenv ("TMPDIR");
  if (!p || !*p)
    p = "/tmp";

  len = strlen (p) + strlen (template) + 2;
  name = malloc (len);
  memset (name, 0, len);
  memcpy (name, p, strlen (p));
  if (p[strlen (p) - 1] != '/')
    name = strcat (name, "/");
  name = strcat (name, template);

  p = strrchr (name, '/');
  *p = '\0';
  if (!mkdtemp (name))
    {
      free (name);
      return NULL;
    }
  *p = '/';
  return name;
}

int
_fep_open_control_socket (const char *template, char **r_path)
{
  struct sockaddr_un sun;
  char *path;
  int fd;

  fd = socket (AF_UNIX, SOCK_STREAM, 0);
  if (fd < 0)
    {
      perror ("socket");
      return -1;
    }

  path = create_socket_name (template);
  if (strlen (path) + 1 >= sizeof(sun.sun_path))
    {
      free (path);
      return -1;
    }

  memset (&sun, 0, sizeof(sun));
  sun.sun_family = AF_UNIX;
  memcpy (sun.sun_path, path, strlen (path));

  if (bind (fd, (const struct sockaddr *) &sun, SUN_LEN (&sun)) < 0)
    {
      perror ("bind");
      free (path);
      close (fd);
      return -1;
    }

  if (listen (fd, 5) < 0)
    {
      perror ("listen");
      free (path);
      close (fd);
      return -1;
    }

  *r_path = path;
  return fd;
}

static void
command_set_cursor_text (Fep *fep,
			 FepControlMessage *request)
{
  _fep_output_cursor_text (fep, request->args[0].str);
}

static void
command_set_status_text (Fep *fep,
			 FepControlMessage *request)
{
  _fep_output_status_text (fep, request->args[0].str);
}

static void
command_send_text (Fep *fep,
		   FepControlMessage *request)
{
  _fep_output_send_text (fep, request->args[0].str);
}

static void
command_send_data (Fep *fep,
		   FepControlMessage *request)
{
  ssize_t total = 0;

  while (total < request->args[0].len)
    {
      ssize_t bytes_sent = _fep_output_send_data (fep,
						  request->args[0].str + total,
						  request->args[0].len - total);
      if (bytes_sent < 0)
	break;
      total += bytes_sent;
    }
}

int
_fep_dispatch_control_message (Fep *fep, int fd)
{
  static const struct
  {
    int command;
    void (*handler) (Fep *fep,
		     FepControlMessage *request);
  } handlers[] =
      {
	{ FEP_CONTROL_SET_CURSOR_TEXT, command_set_cursor_text },
	{ FEP_CONTROL_SET_STATUS_TEXT, command_set_status_text },
	{ FEP_CONTROL_SEND_TEXT, command_send_text },
	{ FEP_CONTROL_SEND_DATA, command_send_data }
      };
  FepControlMessage request;
  int i;

  if (_fep_read_control_message (fd, &request) < 0)
    {
      for (i = 0; i < fep->n_clients; i++)
	if (fep->clients[i] == fd)
	  {
	    close (fd);
	    if (i + 1 < fep->n_clients)
	      memmove (&fep->clients[i],
		       &fep->clients[i + 1],
		       fep->n_clients - (i + 1));
	    fep->clients[--fep->n_clients] = -1;
	    break;
	  }
      return -1;
    }

  for (i = 0;
       i < SIZEOF (handlers) && handlers[i].command != request.command;
       i++)
    ;
  if (i == SIZEOF (handlers))
    {
      _fep_control_message_free_args (&request);
      fep_log (FEP_LOG_LEVEL_WARNING,
	       "no handler defined for %d", request.command);
      return -1;
    }

  handlers[i].handler (fep, &request);
  _fep_control_message_free_args (&request);

  return 0;
}
