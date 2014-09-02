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

#ifndef __INDICATOR_LOCATION_LICENSE_CONTROLLER__H__
#define __INDICATOR_LOCATION_LICENSE_CONTROLLER__H__

#include <memory>
#include <string>
#include <unordered_set>

class LicenseControllerListener
{
public:
  LicenseControllerListener() = default;

  virtual
  ~LicenseControllerListener() = default;

public:
  virtual void
  on_license_accepted_changed(bool license_accepted) = 0;

  virtual void
  on_license_path_changed(const std::string & license_path) = 0;
};

class LicenseController
{
public:
  LicenseController() = default;

  virtual
  ~LicenseController() = default;

  virtual bool
  license_accepted() const = 0;

  virtual std::string
  license_path() const = 0;

  void
  add_listener(LicenseControllerListener * const);

  void
  remove_listener(LicenseControllerListener * const);

protected:

  void
  notify_license_accepted(bool);

  void
  notify_license_path(const std::string & license_path);

private:
  std::unordered_set<LicenseControllerListener *> listeners;
};

#endif // __INDICATOR_LOCATION_LICENSE_CONTROLLER__H__
