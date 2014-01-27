/*
 * Copyright 2014 Canonical Ltd.
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

#include "src/controller-ualc.h"

#include <ubuntu/status.h>
#include <ubuntu/application/location/controller.h>
#include <ubuntu/application/location/service.h>

#include <gtest/gtest.h>

#include <glib.h>

/***
****
***/

class UalcFixture: public ::testing::Test
{
    typedef ::testing::Test super;

  protected:

    GMainLoop * loop = nullptr;
    UALocationServiceController * ualc = nullptr;

    virtual void SetUp()
    {
      super::SetUp();

      loop = g_main_loop_new(nullptr, FALSE);
      ualc = ua_location_service_create_controller();
    }

    virtual void TearDown()
    {
      g_clear_pointer(&ualc, ua_location_service_controller_unref);
      g_clear_pointer(&loop, g_main_loop_unref);

      super::TearDown();
    }
};


/***
****  Sanity check for build + fixture
***/

TEST_F(UalcFixture, HelloWorld)
{
  EXPECT_TRUE(true);
  EXPECT_FALSE(false);
}


/***
****  Simple test to make sure our mock UALC is behaving sanely
***/

namespace
{
  UALocationServiceStatusFlags simple_mock_test_flags = 0;

  void * simple_mock_test_context = (void*)(0xDEADBEEF);

  void simple_mock_test_changed_handler(UALocationServiceStatusFlags flags, void * c)
  {
    g_assert(c == simple_mock_test_context);
    simple_mock_test_flags = flags;
  }
}

TEST_F(UalcFixture, SimpleMockTest)
{
  ua_location_service_controller_set_status_changed_handler(
    ualc, simple_mock_test_changed_handler, simple_mock_test_context);

  UALocationServiceStatusFlags flags = 0;
  EXPECT_EQ(0, simple_mock_test_flags);
  EXPECT_EQ(U_STATUS_SUCCESS, ua_location_service_controller_query_status(ualc, &flags));
  EXPECT_EQ(U_STATUS_SUCCESS, ua_location_service_controller_enable_service(ualc));
  EXPECT_EQ(UA_LOCATION_SERVICE_ENABLED, simple_mock_test_flags);
  EXPECT_EQ(U_STATUS_SUCCESS, ua_location_service_controller_query_status(ualc, &flags));
  EXPECT_EQ(UA_LOCATION_SERVICE_ENABLED, flags);
  EXPECT_EQ(U_STATUS_SUCCESS, ua_location_service_controller_enable_gps(ualc));
  EXPECT_EQ(UA_LOCATION_SERVICE_ENABLED | UA_LOCATION_SERVICE_GPS_ENABLED, simple_mock_test_flags);
  EXPECT_EQ(U_STATUS_SUCCESS, ua_location_service_controller_query_status(ualc, &flags));
  EXPECT_EQ(UA_LOCATION_SERVICE_ENABLED | UA_LOCATION_SERVICE_GPS_ENABLED, flags);
}

/***
****  Now, actual controller tests
***/

TEST_F(UalcFixture, ControllerAccessors)
{
  UbuntuAppLocController c;
  EXPECT_FALSE(c.is_location_service_enabled());
  EXPECT_FALSE(c.is_gps_enabled());

  EXPECT_EQ(U_STATUS_SUCCESS, ua_location_service_controller_enable_service(ualc));
  EXPECT_TRUE(c.is_location_service_enabled());
  EXPECT_FALSE(c.is_gps_enabled());

  EXPECT_EQ(U_STATUS_SUCCESS, ua_location_service_controller_enable_gps(ualc));
  EXPECT_TRUE(c.is_location_service_enabled());
  EXPECT_TRUE(c.is_gps_enabled());

  EXPECT_EQ(U_STATUS_SUCCESS, ua_location_service_controller_disable_service(ualc));
  EXPECT_FALSE(c.is_location_service_enabled());
  EXPECT_TRUE(c.is_gps_enabled());

  EXPECT_EQ(U_STATUS_SUCCESS, ua_location_service_controller_disable_gps(ualc));
  EXPECT_FALSE(c.is_location_service_enabled());
  EXPECT_FALSE(c.is_gps_enabled());
}

TEST_F(UalcFixture, ControllerMutators)
{
  UbuntuAppLocController c;
  UALocationServiceStatusFlags flags;

  flags = 0;
  EXPECT_FALSE(c.is_location_service_enabled());
  EXPECT_FALSE(c.is_gps_enabled());
  EXPECT_EQ(U_STATUS_SUCCESS, ua_location_service_controller_query_status(ualc, &flags));
  EXPECT_EQ(0, flags);

  // enable gps
  c.set_gps_enabled(true);
  EXPECT_FALSE(c.is_location_service_enabled());
  EXPECT_TRUE(c.is_gps_enabled());
  EXPECT_EQ(U_STATUS_SUCCESS, ua_location_service_controller_query_status(ualc, &flags));
  EXPECT_EQ(UA_LOCATION_SERVICE_GPS_ENABLED, flags);

  // disable gps
  c.set_gps_enabled(false);
  EXPECT_FALSE(c.is_location_service_enabled());
  EXPECT_FALSE(c.is_gps_enabled());
  EXPECT_EQ(U_STATUS_SUCCESS, ua_location_service_controller_query_status(ualc, &flags));
  EXPECT_EQ(0, flags);

  // enable service
  c.set_location_service_enabled(true);
  EXPECT_TRUE(c.is_location_service_enabled());
  EXPECT_FALSE(c.is_gps_enabled());
  EXPECT_EQ(U_STATUS_SUCCESS, ua_location_service_controller_query_status(ualc, &flags));
  EXPECT_EQ(UA_LOCATION_SERVICE_ENABLED, flags);

  // disable service
  c.set_location_service_enabled(false);
  EXPECT_FALSE(c.is_location_service_enabled());
  EXPECT_FALSE(c.is_gps_enabled());
  EXPECT_EQ(U_STATUS_SUCCESS, ua_location_service_controller_query_status(ualc, &flags));
  EXPECT_EQ(0, flags);

  // enable both
  c.set_gps_enabled(true);
  c.set_location_service_enabled(true);
  EXPECT_TRUE(c.is_location_service_enabled());
  EXPECT_TRUE(c.is_gps_enabled());
  EXPECT_EQ(U_STATUS_SUCCESS, ua_location_service_controller_query_status(ualc, &flags));
  EXPECT_EQ(UA_LOCATION_SERVICE_ENABLED | UA_LOCATION_SERVICE_GPS_ENABLED, flags);

  // disable both
  c.set_gps_enabled(false);
  c.set_location_service_enabled(false);
  EXPECT_FALSE(c.is_location_service_enabled());
  EXPECT_FALSE(c.is_gps_enabled());
  EXPECT_EQ(U_STATUS_SUCCESS, ua_location_service_controller_query_status(ualc, &flags));
  EXPECT_EQ(0, flags);
}

