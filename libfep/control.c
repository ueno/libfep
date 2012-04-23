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
#include <errno.h>
#include <assert.h>

static const struct
{
  FepControlCommand command;
  char *name;
  size_t n_args;
} commands[] =
  {
    { FEP_CONTROL_SET_CURSOR_TEXT, "SET_CURSOR_TEXT", 2 },
    { FEP_CONTROL_SET_STATUS_TEXT, "SET_STATUS_TEXT", 2 },
    { FEP_CONTROL_SEND_TEXT, "SEND_TEXT", 1 },
    { FEP_CONTROL_SEND_DATA, "SEND_DATA", 1 },
    { FEP_CONTROL_FORWARD_KEY_EVENT, "FORWARD_KEY_EVENT", 2 },
    { FEP_CONTROL_KEY_EVENT, "KEY_EVENT", 3 },
    { FEP_CONTROL_RESIZE_EVENT, "RESIZE_EVENT", 2 },
    { FEP_CONTROL_RESPONSE, "RESPONSE", 2 }
  };

static int
_fep_control_command_get_n_args (FepControlCommand command)
{
  int i;
  for (i = 0; i < SIZEOF (commands); i++)
    if (commands[i].command == command)
      return commands[i].n_args;
  return -1;
}

static const char *
_fep_control_command_get_name (FepControlCommand command)
{
  int i;
  for (i = 0; i < SIZEOF (commands); i++)
    if (commands[i].command == command)
      return commands[i].name;
  return NULL;
}

static char *
_fep_control_message_to_string (FepControlMessage *message)
{
  const char *command_name;
  size_t total, command_length;
  int i;
  char *buf, *p;

  command_name = _fep_control_command_get_name (message->command);
  command_length = strlen (command_name);
  total = command_length + 1;
  for (i = 0; i < message->n_args; i++)
    total += message->args[i].len * 2 + 1;

  buf = xcharalloc (total);
  memset (buf, 0, total);

  p = buf;
  memcpy (p, command_name, command_length);
  p += command_length;
  memcpy (p, " ", 1);
  p += 1;

  for (i = 0; i < message->n_args; i++)
    {
      int j;
      for (j = 0; j < message->args[i].len; j++)
	{
	  snprintf (p, 3, "%02X", (unsigned char) message->args[i].str[j]);
	  p += 2;
	}
      memcpy (p, " ", 1);
      p += 1;
    }
  *--p = '\0';

  return buf;
}

int
_fep_read_control_message (int fd,
			   FepControlMessage *message)
{
  char buf[BUFSIZ];
  FepString *args;
  int retval, i;

  memset (buf, 0, BUFSIZ);
  retval = read (fd, buf, 1);
  if (retval < 0)
    {
      fep_log (FEP_LOG_LEVEL_WARNING,
	       "failed to read from %d: %s",
	       fd, strerror (errno));
      return -1;
    }
  if (retval == 0)
    {
      fep_log (FEP_LOG_LEVEL_DEBUG,
	       "connection %d closed",
	       fd);
      return -1;
    }

  retval = _fep_control_command_get_n_args (buf[0]);
  if (retval < -1)
    {
      fep_log (FEP_LOG_LEVEL_WARNING,
	       "read unknown command %d",
	       buf[0]);
      return -1;
    }

  message->command = buf[0];
  message->n_args = retval;
  message->args = xcalloc (message->n_args, sizeof(FepString));
  for (i = 0, args = message->args; i < message->n_args; i++, args++)
    {
      FepString arg;
      uint32_t len;

      retval = read (fd, buf, 4);
      if (retval < 4)
	{
	  _fep_control_message_free_args (message);
	  fep_log (FEP_LOG_LEVEL_WARNING,
		   "failed to read argument length from %d: %s",
		   fd, errno != 0 ? strerror (errno) : "too short");
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
	  if (retval < 0)
	    {
	      _fep_control_message_free_args (message);
	      fep_log (FEP_LOG_LEVEL_WARNING,
		       "failed to read from %d: %s",
		       fd, strerror (errno));
	      return -1;
	    }
	  _fep_string_append (&arg, buf, retval);
	  len -= retval;
	}
      args->str = xcharalloc (arg.len);
      memcpy (args->str, arg.str, arg.len);
      args->cap = args->len = arg.len;
      _fep_string_clear (&arg);
      free (arg.str);
      arg.str = NULL;
    }

  if (fep_get_log_level () >= FEP_LOG_LEVEL_DEBUG)
    {
      char *str = _fep_control_message_to_string (message);
      fep_log (FEP_LOG_LEVEL_DEBUG, "read %s", str);
      free (str);
    }
  return 0;
}

