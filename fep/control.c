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
#include <stddef.h>		/* offsetof */
#include <assert.h>

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
  name = xcharalloc (len);
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

static void
remove_control_socket (const char *path)
{
  char *_path = xstrdup (path), *p;
  unlink (_path);
  p = strrchr (_path, '/');
  assert (p != NULL);
  *p = '\0';

  rmdir (_path);
}

int
_fep_open_control_socket (Fep *fep)
{
  struct sockaddr_un sun;
  char *path;
  int fd;
  ssize_t sun_len;

  fd = socket (AF_UNIX, SOCK_STREAM, 0);
  if (fd < 0)
    {
      perror ("socket");
      return -1;
    }

  path = create_socket_name ("fep-XXXXXX/control");
  if (strlen (path) + 1 >= sizeof(sun.sun_path))
    {
      fep_log (FEP_LOG_LEVEL_WARNING,
	       "unix domain socket path too long: %d + 1 >= %d",
	       strlen (path),
	       sizeof (sun.sun_path));
      free (path);
      return -1;
    }

  memset (&sun, 0, sizeof(sun));
  sun.sun_family = AF_UNIX;

#ifdef __linux__
  sun.sun_path[0] = '\0';
  memcpy (sun.sun_path + 1, path, strlen (path));
  sun_len = offsetof (struct sockaddr_un, sun_path) + strlen (path) + 1;
  remove_control_socket (path);
#else
  memcpy (sun.sun_path, path, strlen (path));
  sun_len = sizeof (struct sockaddr_un);
#endif

  if (bind (fd, (const struct sockaddr *) &sun, sun_len) < 0)
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

  fep->server = fd;
  fep->control_socket_path = path;
  return 0;
}

void
_fep_close_control_socket (Fep *fep)
{
  if (fep->server >= 0)
    close (fep->server);
  remove_control_socket (fep->control_socket_path);
  free (fep->control_socket_path);
}

static void
command_set_cursor_text (Fep *fep,
			 FepControlMessage *request)
{
  FepAttribute attr;
  if (_fep_control_message_read_attribute_arg (request, 1, &attr) == 0)
    _fep_output_cursor_text (fep, request->args[0].str, &attr);
}

static void
command_set_status_text (Fep *fep,
			 FepControlMessage *request)
{
  FepAttribute attr;
  if (_fep_control_message_read_attribute_arg (request, 1, &attr) == 0)
    _fep_output_status_text (fep, request->args[0].str, &attr);
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
