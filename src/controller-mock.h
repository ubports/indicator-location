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

#ifndef __INDICATOR_LOCATION_CONTROLLER_MOCK__H__
#define __INDICATOR_LOCATION_CONTROLLER_MOCK__H__

#include "controller.h"

class MockController: public Controller
{
  public:

    MockController() =default;
    virtual ~MockController() =default;

    core::Property<bool>& is_valid() { return m_is_valid; }
    const core::Property<bool>& is_valid() const override { return m_is_valid; }
    bool is_gps_enabled () const { return gps; }
    bool is_location_service_enabled () const { return loc; }

    void set_gps_enabled (bool enabled) { notify_gps_enabled (gps=enabled); }
    void set_location_service_enabled (bool enabled) { notify_location_service_enabled (loc=enabled); }

  private:

    core::Property<bool> m_is_valid {true};
    bool gps { false };
    bool loc { false };
};

#endif // __INDICATOR_LOCATION_CONTROLLER_MOCK__H__


