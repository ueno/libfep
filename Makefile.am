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

SUBDIRS = lib libfep fep tools

if ENABLE_GLIB
SUBDIRS += libfep-glib
endif

if ENABLE_GTK_DOC
SUBDIRS += docs
endif

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc --disable-vapigen

BUILT_SOURCES = $(top_srcdir)/.version

ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST = m4/gnulib-cache.m4 bootstrap bootstrap.conf $(BUILT_SOURCES)

$(top_srcdir)/.version:
	echo $(VERSION) > $@-t && mv $@-t $@

# Arrange so that .tarball-version appears only in the distribution
# tarball, and never in a checked-out repository.
# The perl substitution is to change some key uses of "rm" to "/bin/rm".
# See the rm_subst comment for details.
dist-hook: gen-ChangeLog
	$(AM_V_GEN)echo $(VERSION) > $(distdir)/.tarball-version
	$(AM_V_at)perl -pi -e '$(rm_subst)' $(distdir)/Makefile.in

gen_start_date = 2014-04-12
.PHONY: gen-ChangeLog
gen-ChangeLog:
	$(AM_V_GEN)if test -d .git; then				\
	  $(top_srcdir)/build-aux/gitlog-to-changelog			\
	    --amend=$(srcdir)/build-aux/git-log-fix			\
	    --since=$(gen_start_date) > $(distdir)/cl-t;		\
	  rm -f $(distdir)/ChangeLog;					\
	  mv $(distdir)/cl-t $(distdir)/ChangeLog;			\
	fi

GITIGNOREFILES =				\
	INSTALL					\
	aclocal.m4				\
	compile					\
	config.guess				\
	config.h.in				\
	config.sub				\
	depcomp					\
	install-sh				\
	ltmain.sh				\
	missing					\
	mkinstalldirs				\
	lib					\
	snippet					\
	gtk-doc.make				\
	`find "m4" -type f -name "*.m4" ! -name "vapigen.m4" -print` \
	$(top_builddir)/lcov.html/*		\
	$(NULL)

distclean-local:
	if test "x$(srcdir)" = "x."; then :; else \
		rm -f ChangeLog; \
	fi

MAINTAINERCLEANFILES =				\
	$(srcdir)/ChangeLog			\
	$(NULL)

ChangeLog:
	@echo Creating $@
	@if test -d "$(srcdir)/.git"; then \
	  GIT_DIR=$(top_srcdir)/.git $(top_srcdir)/gitlog-to-changelog > $@.tmp \
	  && mv -f $@.tmp $@ \
	  || ($(RM) $@.tmp; \
	      echo Failed to generate ChangeLog, your ChangeLog may be outdated >&2; \
	      (test -f $@ || echo git-log is required to generate this file >> $@)); \
	else \
	  test -f $@ || \
	  (echo A git checkout and git-log is required to generate ChangeLog >&2 && \
	  echo A git checkout and git-log is required to generate this file >> $@); \
	fi

.PHONY: ChangeLog

-include $(top_srcdir)/git.mk
