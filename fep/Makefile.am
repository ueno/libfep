# Copyright (C) 2012 Daiki Ueno <ueno@unixuser.org>
# Copyright (C) 2012 Red Hat, Inc.

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

bin_PROGRAMS = fep

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/fep -I$(top_srcdir)/lib

fep_SOURCES =					\
	csi.c					\
	sgr.c					\
	cap.c					\
	key.c					\
	input.c					\
	output.c				\
	control.c				\
	fep.c					\
	main.c					\
	fep.h					\
	private.h				\
	$(NULL)

fep_CFLAGS = $(NCURSES_CFLAGS)
fep_LDADD =					\
	$(NCURSES_LIBS)				\
	$(PTY_LIB)				\
	$(LTLIBICONV)				\
	$(LIBSOCKET)				\
	$(LIB_SELECT)				\
	$(LIB_PTHREAD_SIGMASK)			\
	$(LTLIBTHREAD)				\
	$(top_builddir)/lib/liblibfep.la	\
	$(top_builddir)/libfep/libfep.la	\
	$(NULL)

dist_man_MANS = fep.1

EXTRA_DIST = README COPYING.BSD

-include $(top_srcdir)/git.mk
