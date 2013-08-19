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

#ifndef __INDICATOR_LOCATION_SERVICE_H__
#define __INDICATOR_LOCATION_SERVICE_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

/* standard GObject macros */
#define INDICATOR_TYPE_LOCATION_SERVICE          (indicator_location_service_get_type())
#define INDICATOR_LOCATION_SERVICE(o)            (G_TYPE_CHECK_INSTANCE_CAST ((o), INDICATOR_TYPE_LOCATION_SERVICE, IndicatorLocationService))
#define INDICATOR_LOCATION_SERVICE_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS ((o), INDICATOR_TYPE_LOCATION_SERVICE, IndicatorLocationServiceClass))
#define INDICATOR_LOCATION_SERVICE_CLASS(k)      (G_TYPE_CHECK_CLASS_CAST ((k), INDICATOR_TYPE_LOCATION_SERVICE, IndicatorLocationServiceClass))
#define INDICATOR_IS_LOCATION_SERVICE(o)         (G_TYPE_CHECK_INSTANCE_TYPE ((o), INDICATOR_TYPE_LOCATION_SERVICE))

typedef struct _IndicatorLocationService         IndicatorLocationService;
typedef struct _IndicatorLocationServiceClass    IndicatorLocationServiceClass;
typedef struct _IndicatorLocationServicePrivate  IndicatorLocationServicePrivate;

/* signal keys */
#define INDICATOR_LOCATION_SERVICE_SIGNAL_NAME_LOST   "name-lost"

/**
 * The Indicator Location Service.
 */
struct _IndicatorLocationService
{
  /*< private >*/
  GObject parent;
  IndicatorLocationServicePrivate * priv;
};

struct _IndicatorLocationServiceClass
{
  GObjectClass parent_class;

  /* signals */
  void (* name_lost)(IndicatorLocationService * self);
};

/***
****
***/

GType indicator_location_service_get_type (void);

IndicatorLocationService * indicator_location_service_new (void);

G_END_DECLS

#endif /* __INDICATOR_LOCATION_SERVICE_H__ */

