
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

#define BUS_NAME "com.ubuntu.location.Service"
#define OBJECT_PATH "/com/ubuntu/location/Service"

#define LOC_IFACE_NAME "com.ubuntu.location.Service"

#define PROP_IFACE_NAME "org.freedesktop.DBus.Properties"
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

        g_bus_get(G_BUS_TYPE_SYSTEM, m_cancellable.get(), on_system_bus_ready, this);
    }

    ~Impl()
    {
        if (m_name_tag)
            g_bus_unwatch_name(m_name_tag);
    }

    const core::Property<bool>& is_valid() const
    {
        return m_is_valid;
    }

    bool is_gps_enabled () const
    {
        return m_is_gps_enabled;
    }

    bool is_location_service_enabled () const
    {
        return m_location_service_enabled;
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

    static void on_system_bus_ready(GObject*, GAsyncResult* res, gpointer gself)
    {
        GError* error;
        GDBusConnection* system_bus;

        error = nullptr;
        system_bus = g_bus_get_finish(res, &error);
        if (system_bus != nullptr)
        {
            auto self = static_cast<Impl*>(gself);

            self->m_system_bus.reset(system_bus, GObjectDeleter());

            self->m_name_tag = g_bus_watch_name_on_connection(system_bus,
                                                              BUS_NAME,
                                                              G_BUS_NAME_WATCHER_FLAGS_AUTO_START,
                                                              on_name_appeared,
                                                              on_name_vanished,
                                                              gself,
                                                              nullptr);
        }
        else if (error != nullptr)
        {
            if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
                g_warning("Couldn't get AccountsService proxy: %s", error->message);
            g_error_free(error);
        }
    }

    static void on_name_appeared(GDBusConnection * system_bus,
                                 const gchar * bus_name,
                                 const gchar * name_owner,
                                 gpointer gself)
    {
        auto self = static_cast<Impl*>(gself);

        self->m_signal_tag = g_dbus_connection_signal_subscribe(system_bus,
                                                                name_owner,
                                                                PROP_IFACE_NAME,
                                                                "PropertiesChanged",
                                                                OBJECT_PATH,
                                                                nullptr, // arg0
                                                                G_DBUS_SIGNAL_FLAGS_NONE,
                                                                on_properties_changed,
                                                                gself,
                                                                nullptr);

        struct {
            const char * name;
            GAsyncReadyCallback callback;
        } properties[] = {
            { PROP_KEY_SERVICE_ENABLED, on_service_enabled_reply },
            { PROP_KEY_GPS_ENABLED, on_gps_enabled_reply }
        };
        for (const auto& prop : properties)
        { 
            g_dbus_connection_call(system_bus,
                                   BUS_NAME,
                                   OBJECT_PATH,
                                   PROP_IFACE_NAME,
                                   "Get",
                                   g_variant_new("(ss)", LOC_IFACE_NAME, prop.name), // args
                                   G_VARIANT_TYPE("(s)"), // return type
                                   G_DBUS_CALL_FLAGS_NONE,
                                   -1, // use default timeout
                                  self->m_cancellable.get(),
                                  prop.callback,
                                  gself);
        }
    }

    static void on_name_vanished(GDBusConnection * system_bus,
                                 const gchar*,
                                 gpointer gself)
    {
        auto self = static_cast<Impl*>(gself);

        if (self->m_signal_tag)
        {
            g_dbus_connection_signal_unsubscribe(system_bus, self->m_signal_tag);
            self->m_signal_tag = 0;
        }
    }

    static void on_properties_changed(GDBusConnection* /*connection*/,
                                      const gchar *sender_name,
                                      const gchar *object_path,
                                      const gchar *interface_name,
                                      const gchar *signal_name,
                                      GVariant *parameters,
                                      gpointer /*gself*/)
    {
        g_message("FIXME");
        g_message("sender_name %s", sender_name);
        g_message("object_path %s", object_path);
        g_message("interface_name %s", interface_name);
        g_message("signal_name %s", signal_name);
        g_message("parameters %s", g_variant_print(parameters, true));
    }

    static void on_service_enabled_reply(GObject      * source_object,
                                         GAsyncResult * res,
                                         gpointer       gself)
    {
        GError * error;
        GVariant * v;

        error = nullptr;
        v = g_dbus_connection_call_finish(G_DBUS_CONNECTION(source_object), res, &error);
        if (v != nullptr)
        {
            g_message("%s got %s", G_STRFUNC, g_variant_print(v, true));
            g_variant_unref(v);
        }
        else if (error != nullptr)
        {
            if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
                g_warning("Couldn't get AccountsService proxy: %s", error->message);
            g_error_free(error);
        }
    }

    static void on_gps_enabled_reply(GObject      * source_object,
                                     GAsyncResult * res,
                                     gpointer       gself)
    {
        GError * error;
        GVariant * v;

        error = nullptr;
        v = g_dbus_connection_call_finish(G_DBUS_CONNECTION(source_object), res, &error);
        if (v != nullptr)
        {
            g_message("%s got %s", G_STRFUNC, g_variant_print(v, true));
            g_variant_unref(v);
        }
        else if (error != nullptr)
        {
            if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
                g_warning("Couldn't get AccountsService proxy: %s", error->message);
            g_error_free(error);
        }
    }

    /***
    ****
    ***/

    void set_bool_property(const char* property_name, bool b)
    {
        g_return_if_fail(m_system_bus);

        auto args = g_variant_new("(ssv)", LOC_IFACE_NAME, property_name, g_variant_new_boolean(b));
g_message("args: %s", g_variant_print(args, true));
        g_dbus_connection_call(m_system_bus.get(),
                               BUS_NAME,
                               OBJECT_PATH,
                               PROP_IFACE_NAME,
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

#if 0
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
#endif

    UbuntuAppLocController& m_owner;
    std::shared_ptr<GCancellable> m_cancellable {};
    std::shared_ptr<GDBusConnection> m_system_bus {};
    core::Property<bool> m_is_valid {false};
    bool m_is_gps_enabled {false};
    bool m_location_service_enabled {false};
    guint m_name_tag {};
    guint m_signal_tag {};
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

