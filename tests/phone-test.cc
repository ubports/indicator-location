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

// must be defined before including gtest-dbus-indicator-fixture
#define INDICATOR_PROFILE "phone"
#include "gtest-dbus-indicator-fixture.h"

#include "controller-mock.h"
#include "src/dbus-shared.h"
#include "src/service.h"

class PhoneTest : public GTestDBusIndicatorFixture
{
protected:
    bool loc_enabled;
    bool loc_enabled_changed;
    bool gps_enabled;
    bool gps_enabled_changed;

    std::shared_ptr<MockController> myController;
    std::shared_ptr<Service> myService;
    std::vector<core::ScopedConnection> myConnections;

public:
    void clear_callbacks()
    {
        gps_enabled = false;
        gps_enabled_changed = false;
        loc_enabled = false;
        loc_enabled_changed = false;
    }

    virtual void on_gps_enabled_changed(bool is_enabled)
    {
        gps_enabled_changed = true;
        gps_enabled = is_enabled;
    }

    virtual void on_location_service_enabled_changed(bool is_enabled)
    {
        loc_enabled_changed = true;
        loc_enabled = is_enabled;
    }

protected:
    virtual void SetUp()
    {
        GTestDBusIndicatorFixture::SetUp();

        clear_callbacks();
    }

    virtual void setup_service()
    {
        myController.reset(new MockController());
        myService.reset(new Service(myController));

        myConnections.push_back(myController->gps_enabled().changed().connect([this](bool enabled)
                                                                              {
                                                                                  gps_enabled_changed = true;
                                                                                  gps_enabled = enabled;
                                                                              }));

        myConnections.push_back(myController->location_service_enabled().changed().connect([this](bool enabled)
                                                                                           {
                                                                                               loc_enabled_changed =
                                                                                                   true;
                                                                                               loc_enabled = enabled;
                                                                                           }));
    }

    virtual void teardown_service()
    {
        myService.reset();
        myConnections.clear();
        myController.reset();
    }
};

TEST_F(PhoneTest, ActionsExit)
{
    ASSERT_TRUE(action_exists("gps-detection-enabled"));
    ASSERT_TRUE(action_exists("location-detection-enabled"));
    ASSERT_TRUE(action_exists("phone-header"));
    ASSERT_TRUE(action_exists("settings"));
}

TEST_F(PhoneTest, MenuitemsExist)
{
    ASSERT_TRUE(action_menuitem_exists("indicator.location-detection-enabled"));
    ASSERT_FALSE(action_menuitem_exists("indicator.gps-detection-enabled"));
    ASSERT_TRUE(action_menuitem_exists("indicator.settings"));
}

TEST_F(PhoneTest, IsValidEnabled)
{
    bool is_valid = myController->is_valid().get();
    auto ag = G_ACTION_GROUP(action_group);
    constexpr int n_iters = 4;
    const std::array<const char*, 2> action_names = {"gps-detection-enabled", "location-detection-enabled"};

    // test that the loc/gps actions are enabled/disabled based on controller->is_valid()
    for (int i = 0; i < n_iters; ++i)
    {
        is_valid = !is_valid;
        myController->is_valid().set(is_valid);
        wait_for_action_enabled_change(action_names[0]);
        for (const auto& action_name : action_names)
        {
            EXPECT_EQ(is_valid, g_action_group_get_action_enabled(ag, action_name));
        }
    }
}

TEST_F(PhoneTest, IsValidVisible)
{
    // make sure something's enabled so that the indicator should be visible
    if (!myController->location_service_enabled().get())
    {
        myController->set_location_service_enabled(true);
        wait_for_action_state_change("location-detection-enabled");
    }

    // test the header's 'invisible' entry tracks the controller's is_valid() state
    constexpr int n_iters = 4;
    const char* header_action_name = INDICATOR_PROFILE "-header";
    auto ag = G_ACTION_GROUP(action_group);
    for (int i = 0; i < n_iters; ++i)
    {
        myController->is_valid().set(!myController->is_valid().get());
        wait_for_action_state_change(header_action_name);

        auto dict = g_action_group_get_action_state(ag, header_action_name);
        EXPECT_TRUE(dict != nullptr);
        EXPECT_TRUE(g_variant_is_of_type(dict, G_VARIANT_TYPE_VARDICT));
        auto v = g_variant_lookup_value(dict, "visible", G_VARIANT_TYPE_BOOLEAN);
        EXPECT_TRUE(v != nullptr);
        EXPECT_EQ(myController->is_valid().get(), g_variant_get_boolean(v));
        g_clear_pointer(&v, g_variant_unref);
        g_clear_pointer(&dict, g_variant_unref);
    }
}

