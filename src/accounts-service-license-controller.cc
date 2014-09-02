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

#include "accounts-service-license-controller.h"

#define ACCOUNTS_NAME "org.freedesktop.Accounts"
#define ACCOUNTS_SERVICE "com.ubuntu.location.providers.here.AccountsService"

namespace
{

std::string
user_path()
{
  return "/org/freedesktop/Accounts/User" + std::to_string(getuid());
}

std::string
make_path(const std::string& path, const std::string& lang)
{
  return std::string("file://") + path + "/" + lang + ".html";
}

std::string
build_full_path(const std::string & path)
{
  std::string result;
  char * lang_char = getenv("LANG");
  if (lang_char)
  {
    std::string lang = lang_char;
    auto pos = lang.find('.');
    if (pos != std::string::npos)
    {
      lang = lang.substr(0, pos);
    }
    result = make_path(path, lang);
  }

  if (!g_file_test(path.c_str(), G_FILE_TEST_EXISTS))
  {
    result = make_path(path, "en_US");
  }

  return result;
}

}

AccountsServiceLicenseController::AccountsServiceLicenseController()
{
  GError * error = nullptr;
  proxy.reset(
      g_dbus_proxy_new_for_bus_sync(
          G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_GET_INVALIDATED_PROPERTIES,
          nullptr, ACCOUNTS_NAME, user_path().c_str(),
          ACCOUNTS_SERVICE,
          nullptr, &error),
      &g_object_unref);

  if (proxy.get() == nullptr)
  {
    g_warning("Could not get AccountsService proxy '%s'", error->message);
    g_error_free(error);
    return;
  }

  g_signal_connect(proxy.get(), "g-properties-changed",
                   G_CALLBACK (on_properties_changed),
                   static_cast<void*>(this));
}

bool
AccountsServiceLicenseController::license_accepted() const
{
  bool result = false;

  GVariant * accepted_variant = g_dbus_proxy_get_cached_property(
      proxy.get(), "LicenseAccepted");
  if (accepted_variant)
  {
    result = g_variant_get_boolean(accepted_variant);
  }

  return result;
}

std::string
AccountsServiceLicenseController::license_path() const
{
  std::string path;

  GVariant * base_path_variant = g_dbus_proxy_get_cached_property(
      proxy.get(), "LicenseBasePath");
  if (base_path_variant)
  {
    path = g_variant_get_string(base_path_variant, NULL);
  }

  return build_full_path(path);
}

void
AccountsServiceLicenseController::on_properties_changed(
    GDBusProxy *proxy, GVariant *changed_properties,
    const gchar* const *invalidated_properties, gpointer user_data)
{
  AccountsServiceLicenseController * self =
      static_cast<AccountsServiceLicenseController *>(user_data);

  if (g_variant_n_children(changed_properties) > 0)
  {
    GVariantIter *iter;
    const gchar *key;
    GVariant *value;
    std::string property_name;

    g_variant_get(changed_properties, "a{sv}", &iter);
    while (g_variant_iter_loop(iter, "{&sv}", &key, &value))
    {
      property_name = key;

      if (property_name == "LicenseAccepted")
      {
        self->notify_license_accepted(g_variant_get_boolean(value));
      }
      else if (property_name == "LicenseBasePath")
      {
        const gchar * path = g_variant_get_string(value, NULL);
        self->notify_license_path(build_full_path(path));
      }
    }
    g_variant_iter_free(iter);
  }
}
