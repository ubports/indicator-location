/*
 * Copyright 2014 Canonical Ltd.
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
 *   Pete Woods <pete.woods@canonical.com>
 */

#include "license-controller.h"

void
LicenseController::add_listener(LicenseControllerListener * const l)
{
  listeners.insert(l);
}

void
LicenseController::remove_listener(LicenseControllerListener * const l)
{
  listeners.erase(l);
}

void
LicenseController::notify_license_accepted(bool license_accepted)
{
  for (auto it : listeners)
  {
    it->on_license_accepted_changed(license_accepted);
  }
}

void
LicenseController::notify_license_path(const std::string & license_path)
{
  for (auto it : listeners)
  {
    it->on_license_path_changed(license_path);
  }
}
