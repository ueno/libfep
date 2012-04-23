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
#include <libfep-glib/fepgmarshalers.h>
#include <libfep/libfep.h>

/**
 * SECTION:fepgclient
 * @short_description: Client connection to FEP server
 */

enum
  {
    PROP_0,
    PROP_ADDRESS
  };

enum
  {
    FILTER_KEY_EVENT_SIGNAL,
    RESIZED_SIGNAL,
    LAST_SIGNAL
  };

static guint signals[LAST_SIGNAL] = {0};

#define I_(string) g_intern_static_string (string)

static void initable_iface_init (GInitableIface *initable_iface);

G_DEFINE_TYPE_WITH_CODE (FepGClient, fep_g_client, G_TYPE_OBJECT,
			 G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE, initable_iface_init));

#define FEP_G_CLIENT_GET_PRIVATE(obj)                                \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), FEP_TYPE_G_CLIENT, FepGClientPrivate))

struct _FepGClientPrivate
{
  FepClient *client;
  char *address;
};

static int
event_filter (FepEvent *event,
              void     *data)
{
  FepGClient *client = FEP_G_CLIENT (data);
  gboolean retval = FALSE;

  switch (event->type)
    {
    case FEP_KEY_PRESS:
      {
	FepEventKey *_event = (FepEventKey *)event;
	g_signal_emit (client, signals[FILTER_KEY_EVENT_SIGNAL], 0,
		       _event->keyval, _event->modifiers, &retval);
      }
      break;
    case FEP_RESIZED:
      {
	FepEventResize *_event = (FepEventResize *)event;
	g_signal_emit (client, signals[RESIZED_SIGNAL], 0,
		       _event->cols, _event->rows);
      }
      break;
    default:
      break;
    }
  return retval ? 1 : 0;
}

static gboolean
initable_init (GInitable    *initable,
               GCancellable *cancellable,
               GError      **error)
{
  FepGClient *client = FEP_G_CLIENT (initable);
  FepGClientPrivate *priv = FEP_G_CLIENT_GET_PRIVATE (client);

  priv->client = fep_client_open (priv->address);
  if (priv->client)
    {
      fep_client_set_event_filter (priv->client,
				   (FepEventFilter) event_filter,
				   client);
#ifdef DEBUG
      fep_set_log_file ("fepgclient.log");
      fep_set_log_level (FEP_LOG_LEVEL_DEBUG);
#endif
      return TRUE;
    }
  return FALSE;
}

static void
initable_iface_init (GInitableIface *initable_iface)
{
  initable_iface->init = initable_init;
}

static gboolean
fep_g_client_real_filter_key_event (FepGClient *client,
				    guint       keyval,
				    guint       modifiers)
{
  /* g_debug ("%u %u", keyval, modifiers); */
  return FALSE;
}

static void
fep_g_client_real_resized (FepGClient *client,
			   guint       cols,
			   guint       rows)
{
  /* g_debug ("%u %u", cols, rows); */
}

static void
fep_g_client_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  FepGClient *client = FEP_G_CLIENT (object);
  FepGClientPrivate *priv = FEP_G_CLIENT_GET_PRIVATE (client);

  switch (prop_id)
    {
    case PROP_ADDRESS:
      priv->address = g_value_dup_string (value);
      break;
    default:
      g_object_set_property (object,
			     g_param_spec_get_name (pspec),
			     value);
      break;
    }
}

static void
fep_g_client_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  FepGClient *client = FEP_G_CLIENT (object);
  FepGClientPrivate *priv = FEP_G_CLIENT_GET_PRIVATE (client);

  switch (prop_id)
    {
    case PROP_ADDRESS:
      g_value_set_string (value, priv->address);
      break;
    default:
      g_object_get_property (object,
			     g_param_spec_get_name (pspec),
			     value);
      break;
    }
}

static void
fep_g_client_finalize (GObject *object)
{
  FepGClient *client = FEP_G_CLIENT (object);
  FepGClientPrivate *priv = FEP_G_CLIENT_GET_PRIVATE (client);

  if (priv->client)
    fep_client_close (priv->client);
  g_free (priv->address);

  G_OBJECT_CLASS (fep_g_client_parent_class)->finalize (object);
}

