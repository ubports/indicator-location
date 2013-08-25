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

#include <map>

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

    Controller (): next_tag(0) {}
    virtual ~Controller() {}

    unsigned int add_listener (ControllerListener *);
    void remove_listener (unsigned int tag);

    virtual bool is_valid () const = 0;
    virtual bool is_gps_enabled () const = 0;
    virtual bool is_location_service_enabled () const = 0;

    virtual void set_gps_enabled (bool enabled) = 0;
    virtual void set_location_service_enabled (bool enabled) = 0;

  private:

    unsigned int next_tag;
    std::map<unsigned int,ControllerListener *> listeners;

  protected:

    void notify_gps_enabled (bool);
    void notify_location_service_enabled (bool);
};

#endif // __INDICATOR_LOCATION_CONTROLLER_H__
