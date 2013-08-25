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

#include "controller.h"

void
Controller :: add_listener (ControllerListener * l)
{
  listeners.insert (l);
}

void
Controller :: remove_listener (ControllerListener * l)
{
  listeners.erase (l);
}

void
Controller :: notify_gps_enabled (bool enabled)
{
  for (auto it : listeners)
    it->on_gps_enabled_changed (enabled);
}

void
Controller :: notify_location_service_enabled (bool enabled)
{
  for (auto it : listeners)
    it->on_location_service_enabled_changed (enabled);
}
