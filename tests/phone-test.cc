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

#define INDICATOR_BUS_NAME    "com.canonical.indicator.location"
#define INDICATOR_OBJECT_PATH "/com/canonical/indicator/location"
#define INDICATOR_PROFILE     "phone"
#include "gtest-dbus-indicator-fixture.h"

#include "src/mock-license-controller.h"
#include "src/controller-mock.h"
#include "src/service.h"

class PhoneTest: public GTestDBusIndicatorFixture,
                 public ControllerListener
{
  protected:

    bool loc_enabled;
    bool loc_enabled_changed;
    bool gps_enabled;
    bool gps_enabled_changed;

    std::shared_ptr<MockController> myController;
    std::shared_ptr<Service> myService;

  public:

    void clear_callbacks ()
    {
      gps_enabled = false;
      gps_enabled_changed = false;
      gps_enabled = false;
      loc_enabled_changed = false;
    }

    virtual void on_gps_enabled_changed (bool is_enabled)
    {
      gps_enabled_changed = true;
      gps_enabled = is_enabled;
    }

    virtual void on_location_service_enabled_changed (bool is_enabled)
    {
      loc_enabled_changed = true;
      loc_enabled = is_enabled;
    }

  protected:

    virtual void SetUp ()
    {
      GTestDBusIndicatorFixture :: SetUp ();

      clear_callbacks ();
    }

    virtual void setup_service ()
    {
      myController.reset (new MockController ());
      myController->add_listener (this);
      myService.reset (new Service (myController, std::make_shared<MockLicenseController>()));
    }

    virtual void teardown_service ()
    {
      myService.reset ();
      myController->remove_listener (this);
      myController.reset ();
    }
};

TEST_F (PhoneTest, ActionsExit)
{
  ASSERT_TRUE (action_exists ("gps-detection-enabled"));
  ASSERT_TRUE (action_exists ("location-detection-enabled"));
  ASSERT_TRUE (action_exists ("phone-header"));
  ASSERT_TRUE (action_exists ("settings"));
}

TEST_F (PhoneTest, MenuitemsExist)
{
  ASSERT_TRUE (action_menuitem_exists ("indicator.location-detection-enabled"));
  ASSERT_TRUE (action_menuitem_exists ("indicator.gps-detection-enabled"));
  //ASSERT_TRUE (action_menuitem_exists ("indicator.settings"));
}

TEST_F (PhoneTest, PlatformTogglesGPS)
{
  bool enabled;
  const char * const key = "gps-detection-enabled";
  GActionGroup * ag = G_ACTION_GROUP (action_group);
  GVariant * v;

  // check that the current state is the default value of 'false'
  v = g_action_group_get_action_state (ag, key);
  ASSERT_TRUE (v != nullptr);
  enabled = g_variant_get_boolean (v);
  g_variant_unref (v);
  ASSERT_FALSE (enabled);

  // confirm that toggling the mock controller's GPS is reflected in the action state
  for (int i=0; i<4; i++)
    {
      enabled = !enabled;

      myController->set_gps_enabled (enabled);
      wait_for_action_state_change (key);
      v = g_action_group_get_action_state (ag, key);
      ASSERT_TRUE (v != nullptr);
      ASSERT_EQ (enabled, g_variant_get_boolean (v));
      g_variant_unref (v);
    }
}

TEST_F (PhoneTest, UserTogglesGPS)
{
  bool enabled;
  const char * const key = "gps-detection-enabled";
  GActionGroup * ag = G_ACTION_GROUP (action_group);
  GVariant * v;

  // check that the current state is the default value of 'false'
  v = g_action_group_get_action_state (ag, key);
  ASSERT_TRUE (v != nullptr);
  enabled = g_variant_get_boolean (v);
  g_variant_unref (v);
  ASSERT_FALSE (enabled);

  // confirm that toggling menu updates the controller
  for (int i=0; i<4; i++)
    {
      clear_callbacks ();

      g_action_group_activate_action (ag, key, nullptr);
      while (!gps_enabled_changed)
        wait_msec (50);

      ASSERT_TRUE (gps_enabled_changed);
      enabled = !enabled;
      ASSERT_EQ (enabled, gps_enabled);
    }
}

TEST_F (PhoneTest, PlatformTogglesLocation)
{
  bool enabled;
  const char * const key = "location-detection-enabled";
  GActionGroup * ag = G_ACTION_GROUP (action_group);
  GVariant * v;

  // check that the current state is the default value of 'false'
  v = g_action_group_get_action_state (ag, key);
  ASSERT_TRUE (v != nullptr);
  enabled = g_variant_get_boolean (v);
  g_variant_unref (v);
  ASSERT_FALSE (enabled);

  // confirm that toggling the mock controller's GPS is reflected in the action state
  for (int i=0; i<4; i++)
    {
      enabled = !enabled;

      myController->set_location_service_enabled (enabled);
      wait_for_action_state_change (key);
      v = g_action_group_get_action_state (ag, key);
      ASSERT_TRUE (v != nullptr);
      ASSERT_EQ (enabled, g_variant_get_boolean (v));
      g_variant_unref (v);
    }
}

TEST_F (PhoneTest, UserTogglesLocation)
{
  bool enabled;
  const char * const key = "location-detection-enabled";
  GActionGroup * ag = G_ACTION_GROUP (action_group);
  GVariant * v;

  // check that the current state is the default value of 'false'
  v = g_action_group_get_action_state (ag, key);
  ASSERT_TRUE (v != nullptr);
  enabled = g_variant_get_boolean (v);
  g_variant_unref (v);
  ASSERT_FALSE (enabled);

  // confirm that toggling menu updates the controller
  for (int i=0; i<4; i++)
    {
      clear_callbacks ();

      g_action_group_activate_action (ag, key, nullptr);
      while (!loc_enabled_changed)
        wait_msec (50);

      ASSERT_TRUE (loc_enabled_changed);
      enabled = !enabled;
      ASSERT_EQ (enabled, loc_enabled);
    }
}
