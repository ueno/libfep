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
#include <byteswap.h>

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
command_set_status (Fep *fep, char **args, size_t n_args)
{
  _fep_output_draw_statusline (fep, args[0]);
}

int
_fep_dispatch_control_message (Fep *fep, int fd)
{
  static const struct
  {
    int command;
    size_t n_args;
    void (*handler) (Fep *fep, char **args, size_t n_args);
  } commands[] =
      {
	{ FEP_CONTROL_SET_STATUS, 1, command_set_status },
      };
  char buf[BUFSIZ];
  int retval, i;

  memset (buf, 0, BUFSIZ);
  retval = read (fd, buf, 1);
  if (retval <= 0)
    return -1;

  for (i = 0; i < SIZEOF (commands); i++)
    {
      if (commands[i].command == buf[0])
	{
	  char **args = calloc (commands[i].n_args, sizeof(char *));
	  size_t n_args = 0;
	  int j;
	  for (j = 0; j < commands[i].n_args; j++)
	    {
	      FepString arg;
	      uint32_t len;

	      retval = read (fd, buf, 4);
	      if (retval < 0)
		{
		  free (args);
		  return -1;
		}
	      len = *(uint32_t *) buf;
#ifdef WORDS_BIGENDIAN
	      len = bswap_32 (len);
#endif
	      memset (&arg, 0, sizeof(FepString));
	      while (len > 0)
		{
		  retval = read (fd, buf, len);
		  if (retval <= 0)
		    {
		      free (args);
		      return -1;
		    }
		  _fep_string_append (&arg, buf, retval);
		  len -= retval;
		}
	      args[n_args++] = strndup (arg.str, arg.len);
	      _fep_string_clear (&arg);
	      free (arg.str);
	    }

	  commands[i].handler (fep, args, n_args);
	  free (args);
	}
    }
  
  return 0;
}
