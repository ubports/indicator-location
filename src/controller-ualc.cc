
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

#include <iostream>

#include "controller-ualc.h"

UbuntuAppLocController :: UbuntuAppLocController ():
  ualc (ua_location_service_create_controller ())
{
  if (ualc)
    {
      ua_location_service_controller_set_status_changed_handler (
        ualc,
        on_location_service_controller_status_changed,
        this);
    }
}

UbuntuAppLocController :: ~UbuntuAppLocController ()
{
  if (ualc)
    ua_location_service_controller_unref (ualc);
}

void
UbuntuAppLocController :: on_location_service_controller_status_changed (
                                                UALocationServiceStatusFlags f,
                                                void * vself)
{
  auto self = static_cast<UbuntuAppLocController*>(vself);

  if (f & (UA_LOCATION_SERVICE_ENABLED | UA_LOCATION_SERVICE_DISABLED))
    self->notify_gps_enabled (self->is_location_service_enabled ());

  if (f & (UA_LOCATION_SERVICE_GPS_ENABLED | UA_LOCATION_SERVICE_GPS_DISABLED))
    self->notify_gps_enabled (self->is_gps_enabled());
}

/***
****
***/

bool
UbuntuAppLocController :: is_gps_enabled () const
{
  UALocationServiceStatusFlags flags = 0;

  return (ualc != 0)
      && (!ua_location_service_controller_query_status (ualc, &flags))
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

  return (ualc != 0)
      && (!ua_location_service_controller_query_status (ualc, &flags))
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
