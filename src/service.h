/*
 * Copyright 2013-2016 Canonical Ltd.
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

#include <memory>
#include <set>

#include "controller.h"
#include "phone.h"
#include "utils.h"  // GObjectDeleter

class Service
{
public:
    explicit Service(const std::shared_ptr<Controller>& controller);
    virtual ~Service();

private:
    std::shared_ptr<GSimpleActionGroup> action_group;
    std::unique_ptr<GDBusConnection, GObjectDeleter> connection;
    Phone phone_profile;

public:
    typedef void (*name_lost_callback_func)(Service*, void* user_data);
    void set_name_lost_callback(name_lost_callback_func callback, void* user_data);

private:
    name_lost_callback_func name_lost_callback;
    void* name_lost_user_data;

private:
    unsigned int action_group_export_id;
    std::set<unsigned int> exported_menus;
    void unexport();

private:  // DBus callbacks
    unsigned int bus_own_id;
    void on_name_lost(GDBusConnection*, const char*);
    void on_bus_acquired(GDBusConnection*, const char*);
    static void on_name_lost(GDBusConnection*, const char*, gpointer);
    static void on_bus_acquired(GDBusConnection*, const char*, gpointer);
};
