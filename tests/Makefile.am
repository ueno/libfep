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

noinst_PROGRAMS = status monitor

status_CFLAGS = -I$(top_srcdir)
status_LDADD = $(top_builddir)/libfep/libfep.la
status_SOURCES = status.c

monitor_CFLAGS = -I$(top_srcdir)
monitor_LDADD = $(top_builddir)/libfep/libfep.la
monitor_SOURCES = monitor.c

-include $(top_srcdir)/git.mk
