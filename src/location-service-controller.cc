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

#include <glib.h>

#include "location-service-controller.h"
#include "utils.h"

/***
****
***/

class LocationServiceController::Impl
{
public:
    Impl()
    {
        m_cancellable.reset(g_cancellable_new(), [](GCancellable* c)
                            {
                                g_cancellable_cancel(c);
                                g_object_unref(c);
                            });

        g_bus_get(G_BUS_TYPE_SYSTEM, m_cancellable.get(), on_system_bus_ready, this);
    }

    ~Impl() = default;

    const core::Property<bool>& is_valid() const
    {
        return m_is_valid;
    }

    const core::Property<bool>& gps_enabled() const
    {
        return m_gps_enabled;
    }

    const core::Property<bool>& location_service_enabled() const
    {
        return m_loc_enabled;
    }

    const core::Property<bool>& location_service_active() const
    {
        return m_loc_active;
    }

    void set_gps_enabled(bool enabled)
    {
        set_bool_property(PROP_KEY_GPS_ENABLED, enabled);
    }

    void set_location_service_enabled(bool enabled)
    {
        set_bool_property(PROP_KEY_LOC_ENABLED, enabled);
    }

private:
    /***
    ****  bus bootstrapping & name watching
    ***/

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

            auto name_tag = g_bus_watch_name_on_connection(system_bus, BUS_NAME, G_BUS_NAME_WATCHER_FLAGS_AUTO_START,
                                                           on_name_appeared, on_name_vanished, gself, nullptr);