int
_fep_write_control_message (int fd,
			    FepControlMessage *message)
{
  char command_char = message->command;
  int retval, i;

  retval = write (fd, &command_char, 1);
  if (retval < 1)
    {
      fep_log (FEP_LOG_LEVEL_WARNING,
	       "failed to write to %d: %s",
	       fd, retval < 0 ? strerror (errno) : "can't write command byte");
      return -1;
    }

  for (i = 0; i < message->n_args; i++)
    {
      size_t total;
      uint32_t length_word;

#ifdef WORDS_BIGENDIAN
      length_word = bswap_32 (message->args[i].len);
#else
      length_word = message->args[i].len;
#endif

      retval = write (fd, &length_word, 4);
      if (retval < 0)
	{
	  fep_log (FEP_LOG_LEVEL_WARNING,
		   "failed to write to %d: %s",
		   fd, strerror (errno));
	  return -1;
	}

      for (total = 0; total < message->args[i].len; total += retval)
	{
	  retval = write (fd,
			  message->args[i].str + total,
			  message->args[i].len - total);
	  if (retval < 0)
	    {
	      fep_log (FEP_LOG_LEVEL_WARNING,
		       "failed write to %d: %s",
		       fd, strerror (errno));
	      return -1;
	    }
	}
    }

  if (fep_get_log_level () >= FEP_LOG_LEVEL_DEBUG)
    {
      char *str = _fep_control_message_to_string (message);
      fep_log (FEP_LOG_LEVEL_DEBUG, "write %s", str);
      free (str);
    }
  return 0;
}

void
_fep_control_message_alloc_args (FepControlMessage *message, size_t n_args)
{
  message->args = xcalloc (n_args, sizeof(FepString));
  message->n_args = n_args;
}

void
_fep_control_message_free_args (FepControlMessage *message)
{
  int i;
  for (i = 0; i < message->n_args; i++)
    free (message->args[i].str);
  free (message->args);
}

int
_fep_control_message_read_uint32_arg (FepControlMessage *message,
                                      off_t              index,
                                      uint32_t          *r_val)
{
  uint32_t intval;

  if (index > message->n_args)
    return -1;

  if (message->args[index].len != sizeof(uint32_t))
    return -1;

  intval = *(uint32_t *) message->args[index].str;
#ifdef WORDS_BIGENDIAN
  *r_val = bswap_32 (intval);
#else
  *r_val = intval;
#endif

  return 0;
}

int
_fep_control_message_write_uint32_arg (FepControlMessage *message,
				       off_t              index,
				       uint32_t           val)
{
  uint32_t intval;

  if (index > message->n_args)
    return -1;

#ifdef WORDS_BIGENDIAN
  intval = bswap_32 (val);
#else
  intval = val;
#endif
  message->args[index].str = xmemdup ((char *) &intval, sizeof(uint32_t));
  message->args[index].cap = message->args[index].len = sizeof(uint32_t);

  return 0;
}

int
_fep_control_message_write_uint8_arg (FepControlMessage *message,
				      off_t index,
				      uint8_t val)
{
  if (index > message->n_args)
    return -1;

  message->args[index].str = xmemdup ((char *) &val, sizeof(uint8_t));
  message->args[index].cap = message->args[index].len = 1;

  return 0;
}

