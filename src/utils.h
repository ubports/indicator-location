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

#include <glib.h>
#include <gio/gio.h>

struct GObjectDeleter
{
    void operator()(GObject* o)
    {
        g_object_unref(o);
    }

    void operator()(GMenu* o)
    {
        operator()(G_OBJECT(o));
    }
    void operator()(GDBusProxy* o)
    {
        operator()(G_OBJECT(o));
    }
    void operator()(GDBusConnection* o)
    {
        operator()(G_OBJECT(o));
    }
    void operator()(GSimpleActionGroup* o)
    {
        operator()(G_OBJECT(o));
    }
};
