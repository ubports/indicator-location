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
 *   Charles Kerr <charles.kerr@canonical.com>
 */

#include <ubuntu/status.h>
#include <ubuntu/application/location/controller.h>
#include <ubuntu/application/location/service.h>

#include <glib.h>

static UALocationServiceController * singleton = NULL;

struct UbuntuApplicationLocationServiceController
{
  int ref_count;
  gboolean loc_enabled;
  gboolean gps_enabled;
  UALocationServiceStatusChangedHandler changed_handler;
  void * changed_handler_context;
};

UALocationServiceController*
ua_location_service_create_controller(void)
{
  if (singleton == NULL)
  {
    UALocationServiceController * c = g_new0 (UALocationServiceController, 1);
    c->loc_enabled = FALSE;
    c->gps_enabled = FALSE;
    singleton = c;
  }

  singleton->ref_count++;

  return singleton;
}

void
ua_location_service_controller_set_status_changed_handler (UALocationServiceController *self,
                                                           UALocationServiceStatusChangedHandler changed_handler,
                                                           void *changed_handler_context)
{
  self->changed_handler = changed_handler;
  self->changed_handler_context = changed_handler_context;
}

static void
notify_status_changed (UALocationServiceController *self)
{
  if (self->changed_handler != NULL)
    {
      UALocationServiceStatusFlags flags = 0;
      ua_location_service_controller_query_status(self, &flags);
      self->changed_handler (flags, self->changed_handler_context);
    }
}
   
UStatus
ua_location_service_controller_query_status(UALocationServiceController *self,
                                            UALocationServiceStatusFlags *out_flags)
{
  UALocationServiceStatusFlags out = 0;

  g_return_val_if_fail (self != NULL, U_STATUS_ERROR);
  g_return_val_if_fail (self->ref_count >= 1, U_STATUS_ERROR);
  g_return_val_if_fail (out_flags != NULL, U_STATUS_ERROR);

  if (self->loc_enabled)
    out |= UA_LOCATION_SERVICE_ENABLED;

  if (self->gps_enabled)
    out |= UA_LOCATION_SERVICE_GPS_ENABLED;

  *out_flags = out;
  return U_STATUS_SUCCESS;
}

void
ua_location_service_controller_unref (UALocationServiceController *self)
{
    g_return_if_fail (self != NULL);
    g_return_if_fail (self->ref_count >= 1);

    if (!--self->ref_count)
    {
      g_free (self);
      singleton = NULL;
    }
}

static UStatus
set_gps_enabled (UALocationServiceController *self, gboolean enabled)
{
    g_return_if_fail (self != NULL);
    g_return_if_fail (self->ref_count >= 1);

    self->gps_enabled = enabled;
    notify_status_changed (self);

    return U_STATUS_SUCCESS;
}

UStatus
ua_location_service_controller_enable_gps (UALocationServiceController *self)
{
    return set_gps_enabled (self, TRUE);
}

UStatus
ua_location_service_controller_disable_gps (UALocationServiceController *self)
{
    return set_gps_enabled (self, FALSE);
}

static UStatus
set_service_enabled (UALocationServiceController *self, gboolean enabled)
{
    g_return_if_fail (self != NULL);
    g_return_if_fail (self->ref_count >= 1);

    self->loc_enabled = enabled;
    notify_status_changed (self);

    return U_STATUS_SUCCESS;
}

UStatus
ua_location_service_controller_enable_service (UALocationServiceController *self)
{
    return set_service_enabled (self, TRUE);
}

UStatus
ua_location_service_controller_disable_service (UALocationServiceController *self)
{
    return set_service_enabled (self, FALSE);
}
