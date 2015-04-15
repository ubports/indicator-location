
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

#include <glib.h>

#include "controller-ualc.h"
#include "utils.h"

/***
****
***/

#define PROP_KEY_SERVICE_ENABLED "IsOnline"
#define PROP_KEY_GPS_ENABLED "DoesSatelliteBasedPositioning"

class UbuntuAppLocController::Impl
{
public:

    Impl(UbuntuAppLocController& owner):
        m_owner(owner)
    {
        m_cancellable.reset(g_cancellable_new(), [](GCancellable* c) {
            g_cancellable_cancel(c);
            g_object_unref(c);
        });

        g_dbus_proxy_new_for_bus(G_BUS_TYPE_SYSTEM,
                                 G_DBUS_PROXY_FLAGS_NONE,
                                 nullptr, // interface info
                                 "com.ubuntu.location.Service", // name
                                 "/com/ubuntu/location/Service", // path
                                 "com.ubuntu.location.Service", // interface name
                                 m_cancellable.get(),
                                 on_proxy_ready,
                                 this);
    }

    ~Impl() =default;

    const core::Property<bool>& is_valid() const
    {
        return m_is_valid;
    }

    bool is_gps_enabled () const
    {
        return get_cached_bool_property(PROP_KEY_GPS_ENABLED);
    }

    bool is_location_service_enabled () const
    {
        return get_cached_bool_property(PROP_KEY_SERVICE_ENABLED);
    }

    void set_gps_enabled (bool enabled)
    {
        set_bool_property(PROP_KEY_GPS_ENABLED, enabled);
    }

    void set_location_service_enabled (bool enabled)
    {
        set_bool_property(PROP_KEY_SERVICE_ENABLED, enabled);
    }

private:

    bool get_cached_bool_property(const char* property_name) const
    {
        bool result = false;

        auto v = g_dbus_proxy_get_cached_property(m_proxy.get(), property_name);
g_message("cached property for %s; %s", property_name, g_variant_print(v, true));
        if (v != nullptr)
        {
            result = g_variant_get_boolean(v);
            g_variant_unref(v);
        }
 
        return result;
    }

    void set_bool_property(const char* property_name, bool b)
    {
        g_return_if_fail(m_proxy);

        auto args = g_variant_new("(ssv)", "com.ubuntu.location.Service", property_name, g_variant_new_boolean(b));
g_message("args: %s", g_variant_print(args, true));
        g_dbus_connection_call(g_dbus_proxy_get_connection(m_proxy.get()),
                                                           "com.ubuntu.location.Service", // bus name
                                                           "/com/ubuntu/location/Service", // object path
                                                           "org.freedesktop.DBus.Properties", // interface name,
                                                           "Set", // method name,
                                                           args,
                                                           nullptr, // reply type
                                                           G_DBUS_CALL_FLAGS_NONE,
                                                           -1, // timeout msec
                                                           m_cancellable.get(),
                                                           on_set_response,
                                                           this);
    }

    static void on_set_response(GObject      *connection,
                                GAsyncResult *res,
                                gpointer      gself)
    {
        GError * error;
        GVariant * v;

g_message("%s on_set_response", G_STRLOC);
        error = nullptr;
        v = g_dbus_connection_call_finish(G_DBUS_CONNECTION(connection), res, &error);
g_message("%s %s", G_STRLOC, g_variant_print(v, true));
        if (error != nullptr)
        {
            if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
                g_warning("Couldn't get AccountsService proxy: %s", error->message);
            g_error_free(error);
        }
        else
        {
            if (v != nullptr)
            {
                g_message("foo %s", g_variant_print(v, true));
                g_variant_unref(v);
            }

            // FIXME:update here?
            //self->notify_gps_enabled(g_variant_get_boolean(value));
            //self->notify_location_service_enabled(g_variant_get_boolean(value));
        }
    }

    static void on_proxy_ready(GObject*, GAsyncResult* res, gpointer gself)
    {
        GError * error;
        GDBusProxy * proxy;

        error = nullptr;
        proxy = g_dbus_proxy_new_for_bus_finish(res, &error);
g_message("%s proxy ready %p", G_STRLOC, (void*)proxy);
        if (proxy != nullptr)
        {
            // if we got one, cache it and start listening for properties-changed
            auto self = static_cast<Impl*>(gself);
            self->m_proxy.reset(proxy, GObjectDeleter());
            g_signal_connect(proxy, "g-properties-changed",
                             G_CALLBACK(on_properties_changed), gself);
            g_signal_connect(proxy, "notify::g-name-owner",
                             G_CALLBACK(on_owner_changed), gself);
            on_owner_changed(G_OBJECT(proxy), nullptr, gself);
        }
        else if (error != nullptr)
        {
            if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
                g_warning("Couldn't get AccountsService proxy: %s", error->message);
            g_error_free(error);
        }
    }

    static void on_owner_changed(GObject* o, GParamSpec*, gpointer gself)
    {
      auto self = static_cast<Impl*>(gself);
      gchar* name_owner = nullptr;
      g_object_get(o, "g-name-owner", &name_owner, nullptr);
      g_message("name owner is now {%s}", name_owner);
      self->m_is_valid.set(name_owner && *name_owner);
    }

    static void on_properties_changed(GDBusProxy   * proxy,
                                      GVariant     * changed_properties,
                                      const gchar ** invalidated_properties,
                                      gpointer       gself)
    {
        auto self = static_cast<Impl*>(gself);

g_message("%s on properties changed", G_STRLOC);
        if (g_variant_n_children(changed_properties) > 0)
        {
            GVariantIter *iter;
            const gchar *key;
            GVariant *value;
            std::string property_name;

            g_variant_get(changed_properties, "a{sv}", &iter);
            while (g_variant_iter_loop(iter, "{&sv}", &key, &value))
            {
                if (key)
                    g_message("key {%s}", key);
                if (!g_strcmp0(key, PROP_KEY_GPS_ENABLED))
                    self->m_owner.notify_gps_enabled(g_variant_get_boolean(value));
                else if (!g_strcmp0(key, PROP_KEY_SERVICE_ENABLED))
                    self->m_owner.notify_location_service_enabled(g_variant_get_boolean(value));
            }
            g_variant_iter_free(iter);
        }
    }

    UbuntuAppLocController& m_owner;
    std::shared_ptr<GCancellable> m_cancellable {};
    std::shared_ptr<GDBusProxy> m_proxy {};
    core::Property<bool> m_is_valid {false};
};

/***
****
***/

UbuntuAppLocController::UbuntuAppLocController():
  impl{new Impl{*this}}
{
}

UbuntuAppLocController::~UbuntuAppLocController()
{
}

const core::Property<bool>&
UbuntuAppLocController::is_valid() const
{
  return impl->is_valid();
}

bool
UbuntuAppLocController::is_gps_enabled () const
{
  return impl->is_gps_enabled();
}

bool
UbuntuAppLocController::is_location_service_enabled () const
{
  return impl->is_location_service_enabled();
}
void
UbuntuAppLocController::set_gps_enabled (bool enabled)
{
  impl->set_gps_enabled(enabled);
}

void
UbuntuAppLocController::set_location_service_enabled (bool enabled)
{
  impl->set_location_service_enabled(enabled);
}

