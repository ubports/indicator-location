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

#ifndef __INDICATOR_LOCATION_CONTROLLER_UALC__H__
#define __INDICATOR_LOCATION_CONTROLLER_UALC__H__

#include <ubuntu/status.h>
#include <ubuntu/application/location/controller.h>
#include <ubuntu/application/location/service.h>

#include "controller.h"

class UbuntuAppLocController: public Controller
{
  public:

    UbuntuAppLocController ();
    virtual ~UbuntuAppLocController();

    bool is_valid () const { return ualc != nullptr; }
    bool is_gps_enabled () const;
    bool is_location_service_enabled () const;

    void set_gps_enabled (bool enabled);
    void set_location_service_enabled (bool enabled);

  private:

    UbuntuApplicationLocationServiceController * ualc;

    static void on_location_service_controller_status_changed (
                              UALocationServiceStatusFlags, void *);
};

#endif // __INDICATOR_LOCATION_CONTROLLER_UALC__H__


