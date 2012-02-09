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

#ifndef __LIBFEP_GOBJECT_FEP_GATTRIBUTE_H__
#define __LIBFEP_GOBJECT_FEP_GATTRIBUTE_H__

#include <glib-object.h>
#include <libfep/libfep.h>

G_BEGIN_DECLS

/**
 * FepGAttrType:
 * @FEP_G_ATTR_UNDERLINE: underline
 */
typedef enum {
  /* compatible with IBusAttrType */
  FEP_G_ATTR_UNDERLINE = 0,
  FEP_G_ATTR_FOREGROUND = 1,
  FEP_G_ATTR_BACKGROUND = 2,

  /* libfep specific */
  FEP_G_ATTR_NONE = 3,
  FEP_G_ATTR_STANDOUT = 4,
  FEP_G_ATTR_BOLD = 5,
  FEP_G_ATTR_BLINK = 6
} FepGAttrType;

/* compatible with IBusAttrUnderline */
typedef enum
  {
    FEP_G_ATTR_UNDERLINE_NONE = 0,
    FEP_G_ATTR_UNDERLINE_SINGLE = 1,
    FEP_G_ATTR_UNDERLINE_DOUBLE = 2,
    FEP_G_ATTR_UNDERLINE_LOW = 3,
    FEP_G_ATTR_UNDERLINE_ERROR = 4,
  } FepGAttrUnderline;

struct FepGAttribute
{
  /*< public >*/
  FepGAttrType type;
  guint value;
  guint start_index;
  guint end_index;
};

typedef struct FepGAttribute FepGAttribute;

#define FEP_TYPE_G_ATTRIBUTE fep_g_attribute_get_type ();

GType fep_g_attribute_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif
