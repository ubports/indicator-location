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

#include <iostream>

#include <QMap>
#include <QString>
#include <QVariant>

#include <QtLocation>

#include <service.h>

/***
****
***/

void
Service :: on_name_lost (GDBusConnection * connection,
                         const char * name,
                         gpointer user_data)
{
  static_cast<Service*>(user_data)->on_name_lost (connection, name);
}

void
Service :: on_name_lost (GDBusConnection * connection,
                         const char * name)
{
  Q_UNUSED (connection);

  g_message ("on name lost: %s", name);
}

/***
****
***/

void
Service :: on_bus_acquired (GDBusConnection * connection,
                            const char * name,
                            gpointer user_data)
{
  static_cast<Service*>(user_data)->on_bus_acquired (connection, name);
}

void
Service :: on_bus_acquired (GDBusConnection * connection,
                            const char * name)
{
  g_assert (_connection == 0);
  g_message ("on name acquired: %s", name);
  _connection = connection;

  // FIXME: export the actions/menus
}

/***
****
***/

Service :: Service ():
  _connection (0),
  _own_id (0),
  _source (QGeoPositionInfoSource::createDefaultSource (this)),
  _service_provider (0)
{
  g_message ("%s %s", G_STRLOC, G_STRFUNC);


  QStringList providerNames = QGeoServiceProvider::availableServiceProviders ();

  for (auto str: providerNames)
    std::cerr << qPrintable(str) << std::endl;

  if (providerNames.contains("nokia"))
    {
      QMap<QString, QVariant> parameters;
      parameters["app_id"] = "hello";
      parameters["token"] = "world";
      _service_provider = new QGeoServiceProvider ("nokia", parameters);
    }

  if (_service_provider == 0)
    g_warning ("No QGeoServiceProvider available!");

  if (_source != 0)
    {
      std::cerr << "_source.sourceName() is " << qPrintable(_source->sourceName()) << std::endl;

      connect (_source, SIGNAL(positionUpdated(QGeoPositionInfo)),
               this, SLOT(positionUpdated(QGeoPositionInfo)));
      _source->requestUpdate (1000);
      _source->startUpdates ();
    }

  _own_id = g_bus_own_name (G_BUS_TYPE_SESSION,
                            "com.canonical.indicator.location",
                            G_BUS_NAME_OWNER_FLAGS_NONE,
                            on_bus_acquired,
                            NULL,
                            on_name_lost,
                            this,
                            NULL);
}

Service :: ~Service ()
{
  if (_source)
    {
      _source->stopUpdates ();
      delete _source;
    }

  g_message ("%s %s", G_STRLOC, G_STRFUNC);
  g_bus_unown_name (_own_id);
}

/***
****
***/

void
Service :: onReplyFinished ()
{
  std::cerr << "_reply->isFinished " << _reply->isFinished() << std::endl;
  std::cerr << "_reply->errorString " << qPrintable(_reply->errorString()) << std::endl;
  std::cerr << "_reply->limit " << _reply->limit() << std::endl;
  std::cerr << "_reply->offset " << _reply->offset() << std::endl;

  //std::cerr << "_reply->viewport " << _reply->viewport() << std::endl;

  for (auto loc: _reply->locations())
    {
      const QGeoAddress address = loc.address();
      std::cerr << "text " << qPrintable(address.text()) << std::endl;
      std::cerr << "country " << qPrintable(address.country()) << std::endl;
      std::cerr << "countryCode " << qPrintable(address.countryCode()) << std::endl;
      std::cerr << "state " << qPrintable(address.state()) << std::endl;
      std::cerr << "county " << qPrintable(address.county()) << std::endl;
      std::cerr << "city " << qPrintable(address.city()) << std::endl;
      std::cerr << "district " << qPrintable(address.district()) << std::endl;
      std::cerr << "postalCode " << qPrintable(address.postalCode()) << std::endl;
      std::cerr << "street " << qPrintable(address.street()) << std::endl;
      std::cerr << "isEmpty " << address.isEmpty() << std::endl;
    }
}

void
Service :: setPosition (const QGeoPositionInfo& info)
{
  g_return_if_fail (info.isValid());

  _position = info;

  std::cerr << "new position: " << qPrintable(info.coordinate().toString()) << std::endl;

  QGeocodingManager * geocodingManager = _service_provider->geocodingManager ();

  g_message ("geocodingManager is %p", geocodingManager);
  std::cerr << "error string is " << qPrintable(_service_provider->errorString()) << std::endl;
  if (geocodingManager != 0)
    {
      _reply = geocodingManager->reverseGeocode (info.coordinate());
      connect (_reply, SIGNAL(finished()),
               this, SLOT(onReplyFinished()));
    }

  for (int i=0; i<=5; i++)
    {
      QGeoPositionInfo::Attribute attribute ((QGeoPositionInfo::Attribute)i);

      if (info.hasAttribute (attribute))
        {
          std::cerr << "has attribute " << attribute << ": " << info.attribute(attribute) << std::endl;
        }
    }
}

void
Service :: positionUpdated (const QGeoPositionInfo& info)

{
  if (info.isValid())
    setPosition (info);
}
