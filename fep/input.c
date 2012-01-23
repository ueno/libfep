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
#include <string.h>
#include <stdlib.h>

ssize_t
_fep_read (Fep *fep, void *buf, size_t count)
{
  if (fep->ttybuf.len > 0)
    {
      size_t _count = MIN(count, fep->ttybuf.len);
      memcpy (buf, fep->ttybuf.str, _count);
      fep->ttybuf.len -= _count;
      memmove (fep->ttybuf.str, fep->ttybuf.str + _count, fep->ttybuf.len);
      return count;
    }
  return read (fep->tty_in, buf, count);
}

int
_fep_pselect (Fep *fep, fd_set *fds, sigset_t *sigmask)
{
  FD_ZERO(fds);

  if (fep->ttybuf.len > 0)
    {
      FD_SET(fep->tty_in, fds);
      return 1;
    }
  else
    {
      int nfds = 0, i;
      FD_SET(fep->tty_in, fds);
      nfds = MAX(nfds, fep->tty_in);
      FD_SET(fep->pty, fds);
      nfds = MAX(nfds, fep->pty);
      FD_SET(fep->server, fds);
      nfds = MAX(nfds, fep->server);
      for (i = 0; i < fep->n_clients; i++)
	if (fep->clients[i] >= 0)
	  {
	    FD_SET(fep->clients[i], fds);
	    nfds = MAX(nfds, fep->clients[i]);
	  }
      return pselect (nfds + 1, fds, NULL, NULL, NULL, sigmask);
    }
}
