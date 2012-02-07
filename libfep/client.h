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

#ifndef __LIBFEP_CLIENT_H__
#define __LIBFEP_CLIENT_H__

enum _FepEventType
  {
    FEP_NOTHING = -1,
    FEP_KEY_PRESS = 0,
    FEP_RESIZED = 1
  };

typedef enum _FepEventType FepEventType;

struct _FepEvent
{
  FepEventType type;
};
typedef struct _FepEvent FepEvent;

struct _FepEventKey
{
  FepEvent event;
  unsigned int keyval;
  FepModifierType modifiers;
};
typedef struct _FepEventKey FepEventKey;

struct _FepEventResize
{
  FepEvent event;
  unsigned int cols;
  unsigned int rows;
};
typedef struct _FepEventResize FepEventResize;

typedef struct _FepClient FepClient;
typedef int (*FepEventFilter) (FepEvent *event, void *data);

FepClient *fep_client_open             (const char    *address);
int        fep_client_get_poll_fd      (FepClient     *client);
void       fep_client_set_cursor_text  (FepClient     *client,
                                        const char    *text);
void       fep_client_set_status_text  (FepClient     *client,
                                        const char    *text);
void       fep_client_send_text        (FepClient     *client,
                                        const char    *text);
void       fep_client_send_data        (FepClient     *client,
                                        const char    *data,
                                        size_t         data_len);
void       fep_client_set_event_filter (FepClient     *client,
                                        FepEventFilter filter,
                                        void          *data);
int        fep_client_dispatch         (FepClient     *client);
void       fep_client_close            (FepClient     *client);

#endif	/* __LIBFEP_CLIENT_H__ */
