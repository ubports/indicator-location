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

#ifndef __INDICATOR_LOCATION_MOCK_LICENSE_CONTROLLER__H__
#define __INDICATOR_LOCATION_MOCK_LICENSE_CONTROLLER__H__

#include "license-controller.h"

class MockLicenseController : public LicenseController
{
public:
  MockLicenseController() = default;

  ~MockLicenseController()
  {
  }

  bool
  license_accepted() const override
  {
    return true;
  }

  std::string
  license_path() const override{
    return "file:///foo/bar/en_US.html";
  }
};

#endif // __INDICATOR_LOCATION_MOCK_LICENSE_CONTROLLER__H__
