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

#include "dbus-shared.h"
#include "service.h"

/**
***
**/

Service :: Service (const std::shared_ptr<Controller>& controller):
  action_group (g_simple_action_group_new(), GObjectDeleter()),
  phone_profile (controller, action_group),
  name_lost_callback (nullptr),
  name_lost_user_data (0),
  action_group_export_id (0),
  bus_own_id (0)
{
  bus_own_id = g_bus_own_name (G_BUS_TYPE_SESSION,
                               INDICATOR_BUS_NAME,
                               G_BUS_NAME_OWNER_FLAGS_NONE,
                               on_bus_acquired,
                               nullptr,
                               on_name_lost,
                               this,
                               nullptr);
}

Service :: ~Service ()
{
  if (connection)
    unexport ();

  if (bus_own_id != 0)
    g_bus_unown_name (bus_own_id);
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
  g_return_if_fail (connection);

  // unexport the menu(s)
  for (auto& id : exported_menus)
    g_dbus_connection_unexport_menu_model (connection.get(), id);
  exported_menus.clear ();

  // unexport the action group
  if (action_group_export_id != 0)
    {
      g_dbus_connection_unexport_action_group (connection.get(), action_group_export_id);
      action_group_export_id = 0;
    }
}

/***
****  GDBus
***/

void
Service :: on_name_lost (GDBusConnection * conn,
                         const char      * name,
                         gpointer          gself)
{
  g_debug ("%s::%s: %s %p", G_STRLOC, G_STRFUNC, name, conn);

  static_cast<Service*>(gself)->on_name_lost (conn, name);
}
void
Service :: on_name_lost (GDBusConnection * conn,
                         const char      * name)
{
  g_debug ("%s::%s: %s %p", G_STRLOC, G_STRFUNC, name, conn);

  if (name_lost_callback != nullptr)
    (name_lost_callback)(this, name_lost_user_data);
}

void
Service :: on_bus_acquired (GDBusConnection * conn,
                            const char      * name,
                            gpointer          gself)
{
  static_cast<Service*>(gself)->on_bus_acquired (conn, name);
}
void
Service :: on_bus_acquired (GDBusConnection * conn,
                            const char * name)
{
  g_debug ("%s::%s: %s %p", G_STRLOC, G_STRFUNC, name, conn);

  this->connection.reset (G_DBUS_CONNECTION (g_object_ref(conn)));

  GError * error = nullptr;

  /* export the action group */

  unsigned int export_id = g_dbus_connection_export_action_group (conn,
                                                                  INDICATOR_OBJECT_PATH,
                                                                  G_ACTION_GROUP (action_group.get()),
                                                                  &error);
  if (error != nullptr)
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
    std::shared_ptr<GMenu> menu;
    const char * path;
  } menus[] = {
    { phone_profile.get_menu(), INDICATOR_OBJECT_PATH "/phone" }
  };

  for (unsigned int i=0, n=G_N_ELEMENTS(menus); i<n; i++)
    { 
      export_id = g_dbus_connection_export_menu_model (conn,
                                                       menus[i].path,
                                                       G_MENU_MODEL (menus[i].menu.get()),
                                                       &error);
      if (error != nullptr)
        {
          g_warning ("Unable to export phone menu: %s", error->message);
          g_clear_error (&error);
        }
      else
        {
          exported_menus.insert (export_id);
        }
    }
}

