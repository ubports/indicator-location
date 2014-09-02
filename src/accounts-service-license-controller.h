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

#ifndef __INDICATOR_LOCATION_ACCOUNTS_SERVICE_LICENSE_CONTROLLER__H__
#define __INDICATOR_LOCATION_ACCOUNTS_SERVICE_LICENSE_CONTROLLER__H__

#include "license-controller.h"

#include <gio/gio.h>
#include <memory>

class AccountsServiceLicenseController : public LicenseController
{
public:
  AccountsServiceLicenseController();

  ~AccountsServiceLicenseController()
  {
  }

  bool
  license_accepted() const override;

  std::string
  license_path() const override;

private:
  std::shared_ptr<GDBusProxy> proxy;

  static void
  on_properties_changed(GDBusProxy *proxy, GVariant *changed_properties,
                        const gchar* const *invalidated_properties,
                        gpointer user_data);
};

#endif // __INDICATOR_LOCATION_ACCOUNTS_SERVICE_LICENSE_CONTROLLER__H__
