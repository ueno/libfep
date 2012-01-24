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

#include <libfep/private.h>
#include <byteswap.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

static const struct
{
  int command;
  size_t n_args;
} commands[] =
  {
    { FEP_CONTROL_SET_STATUS, 1 },
    { FEP_CONTROL_SET_TEXT, 1 },
    { FEP_CONTROL_KEY_EVENT, 2 },
    { FEP_CONTROL_KEY_EVENT_RESPONSE, 1 },
  };

int
_fep_read_control_message (int fd,
			   FepControlMessage *message)
{
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
	  int j;
	  char **args;

	  message->command = commands[i].command;
	  args = message->args = calloc (commands[i].n_args + 1,
					 sizeof(char *));
	  for (j = 0; j < commands[i].n_args; j++)
	    {
	      FepString arg;
	      uint32_t len;

	      retval = read (fd, buf, 4);
	      if (retval < 0)
		{
		  _fep_strfreev (message->args);
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
		      _fep_strfreev (message->args);
		      return -1;
		    }
		  _fep_string_append (&arg, buf, retval);
		  len -= retval;
		}
	      *args++ = strndup (arg.str, arg.len);
	      _fep_string_clear (&arg);
	      free (arg.str);
	    }
	  return 0;
	}
    }
  return -1;
}

int
_fep_write_control_message (int fd,
			    FepControlMessage *message)
{
  int i;

  for (i = 0; i < SIZEOF (commands); i++)
    {
      if (commands[i].command == message->command)
	{
	  char command_char = message->command;
	  ssize_t bytes_written;
	  int j;

	  bytes_written = write (fd, &command_char, 1);
	  if (bytes_written < 0)
	    return -1;

	  for (j = 0; j < commands[i].n_args; j++)
	    {
	      size_t arg_len, total;
	      uint32_t length_word;

	      arg_len = strlen (message->args[j]);
#ifdef WORDS_BIGENDIAN
	      length_word = bswap_32 (arg_len);
#else
	      length_word = arg_len;
#endif

	      bytes_written = write (fd, &length_word, 4);
	      if (bytes_written < 0)
		return -1;

	      total = 0;
	      while (total < arg_len)
		{
		  bytes_written = write (fd,
					 message->args[j] + total,
					 arg_len - total);
		  if (bytes_written <= 0)
		    return -1;
		  total += bytes_written;
		}
	    }
	}
    }
  return 0;
}
