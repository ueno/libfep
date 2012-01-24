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

int
main (int argc, char **argv)
{
  FepClient *client;

  if (argc != 2)
    {
      fprintf (stderr, "Usage: %s TEXT\n", argv[0]);
      exit (1);
    }

  client = fep_client_open (NULL);
  if (!client)
    {
      fprintf (stderr, "Can't open FEP control socket\n");
      exit (1);
    }

  if (fep_client_set_status (client, argv[1]) < 0)
    {
      fprintf (stderr, "Can't set status\n");
      exit (1);
    }

  fep_client_close (client);

  return 0;
}
