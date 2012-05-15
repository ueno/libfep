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

#ifndef __LIBFEP_GOBJECT_FEP_GEVENT_H__
#define __LIBFEP_GOBJECT_FEP_GEVENT_H__

#include <glib-object.h>
#include <libfep/libfep.h>

G_BEGIN_DECLS

/**
 * FepGEventType:
 * @FEP_G_EVENT_TYPE_NOTHING: Nothing happend; used to indicate error
 * @FEP_G_EVENT_TYPE_KEY_PRESS: Key is pressed
 * @FEP_G_EVENT_TYPE_RESIZED: Window is resized
 */
typedef enum {
  FEP_G_EVENT_TYPE_NOTHING = -1,
  FEP_G_EVENT_TYPE_KEY_PRESS = 0,
  FEP_G_EVENT_TYPE_RESIZED = 1,
} FepGEventType;

typedef struct _FepGEventAny FepGEventAny;

/**
 * FepGEventAny:
 * @type: type of the event
 */
struct _FepGEventAny
{
  /*< public >*/
  FepGEventType type;
};

typedef struct _FepGEventKey FepGEventKey;

/**
 * FepGEventKey:
 * @type: type of the event
 * @keyval: keysym value
 * @modifiers: modifier mask
 * @source: original string which generated the event
 * @source_length: length of @source
 */
struct _FepGEventKey
{
  /*< public >*/
  FepGEventType type;
  guint keyval;
  guint modifiers;
  gchar *source;
  gsize source_length;
};

typedef struct _FepGEventResize FepGEventResize;

/**
 * FepGEventResize:
 * @type: type of the event
 * @cols: number of columns
 * @rows: number of rows
 */
struct _FepGEventResize
{
  /*< public >*/
  FepGEventType type;
  guint cols;
  guint rows;
};

typedef union _FepGEvent FepGEvent;

/**
 * FepGEvent:
 *
 * The #FepGEvent struct contains a union of all of the event structs,
 * and allows access to the data fields in a number of ways.
 */
union _FepGEvent
{
  FepGEventAny any;
  FepGEventKey key;
  FepGEventResize resize;
};

#define FEP_TYPE_G_EVENT fep_g_event_get_type ();

GType      fep_g_event_get_type (void) G_GNUC_CONST;

FepGEvent *fep_g_event_new      (FepGEventType    type);
FepGEvent *fep_g_event_copy     (const FepGEvent *event);
void       fep_g_event_free     (FepGEvent       *event);

G_END_DECLS

#endif
