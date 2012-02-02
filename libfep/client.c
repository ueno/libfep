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
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>

/**
 * SECTION:client
 * @short_description: Client connection to FEP server
 */

struct _FepClient
{
  int control;
  FepKeyEventFilter key_event_filter;
  void *key_event_filter_data;
  FepKeyEventHandler key_event_handler;
  void *key_event_handler_data;
};

/**
 * fep_client_open:
 * @address: (allow-none): socket address of the FEP server
 *
 * Connect to the FEP server running at @address.  If @address is
 * %NULL, it gets the address from the environment variable
 * `LIBFEP_CONTROL_SOCK`.
 *
 * Returns: a new FepClient.
 */
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

  memset (&sun, 0, sizeof(struct sockaddr_un));
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

/**
 * fep_client_set_cursor_text:
 * @client: a FepClient
 * @text: a cursor text
 *
 * Request to display @text at the cursor position on the terminal.
 */
void
fep_client_set_cursor_text (FepClient *client, const char *text)
{
  FepControlMessage message;

  message.command = FEP_CONTROL_SET_CURSOR_TEXT;
  _fep_control_message_alloc_args (&message, 1);
  message.args[0].str = strdup (text);
  message.args[0].len = strlen (text) + 1; /* NULL terminate */

  _fep_write_control_message (client->control, &message);
  _fep_control_message_free_args (&message);
}

/**
 * fep_client_set_status_text:
 * @client: a FepClient
 * @text: a status text
 *
 * Request to display @text at the bottom of the terminal.
 */
void
fep_client_set_status_text (FepClient *client, const char *text)
{
  FepControlMessage message;

  message.command = FEP_CONTROL_SET_STATUS_TEXT;
  _fep_control_message_alloc_args (&message, 1);
  message.args[0].str = strdup (text);
  message.args[0].len = strlen (text) + 1; /* NULL terminate */

  _fep_write_control_message (client->control, &message);
  _fep_control_message_free_args (&message);
}

/**
 * fep_client_send_data:
 * @client: a #FepClient
 * @data: data to be sent
 * @length: length of @data
 *
 * Request to send @data to the child process of the FEP server.
 */
void
fep_client_send_data (FepClient *client, const char *data, size_t length)
{
  FepControlMessage message;

  message.command = FEP_CONTROL_SEND_DATA;
  _fep_control_message_alloc_args (&message, 1);
  message.args[0].str = malloc (length * sizeof(char));
  memcpy (message.args[0].str, data, length);
  message.args[0].len = length;

  _fep_write_control_message (client->control, &message);
  _fep_control_message_free_args (&message);
}

/**
 * fep_client_set_key_event_filter:
 * @client: a FepClient
 * @filter: a filter function
 * @data: user supplied data
 *
 * Set a key event filter which will be called when client receives
 * key events.
 */
void
fep_client_set_key_event_filter (FepClient *client,
				 FepKeyEventFilter filter,
				 void *data)
{
  client->key_event_filter = filter;
  client->key_event_filter_data = data;
}

/**
 * fep_client_set_key_event_handler:
 * @client: a FepClient
 * @handler: a handler function
 * @data: user supplied data
 *
 * Set a key event handler which will be called when client receives
 * key events.
 */
void
fep_client_set_key_event_handler (FepClient *client,
				  FepKeyEventHandler handler,
				  void *data)
{
  client->key_event_handler = handler;
  client->key_event_handler_data = data;
}

/**
 * fep_client_get_key_event_poll_fd:
 * @client: a FepClient
 *
 * Get the file descriptor of the control socket which can be used by poll().
 *
 * Returns: a file descriptor
 */
int
fep_client_get_key_event_poll_fd (FepClient *client)
{
  return client->control;
}

/**
 * fep_client_dispatch_key_event:
 * @client: a FepClient
 *
 * Dispatch a key event.
 *
 * Returns: 0 on success, -1 on failure.
 */
int
fep_client_dispatch_key_event (FepClient *client)
{
  FepControlMessage message;
  unsigned int keyval;
  FepModifierType modifiers;
  int retval, intval;

  retval = _fep_read_control_message (client->control, &message);
  if (retval < 0 || message.command != FEP_CONTROL_KEY_EVENT)
    {
      _fep_control_message_free_args (&message);
      fep_log (FEP_LOG_LEVEL_WARNING, "no key event");
      return -1;
    }

  if (message.n_args != 2)
    {
      _fep_control_message_free_args (&message);
      fep_log (FEP_LOG_LEVEL_WARNING, "too few arguments for KEY_EVENT");
      retval = -1;
      goto out;
    }

  retval = _fep_control_message_read_int_arg (&message, 0, &intval);
  if (retval < 0)
    {
      _fep_control_message_free_args (&message);
      goto out;
    }
  keyval = intval;

  retval = _fep_control_message_read_int_arg (&message, 1, &intval);
  if (retval < 0)
    {
      _fep_control_message_free_args (&message);
      goto out;
    }
  modifiers = intval;
  _fep_control_message_free_args (&message);

 out:
  message.command = FEP_CONTROL_RESPONSE;
  _fep_control_message_alloc_args (&message, 2);
  _fep_control_message_write_byte_arg (&message, 0, FEP_CONTROL_KEY_EVENT);
  if (retval >= 0
      && client->key_event_filter
      && client->key_event_filter (keyval, modifiers,
				   client->key_event_filter_data))
    _fep_control_message_write_int_arg (&message, 1, 1);
  else
    _fep_control_message_write_int_arg (&message, 1, 0);

  retval = _fep_write_control_message (client->control, &message);
  _fep_control_message_free_args (&message);

  if (retval >= 0
      && client->key_event_handler)
    client->key_event_handler (keyval, modifiers,
			       client->key_event_handler_data);

  return retval;
}

/**
 * fep_client_close:
 * @client: a FepClient
 *
 * Close the control socket and release the memory allocated for @client.
 */
void
fep_client_close (FepClient *client)
{
  close (client->control);
  free (client);
}