TEST_F(PhoneTest, PlatformTogglesGPS)
{
    bool enabled;
    const char* const key = "gps-detection-enabled";
    GActionGroup* ag = G_ACTION_GROUP(action_group);
    GVariant* v;

    // check that the current state is the default value of 'false'
    v = g_action_group_get_action_state(ag, key);
    ASSERT_TRUE(v != nullptr);
    enabled = g_variant_get_boolean(v);
    g_variant_unref(v);
    ASSERT_FALSE(enabled);

    // confirm that toggling the mock controller's GPS is reflected in the action state
    for (int i = 0; i < 4; i++)
    {
        enabled = !enabled;

        myController->set_gps_enabled(enabled);
        wait_for_action_state_change(key);
        v = g_action_group_get_action_state(ag, key);
        ASSERT_TRUE(v != nullptr);
        ASSERT_EQ(enabled, g_variant_get_boolean(v));
        g_variant_unref(v);
    }
}

TEST_F(PhoneTest, UserTogglesGPS)
{
    bool enabled;
    const char* const key = "gps-detection-enabled";
    GActionGroup* ag = G_ACTION_GROUP(action_group);
    GVariant* v;

    // check that the current state is the default value of 'false'
    v = g_action_group_get_action_state(ag, key);
    ASSERT_TRUE(v != nullptr);
    enabled = g_variant_get_boolean(v);
    g_variant_unref(v);
    ASSERT_FALSE(enabled);

    // confirm that toggling menu updates the controller
    for (int i = 0; i < 4; i++)
    {
        clear_callbacks();

        g_action_group_activate_action(ag, key, nullptr);
        while (!gps_enabled_changed)
        {
            wait_msec(50);
        }

        ASSERT_TRUE(gps_enabled_changed);
        enabled = !enabled;
        ASSERT_EQ(enabled, gps_enabled);
    }
}

TEST_F(PhoneTest, PlatformTogglesLocation)
{
    bool enabled;
    const char* const key = "location-detection-enabled";
    GActionGroup* ag = G_ACTION_GROUP(action_group);
    GVariant* v;

    // check that the current state is the default value of 'false'
    v = g_action_group_get_action_state(ag, key);
    ASSERT_TRUE(v != nullptr);
    enabled = g_variant_get_boolean(v);
    g_variant_unref(v);
    ASSERT_FALSE(enabled);

    // confirm that toggling the mock controller's GPS is reflected in the action state
    for (int i = 0; i < 4; i++)
    {
        enabled = !enabled;

        myController->set_location_service_enabled(enabled);
        wait_for_action_state_change(key);
        v = g_action_group_get_action_state(ag, key);
        ASSERT_TRUE(v != nullptr);
        ASSERT_EQ(enabled, g_variant_get_boolean(v));
        g_variant_unref(v);
    }
}

TEST_F(PhoneTest, UserTogglesLocation)
{
    bool enabled;
    const char* const key = "location-detection-enabled";
    GActionGroup* ag = G_ACTION_GROUP(action_group);
    GVariant* v;

    // check that the current state is the default value of 'false'
    v = g_action_group_get_action_state(ag, key);
    ASSERT_TRUE(v != nullptr);
    enabled = g_variant_get_boolean(v);
    g_variant_unref(v);
    ASSERT_FALSE(enabled);

    // confirm that toggling menu updates the controller
    for (int i = 0; i < 4; i++)
    {
        clear_callbacks();

        g_action_group_activate_action(ag, key, nullptr);
        while (!loc_enabled_changed)
        {
            wait_msec(50);
        }

        ASSERT_TRUE(loc_enabled_changed);
        enabled = !enabled;
        ASSERT_EQ(enabled, loc_enabled);
    }
}

