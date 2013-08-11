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

#ifndef SERVICE_H
#define SERVICE_H

#include <glib.h>
#include <gio/gio.h>

#include <QObject>

class Service: QObject
{
    Q_OBJECT

  public:

    Service ();
    virtual ~Service ();

  private:

    void on_bus_acquired (GDBusConnection *, const char *);
    static void on_bus_acquired (GDBusConnection *, const char *, gpointer);

    void on_name_lost (GDBusConnection *, const char *);
    static void on_name_lost (GDBusConnection *, const char *, gpointer);

    GDBusConnection * connection;
    guint own_id;
};

#endif
