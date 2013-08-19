/*
 * Copyright 2013 Canonical Ltd.
 *
 * Authors:
 *   Charles Kerr <charles.kerr@canonical.com>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib/gi18n.h>
#include <gio/gio.h>

#include "service.h"

#define BUS_NAME "com.canonical.indicator.location"
#define BUS_PATH "/com/canonical/indicator/location"

G_DEFINE_TYPE (IndicatorLocationService,
               indicator_location_service,
               G_TYPE_OBJECT)

/* signals enum */
enum
{
  NAME_LOST,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

struct _IndicatorLocationServicePrivate
{
  guint own_id;
  GDBusConnection * connection;
  GCancellable * cancellable;
};

typedef IndicatorLocationServicePrivate priv_t;


/***
****  GDBus
***/

static void
on_name_lost (GDBusConnection * connection   G_GNUC_UNUSED,
              const char      * name,
              gpointer          gself)
{
  g_debug ("%s::%s: %s", G_STRLOC, G_STRFUNC, name);

  g_signal_emit (gself, signals[NAME_LOST], 0, NULL);
}

static void
on_bus_acquired (GDBusConnection * connection,
                 const char      * name,
                 gpointer          gself)
{
  IndicatorLocationService * self;

  g_debug ("%s::%s: %s", G_STRLOC, G_STRFUNC, name);

  self = INDICATOR_LOCATION_SERVICE (gself);
  self->priv->connection = G_DBUS_CONNECTION (g_object_ref (connection));
}


/***
****  GObject plumbing: life cycle
***/

static void
indicator_location_service_init (IndicatorLocationService * self)
{
  /* init our priv pointer */
  priv_t * p = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                            INDICATOR_TYPE_LOCATION_SERVICE,
                                            IndicatorLocationServicePrivate);
  self->priv = p;

  p->cancellable = g_cancellable_new ();

  p->own_id = g_bus_own_name (G_BUS_TYPE_SESSION,
                              BUS_NAME,
                              G_BUS_NAME_OWNER_FLAGS_NONE,
                              on_bus_acquired,
                              NULL,
                              on_name_lost,
                              self,
                              NULL);
}

static void
indicator_location_service_dispose (GObject * o)
{
  IndicatorLocationService * self = INDICATOR_LOCATION_SERVICE(o);
  priv_t * p = self->priv;

  if (p->own_id)
    {
      g_bus_unown_name (p->own_id);
      p->own_id = 0;
    }

  if (p->cancellable != NULL)
    {
      g_cancellable_cancel (p->cancellable);
      g_clear_object (&p->cancellable);
    }

  g_clear_object (&p->connection);

  G_OBJECT_CLASS (indicator_location_service_parent_class)->dispose (o);
}

static void
indicator_location_service_class_init (IndicatorLocationServiceClass * klass)
{
  GObjectClass * object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = indicator_location_service_dispose;

  g_type_class_add_private (klass, sizeof (IndicatorLocationServicePrivate));

  signals[NAME_LOST] = g_signal_new (INDICATOR_LOCATION_SERVICE_SIGNAL_NAME_LOST,
                                     G_TYPE_FROM_CLASS(klass),
                                     G_SIGNAL_RUN_LAST,
                                     G_STRUCT_OFFSET (IndicatorLocationServiceClass, name_lost),
                                     NULL, NULL,
                                     g_cclosure_marshal_VOID__VOID,
                                     G_TYPE_NONE, 0);
}

IndicatorLocationService *
indicator_location_service_new (void)
{
  return INDICATOR_LOCATION_SERVICE (g_object_new (INDICATOR_TYPE_LOCATION_SERVICE, NULL));
}
