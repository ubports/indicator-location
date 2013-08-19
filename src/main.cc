/*
 * Copyright 2013 Canonical Ltd.
 *
 * Authors:
 *   Charles Kerr <charles.kerr@canonical.com>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <locale.h>

#include <glib/gi18n.h>
#include <glib.h>

#include "service.h"

/***
****
***/

static gboolean
on_idle (gpointer unused G_GNUC_UNUSED)
{
  GMainContext * context = g_main_context_default ();
  g_message ("hello world %p", context);
  return G_SOURCE_CONTINUE;
};

int
main (int argc G_GNUC_UNUSED, char ** argv G_GNUC_UNUSED)
{
  GMainLoop * loop;
  IndicatorLocationService * service;

  /* boilerplate i18n */
  setlocale (LC_ALL, "");
  bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
  textdomain (GETTEXT_PACKAGE);
 
  /* set up the service */ 
  loop = g_main_loop_new (NULL, false);
  service = indicator_location_service_new ();
  g_signal_connect_swapped (service, INDICATOR_LOCATION_SERVICE_SIGNAL_NAME_LOST,
                            G_CALLBACK(g_main_loop_quit), loop);

  /* run */
  g_timeout_add_seconds (2, on_idle, NULL);
  g_main_loop_run (loop);

  /* cleanup */
  g_object_unref (service);
  g_main_loop_unref (loop);
  return 0;
}
