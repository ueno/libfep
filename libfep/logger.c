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

#include <libfep/private.h>
#include <unistd.h>
#include <libfep/libfep.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>
#include <assert.h>

/**
 * SECTION:logger
 * @short_description: Logging facility
 */

static FILE *log_fp;
static FepLogLevel log_level = FEP_LOG_LEVEL_WARNING;

void
fep_set_log_file (const char *file)
{
  if (log_fp != NULL)
    fclose (log_fp);
  log_fp = fopen (file, "w");
}

void
fep_set_log_fd (int fd)
{
  if (log_fp != NULL)
    fclose (log_fp);
  log_fp = fdopen (fd, "w");
}

int
fep_get_log_level (void)
{
  return log_level;
}

void
fep_set_log_level (FepLogLevel level)
{
  assert (level <= FEP_LOG_LEVEL_DEBUG);
  log_level = level;
}

void
fep_log (FepLogLevel level, const char *fmt, ...)
{
  static const char *levels[] =
    {
      "ERROR",
      "CRITICAL",
      "WARNING",
      "MESSAGE",
      "INFO",
      "DEBUG"
    };
  va_list ap;

  va_start (ap, fmt);
  if (log_fp != NULL && level <= log_level)
    {
      struct timeval tv;
      gettimeofday (&tv, NULL);
      fprintf (log_fp, "%lu.%lu %s ", tv.tv_sec, tv.tv_usec, levels[level]);
      vfprintf (log_fp, fmt, ap);
      fputc ('\n', log_fp);
      fflush (log_fp);
    }
  va_end (ap);
}
