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

#include <glib.h>
#include <gio/gio.h>

#include "license-controller.h"
#include "controller.h"

class Phone: public ControllerListener, public LicenseControllerListener
{
  public:
    Phone (const std::shared_ptr<Controller>& controller,
           const std::shared_ptr<LicenseController>& license_controller,
           const std::shared_ptr<GSimpleActionGroup>& action_group);
    virtual ~Phone ();
    std::shared_ptr<GMenu> get_menu () { return menu; }

  protected:
    std::shared_ptr<Controller> controller;
    std::shared_ptr<LicenseController> license_controller;
    virtual void on_gps_enabled_changed (bool is_enabled);
    virtual void on_location_service_enabled_changed (bool is_enabled);
    void on_license_accepted_changed(bool license_accepted) override;
    void on_license_path_changed(const std::string & license_path) override;

  private:
    std::shared_ptr<GMenu> menu;
    std::shared_ptr<GSimpleActionGroup> action_group;

  private:
    std::shared_ptr<GMenu> create_menu ();

  private:
    bool should_be_visible ();
    GVariant * action_state_for_root ();
    GSimpleAction * create_root_action ();

  private:
    GVariant * action_state_for_location_detection ();
    GSimpleAction * create_detection_enabled_action ();
    static void on_detection_location_activated (GSimpleAction*, GVariant*, gpointer);

  private:
    GVariant * action_state_for_gps_detection ();
    GSimpleAction * create_gps_enabled_action ();
    static void on_detection_gps_activated (GSimpleAction*, GVariant*, gpointer);

  private:
    GSimpleAction * create_settings_action ();

  private:
    GSimpleAction * create_licence_action ();
};

#endif /* __INDICATOR_LOCATION_PHONE_H__ */
