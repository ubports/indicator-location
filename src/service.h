/*
 * Copyright 2013 Canonical Ltd.
 *
 * Authors:
 *   Charles Kerr <charles.kerr@canonical.com>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __INDICATOR_LOCATION_SERVICE_H__
#define __INDICATOR_LOCATION_SERVICE_H__

#include <set>

#include "phone.h"

class Service
{
  public:
    Service ();
    virtual ~Service ();

  private:
    GSimpleActionGroup * action_group;
    GDBusConnection * connection;
    Phone phone_profile;


  public:
    typedef void (*name_lost_callback_func)(Service*, void* user_data);
    void set_name_lost_callback (name_lost_callback_func callback, void* user_data);
  private:
    name_lost_callback_func name_lost_callback;
    void * name_lost_user_data;


  private:
    unsigned int action_group_export_id;
    std::set<unsigned int> exported_menus;
    void unexport ();


  private: // DBus callbacks
    unsigned int bus_own_id;
    void on_name_lost (GDBusConnection*, const char*);
    void on_bus_acquired (GDBusConnection*, const char*);
    static void on_name_lost (GDBusConnection*, const char*, gpointer);
    static void on_bus_acquired (GDBusConnection*, const char*, gpointer);
};

#endif /* __INDICATOR_LOCATION_SERVICE_H__ */

