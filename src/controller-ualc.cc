
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

#include <glib.h>

#include "controller-ualc.h"

UbuntuAppLocController :: UbuntuAppLocController ():
  ualc (ua_location_service_create_controller ())
{
  g_return_if_fail (ualc != nullptr);

  // update our status when the ualc changes
  ua_location_service_controller_set_status_changed_handler (ualc,
                                                             on_ualc_status_changed,
                                                             this);

  // query the ualc to bootstrap our initial status
  UALocationServiceStatusFlags initial_status;
  if (ua_location_service_controller_query_status (ualc, &initial_status) == U_STATUS_SUCCESS)
    set_status (initial_status);
}

UbuntuAppLocController :: ~UbuntuAppLocController ()
{
  if (ualc != nullptr)
    ua_location_service_controller_unref (ualc);
}

void
UbuntuAppLocController :: on_ualc_status_changed (UALocationServiceStatusFlags flags, void *vself)
{
  static_cast<UbuntuAppLocController*>(vself)->set_status (flags);
}

void
UbuntuAppLocController :: set_status (UALocationServiceStatusFlags new_status)
{
  const bool loc_was_enabled = is_location_service_enabled();
  const bool gps_was_enabled = is_gps_enabled();
  current_status = new_status;
  const bool loc_is_enabled = is_location_service_enabled();
  const bool gps_is_enabled = is_gps_enabled();

  if (loc_was_enabled != loc_is_enabled)
    notify_location_service_enabled (loc_is_enabled);

  if (gps_was_enabled != gps_is_enabled)
      notify_gps_enabled (gps_is_enabled);
}

/***
****
***/

void
UbuntuAppLocController :: set_gps_enabled (bool enabled)
{
  auto status = enabled ? ua_location_service_controller_enable_gps (ualc)
                        : ua_location_service_controller_disable_gps (ualc);

  if (status != U_STATUS_SUCCESS)
    g_warning ("Error turning GPS %s", (enabled?"on":"off"));
}

void
UbuntuAppLocController :: set_location_service_enabled (bool enabled)
{
  auto status = enabled ? ua_location_service_controller_enable_service (ualc)
                        : ua_location_service_controller_disable_service (ualc);

  if (status != U_STATUS_SUCCESS)
    g_warning ("Error turning Location Service %s", (enabled?"on":"off"));
}
