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

#ifndef __INDICATOR_LOCATION_CONTROLLER_H__
#define __INDICATOR_LOCATION_CONTROLLER_H__

#include <set>

#include <core/property.h>

class ControllerListener
{
  public:
    ControllerListener() {}
    virtual ~ControllerListener() {}

  public:
    virtual void on_gps_enabled_changed (bool is_enabled) = 0;
    virtual void on_location_service_enabled_changed (bool is_enabled) = 0;
};

class Controller
{
  public:

    Controller () {}
    virtual ~Controller() {}

    void add_listener (ControllerListener *);
    void remove_listener (ControllerListener *);

    /// True iff we've gotten status info from the location service
    virtual const core::Property<bool>& is_valid() const =0;

    virtual bool is_gps_enabled () const = 0;
    virtual bool is_location_service_enabled () const = 0;

    virtual void set_gps_enabled (bool enabled) = 0;
    virtual void set_location_service_enabled (bool enabled) = 0;

  private:

    std::set<ControllerListener*> listeners;

  protected:

    void notify_gps_enabled (bool);
    void notify_location_service_enabled (bool);
};

#endif // __INDICATOR_LOCATION_CONTROLLER_H__
