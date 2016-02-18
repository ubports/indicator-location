/*
 * Copyright 2013-2016 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *   Charles Kerr <charles.kerr@canonical.com>
 */

#include <locale.h>
#include <glib/gi18n.h>
#include <glib.h>

#include "location-service-controller.h"
#include "service.h"

static void on_name_lost(Service* service G_GNUC_UNUSED, gpointer loop)
{
    g_main_loop_quit(static_cast<GMainLoop*>(loop));
}

int main(int argc G_GNUC_UNUSED, char** argv G_GNUC_UNUSED)
{
    GMainLoop* loop;

    /* boilerplate i18n */
    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, GNOMELOCALEDIR);
    textdomain(GETTEXT_PACKAGE);

    /* set up the service */
    loop = g_main_loop_new(nullptr, false);
    auto controller = std::make_shared<LocationServiceController>();
    Service service(controller);
    service.set_name_lost_callback(on_name_lost, loop);
    g_main_loop_run(loop);

    /* cleanup */
    g_main_loop_unref(loop);
    return 0;
}
