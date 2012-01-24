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

#include <libfep/libfep.h>
#include <libfep/private.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>

/**
 * SECTION:client
 * @short_description: Class for managing client connection to FEP server
 */

struct _FepClient
{
  int control;
  FepKeyEventHandler key_event_handler;
  void *key_event_handler_data;
};

FepClient *
fep_client_open (const char *address)
{
  FepClient *client;
  struct sockaddr_un sun;
  int retval;

  if (!address)
    address = getenv ("LIBFEP_CONTROL_SOCK");
  if (!address)
    return NULL;

  client = malloc (sizeof(FepClient));

  sun.sun_family = AF_UNIX;
  memcpy (sun.sun_path, address, strlen (address));

  client->control = socket (AF_UNIX, SOCK_STREAM, 0);
  if (client->control < 0)
    {
      free (client);
      return NULL;
    }

  retval = connect (client->control,
		    (const struct sockaddr *) &sun,
		    SUN_LEN (&sun));
  if (retval < 0)
    {
      close (client->control);
      free (client);
      return NULL;
    }

  return client;
}

int
fep_client_set_status (FepClient *client, const char *text)
{
  FepControlMessage message;

  message.command = FEP_CONTROL_SET_STATUS;
  message.args = calloc (2, sizeof(char *));
  message.args[0] = strdup (text);

  return _fep_write_control_message (client->control, &message);
}

int
fep_client_set_key_event_handler (FepClient *client,
				  FepKeyEventHandler handler,
				  void *data)
{
  client->key_event_handler = handler;
  client->key_event_handler_data = data;
}

int
fep_client_get_key_event_poll_fd (FepClient *client)
{
  return client->control;
}

int
fep_client_dispatch_key_event (FepClient *client)
{
  FepControlMessage message;
  char *endptr;
  unsigned int key;
  FepModifierType modifiers;
  int retval;

  retval = _fep_read_control_message (client->control, &message);
  if (retval < 0)
    return retval;

  if (message.command != FEP_CONTROL_KEY_EVENT)
    {
      _fep_strfreev (message.args);
      return -1;
    }

  errno = 0;
  key = strtoul (message.args[0], &endptr, 10);
  if (errno != 0 || *endptr != '\0')
    {
      _fep_strfreev (message.args);
      return -1;
    }

  errno = 0;
  modifiers = strtoul (message.args[1], &endptr, 10);
  if (errno != 0 || *endptr != '\0')
    {
      _fep_strfreev (message.args);
      return -1;
    }

  _fep_strfreev (message.args);
  message.command = FEP_CONTROL_KEY_EVENT_RESPONSE;
  message.args = calloc (2, sizeof(char *));
  if (client->key_event_handler &&
      client->key_event_handler (key, modifiers,
				 client->key_event_handler_data))
    message.args[0] = strdup ("1");
  else
    message.args[0] = strdup ("0");

  retval = _fep_write_control_message (client->control, &message);
  _fep_strfreev (message.args);
  return retval;
}

void
fep_client_close (FepClient *client)
{
  close (client->control);
  free (client);
}
