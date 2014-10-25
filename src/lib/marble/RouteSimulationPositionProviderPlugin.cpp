//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011      Konrad Enzensberger <e.konrad@mpegcode.com>
// Copyright 2011      Dennis Nienhüser <earthwings@gentoo.org>
// Copyright 2012      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include "RouteSimulationPositionProviderPlugin.h"

#include <QTimer>

#include "MarbleMath.h"
#include "MarbleModel.h"
#include "routing/Route.h"
#include "routing/RoutingManager.h"
#include "routing/RoutingModel.h"

namespace Marble
{

namespace {
    qreal const c_frequency = 4.0; // Hz
}

QString RouteSimulationPositionProviderPlugin::name() const
{
    return tr( "Current Route Position Provider Plugin" );
}

QString RouteSimulationPositionProviderPlugin::nameId() const
{
    return "RouteSimulationPositionProviderPlugin";
}

QString RouteSimulationPositionProviderPlugin::guiString() const
{
    return tr( "Current Route" );
}

QString RouteSimulationPositionProviderPlugin::version() const
{
    return "1.1";
}

QString RouteSimulationPositionProviderPlugin::description() const
{
    return tr( "Simulates traveling along the current route." );
}

QString RouteSimulationPositionProviderPlugin::copyrightYears() const
{
    return "2011, 2012";
}

QList<PluginAuthor> RouteSimulationPositionProviderPlugin::pluginAuthors() const
{
    return QList<PluginAuthor>()
            << PluginAuthor( "Konrad Enzensberger", "e.konrad@mpegcode.com" )
            << PluginAuthor( QString::fromUtf8( "Dennis Nienhüser" ), "earthwings@gentoo.org" )
            << PluginAuthor( "Bernhard Beschow", "bbeschow@cs.tu-berlin.de" );
}

QIcon RouteSimulationPositionProviderPlugin::icon() const
{
    return QIcon();
}

PositionProviderPlugin* RouteSimulationPositionProviderPlugin::newInstance() const
{
    return new RouteSimulationPositionProviderPlugin( m_marbleModel );
}

PositionProviderStatus RouteSimulationPositionProviderPlugin::status() const
{
    return m_status;
}

GeoDataCoordinates RouteSimulationPositionProviderPlugin::position() const
{
    return m_currentPosition;
}

GeoDataAccuracy RouteSimulationPositionProviderPlugin::accuracy() const
{
    GeoDataAccuracy result;

    // faked values
    result.level = GeoDataAccuracy::Detailed;
    result.horizontal = 10.0;
    result.vertical = 10.0;

    return result;
}

RouteSimulationPositionProviderPlugin::RouteSimulationPositionProviderPlugin( MarbleModel *marbleModel ) :
    PositionProviderPlugin(),
    m_marbleModel( marbleModel ),
    m_status( PositionProviderStatusUnavailable ),
    m_currentDateTime(),
    m_speed( 0.0 ),
    m_direction( 0.0 )
{
    m_track.setInterpolate( true );
}

RouteSimulationPositionProviderPlugin::~RouteSimulationPositionProviderPlugin()
{
}

void RouteSimulationPositionProviderPlugin::initialize()
{
    m_track.clear();
    m_currentDateTime = QDateTime::currentDateTime();

    const Route route = m_marbleModel->routingManager()->routingModel()->route();
    int totalTime = 0;
    for ( int i = 0; i < route.size(); ++i ) {
        const RouteSegment &segment = route.at( i );
        const GeoDataLineString segmentPath = segment.path();
        const int segmentDuration = segment.travelTime();
        const qreal segmentLength = segment.distance();
        for ( int j = 0; j < segmentPath.size(); ++j ) {
            const qreal fraction = ( segmentLength - segmentPath.length( EARTH_RADIUS, j ) ) / segmentLength;
            const QDateTime when = m_currentDateTime.addMSecs( 1000 * totalTime + fraction * 1000 * segmentDuration );
            m_track.addPoint( when, segmentPath.at( j ) );
        }
        totalTime += segmentDuration;
    }

    m_status = m_track.isEmpty() ? PositionProviderStatusError : PositionProviderStatusAcquiring;

    if ( !m_track.isEmpty() ) {
        QTimer::singleShot( 1000.0 / c_frequency, this, SLOT(update()) );
    }
}

bool RouteSimulationPositionProviderPlugin::isInitialized() const
{
    return m_status != PositionProviderStatusUnavailable;
}

qreal RouteSimulationPositionProviderPlugin::speed() const
{
    return m_speed;
}

qreal RouteSimulationPositionProviderPlugin::direction() const
{
    return m_direction;
}

QDateTime RouteSimulationPositionProviderPlugin::timestamp() const
{
    return m_currentDateTime;
}

void RouteSimulationPositionProviderPlugin::update()
{
        if ( m_status != PositionProviderStatusAvailable ) {
            m_status = PositionProviderStatusAvailable;
            emit statusChanged( PositionProviderStatusAvailable );
        }

        const QDateTime newDateTime = QDateTime::currentDateTime();
        const GeoDataCoordinates newPosition = m_track.coordinatesAt( newDateTime );
        if ( m_currentPosition.isValid() ) {
            m_speed = distanceSphere( m_currentPosition, newPosition ) * m_marbleModel->planetRadius() / ( m_currentDateTime.msecsTo( newDateTime ) ) * 1000;
            m_direction = m_currentPosition.bearing( newPosition, GeoDataCoordinates::Degree, GeoDataCoordinates::FinalBearing );
        }
        m_currentPosition = newPosition;
        m_currentDateTime = newDateTime;
        emit positionChanged( position(), accuracy() );

    QTimer::singleShot( 1000.0 / c_frequency, this, SLOT(update()) );
}

} // namespace Marble

#include "RouteSimulationPositionProviderPlugin.moc"