TEST_F(PhoneTest, Header)
{
    wait_msec();

    auto connection = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, nullptr);

    // SETUP: get the action group and wait for it to be populated
    auto dbus_action_group = g_dbus_action_group_get(connection, INDICATOR_BUS_NAME, INDICATOR_OBJECT_PATH);
    auto action_group = G_ACTION_GROUP(dbus_action_group);
    auto names_strv = g_action_group_list_actions(action_group);
    if (g_strv_length(names_strv) == 0)
    {
        g_strfreev(names_strv);
        wait_for_signal(dbus_action_group, "action-added");
        names_strv = g_action_group_list_actions(action_group);
    }
    g_clear_pointer(&names_strv, g_strfreev);

    // SETUP: get the menu model and wait for it to be activated
    auto dbus_menu_model =
        g_dbus_menu_model_get(connection, INDICATOR_BUS_NAME, INDICATOR_OBJECT_PATH "/" INDICATOR_PROFILE);
    auto menu_model = G_MENU_MODEL(dbus_menu_model);
    int n = g_menu_model_get_n_items(menu_model);
    if (!n)
    {
        // give the model a moment to populate its info
        wait_msec(100);
        n = g_menu_model_get_n_items(menu_model);
    }
    EXPECT_TRUE(menu_model != nullptr);
    EXPECT_NE(0, n);

    // test to confirm that a header menuitem exists
    gchar* str = nullptr;
    g_menu_model_get_item_attribute(menu_model, 0, "x-canonical-type", "s", &str);
    EXPECT_STREQ("com.canonical.indicator.root", str);
    g_clear_pointer(&str, g_free);
    g_menu_model_get_item_attribute(menu_model, 0, G_MENU_ATTRIBUTE_ACTION, "s", &str);
    const auto action_name = INDICATOR_PROFILE "-header";
    EXPECT_EQ(std::string("indicator.") + action_name, str);
    g_clear_pointer(&str, g_free);

    // cursory first look at the header
    auto dict = g_action_group_get_action_state(action_group, action_name);
    EXPECT_TRUE(dict != nullptr);
    EXPECT_TRUE(g_variant_is_of_type(dict, G_VARIANT_TYPE_VARDICT));
    auto v = g_variant_lookup_value(dict, "accessible-desc", G_VARIANT_TYPE_STRING);
    EXPECT_TRUE(v != nullptr);
    g_variant_unref(v);
    v = g_variant_lookup_value(dict, "title", G_VARIANT_TYPE_STRING);
    EXPECT_TRUE(v != nullptr);
    g_variant_unref(v);
    v = g_variant_lookup_value(dict, "visible", G_VARIANT_TYPE_BOOLEAN);
    EXPECT_TRUE(v != nullptr);
    g_clear_pointer(&v, g_variant_unref);
    g_clear_pointer(&dict, g_variant_unref);

    // test visibility states
    struct
    {
        bool gps_enabled;
        bool location_service_enabled;
        bool expected_visible;
    } visibility_tests[] = {{false, false, false}, {true, false, false}, {false, true, true}, {true, true, true}};
    for (const auto& test : visibility_tests)
    {
        myController->set_gps_enabled(test.gps_enabled);
        myController->set_location_service_enabled(test.location_service_enabled);
        wait_msec();

        // cusory first look at the header
        dict = g_action_group_get_action_state(action_group, action_name);
        EXPECT_TRUE(dict != nullptr);
        EXPECT_TRUE(g_variant_is_of_type(dict, G_VARIANT_TYPE_VARDICT));
        v = g_variant_lookup_value(dict, "visible", G_VARIANT_TYPE_BOOLEAN);
        EXPECT_TRUE(v != nullptr);
        EXPECT_EQ(test.expected_visible, g_variant_get_boolean(v));
        g_clear_pointer(&v, g_variant_unref);
        g_clear_pointer(&dict, g_variant_unref);
    }

    // cleanup
    g_clear_object(&action_group);
    g_clear_object(&dbus_menu_model);
    g_clear_object(&connection);
}
