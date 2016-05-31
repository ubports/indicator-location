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

#include <array>

#include <glib/gi18n.h>

#include <url-dispatcher.h>
#include <ubuntu-app-launch.h>

#include "phone.h"
#include "utils.h"  // GObjectDeleter

#define PROFILE_NAME "phone"

#define LOCATION_ACTION_KEY "location-detection-enabled"
#define GPS_ACTION_KEY "gps-detection-enabled"

Phone::Phone(const std::shared_ptr<Controller>& controller_, const std::shared_ptr<GSimpleActionGroup>& action_group_)
    : controller(controller_)
    , action_group(action_group_)
{
    create_menu();

    auto on_gps = [this](bool enabled)
    {
        update_gps_enabled_action();
        update_header();
    };
    controller_connections.push_back(controller->gps_enabled().changed().connect(on_gps));

    auto on_loc = [this](bool enabled)
    {
        update_detection_enabled_action();
        update_header();
    };
    controller_connections.push_back(controller->location_service_enabled().changed().connect(on_loc));

    auto on_loc_active = [this](bool active)
    {
        update_header();
    };
    controller_connections.push_back(controller->location_service_active().changed().connect(on_loc_active));

    auto on_valid = [this](bool valid)
    {
        update_actions_enabled();
    };
    controller_connections.push_back(controller->is_valid().changed().connect(on_valid));

    /* create the actions & add them to the group */
    std::array<GSimpleAction*, 4> actions = {
        create_root_action(), create_detection_enabled_action(), create_gps_enabled_action(), create_settings_action()};
    for (auto a : actions)
    {
        g_action_map_add_action(G_ACTION_MAP(action_group.get()), G_ACTION(a));
        g_object_unref(a);
    }

    update_actions_enabled();
}

Phone::~Phone()
{
}

/***
****
***/

bool Phone::should_be_visible() const
{
    if (!controller->is_valid())
    {
        return false;
    }

    // as per "Indicators - RTM Usability Fix" document:
    // visible iff location is enabled
    return controller->location_service_enabled().get();
}

bool Phone::location_service_active() const
{
    if (!controller->is_valid())
    {
        return false;
    }

    return controller->location_service_active().get();
}

GVariant* Phone::action_state_for_root() const
{
    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);

    const char* a11y = _("Location");
    g_variant_builder_add(&builder, "{sv}", "accessible-desc", g_variant_new_string(a11y));

    const char* title = _("Location");
    g_variant_builder_add(&builder, "{sv}", "title", g_variant_new_string(title));

    gboolean visible = should_be_visible();
    g_variant_builder_add(&builder, "{sv}", "visible", g_variant_new_boolean(visible));

    GIcon* icon;
    if (!visible)
    {
        icon = g_themed_icon_new_with_default_fallbacks("location-disabled");
    }
    else if (location_service_active())
    {
        icon = g_themed_icon_new_with_default_fallbacks("location-active");
    }
    else
    {
        icon = g_themed_icon_new_with_default_fallbacks("location-idle");
    }
    GVariant* serialized_icon = g_icon_serialize(icon);
    if (serialized_icon != NULL)
    {
        g_variant_builder_add(&builder, "{sv}", "icon", serialized_icon);
        g_variant_unref(serialized_icon);
    }
    g_object_unref(icon);

    return g_variant_builder_end(&builder);
}

#define HEADER_ACTION_KEY PROFILE_NAME "-header"

GSimpleAction* Phone::create_root_action()
{
    return g_simple_action_new_stateful(HEADER_ACTION_KEY, nullptr, action_state_for_root());
}

void Phone::update_header()
{
    g_action_group_change_action_state(G_ACTION_GROUP(action_group.get()), HEADER_ACTION_KEY, action_state_for_root());
}

void Phone::update_actions_enabled()
{
    const auto map = G_ACTION_MAP(action_group.get());
    const bool is_valid = controller->is_valid().get();
    std::array<const char*, 2> keys = {LOCATION_ACTION_KEY, GPS_ACTION_KEY};
    for (const auto& key : keys)
    {
        g_simple_action_set_enabled(G_SIMPLE_ACTION(g_action_map_lookup_action(map, key)), is_valid);
    }

    update_header();
}

/***
****
***/

GVariant* Phone::action_state_for_location_detection()
{
    return g_variant_new_boolean(controller->location_service_enabled().get());
}

