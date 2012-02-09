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

#ifndef __LIBFEP_ATTRIBUTE_H__
#define __LIBFEP_ATTRIBUTE_H__

enum _FepAttrType
  {
    /* compatible with IBusAttrType */
    FEP_ATTR_UNDERLINE = 0,
    FEP_ATTR_FOREGROUND = 1,
    FEP_ATTR_BACKGROUND = 2,

    /* libfep specific */
    FEP_ATTR_NONE = 3,
    FEP_ATTR_STANDOUT = 4,
    FEP_ATTR_BOLD = 5,
    FEP_ATTR_BLINK = 6
  };

typedef enum _FepAttrType FepAttrType;

/* compatible with IBusAttrUnderline */
enum _FepAttrUnderline
  {
    FEP_ATTR_UNDERLINE_NONE = 0,
    FEP_ATTR_UNDERLINE_SINGLE = 1,
    FEP_ATTR_UNDERLINE_DOUBLE = 2,
    FEP_ATTR_UNDERLINE_LOW = 3,
    FEP_ATTR_UNDERLINE_ERROR = 4,
  };

typedef enum _FepAttrUnderline FepAttrUnderline;

struct _FepAttribute
{
  FepAttrType type;
  unsigned int value;
  unsigned int start_index;
  unsigned int end_index;
};

typedef struct _FepAttribute FepAttribute;

#endif