            //  manage the name_tag's lifespan
            self->m_name_tag.reset(new guint{name_tag}, [](guint* tag)
                                   {
                                       g_bus_unwatch_name(*tag);
                                       delete tag;
                                   });
        }
        else if (error != nullptr)
        {
            if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
            {
                g_warning("Couldn't get system bus: %s", error->message);
            }
            g_error_free(error);
        }
    }

    static void on_name_appeared(GDBusConnection* system_bus,
                                 const gchar* /*bus_name*/,
                                 const gchar* name_owner,
                                 gpointer gself)
    {
        auto self = static_cast<Impl*>(gself);

        // Why do we use PropertiesChanged, Get, and Set by hand instead
        // of letting gdbus-codegen or g_dbus_proxy_new() do the dirty work?
        // ubuntu-location-service's GetAll method is broken; proxies aren't
        // able to bootstrap themselves and cache the object properties. Ugh.

        // subscribe to PropertiesChanged signals from the service
        auto signal_tag = g_dbus_connection_signal_subscribe(
            system_bus, name_owner, PROP_IFACE_NAME, "PropertiesChanged", OBJECT_PATH,
            nullptr,  // arg0
            G_DBUS_SIGNAL_FLAGS_NONE, on_properties_changed, gself, nullptr);

        // manage the signal_tag lifespan
        g_object_ref(system_bus);
        self->m_signal_tag.reset(new guint{signal_tag}, [system_bus](guint* tag)
                                 {
                                     g_dbus_connection_signal_unsubscribe(system_bus, *tag);
                                     g_object_unref(system_bus);
                                     delete tag;
                                 });

        // GetAll is borked so call Get on each property we care about
        struct
        {
            const char* name;
            GAsyncReadyCallback callback;
        } props[] = {{PROP_KEY_LOC_ENABLED, on_loc_enabled_reply}, {PROP_KEY_GPS_ENABLED, on_gps_enabled_reply},
                     {PROP_KEY_LOC_STATE, on_loc_state_reply}};
        for (const auto& prop : props)
        {
            g_dbus_connection_call(system_bus, BUS_NAME, OBJECT_PATH, PROP_IFACE_NAME, "Get",
                                   g_variant_new("(ss)", LOC_IFACE_NAME, prop.name),  // args
                                   G_VARIANT_TYPE("(v)"),                             // return type
                                   G_DBUS_CALL_FLAGS_NONE,
                                   -1,  // use default timeout
                                   self->m_cancellable.get(), prop.callback, gself);
        }

        g_debug("setting is_valid to true: location-service appeared");
        self->m_is_valid.set(true);
    }

    static void on_name_vanished(GDBusConnection*, const gchar*, gpointer gself)
    {
        auto self = static_cast<Impl*>(gself);

        g_debug("setting is_valid to false: location-service vanished");
        self->m_is_valid.set(false);
        self->m_signal_tag.reset();
    }

    /***
    ****  org.freedesktop.dbus.properties.PropertiesChanged handling
    ***/

    static void on_properties_changed(GDBusConnection* /*connection*/,
                                      const gchar* /*sender_name*/,
                                      const gchar* /*object_path*/,
                                      const gchar* /*interface_name*/,
                                      const gchar* /*signal_name*/,
                                      GVariant* parameters,
                                      gpointer gself)
    {
        auto self = static_cast<Impl*>(gself);
        const gchar* interface_name;
        GVariant* changed_properties;
        const gchar** invalidated_properties;
        GVariantIter property_iter;
        const gchar* key;
        GVariant* val;

        g_variant_get(parameters, "(&s@a{sv}^a&s)", &interface_name, &changed_properties, &invalidated_properties);

        g_variant_iter_init(&property_iter, changed_properties);
        while (g_variant_iter_next(&property_iter, "{&sv}", &key, &val))
        {
            if (!g_strcmp0(key, PROP_KEY_LOC_ENABLED))
            {
                self->m_loc_enabled.set(g_variant_get_boolean(val));
            }
            else if (!g_strcmp0(key, PROP_KEY_GPS_ENABLED))
            {
                self->m_gps_enabled.set(g_variant_get_boolean(val));
            }
            else if (!g_strcmp0(key, PROP_KEY_LOC_STATE))
            {
                self->m_loc_active.set(g_variant_get_boolean(val));///!
            }

            g_variant_unref(val);
        }

        g_variant_unref(changed_properties);
        g_free(invalidated_properties);
    }

    /***
    ****  org.freedesktop.dbus.properties.Get handling
    ***/

    static std::tuple<bool, bool> get_bool_reply_from_call(GObject* source, GAsyncResult* res)
    {
        GError* error;
        GVariant* v;
        bool success{false};
        bool result{false};

        error = nullptr;
        GDBusConnection* conn = G_DBUS_CONNECTION(source);
        v = g_dbus_connection_call_finish(conn, res, &error);
        if (v != nullptr)
        {
            GVariant* inner{};
            g_variant_get(v, "(v)", &inner);
            success = true;
            result = g_variant_get_boolean(inner);
            g_variant_unref(inner);
            g_variant_unref(v);
        }
        else if (error != nullptr)
        {
            if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
            {
                g_warning("Error calling dbus method: %s", error->message);
            }
            g_error_free(error);
            success = false;
        }

        return std::make_tuple(success, result);
    }

    static void on_loc_enabled_reply(GObject* source_object, GAsyncResult* res, gpointer gself)
    {
        bool success, value;
        std::tie(success, value) = get_bool_reply_from_call(source_object, res);
        g_debug("service loc reply: success %d value %d", int(success), int(value));
        if (success)
        {
            static_cast<Impl*>(gself)->m_loc_enabled.set(value);
        }
    }

    static void on_gps_enabled_reply(GObject* source_object, GAsyncResult* res, gpointer gself)
    {
        bool success, value;
        std::tie(success, value) = get_bool_reply_from_call(source_object, res);
        g_debug("service gps reply: success %d value %d", int(success), int(value));
        if (success)
        {
            static_cast<Impl*>(gself)->m_gps_enabled.set(value);
        }
    }

    static void on_loc_state_reply(GObject* source_object, GAsyncResult* res, gpointer gself)
    {
        bool success, value;
        std::tie(success, value) = get_bool_reply_from_call(source_object, res);
        g_debug("service loc reply: success %d value %d", int(success), int(value));
        if (success)
        {
            static_cast<Impl*>(gself)->m_loc_active.set(value);///!
        }
    }

    /***
    ****  org.freedesktop.dbus.properties.Set handling
    ***/

    void set_bool_property(const char* property_name, bool b)
    {
        g_return_if_fail(m_system_bus);

        auto args = g_variant_new("(ssv)", LOC_IFACE_NAME, property_name, g_variant_new_boolean(b));
        g_dbus_connection_call(m_system_bus.get(), BUS_NAME, OBJECT_PATH, PROP_IFACE_NAME,
                               "Set",  // method name,
                               args,
                               nullptr,  // reply type
                               G_DBUS_CALL_FLAGS_NONE,
                               -1,  // timeout msec
                               m_cancellable.get(), check_method_call_reply, this);
    }

    static void check_method_call_reply(GObject* connection, GAsyncResult* res, gpointer /*gself*/)
    {
        GError* error;
        GVariant* v;

        error = nullptr;
        v = g_dbus_connection_call_finish(G_DBUS_CONNECTION(connection), res, &error);
        if (v != nullptr)
        {
            auto vs = g_variant_print(v, true);
            g_debug("method call returned '%s'", vs);
            g_free(vs);
            g_variant_unref(v);
        }
        else if (error != nullptr)
        {
            if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
            {
                g_warning("dbus method returned an error : %s", error->message);
            }

            g_error_free(error);
        }
    }

    /***
    ****
    ***/

    static constexpr const char* BUS_NAME{"com.ubuntu.location.Service"};
    static constexpr const char* OBJECT_PATH{"/com/ubuntu/location/Service"};
    static constexpr const char* LOC_IFACE_NAME{"com.ubuntu.location.Service"};
    static constexpr const char* PROP_IFACE_NAME{"org.freedesktop.DBus.Properties"};
    static constexpr const char* PROP_KEY_LOC_ENABLED{"IsOnline"};
    static constexpr const char* PROP_KEY_GPS_ENABLED{"DoesSatelliteBasedPositioning"};
    static constexpr const char* PROP_KEY_LOC_STATE{"State"};

    core::Property<bool> m_gps_enabled{false};
    core::Property<bool> m_loc_enabled{false};
    core::Property<bool> m_loc_active{false};
    core::Property<bool> m_is_valid{false};

    std::shared_ptr<GCancellable> m_cancellable{};
    std::shared_ptr<GDBusConnection> m_system_bus{};
    std::shared_ptr<guint> m_name_tag{};
    std::shared_ptr<guint> m_signal_tag{};
};

/***
****
***/

LocationServiceController::LocationServiceController()
    : impl{new Impl{}}
{
}

LocationServiceController::~LocationServiceController()
{
}

const core::Property<bool>& LocationServiceController::is_valid() const
{
    return impl->is_valid();
}

const core::Property<bool>& LocationServiceController::gps_enabled() const
{
    return impl->gps_enabled();
}

const core::Property<bool>& LocationServiceController::location_service_enabled() const
{
    return impl->location_service_enabled();
}

const core::Property<bool>& LocationServiceController::location_service_active() const
{
    return impl->location_service_active();
}

void LocationServiceController::set_gps_enabled(bool enabled)
{
    impl->set_gps_enabled(enabled);
}

void LocationServiceController::set_location_service_enabled(bool enabled)
{
    impl->set_location_service_enabled(enabled);
}
