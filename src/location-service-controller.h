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

#pragma once

#include "controller.h"  // parent class

#include <memory>  // std::unique_ptr

class LocationServiceController : public Controller
{
public:
    LocationServiceController();
    virtual ~LocationServiceController();

    const core::Property<bool>& is_valid() const override;
    const core::Property<bool>& gps_enabled() const override;
    const core::Property<bool>& location_service_enabled() const override;
    void set_gps_enabled(bool enabled) override;
    void set_location_service_enabled(bool enabled) override;

    LocationServiceController(const LocationServiceController&) = delete;
    LocationServiceController& operator=(const LocationServiceController&) = delete;

private:
    friend class Impl;
    class Impl;
    std::unique_ptr<Impl> impl;
};
