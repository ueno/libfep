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

#ifndef __LIBFEP_LOGGER_H__
#define __LIBFEP_LOGGER_H__

typedef enum
  {
    FEP_LOG_LEVEL_ERROR = 0,
    FEP_LOG_LEVEL_CRITICAL,
    FEP_LOG_LEVEL_WARNING,
    FEP_LOG_LEVEL_MESSAGE,
    FEP_LOG_LEVEL_INFO,
    FEP_LOG_LEVEL_DEBUG
  } FepLogLevel;

void fep_set_log_file  (const char *file);
void fep_set_log_fd    (int fd);
int  fep_get_log_level (void);
void fep_set_log_level (FepLogLevel level);
void fep_log           (FepLogLevel level,
                        const char *fmt,
                        ...);

#endif	/* __LIBFEP_LOGGER_H__ */
