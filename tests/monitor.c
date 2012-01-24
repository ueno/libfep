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
