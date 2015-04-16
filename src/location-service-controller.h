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

#ifndef INDICATOR_LOCATION_CONTROLLER_LOCATION_SERVICE
#define INDICATOR_LOCATION_CONTROLLER_LOCATION_SERVICE

#include "controller.h"

#include <memory> // std::unique_ptr

class LocationServiceController: public Controller
{
  public:
    LocationServiceController();
    virtual ~LocationServiceController();

    virtual const core::Property<bool>& is_valid() const override;
    bool is_gps_enabled () const override;
    bool is_location_service_enabled () const override;
    void set_gps_enabled (bool enabled) override;
    void set_location_service_enabled (bool enabled) override;

  private:
    friend class Impl;
    class Impl;
    std::unique_ptr<Impl> impl;
};

#endif // INDICATOR_LOCATION_CONTROLLER_LOCATION_SERVICE

