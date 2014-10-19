//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011      Dennis Nienhüser <nienhueser@kde.org>
//

#include "Route.h"

#include <QTime>

namespace Marble
{

Route::Route() :
    m_providerName(),
    m_distance( 0.0 ),
    m_travelTime( 0 ),
    m_positionDirty( true ),
    m_closestSegmentIndex( -1 )
{
    // nothing to do
}

void Route::setProviderName(const QString &providerName)
{
    m_providerName = providerName;
}

QString Route::providerName() const
{
    return m_providerName;
}

void Route::addRouteSegment( const RouteSegment &segment )
{
    if ( segment.isValid() ) {
        m_bounds = m_bounds.united( segment.bounds() );
        m_distance += segment.distance();
        const GeoDataLineString path = segment.path();
        if (!m_path.isEmpty() && !path.isEmpty() && m_path.last() == path.first()) {
            m_path.remove(m_path.size() - 1);
        }
        m_path << path;
        if ( segment.maneuver().position().isValid() ) {
            m_turnPoints << segment.maneuver().position();
        }
        if ( segment.maneuver().hasWaypoint() ) {
            m_waypoints << segment.maneuver().waypoint();
        }
        m_segments.push_back( segment );
        m_positionDirty = true;

        for ( int i=1; i<m_segments.size(); ++i ) {
            m_segments[i-1].setNextRouteSegment(&m_segments[i]);
        }
    }
}

GeoDataLatLonBox Route::bounds() const
{
    return m_bounds;
}

qreal Route::distance() const
{
    return m_distance;
}

int Route::size() const
{
    return m_segments.size();
}

const RouteSegment & Route::at( int index ) const
{
    return m_segments[index];
}

int Route::indexOf(const RouteSegment &segment) const
{
    return m_segments.indexOf(segment);
}

const GeoDataLineString & Route::path() const
{
    return m_path;
}

int Route::travelTime() const
{
    return m_travelTime;
}

QString Route::guiString() const
{
    return QString("%1; %2 (%3)").arg(lengthString(m_distance)).arg(durationString(QTime().addSecs(m_travelTime))).arg(m_providerName);
}

const GeoDataLineString & Route::turnPoints() const
{
    return m_turnPoints;
}

const GeoDataLineString & Route::waypoints() const
{
    return m_waypoints;
}

void Route::setPosition( const GeoDataCoordinates &position )
{
    m_position = position;
    m_positionDirty = true;
}

GeoDataCoordinates Route::position() const
{
    return m_position;
}

void Route::updatePosition() const
{
    if ( !m_segments.isEmpty() ) {
        if ( m_closestSegmentIndex < 0 || m_closestSegmentIndex >= m_segments.size() ) {
            m_closestSegmentIndex = 0;
        }

        qreal distance = m_segments[m_closestSegmentIndex].distanceTo( m_position, m_currentWaypoint, m_positionOnRoute );
        QList<int> candidates;

        for ( int i=0; i<m_segments.size(); ++i ) {
            if ( i != m_closestSegmentIndex && m_segments[i].minimalDistanceTo( m_position ) <= distance ) {
                candidates << i;
            }
        }

        GeoDataCoordinates closest, interpolated;
        for( int i: candidates ) {
            qreal const dist = m_segments[i].distanceTo( m_position, closest, interpolated );
            if ( distance < 0.0 || dist < distance ) {
                distance = dist;
                m_closestSegmentIndex = i;
                m_positionOnRoute = interpolated;
                m_currentWaypoint = closest;
            }
        }
    }

    m_positionDirty = false;
}

const RouteSegment & Route::currentSegment() const
{
    if ( m_positionDirty ) {
        updatePosition();
    }

    if ( m_closestSegmentIndex < 0 || m_closestSegmentIndex >= m_segments.size() ) {
        static RouteSegment invalid;
        return invalid;
    }

    return m_segments[m_closestSegmentIndex];
}

GeoDataCoordinates Route::positionOnRoute() const
{
    if ( m_positionDirty ) {
        updatePosition();
    }

    return m_positionOnRoute;
}

GeoDataCoordinates Route::currentWaypoint() const
{
    if ( m_positionDirty ) {
        updatePosition();
    }

    return m_currentWaypoint;
}

const QString Route::lengthString(qreal length)
{
    QString unit = QLatin1String("m");
    if (length >= 1000) {
        length /= 1000.0;
        unit = "km";
    }

    return QString("%1 %2").arg(length, 0, 'f', 1).arg(unit);
}

const QString Route::durationString(const QTime &duration)
{
    const QString hoursString = duration.toString("hh");
    const QString minutesString = duration.toString("mm");
    const QString timeString = QObject::tr("%1:%2 h","journey duration").arg(hoursString).arg(minutesString);

    return timeString;
}

}
