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

#ifndef __LIBFEP_CONTROL_H__
#define __LIBFEP_CONTROL_H__

typedef enum {
  FEP_CONTROL_SET_STATUS = 0,
  FEP_CONTROL_SET_TEXT = 1,
  FEP_CONTROL_KEY_EVENT = 2,
  FEP_CONTROL_KEY_EVENT_RESPONSE = 3,
} FepControlCommand;

#endif	/* __LIBFEP_CONTROL_H__ */
