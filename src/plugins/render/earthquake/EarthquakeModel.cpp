//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010 Utku Aydin <utkuaydin34@gmail.com>
//

#include "EarthquakeModel.h"
#include "EarthquakeItem.h"

#include "MarbleGlobal.h"
#include "MarbleModel.h"
#include "GeoDataCoordinates.h"
#include "GeoDataLatLonAltBox.h"
#include "MarbleDebug.h"

#include <QDebug>
#include <QString>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

namespace Marble {

EarthquakeModel::EarthquakeModel( const MarbleModel *marbleModel, QObject *parent )
    : AbstractDataPluginModel( "earthquake", marbleModel, parent ),
      m_minMagnitude( 0.0 ),
      m_startDate( QDateTime::fromString( "2006-02-04", "yyyy-MM-dd" ) ),
      m_endDate( QDateTime::currentDateTime() )
{
    // nothing to do
}

EarthquakeModel::~EarthquakeModel()
{
}

void EarthquakeModel::setMinMagnitude( double minMagnitude )
{
    m_minMagnitude = minMagnitude;
}

void EarthquakeModel::setStartDate( const QDateTime& startDate )
{
    m_startDate = startDate;
}

void EarthquakeModel::setEndDate( const QDateTime& endDate )
{
    m_endDate = endDate;
}

void EarthquakeModel::getAdditionalItems( const GeoDataLatLonAltBox& box, qint32 number )
{
    if (marbleModel()->planetId() != QLatin1String("earth")) {
        return;
    }

    const QString geonamesUrl( QLatin1String("http://ws.geonames.org/earthquakesJSON") +
        QLatin1String("?north=")   + QString::number(box.north().toDegree()) +
        QLatin1String("&south=")   + QString::number(box.south().toDegree()) +
        QLatin1String("&east=")    + QString::number(box.east().toDegree()) +
        QLatin1String("&west=")    + QString::number(box.west().toDegree()) +
        QLatin1String("&date=")    + m_endDate.toString("yyyy-MM-dd") +
        QLatin1String("&maxRows=") + QString::number(number) +
        QLatin1String("&username=marble") +
        QLatin1String("&formatted=true"));
    downloadDescriptionFile( QUrl( geonamesUrl ) );
}

void EarthquakeModel::parseFile( const QByteArray& file )
{
    QJsonDocument jsonDoc = QJsonDocument::fromJson(file);
    QJsonValue earthquakesValue = jsonDoc.object().value(QStringLiteral("earthquakes"));

    // Parse if any result exists
    if (earthquakesValue.isArray()) {
        // Add items to the list
        QList<AbstractDataPluginItem*> items;

        QJsonArray earthquakeArray = earthquakesValue.toArray();
        for (int earthquakeIndex = 0; earthquakeIndex < earthquakeArray.size(); ++earthquakeIndex) {
            QJsonObject levelObject = earthquakeArray[earthquakeIndex].toObject();

            // Converting earthquake's properties from JSON to appropriate types
            const QString eqid = levelObject.value(QStringLiteral("eqid")).toString(); // Earthquake's ID
            const GeoDataLongitude longitude = GeoDataLongitude::fromDegrees(levelObject.value(QStringLiteral("lng")).toDouble());
            const GeoDataLatitude latitude = GeoDataLatitude::fromDegrees(levelObject.value(QStringLiteral("lat")).toDouble());
            const double magnitude = levelObject.value(QStringLiteral("magnitude")).toDouble();
            const QString dateString = levelObject.value(QStringLiteral("datetime")).toString();
            const QDateTime date = QDateTime::fromString(dateString, QStringLiteral("yyyy-MM-dd hh:mm:ss"));
            const double depth = levelObject.value(QStringLiteral("depth")).toDouble();

            if( date <= m_endDate && date >= m_startDate && magnitude >= m_minMagnitude ) {
                if( !itemExists( eqid ) ) {
                    // If it does not exists, create it
                    const GeoDataCoordinates coordinates(longitude, latitude);
                    EarthquakeItem *item = new EarthquakeItem( this );
                    item->setId( eqid );
                    item->setCoordinate( coordinates );
                    item->setMagnitude( magnitude );
                    item->setDateTime( date );
                    item->setDepth( depth );
                    items << item;
                }
            }
        }

        addItemsToList( items );
    }
}


}

#include "moc_EarthquakeModel.cpp"
