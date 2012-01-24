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

#include <libfep/libfep.h>
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
command_set_status (Fep *fep, FepControlMessage *message)
{
  _fep_output_statusline (fep, message->args[0]);
}

int
_fep_dispatch_control_message (Fep *fep, int fd)
{
  static const struct
  {
    int command;
    void (*handler) (Fep *fep, FepControlMessage *message);
  } handlers[] =
      {
	{ FEP_CONTROL_SET_STATUS, command_set_status },
      };
  FepControlMessage message;
  int i;

  if (_fep_read_control_message (fd, &message) < 0)
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

  for (i = 0; i < SIZEOF (handlers); i++)
    {
      if (handlers[i].command == message.command)
	{
	  handlers[i].handler (fep, &message);
	  _fep_strfreev (message.args);
	}
    }
  return 0;
}
