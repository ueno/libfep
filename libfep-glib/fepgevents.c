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

/**
 * SECTION:fepgevent
 * @short_description: Events
 */

G_DEFINE_BOXED_TYPE (FepGEvent,
		     fep_g_event,
		     fep_g_event_copy,
		     fep_g_event_free);

FepGEvent *
fep_g_event_new (FepGEventType type)
{
  FepGEvent *event = g_slice_new0 (FepGEvent);
  event->any.type = type;
  return event;
}

FepGEvent *
fep_g_event_copy (const FepGEvent *event)
{
  FepGEvent *new_event = g_slice_dup (FepGEvent, event);
  switch (event->any.type)
    {
    case FEP_G_EVENT_TYPE_NOTHING:
      break;
    case FEP_G_EVENT_TYPE_KEY_PRESS:
      new_event->key.source = g_memdup (event->key.source,
					event->key.source_length);
      break;
    case FEP_G_EVENT_TYPE_RESIZED:
      break;
    }
  return new_event;
}

void
fep_g_event_free (FepGEvent *event)
{
  switch (event->any.type)
    {
    case FEP_G_EVENT_TYPE_NOTHING:
      break;
    case FEP_G_EVENT_TYPE_KEY_PRESS:
      g_free (event->key.source);
      break;
    case FEP_G_EVENT_TYPE_RESIZED:
      break;
    }
  g_slice_free (FepGEvent, event);
}
