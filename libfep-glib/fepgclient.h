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

#ifndef __LIBFEP_GOBJECT_FEP_GCLIENT_H__
#define __LIBFEP_GOBJECT_FEP_GCLIENT_H__

#include <gio/gio.h>

G_BEGIN_DECLS

#define FEP_TYPE_G_CLIENT (fep_g_client_get_type())
#define FEP_G_CLIENT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), FEP_TYPE_G_CLIENT, FepGClient))
#define FEP_G_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), FEP_TYPE_G_CLIENT, FepGClientClass))
#define FEP_IS_G_CLIENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FEP_TYPE_G_CLIENT))
#define FEP_IS_G_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FEP_TYPE_G_CLIENT))
#define FEP_G_CLIENT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), FEP_TYPE_G_CLIENT, FepGClientClass))

typedef struct _FepGClient FepGClient;
typedef struct _FepGClientPrivate FepGClientPrivate;
typedef struct _FepGClientClass FepGClientClass;

struct _FepGClient {
  /*< private >*/
  GObject parent;

  FepGClientPrivate *priv;
};

struct _FepGClientClass {
  /*< private >*/
  GObjectClass parent_class;

  /*< public >*/
  /* signals */
  gboolean (*filter_key_event) (FepGClient *client, guint keyval, guint modifiers);
  void (*process_key_event) (FepGClient *client, guint keyval, guint modifiers);

  /*< private >*/
  gpointer padding[32];
};

GType        fep_g_client_get_type              (void) G_GNUC_CONST;
FepGClient * fep_g_client_new                   (const char   *address,
                                                 GCancellable *cancellable,
                                                 GError      **error);
void         fep_g_client_set_cursor_text       (FepGClient   *client,
                                                 const char   *text);
void         fep_g_client_set_status_text       (FepGClient   *client,
                                                 const char   *text);
void         fep_g_client_send_data             (FepGClient   *client,
                                                 const char   *data,
                                                 gsize         length);
gint         fep_g_client_get_key_event_poll_fd (FepGClient   *client);
gboolean     fep_g_client_dispatch_key_event    (FepGClient   *client);

G_END_DECLS

#endif
