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

#ifndef __INDICATOR_LOCATION_PHONE_H__
#define __INDICATOR_LOCATION_PHONE_H__

#include <memory>
#include <gio/gio.h>
//#include <ubuntu/application/location/controller.h>


class Phone
{
  public:
    Phone (std::shared_ptr<GSimpleActionGroup> action_group);
    virtual ~Phone ();
    std::shared_ptr<GMenu> get_menu () { return menu; }

  private:
    std::shared_ptr<GMenu> menu;
    std::shared_ptr<GSimpleActionGroup> action_group;

  private:
    //UALocationServiceController * location_service_controller;
    //static void on_location_service_controller_status_changed (UALocationServiceStatusFlags, void*);
    //UALocationServiceController * create_location_service_controller ();

  private:
    std::shared_ptr<GMenu> create_menu ();

  private:
    bool should_be_visible ();
    GVariant * action_state_for_root ();
    GSimpleAction * create_root_action ();

  private:
    GSimpleAction * create_detection_enabled_action ();
    GVariant * action_state_for_location_detection ();
    static void on_detection_location_activated (GSimpleAction*, GVariant*, gpointer);
    void update_location_detection_state ();

  private:
    GVariant * action_state_for_gps_detection ();
    void update_gps_detection_state ();
    static void on_detection_gps_activated (GSimpleAction*, GVariant*, gpointer);
    GSimpleAction * create_gps_enabled_action ();

  private:
    GSimpleAction * create_settings_action ();
};

#endif /* __INDICATOR_LOCATION_PHONE_H__ */
