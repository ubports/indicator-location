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

#include <service.h>

/***
****
***/

void
Service :: on_name_lost (GDBusConnection * connection,
                         const char * name,
                         gpointer user_data)
{
  static_cast<Service*>(user_data)->on_name_lost (connection, name);
}

void
Service :: on_name_lost (GDBusConnection * connection,
                         const char * name)
{
  g_message ("on name lost: %s", name);
}

/***
****
***/

void
Service :: on_bus_acquired (GDBusConnection * connection,
                            const char * name,
                            gpointer user_data)
{
  static_cast<Service*>(user_data)->on_bus_acquired (connection, name);
}

void
Service :: on_bus_acquired (GDBusConnection * connection,
                            const char * name)
{
  g_assert (this->connection == 0);

  g_message ("on name acquired: %s", name);

  this->connection = connection;

  g_message ("FIXME: export the actions/menus");
}

/***
****
***/

Service :: Service ():
  connection (0),
  own_id (0)
{
  g_message ("%s %s", G_STRLOC, G_STRFUNC);

  own_id = g_bus_own_name (G_BUS_TYPE_SESSION,
                           "com.canonical.indicator.location",
                           G_BUS_NAME_OWNER_FLAGS_NONE,
                           on_bus_acquired,
                           NULL,
                           on_name_lost,
                           this,
                           NULL);
}

Service :: ~Service ()
{
  g_message ("%s %s", G_STRLOC, G_STRFUNC);
  g_bus_unown_name (own_id);
}
