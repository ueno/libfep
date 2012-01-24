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
#include <stdio.h>
#include <stdlib.h>

static int
key_event_handler (unsigned int key, FepModifierType modifiers)
{
  printf ("key = %u, modifiers = %u\n",
	  key, modifiers);
  return 1;
}

int
main (int argc, char **argv)
{
  FepClient *client;

  client = fep_client_open (NULL);
  if (!client)
    {
      fprintf (stderr, "Can't open FEP control socket\n");
      exit (1);
    }

  fep_client_set_key_event_handler (client,
				    (FepKeyEventHandler) key_event_handler,
				    NULL);
  if (fep_client_dispatch_key_event (client) < 0)
    {
      fprintf (stderr, "Can't dispatch key event\n");
      exit (1);
    }

  fep_client_close (client);

  return 0;
}
