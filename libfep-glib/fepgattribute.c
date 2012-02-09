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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libfep-glib/libfep-glib.h>

static FepGAttribute * fep_g_attribute_copy (const FepGAttribute *attr);
static void            fep_g_attribute_free (FepGAttribute       *attr);

G_DEFINE_BOXED_TYPE (FepGAttribute,
		     fep_g_attribute,
		     fep_g_attribute_copy,
		     fep_g_attribute_free);

static FepGAttribute *
fep_g_attribute_copy (const FepGAttribute *attr)
{
  return g_slice_dup (FepGAttribute, attr);
}

static void
fep_g_attribute_free (FepGAttribute *attr)
{
  g_slice_free (FepGAttribute, attr);
}
