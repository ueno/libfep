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
