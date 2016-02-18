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

#pragma once

#include "src/dbus-shared.h"
#include "gtest-dbus-fixture.h"

class GTestDBusIndicatorFixture : public GTestDBusFixture
{
    typedef GTestDBusFixture super;

    enum
    {
        TIME_LIMIT_SEC = 10
    };

private:
    static void on_name_appeared(GDBusConnection* connection G_GNUC_UNUSED,
                                 const gchar* name G_GNUC_UNUSED,
                                 const gchar* name_owner G_GNUC_UNUSED,
                                 gpointer gself)
    {
        g_main_loop_quit(static_cast<GTestDBusIndicatorFixture*>(gself)->loop);
    }

    GSList* menu_references;

    gboolean any_item_changed;

    static void on_items_changed(GMenuModel* model G_GNUC_UNUSED,
                                 gint position G_GNUC_UNUSED,
                                 gint removed G_GNUC_UNUSED,
                                 gint added G_GNUC_UNUSED,
                                 gpointer gany_item_changed)
    {
        *static_cast<gboolean*>(gany_item_changed) = true;
    }

protected:
    void activate_subtree(GMenuModel* model)
    {
        // query the GDBusMenuModel for information to activate it
        int n = g_menu_model_get_n_items(model);
        if (!n)
        {
            // give the model a moment to populate its info
            wait_msec(100);
            n = g_menu_model_get_n_items(model);
        }

        // keep a ref so that it stays activated
        menu_references = g_slist_prepend(menu_references, g_object_ref(model));

        g_signal_connect(model, "items-changed", G_CALLBACK(on_items_changed), &any_item_changed);

        // recurse
        for (int i = 0; i < n; ++i)
        {
            GMenuModel* link;
            GMenuLinkIter* iter = g_menu_model_iterate_item_links(model, i);
            while (g_menu_link_iter_get_next(iter, nullptr, &link))
            {
                activate_subtree(link);
                g_object_unref(link);
            }
            g_clear_object(&iter);
        }
    }

    void sync_menu(void)
    {
        g_slist_free_full(menu_references, GDestroyNotify(g_object_unref));
        menu_references = nullptr;
        activate_subtree(G_MENU_MODEL(menu_model));
    }

    GDBusMenuModel* menu_model;
    GDBusActionGroup* action_group;
    GTimer* timer;

    virtual void setup_service() = 0;
    virtual void teardown_service() = 0;

    virtual void SetUp()
    {
        super::SetUp();

        menu_references = nullptr;
        any_item_changed = FALSE;

        timer = g_timer_new();

        // Start an IndicatorSessionService and wait for it to appear on the bus.
        setup_service();
        const guint watch_id = g_bus_watch_name_on_connection(conn, INDICATOR_BUS_NAME, G_BUS_NAME_WATCHER_FLAGS_NONE,
                                                              on_name_appeared,  // quits the loop
                                                              nullptr, this, nullptr);
        const guint timer_id = g_timeout_add_seconds(TIME_LIMIT_SEC, GSourceFunc(g_main_loop_quit), loop);
        g_main_loop_run(loop);
        g_source_remove(timer_id);
        g_bus_unwatch_name(watch_id);
        ASSERT_FALSE(times_up());

        // get the actions & menus that the service exported.
        action_group = g_dbus_action_group_get(conn, INDICATOR_BUS_NAME, INDICATOR_OBJECT_PATH);
        menu_model = g_dbus_menu_model_get(conn, INDICATOR_BUS_NAME, INDICATOR_OBJECT_PATH "/" INDICATOR_PROFILE);
        // the actions are added asynchronously, so wait for the actions
        if (!g_action_group_has_action(G_ACTION_GROUP(action_group), INDICATOR_PROFILE "-header"))
        {
            wait_for_signal(action_group, "action-added");
        }
        // activate all the groups in the menu so it'll be ready when we need it
        g_signal_connect(menu_model, "items-changed", G_CALLBACK(on_items_changed), &any_item_changed);
        sync_menu();
    }

