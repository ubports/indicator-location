/*
 * Copyright 2013 Canonical Ltd.
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

#include <array>

#include <glib/gi18n.h>
//#include <ubuntu/application/location/controller.h>
//#include <ubuntu/application/location/service.h>
#include "phone.h"
#include "utils.h" // GObjectDeleter

#define PROFILE_NAME "phone"

Phone :: Phone (std::shared_ptr<GSimpleActionGroup> action_group_):
  menu (create_menu ()),
  action_group (action_group_)
  //location_service_controller (create_location_service_controller ())
{
  g_debug ("%s %s: %s", G_STRLOC, G_STRFUNC, PROFILE_NAME);

  /* create the actions & add them to the group */
  std::array<GSimpleAction*, 4> actions = { create_root_action(),
                                            create_detection_enabled_action(),
                                            create_gps_enabled_action(),
                                            create_settings_action() };
  for (auto& a : actions)
    {
      g_action_map_add_action (G_ACTION_MAP(action_group.get()), G_ACTION(a));
      g_object_unref (a);
    }
}

Phone :: ~Phone ()
{
  //g_clear_pointer (&location_service_controller, ua_location_service_controller_unref);
}

/***
****
***/

bool
Phone :: should_be_visible ()
{
  // NB: this is a placeholder for now.
  // The spec requires that the location indicator be visible iff
  // an application has accessed location info recently,
  // but there's not a platform API to support that yet.

  return true;
}

GVariant *
Phone :: action_state_for_root ()
{
  GVariantBuilder builder;
  g_variant_builder_init (&builder, G_VARIANT_TYPE("a{sv}"));

  const char * a11y = "Location";
  g_variant_builder_add (&builder, "{sv}", "accessible-desc", g_variant_new_string (a11y));

  gboolean visible = should_be_visible ();
  g_variant_builder_add (&builder, "{sv}", "visible", g_variant_new_boolean (visible));

  const char * icon_name = "gps";
  GIcon * icon = g_themed_icon_new_with_default_fallbacks (icon_name);
  g_variant_builder_add (&builder, "{sv}", "icon", g_icon_serialize (icon));
  g_object_unref (icon);

  return g_variant_builder_end (&builder);
}

#define HEADER_ACTION_KEY PROFILE_NAME"-header"

GSimpleAction *
Phone :: create_root_action ()
{
  return g_simple_action_new_stateful (HEADER_ACTION_KEY,
                                       NULL,
                                       action_state_for_root ());
}

/***
****
***/

#define LOCATION_ACTION_KEY "location-detection-enabled"

GVariant *
Phone :: action_state_for_location_detection ()
{
#if 0
  UALocationServiceStatusFlags flags = 0;
  bool enabled = (location_service_controller != 0)
              && (ua_location_service_controller_query_status (location_service_controller, &flags) == U_STATUS_SUCCESS)
              && (flags & UA_LOCATION_SERVICE_ENABLED);
#else
  bool enabled = true;
#endif
  
  return g_variant_new_boolean (enabled);
}

void
Phone :: update_location_detection_state ()
{
  GAction * action = g_action_map_lookup_action (G_ACTION_MAP(action_group.get()), LOCATION_ACTION_KEY);
  g_simple_action_set_state (G_SIMPLE_ACTION(action), action_state_for_location_detection());
}

void
Phone :: on_detection_location_activated (GSimpleAction * action    G_GNUC_UNUSED,
                                          GVariant      * parameter G_GNUC_UNUSED,
                                          gpointer        gself     G_GNUC_UNUSED)
{
#if 0
  Phone * self = static_cast<Phone*>(gself);
  g_assert (G_ACTION(action) == g_action_map_lookup_action (G_ACTION_MAP(self->action_group.get()), LOCATION_ACTION_KEY));

  GVariant * state = g_action_get_state (G_ACTION (action));
  const bool old_enabled = g_variant_get_boolean (state);
  const bool new_enabled = !old_enabled;

  UStatus status = new_enabled
                 ? ua_location_service_controller_enable_service (self->location_service_controller)
                 : ua_location_service_controller_disable_service (self->location_service_controller);
  if (status != U_STATUS_SUCCESS)
    g_warning ("Unable to %s the location service", new_enabled ? "enable" : "disable");
  
  g_variant_unref (state);
#endif
}

GSimpleAction *
Phone :: create_detection_enabled_action ()
{
  GSimpleAction * action;

  action = g_simple_action_new_stateful (LOCATION_ACTION_KEY,
                                         NULL,
                                         action_state_for_location_detection());

//  g_simple_action_set_enabled (action, location_service_controller != 0);

  g_signal_connect (action, "activate",
                    G_CALLBACK(on_detection_location_activated), this);

  return action;
}

/***
****
***/

#define GPS_ACTION_KEY "gps-detection-enabled"