int
_fep_control_message_write_string_arg (FepControlMessage *message,
				       off_t index,
				       const char *str,
				       size_t length)
{
  if (index > message->n_args)
    return -1;

  message->args[index].str = xmemdup (str, length);
  message->args[index].cap = message->args[index].len = length;

  return 0;
}

int
_fep_control_message_read_attribute_arg (FepControlMessage *message,
					 off_t index,
					 FepAttribute *r_attr)
{
  char *p;
  uint8_t intval;

  if (index > message->n_args)
    return -1;

  if (message->args[index].len != 4 * sizeof(uint32_t))
    return -1;

  p = message->args[index].str;
  intval = *(uint32_t *) p;
  p += sizeof(uint32_t);
#ifdef WORDS_BIGENDIAN
  intval = bswap_32 (intval);
#endif
  r_attr->type = intval;

  intval = *(uint32_t *) p;
  p += sizeof(uint32_t);
#ifdef WORDS_BIGENDIAN
  intval = bswap_32 (intval);
#endif
  r_attr->value = intval;

  intval = *(uint32_t *) p;
  p += sizeof(uint32_t);
#ifdef WORDS_BIGENDIAN
  intval = bswap_32 (intval);
#endif
  r_attr->start_index = intval;

  intval = *(uint32_t *) p;
  p += sizeof(uint32_t);
#ifdef WORDS_BIGENDIAN
  intval = bswap_32 (intval);
#endif
  r_attr->end_index = intval;

  return 0;
}

int
_fep_control_message_write_attribute_arg (FepControlMessage *message,
					 off_t index,
					 const FepAttribute *attr)
{
  uint32_t intval;
  char *p;

  if (index > message->n_args)
    return -1;

  p = message->args[index].str = xcalloc (4, sizeof(uint32_t));
  message->args[index].cap = message->args[index].len = 4 * sizeof(uint32_t);

  intval = attr->type;
#ifdef WORDS_BIGENDIAN
  intval = bswap_32 (intval);
#endif
  memcpy (p, (char *) &intval, sizeof(uint32_t));
  p += sizeof(uint32_t);

  intval = attr->value;
#ifdef WORDS_BIGENDIAN
  intval = bswap_32 (intval);
#endif
  memcpy (p, (char *) &intval, sizeof(uint32_t));
  p += sizeof(uint32_t);

  intval = attr->start_index;
#ifdef WORDS_BIGENDIAN
  intval = bswap_32 (intval);
#endif
  memcpy (p, (char *) &intval, sizeof(uint32_t));
  p += sizeof(uint32_t);

  intval = attr->end_index;
#ifdef WORDS_BIGENDIAN
  intval = bswap_32 (intval);
#endif
  memcpy (p, (char *) &intval, sizeof(uint32_t));
  p += sizeof(uint32_t);

  return 0;
}

static void
_fep_control_message_copy (FepControlMessage *dst, FepControlMessage *src)
{
  int i;

  dst->command = src->command;
  _fep_control_message_alloc_args (dst, src->n_args);
  for (i = 0; i < src->n_args; i++)
    _fep_string_copy (&dst->args[i], &src->args[i]);
}

void
_fep_control_message_free (FepControlMessage *message)
{
  _fep_control_message_free_args (message);
  free (message);
}

FepList *
_fep_append_control_message (FepList *head,
			     FepControlMessage *message)
{
  FepControlMessage *_message = xzalloc (sizeof(FepControlMessage));
  _fep_control_message_copy (_message, message);

  if (fep_get_log_level () >= FEP_LOG_LEVEL_DEBUG)
    {
      char *str = _fep_control_message_to_string (_message);
      fep_log (FEP_LOG_LEVEL_DEBUG, "queue %s", str);
      free (str);
    }

  return _fep_list_append (head, _message);
}
