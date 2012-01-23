#include <libfep/libfep.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdint.h>
#include <byteswap.h>
#include <unistd.h>

struct _FepClient
{
  int control;
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
  char command = FEP_CONTROL_SET_STATUS;
  uint32_t command_len;
  size_t len;
  ssize_t bytes_written, total = 0;

  bytes_written = write (client->control, &command, 1);
  if (bytes_written < 0)
    return -1;

  len = strlen (text);
#ifdef WORDS_BIGENDIAN
  command_len = bswap_32 (len);
#else
  command_len = len;
#endif

  bytes_written = write (client->control, &command_len, 4);
  if (bytes_written < 0)
    return -1;

  bytes_written = 0;
  while (total < len)
    {
      bytes_written = write (client->control,
			     text + bytes_written,
			     len - bytes_written);
      if (bytes_written <= 0)
	return -1;
      total += bytes_written;
    }
  return 0;
}

void
fep_client_close (FepClient *client)
{
  close (client->control);
  free (client);
}
