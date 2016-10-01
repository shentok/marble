//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2015      Dennis Nienhüser <nienhueser@kde.org>
//

#include <OsmNode.h>

#include "osm/OsmObjectManager.h"
#include <GeoDataPlacemark.h>
#include <GeoDataStyle.h>
#include <GeoDataIconStyle.h>
#include <GeoDataDocument.h>
#include <MarbleDirs.h>
#include <StyleBuilder.h>

#include <QXmlStreamAttributes>

namespace Marble {

OsmNode OsmNode::fromOsmData(const OsmPlacemarkData &osmData)
{
    Q_ASSERT(osmData.containsTagKey("lat"));
    Q_ASSERT(osmData.containsTagKey("lon"));

    OsmNode result;

    result.osmData() = osmData;

    return result;
}

void OsmNode::parseCoordinates(const QXmlStreamAttributes &attributes)
{
    static const QString latKey = QLatin1String("lat");
    static const QString lonKey = QLatin1String("lon");

    const QString lonValue = attributes.value(lonKey).toString();
    const QString latValue = attributes.value(latKey).toString();

    m_osmData.insertTag(latKey, latValue);
    m_osmData.insertTag(lonKey, lonValue);
}

void OsmNode::setCoordinates(const GeoDataCoordinates &coordinates)
{
    static const QString latKey = QLatin1String("lat");
    static const QString lonKey = QLatin1String("lon");

    const QString latValue = QString::number(coordinates.latitude(GeoDataCoordinates::Degree), 'f', 7);
    const QString lonValue = QString::number(coordinates.longitude(GeoDataCoordinates::Degree), 'f', 7);

    m_osmData.insertTag(latKey, latValue);
    m_osmData.insertTag(lonKey, lonValue);
}

const GeoDataCoordinates OsmNode::coordinates() const
{
    if (!m_osmData.containsTagKey("lon") || !m_osmData.containsTagKey("lat")) {
        return GeoDataCoordinates();
    }

    const qreal lon = m_osmData.tagValue(QLatin1String("lon")).toDouble();
    const qreal lat = m_osmData.tagValue(QLatin1String("lat")).toDouble();

    return GeoDataCoordinates(lon, lat, 0, GeoDataCoordinates::Degree);
}

OsmPlacemarkData &OsmNode::osmData()
{
    return m_osmData;
}

const OsmPlacemarkData &OsmNode::osmData() const
{
    return m_osmData;
}

}
