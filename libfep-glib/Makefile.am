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

lib_LTLIBRARIES = libfep-glib.la

libfep_glib_la_sources = fepgclient.c
libfep_glib_la_SOURCES = fepgmarshalers.c $(libfep_glib_la_sources)
libfep_glib_la_CFLAGS = $(GIO_CFLAGS) -I$(top_srcdir)
libfep_glib_la_LIBADD = $(GIO_LIBS) $(top_builddir)/libfep/libfep.la

libfepglibincludedir = $(includedir)/libfep-glib
libfepglibinclude_HEADERS = libfep-glib.h fepgclient.h
noinst_HEADERS = fepgmarshalers.h

# gen marshal
fepgmarshalers.h: fepgmarshalers.list
	$(AM_V_GEN) $(GLIB_GENMARSHAL) --prefix=_fep_g_marshal $(srcdir)/fepgmarshalers.list --header --internal > $@.tmp && \
	mv $@.tmp $@

fepgmarshalers.c: fepgmarshalers.list fepgmarshalers.h
	$(AM_V_GEN) (echo "#include \"fepgmarshalers.h\""; \
	$(GLIB_GENMARSHAL) --prefix=_fep_g_marshal $(srcdir)/fepgmarshalers.list --body --internal) > $@.tmp && \
	mv $@.tmp $@

DISTCLEANFILES = fepgmarshalers.h fepgmarshalers.c

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libfep-glib.pc

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(builddir)
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

CLEANFILES =

if HAVE_INTROSPECTION
Fep@FEP_LIBRARY_SUFFIX@.gir: libfep-glib.la
Fep@FEP_LIBRARY_SUFFIX_U@_gir_SCANNERFLAGS = --pkg=glib-2.0
Fep@FEP_LIBRARY_SUFFIX_U@_gir_INCLUDES = GLib-2.0 GObject-2.0 Gio-2.0
Fep@FEP_LIBRARY_SUFFIX_U@_gir_CFLAGS = $(libfep_glib_la_CFLAGS)
Fep@FEP_LIBRARY_SUFFIX_U@_gir_LIBS = libfep-glib.la
Fep@FEP_LIBRARY_SUFFIX_U@_gir_FILES = $(libfep_glib_la_sources) $(libfepglibinclude_HEADERS)

INTROSPECTION_GIRS += Fep@FEP_LIBRARY_SUFFIX@.gir
girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)
endif

-include $(top_srcdir)/git.mk