GVariant *
Phone :: action_state_for_gps_detection ()
{
#if 0
  UALocationServiceStatusFlags flags = 0;
  bool enabled = (location_service_controller != 0)
              && (ua_location_service_controller_query_status (location_service_controller, &flags) == U_STATUS_SUCCESS)
              && (flags & UA_LOCATION_SERVICE_GPS_ENABLED);
#else
  bool enabled = false;
#endif
  return g_variant_new_boolean (enabled);
}

void
Phone :: update_gps_detection_state ()
{
  GAction * action = g_action_map_lookup_action (G_ACTION_MAP(action_group.get()), GPS_ACTION_KEY);
  g_simple_action_set_state (G_SIMPLE_ACTION(action), action_state_for_gps_detection());
}

void
Phone :: on_detection_gps_activated (GSimpleAction * action      G_GNUC_UNUSED,
                                     GVariant      * parameter   G_GNUC_UNUSED,
                                     gpointer        gself       G_GNUC_UNUSED)
{
#if 0
  Phone * self = static_cast<Phone*>(gself);
  g_assert (G_ACTION(action) == g_action_map_lookup_action (G_ACTION_MAP(self->action_group.get()), GPS_ACTION_KEY));
  g_return_if_fail (self->location_service_controller != 0);

  GVariant * state = g_action_get_state (G_ACTION (action));
  const bool old_enabled = g_variant_get_boolean (state);
  const bool new_enabled = !old_enabled;

  UStatus status = new_enabled
                 ? ua_location_service_controller_enable_gps (self->location_service_controller)
                 : ua_location_service_controller_disable_gps (self->location_service_controller);
  if (status != U_STATUS_SUCCESS)
    g_warning ("Unable to %s GPS", new_enabled ? "enable" : "disable");
  
  g_variant_unref (state);
#endif
}

GSimpleAction *
Phone :: create_gps_enabled_action ()
{
  GSimpleAction * action;

  action = g_simple_action_new_stateful (GPS_ACTION_KEY,
                                         NULL,
                                         action_state_for_gps_detection());

  //g_simple_action_set_enabled (action, location_service_controller != 0);

  g_signal_connect (action, "activate",
                    G_CALLBACK(on_detection_gps_activated), this);

  return action;
}

/***
****
***/

#if 0
void
Phone :: on_location_service_controller_status_changed (UALocationServiceStatusFlags flags,
                                                        void * vself)
{
  Phone * self = static_cast<Phone*>(vself);

  if (flags & (UA_LOCATION_SERVICE_ENABLED | UA_LOCATION_SERVICE_DISABLED))
    self->update_location_detection_state ();

  if (flags & (UA_LOCATION_SERVICE_GPS_ENABLED | UA_LOCATION_SERVICE_GPS_DISABLED))
    self->update_gps_detection_state ();
}

UALocationServiceController *
Phone :: create_location_service_controller ()
{
  UALocationServiceController * c = ua_location_service_create_controller ();

  if (c == 0)
    {
      g_warning ("Unable to load a location_service_controller.");
    }
  else
    {
      UStatus status = ua_location_service_controller_set_status_changed_handler (location_service_controller,
                                                                                  on_location_service_controller_status_changed,
                                                                                  this);
      if (status != U_STATUS_SUCCESS)
        g_warning ("Unable to monitor location service's status.");
    }

  return c;
}
#endif

/***
****
***/

#define SETTINGS_ACTION_KEY "settings"

static void
show_settings (void)
{
  const char * cmd = "ubuntu-system-settings location";

  g_debug ("%s calling \"%s\"", G_STRFUNC, cmd);

  GError * err = NULL;
  g_spawn_command_line_async (cmd, &err);
  if (err != NULL)
    {
      g_warning ("Unable to show location settings: %s", err->message);
      g_error_free (err);
    }
}

GSimpleAction *
Phone :: create_settings_action ()
{
  GSimpleAction * action;

  action = g_simple_action_new (SETTINGS_ACTION_KEY, NULL);

  g_signal_connect (action, "activate",
                    G_CALLBACK(show_settings), NULL);

  return action;
}

/***
****
***/

std::shared_ptr <GMenu>
Phone :: create_menu ()
{
  GMenu * menu;
  GMenu * submenu;
  GMenuItem * header;

  /* create the submenu */
  submenu = g_menu_new ();
  g_menu_append (submenu, _("Location detection"), "indicator." LOCATION_ACTION_KEY);
  g_menu_append (submenu, _("GPS"), "indicator." GPS_ACTION_KEY);
  g_menu_append (submenu, _("Location settings…"), "indicator." SETTINGS_ACTION_KEY);

  /* add the submenu to a new header */
  header = g_menu_item_new (NULL, "indicator." HEADER_ACTION_KEY);
  g_menu_item_set_attribute (header, "x-canonical-type", "s", "com.canonical.indicator.root");
  g_menu_item_set_submenu (header, G_MENU_MODEL (submenu));
  g_object_unref (submenu);

  /* add the header to a new menu */
  menu = g_menu_new ();
  g_menu_append_item (menu, header);
  g_object_unref (header);

  return std::shared_ptr<GMenu>(menu, GObjectDeleter());
}
