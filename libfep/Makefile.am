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

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

lib_LTLIBRARIES = libfep.la
libfep_la_LDFLAGS =							\
	-version-info "$(LT_CURRENT)":"$(LT_REVISION)":"$(LT_AGE)"	\
        -export-dynamic							\
        -export-symbols-regex "^(fep|_fep)"				\
	$(NULL)

libfep_la_SOURCES = string.c list.c control.c client.c logger.c
libfep_la_LIBADD = $(top_builddir)/lib/libgnu.la

libfepincludedir = $(includedir)/fep-@FEP_API_VERSION@/libfep
libfepinclude_HEADERS = libfep.h keydefs.h client.h logger.h attribute.h
noinst_HEADERS = private.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libfep.pc

-include $(top_srcdir)/git.mk
