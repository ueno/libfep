/*

  Copyright (c) 2003-2011 uim Project http://code.google.com/p/uim/

  All rights reserved.

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

#include "private.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>

static sigset_t orig_sigmask;
static sig_atomic_t signals;

static int main_loop (Fep *fep);

static void *
find_match_end (const void *haystack, size_t haystacklen,
		const void *needle, size_t needlelen)
{
  size_t _haystacklen = haystacklen;
  while (_haystacklen > 0)
    {
      char *p = memrchr (haystack, *(const char *) needle, _haystacklen);

      if (p == NULL)
	return NULL;

      if (haystacklen - (p - (const char *) haystack) >= needlelen
	  && memcmp (p, needle, needlelen) == 0)
	return p + needlelen;

      _haystacklen = p - (const char *) haystack;
    }
  return NULL;
}

static void
done (Fep *fep, int code)
{
  _fep_output_change_scroll_region (fep, 0, fep->winsize.ws_row);
  tcsetattr (fep->tty_in, TCSAFLUSH, &fep->orig_termios);
  fep_free (fep);
  exit (code);
}

static void
signal_handler (int sig)
{
  switch (sig)
    {
    case SIGHUP:
    case SIGTERM:
    case SIGQUIT:
      signals |= FEP_SIG_FLAG_TERM;
      break;
    case SIGWINCH:
      signals |= FEP_SIG_FLAG_WINCH;
      break;
    case SIGTSTP:
      signals |= FEP_SIG_FLAG_TSTP;
      break;
    }
}

static void
set_signal_handler (void)
{
  struct sigaction act;
  sigset_t sigmask;

  sigemptyset (&sigmask);
  sigaddset (&sigmask, SIGHUP);
  sigaddset (&sigmask, SIGTERM);
  sigaddset (&sigmask, SIGQUIT);
  sigaddset (&sigmask, SIGINT);
  sigaddset (&sigmask, SIGWINCH);
  sigaddset (&sigmask, SIGTSTP);
  sigaddset (&sigmask, SIGCONT);
  sigprocmask (SIG_BLOCK, &sigmask, &orig_sigmask);

  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;

  act.sa_handler = signal_handler;
  sigaction (SIGHUP, &act, NULL);
  sigaction (SIGTERM, &act, NULL);
  sigaction (SIGQUIT, &act, NULL);
  sigaction (SIGINT, &act, NULL);
  sigaction (SIGWINCH, &act, NULL);
  sigaction (SIGTSTP, &act, NULL);
  sigaction (SIGCONT, &act, NULL);
}

static void
reset_signal_handler (void)
{
  struct sigaction act;

  sigprocmask (SIG_SETMASK, &orig_sigmask, NULL);

  sigemptyset (&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = SIG_DFL;
  sigaction (SIGHUP, &act, NULL);
  sigaction (SIGTERM, &act, NULL);
  sigaction (SIGQUIT, &act, NULL);
  sigaction (SIGINT, &act, NULL);
  sigaction (SIGWINCH, &act, NULL);
  sigaction (SIGTSTP, &act, NULL);
  sigaction (SIGCONT, &act, NULL);
}

static void
handle_term_signal (Fep *fep)
{
  done (fep, 1);
}

static void
handle_winch_signal (Fep *fep)
{
  struct winsize _winsize;
  FepControlMessage request;
  int i;

  memcpy (&_winsize, &fep->winsize, sizeof(struct winsize));
  ioctl (fep->tty_in, TIOCGWINSZ, &fep->winsize);
  fep->winsize.ws_row--;
  _fep_output_set_screen_size (fep, _winsize.ws_col, _winsize.ws_row);
  ioctl (fep->pty, TIOCSWINSZ, &fep->winsize);

  request.command = FEP_CONTROL_RESIZE_EVENT;
  _fep_control_message_alloc_args (&request, 2);
  _fep_control_message_write_uint32_arg (&request,
					 0,
					 (uint32_t) _winsize.ws_col);
  _fep_control_message_write_uint32_arg (&request,
					 1,
					 (uint32_t) _winsize.ws_row);
  for (i = 0; i < fep->n_clients; i++)
    {
      FepControlMessage response;

      if (_fep_transceive_control_message (fep->clients[i],
					   &request,
					   &response) == 0)
	_fep_control_message_free_args (&response);
    }
  _fep_control_message_free_args (&request);
}

static void
handle_tstp_signal (Fep *fep)
{
  struct sigaction act;
  sigset_t sigmask;

  tcsetattr (fep->tty_in, TCSAFLUSH, &fep->orig_termios);

  sigemptyset (&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = SIG_DFL;
  sigaction (SIGTSTP, &act, NULL);
  sigaction (SIGCONT, &act, NULL);

  sigemptyset (&sigmask);
  sigaddset (&sigmask, SIGTSTP);
  sigaddset (&sigmask, SIGCONT);
  sigprocmask (SIG_UNBLOCK, &sigmask, NULL);

  kill (getpid (), SIGTSTP);

  sigprocmask (SIG_BLOCK, &sigmask, NULL);
  
  act.sa_handler = signal_handler;
  sigaction (SIGTSTP, &act, NULL);
  sigaction (SIGCONT, &act, NULL);
}

Fep *
fep_new (void)
{
  Fep *fep = xzalloc (sizeof(Fep));
  int i;

  fep->tty_in = STDIN_FILENO;
  fep->tty_out = STDOUT_FILENO;
  fep->pty = -1;
  fep->server = -1;
  for (i = 0; i < FEP_MAX_CLIENTS; i++)
    fep->clients[i] = -1;
  fep->status_text = xstrdup ("");
  return fep;
}

int
fep_run (Fep *fep, const char *command[])
{
  pid_t pid;
  int retval;

  if (isatty (fep->tty_in))
    fep->tty_in = open ("/dev/tty", O_RDONLY);

  tcgetattr (fep->tty_in, &fep->orig_termios);
  setupterm (NULL, fep->tty_out, NULL);
  ioctl (fep->tty_in, TIOCGWINSZ, &fep->winsize);
  fep->winsize.ws_row--;

  retval = _fep_open_control_socket (fep);
  if (retval < 0)
    return -1;

  setenv ("LIBFEP_CONTROL_SOCK", fep->control_socket_path, 1);

  pid = forkpty (&fep->pty, NULL, &fep->orig_termios, &fep->winsize);
  if (pid < 0)
    {
      perror ("forkpty");
      return -1;
    }
  else if (pid == 0)
    {
      if (fep->tty_in != STDIN_FILENO)
	close (fep->tty_in);
      execvp (command[0], (char * const *) command);
      perror (command[0]);
      exit (1);
    }
  else
    {
      static struct termios termios;
      int retval;

      tcgetattr (fep->tty_in, &termios);
      cfmakeraw (&termios);
      termios.c_cc[VMIN] = 0;
      termios.c_cc[VTIME] = 3;
      tcsetattr (fep->tty_in, TCSANOW, &termios);

      _fep_get_sgr_codes (fep->sgr_codes);
      _fep_output_init_screen (fep);

      if (_fep_output_get_cursor_position (fep, &fep->cursor))
	fep->has_cpr = true;

      set_signal_handler ();
      retval = main_loop (fep);
      done (fep, retval < 0 ? 1 : 0);
    }
  return 0;
}

char *
remove_padding (const char *str)
{
  ssize_t len = strlen (str);
  char *start, *p, *dest;
  start = memmem (str, len, "$<", 2);
  if (start == NULL)
    return xstrdup (str);
  for (p = start; *p != '\0' && isdigit (*p); p++)
    ;
  if (*p != '>')
    return xstrdup (str);
  p++;
  dest = xcharalloc (len - (p - start) + 1);
  memcpy (dest, str, start - str);
  memcpy (dest + (start - str), p, len - (p - str));
  return dest;
}

static int
main_loop (Fep *fep)
{
  char buf[BUFSIZ];
  fd_set fds;
  char *_clear_screen = remove_padding (clear_screen);
  char *_clr_eos = remove_padding (clr_eos);
  int retval = 0, i;

  while (true)
    {
      if (_fep_pselect (fep, &fds, &orig_sigmask) <= 0)
	{
	  if (signals & FEP_SIG_FLAG_TERM)
	    {
	      signals &= ~FEP_SIG_FLAG_TERM;
	      handle_term_signal (fep);
	    }
	  if (signals & FEP_SIG_FLAG_WINCH)
	    {
	      signals &= ~FEP_SIG_FLAG_WINCH;
	      handle_winch_signal (fep);
	    }
	  if (signals & FEP_SIG_FLAG_TSTP)
	    {
	      signals &= ~FEP_SIG_FLAG_TSTP;
	      handle_tstp_signal (fep);
	    }
	  continue;
	}

      if (FD_ISSET(fep->tty_in, &fds))
	{
	  ssize_t bytes_read, i;

	  memset (buf, 0, sizeof(buf));
	  bytes_read = _fep_read (fep, buf, sizeof(buf) - 1);
	  if (bytes_read < 0)
	    {
	      fprintf (stderr, "Can't read from tty: %s\n",
		       strerror (errno));
	      retval = -1;
	      break;
	    }
	  if (bytes_read == 0)
	    break;

	  buf[bytes_read] = '\0';

	  for (i = 0; i < bytes_read; )
	    {
	      uint32_t keyval;
	      uint32_t state;
	      char *endptr;
	      bool is_key_read, is_key_handled;
	      FepCSI *csi;

	      is_key_read = false;
	      csi = _fep_csi_parse (buf + i, bytes_read - i, &endptr);
	      if (csi)
		{
		  if (_fep_csi_to_key (csi, &keyval, &state))
		    is_key_read = true;
		  _fep_csi_free (csi);
		}
	      else
		{
		  if (_fep_char_to_key (buf[i], &keyval, &state))
		    {
		      endptr = buf + i + 1;
		      is_key_read = true;
		    }
		}

	      is_key_handled = false;
	      if (is_key_read)
		{
		  FepControlMessage request;
		  int j;

		  request.command = FEP_CONTROL_KEY_EVENT;
		  _fep_control_message_alloc_args (&request, 2);
		  _fep_control_message_write_uint32_arg (&request,
							 0,
							 (uint32_t) keyval);
		  _fep_control_message_write_uint32_arg (&request,
							 1,
							 (uint32_t) state);
		  for (j = 0; j < fep->n_clients; j++)
		    {
		      FepControlMessage response;
		      if (fep->clients[j] < 0)
			continue;
		      if (_fep_transceive_control_message (fep->clients[j],
							   &request,
							   &response) == 0)
			{
			  uint32_t intval;
			  _fep_control_message_read_uint32_arg (&response,
								1,
								&intval);
			  if (intval > 0)
			    is_key_handled = true;
			  _fep_control_message_free_args (&response);
			}
		      FD_CLR (fep->clients[j], &fds);
		    }
		  _fep_control_message_free_args (&request);
		}
	      if (!is_key_handled)
		write (fep->pty, buf + i, endptr - (buf + i));
	      i += endptr - (buf + i);
	    }
	}

      /* input from pty (child process) */
      if (FD_ISSET(fep->pty, &fds))
	{
	  ssize_t bytes_read;
	  char *str1, *str2;

	  memset (buf, 0, sizeof(buf));
	  bytes_read = read (fep->pty, buf, sizeof(buf) - 1);
	  if (bytes_read <= 0)
	    /* ignore errors when reading from pty */
	    break;
	  buf[bytes_read] = '\0';

	  fep_log (FEP_LOG_LEVEL_DEBUG,
		   "pty read \"%s\"", buf);

	  str1 = find_match_end (buf,
				 bytes_read,
				 clear_screen,
				 strlen (clear_screen));
	  str2 = find_match_end (buf,
				 bytes_read,
				 clr_eos,
				 strlen (clr_eos));
	  if (str1 != NULL || str2 != NULL)
	    {
	      int str1_len;
	      if (str2 > str1)
		str1 = str2;
	      str1_len = bytes_read - (str1 - buf);
	      _fep_output_string_from_pty (fep, buf, bytes_read - str1_len);
	      _fep_output_status_text (fep,
				       fep->status_text,
				       &fep->status_text_attr);
	      _fep_output_string_from_pty (fep, str1, str1_len);
	    }
	  else
	    _fep_output_string_from_pty (fep, buf, bytes_read);
	}
      /* accept client connection via control socket */
      if (FD_ISSET(fep->server, &fds))
	{
	  int fd = accept (fep->server, NULL, NULL);
	  if (fd >= 0)
	    fep->clients[fep->n_clients++] = fd;
	}
      /* input from control socket */
      for (i = 0; i < fep->n_clients; i++)
	{
	  if (fep->clients[i] >= 0 && FD_ISSET(fep->clients[i], &fds))
	    _fep_dispatch_control_message (fep, fep->clients[i]);
	}
    }
  free (_clear_screen);
  free (_clr_eos);

  return retval;
}

void
fep_free (Fep *fep)
{
  int i;

  reset_signal_handler ();

  if (fep->pty >= 0)
    close (fep->pty);

  for (i = 0; i < fep->n_clients; i++)
    if (fep->clients[i] >= 0)
      close (fep->clients[i]);

  _fep_close_control_socket (fep);

  free (fep->cursor_text);
  free (fep->status_text);
  free (fep);
}
