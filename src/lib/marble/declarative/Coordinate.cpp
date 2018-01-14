//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011      Dennis Nienhüser <nienhueser@kde.org>
//

#include "Coordinate.h"

#include "MarbleGlobal.h"

using Marble::GeoDataCoordinates;
using Marble::GeoDataLatitude;
using Marble::GeoDataLongitude;
using Marble::EARTH_RADIUS;
using Marble::DEG2RAD;

Coordinate::Coordinate( qreal lon, qreal lat, qreal alt, QObject *parent ) :
    QObject( parent )
{
    setLongitude( lon );
    setLatitude( lat );
    setAltitude( alt );
}

Coordinate::Coordinate(const Marble::GeoDataCoordinates &coordinates)
{
    setCoordinates(coordinates);
}

qreal Coordinate::longitude() const
{
    return m_coordinate.longitude().toDegree();
}

void Coordinate::setLongitude( qreal lon )
{
    m_coordinate.setLongitude(Marble::GeoDataLongitude::fromDegrees(lon));
    emit longitudeChanged();
}

qreal Coordinate::latitude() const
{
    return m_coordinate.latitude().toDegree();
}

void Coordinate::setLatitude( qreal lat )
{
    m_coordinate.setLatitude(Marble::GeoDataLatitude::fromDegrees(lat));
    emit latitudeChanged();
}

qreal Coordinate::altitude() const
{
    return m_coordinate.altitude();
}

void Coordinate::setAltitude( qreal alt )
{
    m_coordinate.setAltitude( alt );
    emit altitudeChanged();
}

GeoDataCoordinates Coordinate::coordinates() const
{
    return m_coordinate;
}

void Coordinate::setCoordinates( const GeoDataCoordinates &coordinates )
{
    m_coordinate = coordinates;
}

qreal Coordinate::distance( qreal longitude, qreal latitude ) const
{
    const GeoDataCoordinates other(GeoDataLongitude::fromDegrees(longitude), GeoDataLatitude::fromDegrees(latitude));
    return EARTH_RADIUS * coordinates().sphericalDistanceTo(other);
}

qreal Coordinate::bearing( qreal longitude, qreal latitude ) const
{
    auto deltaLon = GeoDataLongitude::fromDegrees(longitude) - m_coordinate.longitude();
    qreal y = sin(deltaLon.toRadian()) * cos( latitude * DEG2RAD );
    qreal x = cos( m_coordinate.latitude().toRadian() ) * sin( latitude * DEG2RAD ) -
              sin( m_coordinate.latitude().toRadian() ) * cos( latitude * DEG2RAD ) * cos(deltaLon.toRadian());
    return Marble::RAD2DEG * atan2( y, x );
}

bool Coordinate::operator == ( const Coordinate &other ) const
{
    return m_coordinate == other.m_coordinate;
}

bool Coordinate::operator != ( const Coordinate &other ) const
{
    return !operator == ( other );
}

#include "moc_Coordinate.cpp"
