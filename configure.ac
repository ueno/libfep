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

AC_PREREQ(2.63)
AC_CONFIG_MACRO_DIR([m4])

AC_INIT([libfep], [0.0.0], [ueno@unixuser.org])

FEP_API_VERSION=1.0
FEP_API_MAJOR_VERSION=1
FEP_API_MINOR_VERSION=0
FEP_API_PC_VERSION=1.0
FEP_LIBRARY_SUFFIX="-$FEP_API_VERSION"

AC_SUBST([FEP_API_VERSION])
AC_SUBST([FEP_API_MAJOR_VERSION])
AC_SUBST([FEP_API_MINOR_VERSION])
AC_SUBST([FEP_API_PC_VERSION])
AC_SUBST([FEP_LIBRARY_SUFFIX])
AC_SUBST([FEP_LIBRARY_SUFFIX_U],[AS_TR_SH([$FEP_LIBRARY_SUFFIX])])

dnl Init automake
AM_INIT_AUTOMAKE([color-tests])
AM_MAINTAINER_MODE([enable])
AC_GNU_SOURCE

dnl Check for programs
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CC_STDC
AC_PROG_INSTALL
AC_C_BIGENDIAN

dnl gnulib
gl_EARLY
gl_INIT

# define PACKAGE_VERSION_* variables
AM_DISABLE_STATIC
AC_ISC_POSIX
AC_HEADER_STDC
LT_INIT

# If library source has changed since last release, increment revision
# If public symbols have been added, removed or changed since last release,
#  increment current and set revision to 0
# If public symbols have been added since last release, increment age
# If public symbols have been removed since last release, set age to 0
m4_define([libfep_lt_current], [0])
m4_define([libfep_lt_revision], [0])
m4_define([libfep_lt_age], [0])
LT_CURRENT=libfep_lt_current
LT_REVISION=libfep_lt_revision
LT_AGE=libfep_lt_age
AC_SUBST([LT_CURRENT])
AC_SUBST([LT_REVISION])
AC_SUBST([LT_AGE])

PKG_CHECK_MODULES([NCURSES], [ncurses])

# check for gtk-doc
m4_ifdef([GTK_DOC_CHECK], [
GTK_DOC_CHECK([1.14],[--flavour no-tmpl])
],[
AM_CONDITIONAL([ENABLE_GTK_DOC], false)
])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile
lib/Makefile
fep/Makefile
libfep/Makefile
libfep/libfep.pc
tests/Makefile
docs/Makefile
docs/version.xml])
AC_OUTPUT
