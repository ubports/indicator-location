
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

#include <cassert>
#include <iostream>

#include "controller-ualc.h"

UbuntuAppLocController :: UbuntuAppLocController ():
  ualc (ua_location_service_create_controller ())
{
  if (ualc == nullptr)
    return;

  // update our state when the location service changes
  ua_location_service_controller_set_status_changed_handler (
    ualc,
    on_controller_status_changed_static,
    this);

  // bootstrap our initial state
  UALocationServiceStatusFlags new_status = 0;
  if (ua_location_service_controller_query_status (ualc, &new_status) == U_STATUS_SUCCESS)
    on_controller_status_changed (new_status);
}

UbuntuAppLocController :: ~UbuntuAppLocController ()
{
  if (ualc != nullptr)
    ua_location_service_controller_unref (ualc);
}

void
UbuntuAppLocController :: on_controller_status_changed_static (UALocationServiceStatusFlags flags, void *vself)
{
  static_cast<UbuntuAppLocController*>(vself)->on_controller_status_changed (flags);
}

void
UbuntuAppLocController :: on_controller_status_changed (UALocationServiceStatusFlags new_status)
{
  const bool loc_was_enabled = (old_status & UA_LOCATION_SERVICE_ENABLED) != 0;
  const bool loc_is_enabled = (new_status & UA_LOCATION_SERVICE_ENABLED) != 0;
  const bool gps_was_enabled = (old_status & UA_LOCATION_SERVICE_GPS_ENABLED) != 0;
  const bool gps_is_enabled = (new_status & UA_LOCATION_SERVICE_GPS_ENABLED) != 0;
  old_status = new_status;

  if (loc_was_enabled != loc_is_enabled)
    {
      g_assert (loc_is_enabled == is_location_service_enabled());

      notify_location_service_enabled (loc_is_enabled);
    }

  if (gps_was_enabled != gps_is_enabled)
    {
      g_assert (gps_is_enabled == is_gps_enabled());

      notify_gps_enabled (is_gps_enabled());
    }
}

/***
****
***/

bool
UbuntuAppLocController :: is_gps_enabled () const
{
  UALocationServiceStatusFlags flags = 0;

  return (ualc != nullptr)
      && (ua_location_service_controller_query_status (ualc, &flags) == U_STATUS_SUCCESS)
      && (flags & UA_LOCATION_SERVICE_GPS_ENABLED);
}

void
UbuntuAppLocController :: set_gps_enabled (bool enabled)
{
  UStatus status = enabled
    ? ua_location_service_controller_enable_gps (ualc)
    : ua_location_service_controller_disable_gps (ualc);

  if (status != U_STATUS_SUCCESS)
    std::cerr << "Error turning GPS " << (enabled?"on":"off") << std::endl;
}

/***
****
***/

bool
UbuntuAppLocController :: is_location_service_enabled () const
{
  UALocationServiceStatusFlags flags = 0;

  return (ualc != nullptr)
      && (ua_location_service_controller_query_status (ualc, &flags) == U_STATUS_SUCCESS)
      && (flags & UA_LOCATION_SERVICE_ENABLED);
}

void
UbuntuAppLocController :: set_location_service_enabled (bool enabled)
{
  UStatus status = enabled
    ? ua_location_service_controller_enable_service (ualc)
    : ua_location_service_controller_disable_service (ualc);

  if (status != U_STATUS_SUCCESS)
    std::cerr << "Error turning Location Service " << (enabled?"on":"off")
              << std::endl;
}
