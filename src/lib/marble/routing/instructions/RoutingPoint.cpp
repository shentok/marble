//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010      Dennis Nienhüser <nienhueser@kde.org>
//

#include "RoutingPoint.h"

#include <QTextStream>

#include <cmath>

namespace Marble
{

RoutingPoint::RoutingPoint(GeoDataLongitude lon, GeoDataLatitude lat) :
    m_lon(lon),
    m_lat(lat)
{
    // nothing to do
}

GeoDataLongitude RoutingPoint::lon() const
{
    return m_lon;
}

GeoDataLatitude RoutingPoint::lat() const
{
    return m_lat;
}

// Code based on https://www.movable-type.co.uk/scripts/latlong.html
GeoDataAngle RoutingPoint::bearing(const RoutingPoint &other) const
{
    const qreal deltaLon = (other.m_lon - m_lon).toRadian();
    const qreal y = sin(deltaLon) * cos(other.m_lat.toRadian());
    const qreal x = cos(m_lat.toRadian()) * sin(other.m_lat.toRadian()) -
              sin(m_lat.toRadian()) * cos(other.m_lat.toRadian()) * cos(deltaLon);

    return GeoDataAngle::fromRadians(atan2(y, x));
}

// From MarbleMath::distanceSphere
qreal RoutingPoint::distance( const RoutingPoint &other ) const
{
    const qreal h1 = sin(0.5 * (other.m_lat - m_lat).toRadian());
    const qreal h2 = sin(0.5 * (other.m_lon - m_lon).toRadian());
    const qreal d = h1 * h1 + cos(m_lat.toRadian()) * cos(other.m_lat.toRadian()) * h2 * h2;

    return 6378137.0 * 2.0 * atan2( sqrt( d ), sqrt( 1.0 - d ) );
}

QTextStream& operator<<( QTextStream& stream, const RoutingPoint &p )
{
    stream << "(" << p.lon().toDegree() << ", " << p.lat().toDegree() << ")";
    return stream;
}

} // namespace Marble
