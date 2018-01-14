//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011      Dennis Nienhüser <nienhueser@kde.org>
//

#include "RouteSegment.h"

#include "GeoDataLatLonAltBox.h"

namespace Marble
{

RouteSegment::RouteSegment() :
    m_valid( false ),
    m_distance( 0.0 ),
    m_travelTime( 0 ),
    m_nextRouteSegment( nullptr )
{
    // nothing to do
}

qreal RouteSegment::distance() const
{
    return m_distance;
}

const Maneuver & RouteSegment::maneuver() const
{
    return m_maneuver;
}

void RouteSegment::setManeuver( const Maneuver &maneuver )
{
    m_maneuver = maneuver;
    m_valid = true;
}

const GeoDataLineString & RouteSegment::path() const
{
    return m_path;
}

void RouteSegment::setPath( const GeoDataLineString &path )
{
    m_path = path;
    m_distance = m_path.length( EARTH_RADIUS );
    m_bounds = m_path.latLonAltBox();
    m_valid = true;
}

int RouteSegment::travelTime() const
{
    return m_travelTime;
}

void RouteSegment::setTravelTime( int seconds )
{
    m_travelTime = seconds;
    m_valid = true;
}

GeoDataLatLonBox RouteSegment::bounds() const
{
    return m_bounds;
}

const RouteSegment & RouteSegment::nextRouteSegment() const
{
    if ( m_nextRouteSegment ) {
        return *m_nextRouteSegment;
    }

    static RouteSegment invalid;
    return invalid;
}

void RouteSegment::setNextRouteSegment( const RouteSegment* segment )
{
    m_nextRouteSegment = segment;
    if ( segment ) {
        m_valid = true;
    }
}

bool RouteSegment::isValid() const
{
    return m_valid;
}

qreal RouteSegment::distancePointToLine(const GeoDataCoordinates &p, const GeoDataCoordinates &a, const GeoDataCoordinates &b)
{
    return EARTH_RADIUS * p.sphericalDistanceTo(projected(p, a, b));
}

GeoDataCoordinates RouteSegment::projected(const GeoDataCoordinates &p, const GeoDataCoordinates &a, const GeoDataCoordinates &b)
{
    auto const y0 = p.latitude();
    auto const x0 = p.longitude();
    auto const y1 = a.latitude();
    auto const x1 = a.longitude();
    auto const y2 = b.latitude();
    auto const x2 = b.longitude();
    qreal const y01 = (x0 - x1).toRadian();
    qreal const x01 = (y0 - y1).toRadian();
    qreal const y21 = (x2 - x1).toRadian();
    qreal const x21 = (y2 - y1).toRadian();
    qreal const len = x21*x21 + y21*y21;
    qreal const t = (x01*x21 + y01*y21) / len;
    if ( t<0.0 ) {
        return a;
    } else if ( t > 1.0 ) {
        return b;
    } else {
        // a + t (b - a);
        auto const lon = x1 + t * ( x2 - x1 );
        auto const lat = y1 + t * ( y2 - y1 );
        return GeoDataCoordinates( lon, lat );
    }

}

qreal RouteSegment::distanceTo( const GeoDataCoordinates &point, GeoDataCoordinates &closest, GeoDataCoordinates &interpolated ) const
{
    Q_ASSERT( !m_path.isEmpty() );

    if ( m_path.size() == 1 ) {
        closest = m_path.first();
        return EARTH_RADIUS * m_path.first().sphericalDistanceTo(point);
    }

    qreal minDistance = -1.0;
    int minIndex = 0;
    for ( int i=1; i<m_path.size(); ++i ) {
        qreal const distance = distancePointToLine( point, m_path[i-1], m_path[i] );
        if ( minDistance < 0.0 || distance < minDistance ) {
            minDistance = distance;
            minIndex = i;
        }
    }

    closest = m_path[minIndex];
    if ( minIndex == 0 ) {
        interpolated = closest;
    } else {
        interpolated = projected( point, m_path[minIndex-1], m_path[minIndex] );
    }

    return minDistance;
}

qreal RouteSegment::minimalDistanceTo( const GeoDataCoordinates &point ) const
{
    if ( bounds().contains( point) ) {
        return 0.0;
    }

    GeoDataCoordinates const northWest(bounds().west(), bounds().north());
    GeoDataCoordinates const northEast(bounds().east(), bounds().north());
    GeoDataCoordinates const southhWest(bounds().west(), bounds().south());
    GeoDataCoordinates const southEast(bounds().east(), bounds().south());

    qreal distNorth = distancePointToLine( point, northWest, northEast );
    qreal distEast = distancePointToLine( point, northEast, southEast );
    qreal distSouth = distancePointToLine( point, southhWest, southEast );
    qreal distWest = distancePointToLine( point, northWest, southhWest );
    return qMin( qMin( distNorth, distEast ), qMin( distWest, distSouth ) );
}

GeoDataAngle RouteSegment::projectedDirection(const GeoDataCoordinates &point) const
{
    if (m_path.size() < 2){
        return GeoDataAngle::fromRadians(0);
    }

    qreal minDistance = -1.0;
    int minIndex = 0;
    for ( int i=1; i<m_path.size(); ++i ) {
        qreal const distance = distancePointToLine( point, m_path[i-1], m_path[i] );
        if ( minDistance < 0.0 || distance < minDistance ) {
            minDistance = distance;
            minIndex = i;
        }
    }

    if ( minIndex == 0 ) {
        return m_path[0].bearing(m_path[1], GeoDataCoordinates::FinalBearing);
    } else {
        return m_path[minIndex-1].bearing(m_path[minIndex], GeoDataCoordinates::FinalBearing);
    }
}

bool RouteSegment::operator ==(const RouteSegment &other) const
{
    return  m_valid == other.m_valid &&
            m_distance == other.m_distance &&
            m_maneuver == other.m_maneuver &&
            m_travelTime == other.m_travelTime &&
            m_bounds == other.m_bounds &&
            m_nextRouteSegment == other.m_nextRouteSegment;
}

bool RouteSegment::operator !=(const RouteSegment &other) const
{
    return !(other == *this);
}

}