static void
fep_g_client_class_init (FepGClientClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GParamSpec *pspec;

  g_type_class_add_private (gobject_class,
			    sizeof (FepGClientPrivate));

  klass->filter_key_event = fep_g_client_real_filter_key_event;
  klass->resized = fep_g_client_real_resized;

  gobject_class->set_property = fep_g_client_set_property;
  gobject_class->get_property = fep_g_client_get_property;
  gobject_class->finalize = fep_g_client_finalize;

  /**
   * FepGClient::filter-key-event:
   * @client: a #FepGClient
   * @keyval: a keyval
   * @modifiers: modifier mask
   *
   * The ::filter-key-event signal is emitted when key event is dispatched.
   */
  signals[FILTER_KEY_EVENT_SIGNAL] =
    g_signal_new (I_("filter-key-event"),
		  G_TYPE_FROM_CLASS(gobject_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET(FepGClientClass, filter_key_event),
		  g_signal_accumulator_true_handled,
		  NULL,
		  _fep_g_marshal_BOOLEAN__UINT_UINT,
		  G_TYPE_BOOLEAN,
		  2,
		  G_TYPE_UINT,
		  G_TYPE_UINT);

  /**
   * FepGClient::resized:
   * @client: a #FepGClient
   * @cols: number of columns
   * @rows: number of rows
   *
   * The ::resized signal is emitted when terminal is resized.
   */
  signals[RESIZED_SIGNAL] =
    g_signal_new (I_("resized"),
		  G_TYPE_FROM_CLASS(gobject_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET(FepGClientClass, resized),
		  NULL,
		  NULL,
		  _fep_g_marshal_VOID__UINT_UINT,
		  G_TYPE_NONE,
		  2,
		  G_TYPE_UINT,
		  G_TYPE_UINT);

  pspec = g_param_spec_string ("address",
			       "address",
			       "FEP control socket address",
			       NULL,
			       G_PARAM_READABLE |
			       G_PARAM_WRITABLE |
			       G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_property (gobject_class,
				   PROP_ADDRESS,
				   pspec);
}

static void
fep_g_client_init (FepGClient *client)
{
  client->priv = FEP_G_CLIENT_GET_PRIVATE (client);
}

/**
 * fep_g_client_new:
 * @address: (allow-none): socket address of the FEP server
 * @cancellable: a #GCancellable or %NULL
 * @error: a pointer to a NULL #GError, or %NULL
 *
 * Connect to the FEP server running at @address.  If @address is
 * %NULL, it gets the address from the environment variable
 * `LIBFEP_CONTROL_SOCK`.
 *
 * Returns: a new #FepGClient.
 */
FepGClient *
fep_g_client_new (const char   *address,
                  GCancellable *cancellable,
                  GError      **error)
{
  return g_initable_new (FEP_TYPE_G_CLIENT,
			 cancellable,
			 error,
			 "address", address,
			 NULL);
}

/**
 * fep_g_client_set_cursor_text:
 * @client: a #FepGClient
 * @text: a cursor text
 * @attr: (allow-none): a #FepGAttribute
 *
 * Request to display @text at the cursor position on the terminal.
 */
void
fep_g_client_set_cursor_text (FepGClient    *client,
                              const char    *text,
                              FepGAttribute *attr)
{
  FepGClientPrivate *priv = FEP_G_CLIENT_GET_PRIVATE (client);
  fep_client_set_cursor_text (priv->client,
			      text,
			      (FepAttribute *) attr);
}

/**
 * fep_g_client_set_status_text:
 * @client: a #FepGClient
 * @text: a status text
 * @attr: (allow-none): a #FepGAttribute
 *
 * Request to display @text at the bottom of the terminal.
 */
void
fep_g_client_set_status_text (FepGClient    *client,
                              const char    *text,
                              FepGAttribute *attr)
{
  FepGClientPrivate *priv = FEP_G_CLIENT_GET_PRIVATE (client);
  fep_client_set_status_text (priv->client,
			      text,
			      (FepAttribute *) attr);
}

/**
 * fep_g_client_send_text:
 * @client: a #FepGClient
 * @text: text to be sent
 *
 * Request to send @data to the child process of the FEP server.
 * @text will be converted from UTF-8 to the local encoding in the
 * server.
 */
void
fep_g_client_send_text (FepGClient *client,
                        const char *text)
{
  FepGClientPrivate *priv = FEP_G_CLIENT_GET_PRIVATE (client);
  fep_client_send_text (priv->client, text);
}

/**
 * fep_g_client_send_data:
 * @client: a #FepGClient
 * @data: data to be sent
 * @length: length of @data
 *
 * Request to send @data to the child process of the FEP server.
 */
void
fep_g_client_send_data (FepGClient *client,
                        const char *data,
                        gsize       length)
{
  FepGClientPrivate *priv = FEP_G_CLIENT_GET_PRIVATE (client);
  fep_client_send_data (priv->client, data, length);
}

/**
 * fep_g_client_forward_key_event:
 * @client: a #FepGClient
 * @keyval: keysym value
 * @modifiers: modifiers
 *
 * Request to send a key event to the child process of the FEP server.
 */
void
fep_g_client_forward_key_event (FepGClient *client,
                                guint       keyval,
                                guint       modifiers)
{
  FepGClientPrivate *priv = FEP_G_CLIENT_GET_PRIVATE (client);
  fep_client_forward_key_event (priv->client, keyval, modifiers);
}

/**
 * fep_g_client_get_poll_fd:
 * @client: a #FepGClient
 *
 * Get the file descriptor of the control socket which can be used by poll().
 *
 * Returns: a file descriptor
 */
gint
fep_g_client_get_poll_fd (FepGClient *client)
{
  FepGClientPrivate *priv = FEP_G_CLIENT_GET_PRIVATE (client);
  return fep_client_get_poll_fd (priv->client);
}

/**
 * fep_g_client_dispatch:
 * @client: a #FepGClient
 *
 * Dispatch a key event.
 *
 * Returns: %TRUE if success, %FALSE on error.
 */
gboolean
fep_g_client_dispatch (FepGClient *client)
{
  FepGClientPrivate *priv = FEP_G_CLIENT_GET_PRIVATE (client);
  gint retval = fep_client_dispatch (priv->client);
  return retval == 0;
}
