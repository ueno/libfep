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
 * @FEP_G_ATTR_TYPE_UNDERLINE: Decorate with underline
 * @FEP_G_ATTR_TYPE_FOREGROUND: Foreground color
 * @FEP_G_ATTR_TYPE_BACKGROUND: Background color
 * @FEP_G_ATTR_TYPE_NONE: No attribute
 * @FEP_G_ATTR_TYPE_STANDOUT: Reverse video
 * @FEP_G_ATTR_TYPE_BOLD: Bold
 * @FEP_G_ATTR_TYPE_BLINK: Blink
 */
typedef enum {
  /* compatible with IBusAttrType */
  FEP_G_ATTR_TYPE_UNDERLINE = 0,
  FEP_G_ATTR_TYPE_FOREGROUND = 1,
  FEP_G_ATTR_TYPE_BACKGROUND = 2,

  /* libfep specific */
  FEP_G_ATTR_TYPE_NONE = 3,
  FEP_G_ATTR_TYPE_STANDOUT = 4,
  FEP_G_ATTR_TYPE_BOLD = 5,
  FEP_G_ATTR_TYPE_BLINK = 6
} FepGAttrType;

/**
 * FepGAttrUnderline:
 * @FEP_G_ATTR_UNDERLINE_NONE: No underline
 * @FEP_G_ATTR_UNDERLINE_SINGLE: Single underline
 * @FEP_G_ATTR_UNDERLINE_DOUBLE: Double underline
 * @FEP_G_ATTR_UNDERLINE_LOW: Low underline? FIXME
 * @FEP_G_ATTR_UNDERLINE_ERROR: Error underline
 */
typedef enum
  {
    /* compatible with IBusAttrUnderline */
    FEP_G_ATTR_UNDERLINE_NONE = 0,
    FEP_G_ATTR_UNDERLINE_SINGLE = 1,
    FEP_G_ATTR_UNDERLINE_DOUBLE = 2,
    FEP_G_ATTR_UNDERLINE_LOW = 3,
    FEP_G_ATTR_UNDERLINE_ERROR = 4,
  } FepGAttrUnderline;

typedef struct _FepGAttribute FepGAttribute;

/**
 * FepGAttribute:
 * @type: type of the attribute
 * @value: value of the attribute
 * @start_index: starting position of the attribute
 * @end_index: end position (exclusive) of the attribute
 */
struct _FepGAttribute
{
  /*< public >*/
  FepGAttrType type;
  guint value;
  guint start_index;
  guint end_index;
};

#define FEP_TYPE_G_ATTRIBUTE fep_g_attribute_get_type ();

GType fep_g_attribute_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif
