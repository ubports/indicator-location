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

#include <url-dispatcher.h>

#include "phone.h"
#include "utils.h" // GObjectDeleter

#define PROFILE_NAME "phone"

Phone :: Phone (const std::shared_ptr<Controller>& controller_,
                const std::shared_ptr<GSimpleActionGroup>& action_group_):
  controller (controller_),
  menu (create_menu ()),
  action_group (action_group_)
{
  controller->add_listener (this);

  /* create the actions & add them to the group */
  std::array<GSimpleAction*, 4> actions = { create_root_action(),
                                            create_detection_enabled_action(),
                                            create_gps_enabled_action(),
                                            create_settings_action() };
  for (auto a : actions)
    {
      g_action_map_add_action (G_ACTION_MAP(action_group.get()), G_ACTION(a));
      g_object_unref (a);
    }
}

Phone :: ~Phone ()
{
  controller->remove_listener (this);
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
  g_variant_builder_init (&builder, G_VARIANT_TYPE_VARDICT);

  const char * a11y = _("Location");
  g_variant_builder_add (&builder, "{sv}", "accessible-desc", g_variant_new_string (a11y));

  const char * title = _("Location");
  g_variant_builder_add (&builder, "{sv}", "title", g_variant_new_string (title));

  gboolean visible = should_be_visible ();
  g_variant_builder_add (&builder, "{sv}", "visible", g_variant_new_boolean (visible));

  const char * icon_name = "gps";
  GIcon * icon = g_themed_icon_new_with_default_fallbacks (icon_name);
  GVariant * serialized_icon = g_icon_serialize (icon);
  if (serialized_icon != NULL)
    {
      g_variant_builder_add (&builder, "{sv}", "icon", serialized_icon);
      g_variant_unref (serialized_icon);
    }
  g_object_unref (icon);

  return g_variant_builder_end (&builder);
}

#define HEADER_ACTION_KEY PROFILE_NAME"-header"

GSimpleAction *
Phone :: create_root_action ()
{
  return g_simple_action_new_stateful (HEADER_ACTION_KEY,
                                       nullptr,
                                       action_state_for_root ());
}

/***
****
***/

#define LOCATION_ACTION_KEY "location-detection-enabled"

GVariant *
Phone :: action_state_for_location_detection ()
{
  return g_variant_new_boolean (controller->is_location_service_enabled());
}

void
Phone :: on_location_service_enabled_changed (bool is_enabled G_GNUC_UNUSED)
{
  GAction * action = g_action_map_lookup_action (G_ACTION_MAP(action_group.get()), LOCATION_ACTION_KEY);
  g_simple_action_set_state (G_SIMPLE_ACTION(action), action_state_for_location_detection());
}

void
Phone :: on_detection_location_activated (GSimpleAction * action,
                                          GVariant      * parameter G_GNUC_UNUSED,
                                          gpointer        gself)
{
  GVariant * state = g_action_get_state (G_ACTION (action));
  static_cast<Phone*>(gself)->controller->set_location_service_enabled (!g_variant_get_boolean (state));
  g_variant_unref (state);
}

GSimpleAction *
Phone :: create_detection_enabled_action ()
{
  GSimpleAction * action;

  action = g_simple_action_new_stateful (LOCATION_ACTION_KEY,
                                         nullptr,
                                         action_state_for_location_detection());

  g_simple_action_set_enabled (action, controller->is_valid());

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
  return g_variant_new_boolean (controller->is_gps_enabled());
}

void
Phone :: on_gps_enabled_changed (bool is_enabled G_GNUC_UNUSED)
{
  GAction * action = g_action_map_lookup_action (G_ACTION_MAP(action_group.get()), GPS_ACTION_KEY);
  g_simple_action_set_state (G_SIMPLE_ACTION(action), action_state_for_gps_detection());
}

void
Phone :: on_detection_gps_activated (GSimpleAction * action,
                                     GVariant      * parameter   G_GNUC_UNUSED,
                                     gpointer        gself)
{
  GVariant * state = g_action_get_state (G_ACTION (action));
  static_cast<Phone*>(gself)->controller->set_gps_enabled (!g_variant_get_boolean (state));
  g_variant_unref (state);
}

GSimpleAction *
Phone :: create_gps_enabled_action ()
{
  GSimpleAction * action;

  action = g_simple_action_new_stateful (GPS_ACTION_KEY,
                                         nullptr,
                                         action_state_for_gps_detection());

  g_simple_action_set_enabled (action, controller->is_valid());

  g_signal_connect (action, "activate",
                    G_CALLBACK(on_detection_gps_activated), this);

  return action;
}


/***
****
***/

#define SETTINGS_ACTION_KEY "settings"

namespace
{
  void
  on_uri_dispatched (const gchar * uri,
                     gboolean      success     G_GNUC_UNUSED,
                     gpointer      user_data   G_GNUC_UNUSED)
  {
    if (!success)
      g_warning ("Unable to activate '%s'", uri);
  }

  void
  on_settings_activated (GSimpleAction * simple      G_GNUC_UNUSED,
                         GVariant      * parameter,
                         gpointer        user_data   G_GNUC_UNUSED)
  {
    const char * key = g_variant_get_string (parameter, nullptr);
    gchar * uri = g_strdup_printf ("settings:///system/%s", key);
    url_dispatch_send (uri, on_uri_dispatched, nullptr);
    g_free (uri);
  }
}

GSimpleAction *
Phone :: create_settings_action ()
{
  GSimpleAction * action;

  action = g_simple_action_new (SETTINGS_ACTION_KEY, G_VARIANT_TYPE_STRING);

  g_signal_connect (action, "activate",
                    G_CALLBACK(on_settings_activated), nullptr);

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
  GMenuItem * location;
  GMenuItem * gps;

  /* create the submenu */
  submenu = g_menu_new ();

  location = g_menu_item_new (_("Location detection"), "indicator." LOCATION_ACTION_KEY);
  g_menu_item_set_attribute (location, "x-canonical-type", "s", "com.canonical.indicator.switch");
  g_menu_append_item (submenu, location);
  g_object_unref (location);

  gps = g_menu_item_new (_("GPS"), "indicator." GPS_ACTION_KEY);
  g_menu_item_set_attribute (gps, "x-canonical-type", "s", "com.canonical.indicator.switch");
  g_menu_append_item (submenu, gps);
  g_object_unref (gps);

  // disabled for 13.04 -- the location settings panel isn't complete
  // g_menu_append (submenu, _("Location settings…"), "indicator." SETTINGS_ACTION_KEY "::location");

  /* add the submenu to a new header */
  header = g_menu_item_new (nullptr, "indicator." HEADER_ACTION_KEY);
  g_menu_item_set_attribute (header, "x-canonical-type", "s", "com.canonical.indicator.root");
  g_menu_item_set_submenu (header, G_MENU_MODEL (submenu));
  g_object_unref (submenu);

  /* add the header to a new menu */
  menu = g_menu_new ();
  g_menu_append_item (menu, header);
  g_object_unref (header);

  return std::shared_ptr<GMenu>(menu, GObjectDeleter());
}
