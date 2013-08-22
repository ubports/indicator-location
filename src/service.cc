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

#include <glib/gi18n.h>
#include <gio/gio.h>

#include "service.h"

#define BUS_NAME "com.canonical.indicator.location"
#define BUS_PATH "/com/canonical/indicator/location"

/**
***
**/

Service :: Service ():
  action_group (g_simple_action_group_new ()),
  connection (0),
  phone_profile (G_ACTION_MAP (action_group)),
  name_lost_callback (0),
  name_lost_user_data (0),
  action_group_export_id (0),
  bus_own_id (0)
{
  bus_own_id = g_bus_own_name (G_BUS_TYPE_SESSION,
                               BUS_NAME,
                               G_BUS_NAME_OWNER_FLAGS_NONE,
                               on_bus_acquired,
                               NULL,
                               on_name_lost,
                               this,
                               NULL);
}

Service :: ~Service ()
{
  if (connection != 0)
    {
      unexport ();
      g_object_unref (connection);
    }

  if (bus_own_id != 0)
    g_bus_unown_name (bus_own_id);

  g_object_unref (action_group);
}

void
Service :: set_name_lost_callback (name_lost_callback_func callback, void* user_data)
{
  name_lost_callback = callback;
  name_lost_user_data = user_data;
}

void
Service :: unexport ()
{
  g_return_if_fail (connection != 0);

  // unexport the menu(s)
  for (auto& id : exported_menus)
    g_dbus_connection_unexport_menu_model (connection, id);
  exported_menus.clear ();

  // unexport the action group
  if (action_group_export_id != 0)
    {
      g_dbus_connection_unexport_action_group (connection, action_group_export_id);
      action_group_export_id = 0;
    }
}

/***
****  GDBus
***/

void
Service :: on_name_lost (GDBusConnection * connection,
                         const char      * name,
                         gpointer          gself)
{
  static_cast<Service*>(gself)->on_name_lost (connection, name);
}
void
Service :: on_name_lost (GDBusConnection * connection,
                         const char      * name)
{
  g_debug ("%s::%s: %s %p", G_STRLOC, G_STRFUNC, name, connection);

  if (name_lost_callback != 0)
    (name_lost_callback)(this, name_lost_user_data);
}

void
Service :: on_bus_acquired (GDBusConnection * connection,
                            const char      * name,
                            gpointer          gself)
{
  static_cast<Service*>(gself)->on_bus_acquired (connection, name);
}
void
Service :: on_bus_acquired (GDBusConnection * connection,
                            const char * name)
{
  g_debug ("%s::%s: %s %p", G_STRLOC, G_STRFUNC, name, connection);

  connection = G_DBUS_CONNECTION (g_object_ref (connection));

  GError * error = 0;

  /* export the action group */

  unsigned int export_id = g_dbus_connection_export_action_group (connection,
                                                                  BUS_PATH,
                                                                  G_ACTION_GROUP (action_group),
                                                                  &error);
  if (error != 0)
    {
      g_warning ("Unable to export action group: %s", error->message);
      g_clear_error (&error);
    }
  else
    {
      action_group_export_id = export_id;
    }

  /* export the menu(s) */

  struct {
    GMenu * menu;
    const char * path;
  } menus[] = {
    { phone_profile.get_menu(), BUS_PATH "/phone" }
  };

  for (unsigned int i=0, n=G_N_ELEMENTS(menus); i<n; i++)
    { 
      export_id = g_dbus_connection_export_menu_model (connection,
                                                       menus[i].path,
                                                       G_MENU_MODEL (menus[i].menu),
                                                       &error);
      if (error != 0)
        {
          g_warning ("Unable to export phone menu: %s", error->message);
          g_clear_error (&error);
        }
      else
        {
          exported_menus.insert (export_id);
        }

      g_object_unref (menus[i].menu);
    }
}