void Phone::on_detection_location_activated(GSimpleAction* action, GVariant* parameter G_GNUC_UNUSED, gpointer gself)
{
    GVariant* state = g_action_get_state(G_ACTION(action));
    static_cast<Phone*>(gself)->controller->set_location_service_enabled(!g_variant_get_boolean(state));
    g_variant_unref(state);
}

GSimpleAction* Phone::create_detection_enabled_action()
{
    GSimpleAction* action;

    action = g_simple_action_new_stateful(LOCATION_ACTION_KEY, nullptr, action_state_for_location_detection());

    g_signal_connect(action, "activate", G_CALLBACK(on_detection_location_activated), this);

    return action;
}

void Phone::update_detection_enabled_action()
{
    GAction* action = g_action_map_lookup_action(G_ACTION_MAP(action_group.get()), LOCATION_ACTION_KEY);
    g_simple_action_set_state(G_SIMPLE_ACTION(action), action_state_for_location_detection());
}

/***
****
***/

GVariant* Phone::action_state_for_gps_detection()
{
    return g_variant_new_boolean(controller->gps_enabled().get());
}

void Phone::on_detection_gps_activated(GSimpleAction* action, GVariant* parameter G_GNUC_UNUSED, gpointer gself)
{
    GVariant* state = g_action_get_state(G_ACTION(action));
    static_cast<Phone*>(gself)->controller->set_gps_enabled(!g_variant_get_boolean(state));
    g_variant_unref(state);
}

GSimpleAction* Phone::create_gps_enabled_action()
{
    GSimpleAction* action;

    action = g_simple_action_new_stateful(GPS_ACTION_KEY, nullptr, action_state_for_gps_detection());

    g_signal_connect(action, "activate", G_CALLBACK(on_detection_gps_activated), this);

    return action;
}

void Phone::update_gps_enabled_action()
{
    GAction* action = g_action_map_lookup_action(G_ACTION_MAP(action_group.get()), GPS_ACTION_KEY);
    g_simple_action_set_state(G_SIMPLE_ACTION(action), action_state_for_gps_detection());
}

/***
****
***/

#define SETTINGS_ACTION_KEY "settings"

namespace
{
void on_uri_dispatched(const gchar* uri, gboolean success, gpointer user_data G_GNUC_UNUSED)
{
    if (!success)
    {
        g_warning("Unable to activate '%s'", uri);
    }
}

void on_settings_activated(GSimpleAction* simple G_GNUC_UNUSED, GVariant* parameter, gpointer user_data G_GNUC_UNUSED)
{
    const char* key = g_variant_get_string(parameter, nullptr);
    gchar* uri = g_strdup_printf("settings:///%s", key);
    g_debug("%s calling url_dispatch_send '%s'", G_STRLOC, uri);
    url_dispatch_send(uri, on_uri_dispatched, nullptr);
    g_free(uri);
}
}

GSimpleAction* Phone::create_settings_action()
{
    GSimpleAction* action;

    action = g_simple_action_new(SETTINGS_ACTION_KEY, G_VARIANT_TYPE_STRING);

    g_signal_connect(action, "activate", G_CALLBACK(on_settings_activated), nullptr);

    return action;
}

/***
****
***/

void Phone::create_menu()
{
    GMenuItem* header;

    /* create the submenu */
    submenu.reset(g_menu_new(), GObjectDeleter());

    /* populate the submenu */
    rebuild_submenu();

    /* add the submenu to a new header */
    header = g_menu_item_new(nullptr, "indicator." HEADER_ACTION_KEY);
    g_menu_item_set_attribute(header, "x-canonical-type", "s", "com.canonical.indicator.root");
    g_menu_item_set_submenu(header, G_MENU_MODEL(submenu.get()));

    /* add the header to a new menu */
    menu.reset(g_menu_new(), GObjectDeleter());
    g_menu_append_item(menu.get(), header);
    g_object_unref(header);
}

void Phone::rebuild_submenu()
{
    g_menu_remove_all(submenu.get());

    GMenuItem* location = g_menu_item_new(_("Location detection"), "indicator." LOCATION_ACTION_KEY);
    g_menu_item_set_attribute(location, "x-canonical-type", "s", "com.canonical.indicator.switch");
    g_menu_append_item(submenu.get(), location);
    g_object_unref(location);

    g_menu_append(submenu.get(), _("Location settings…"), "indicator." SETTINGS_ACTION_KEY "::location");
}