    virtual void TearDown()
    {
        g_clear_pointer(&timer, g_timer_destroy);

        g_slist_free_full(menu_references, GDestroyNotify(g_object_unref));
        menu_references = nullptr;
        g_clear_object(&menu_model);

        g_clear_object(&action_group);
        teardown_service();
        wait_msec(100);

        super::TearDown();
    }

private:
    bool times_up() const
    {
        return g_timer_elapsed(timer, nullptr) >= TIME_LIMIT_SEC;
    }

protected:
    void wait_for_has_action(const char* name)
    {
        while (!g_action_group_has_action(G_ACTION_GROUP(action_group), name) && !times_up())
        {
            wait_msec(50);
        }

        ASSERT_FALSE(times_up());
        ASSERT_TRUE(g_action_group_has_action(G_ACTION_GROUP(action_group), name));
    }

    void wait_for_menu_resync(void)
    {
        any_item_changed = false;
        while (!times_up() && !any_item_changed)
        {
            wait_msec(50);
        }
        g_warn_if_fail(any_item_changed);
        sync_menu();
    }

    void wait_for_action_state_change(const gchar* action_name)
    {
        gchar* signal_name = g_strdup_printf("action-state-changed::%s", action_name);
        wait_for_signal(action_group, signal_name);
        g_free(signal_name);
    }

    void wait_for_action_enabled_change(const gchar* action_name)
    {
        gchar* signal_name = g_strdup_printf("action-enabled-changed::%s", action_name);
        wait_for_signal(action_group, signal_name);
        g_free(signal_name);
    }

protected:
    bool find_menu_item_for_action(const char* action_key, GMenuModel** setme, int* item_index)
    {
        bool success = false;

        for (GSList* l = menu_references; !success && (l != nullptr); l = l->next)
        {
            GMenuModel* mm = G_MENU_MODEL(l->data);
            const int n = g_menu_model_get_n_items(mm);

            for (int i = 0; !success && i < n; ++i)
            {
                char* action = nullptr;
                if (!g_menu_model_get_item_attribute(mm, i, G_MENU_ATTRIBUTE_ACTION, "s", &action))
                {
                    continue;
                }

                if ((success = !g_strcmp0(action, action_key)))
                {
                    if (setme != nullptr)
                    {
                        *setme = G_MENU_MODEL(g_object_ref(G_OBJECT(mm)));
                    }

                    if (item_index != nullptr)
                    {
                        *item_index = i;
                    }
                }

                g_free(action);
            }
        }

        return success;
    }

    bool action_exists(const char* action_name)
    {
#if 0
        gchar** actions = g_action_group_list_actions (G_ACTION_GROUP (action_group));
        for (int i=0; actions && actions[i]; i++)
        {
            g_message ("[%d][%s]", i, actions[i]);
        }
#endif

        return g_action_group_has_action(G_ACTION_GROUP(action_group), action_name);
    }

    bool action_menuitem_exists(const char* action_name)
    {
        bool found;
        GMenuModel* model = nullptr;
        int pos = -1;

        if ((found = find_menu_item_for_action(action_name, &model, &pos)))
        {
            g_object_unref(G_OBJECT(model));
        }

        return found;
    }

    void check_header(const char* expected_label, const char* expected_icon, const char* expected_a11y)
    {
        GVariant* variant;
        const gchar* label = nullptr;
        const gchar* icon = nullptr;
        const gchar* a11y = nullptr;
        gboolean visible;

        variant = g_action_group_get_action_state(G_ACTION_GROUP(action_group), "_header");
        g_variant_get(variant, "(&s&s&sb)", &label, &icon, &a11y, &visible);

        if (expected_label != nullptr)
        {
            ASSERT_STREQ(expected_label, label);
        }

        if (expected_icon != nullptr)
        {
            ASSERT_STREQ(expected_icon, icon);
        }

        if (expected_a11y != nullptr)
        {
            ASSERT_STREQ(expected_a11y, a11y);
        }

        // the session menu is always visible...
        ASSERT_TRUE(visible);

        g_variant_unref(variant);
    }

    void check_label(const char* expected_label, GMenuModel* model, int pos)
    {
        char* label = nullptr;
        ASSERT_TRUE(g_menu_model_get_item_attribute(model, pos, G_MENU_ATTRIBUTE_LABEL, "s", &label));
        ASSERT_STREQ(expected_label, label);
        g_free(label);
    }
};
