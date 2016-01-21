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

#include <core/property.h>

class Controller
{
public:
    Controller() =default;
    virtual ~Controller() =default;

    /// True iff we've gotten status info from the location service
    virtual const core::Property<bool>& is_valid() const =0;

    virtual const core::Property<bool>& gps_enabled() const =0;
    virtual const core::Property<bool>& location_service_enabled() const =0;

    virtual void set_gps_enabled (bool enabled) =0;
    virtual void set_location_service_enabled (bool enabled) =0;
};

