//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2015      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include "GraphhopperRoutingRunner.h"

#include "MarbleDebug.h"
#include "MarbleLocale.h"
#include "GeoDataDocument.h"
#include "GeoDataPlacemark.h"
#include "GeoDataExtendedData.h"
#include "routing/Maneuver.h"
#include "routing/RouteRequest.h"
#include "TinyWebBrowser.h"

#include <QString>
#include <QVector>
#include <QUrl>
#include <QTime>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QScriptValue>
#include <QScriptEngine>
#include <QScriptValueIterator>

namespace Marble
{

GraphhopperRoutingRunner::GraphhopperRoutingRunner( QObject *parent ) :
    RoutingRunner( parent ),
    m_networkAccessManager()
{
    connect( &m_networkAccessManager, SIGNAL(finished(QNetworkReply*)),
             this, SLOT(retrieveData(QNetworkReply*)) );
}

GraphhopperRoutingRunner::~GraphhopperRoutingRunner()
{
    // nothing to do
}

void GraphhopperRoutingRunner::retrieveRoute( const RouteRequest *route )
{
    if ( route->size() < 2 ) {
        return;
    }

    QString url = "https://graphhopper.com/api/1/route?key=LijBPDQGfu7Iiq80w3HzwB4RUDJbMbhs6BU0dEnn&type=jsonp&instructions=true&callback=jQuery111209331333964626075_1425164173166&elevation=false&debug=true";
    url += "&vehicle=bike";
    url += "&locale=de";

    for ( int i = 0; i < route->size(); ++i ) {
        const qreal lat = route->at(i).latitude( GeoDataCoordinates::Degree );
        const qreal lon = route->at(i).longitude( GeoDataCoordinates::Degree );
        url += "&point=" + QString::number( lat ) + ',' + QString::number( lon );
    }

    m_request = QNetworkRequest( QUrl( url ) );
    m_request.setRawHeader( "User-Agent", TinyWebBrowser::userAgent( "Browser", "GraphhopperRoutingRunner" ) );

    QEventLoop eventLoop;

    QTimer timer;
    timer.setSingleShot( true );
    timer.setInterval( 15000 );

    connect( &timer, SIGNAL(timeout()),
             &eventLoop, SLOT(quit()));
    connect( this, SIGNAL(routeCalculated(GeoDataDocument*)),
             &eventLoop, SLOT(quit()) );

    // @todo FIXME Must currently be done in the main thread, see bug 257376
    QTimer::singleShot( 0, this, SLOT(get()) );
    timer.start();

    eventLoop.exec();
}

void GraphhopperRoutingRunner::retrieveData( QNetworkReply *reply )
{
    if ( reply->isFinished() ) {
        QByteArray data = reply->readAll();
        reply->deleteLater();
        GeoDataDocument* document = parse( data );

        if ( !document ) {
            mDebug() << "Failed to parse the downloaded route data" << data;
        }

        emit routeCalculated( document );
    }
}

void GraphhopperRoutingRunner::handleError( QNetworkReply::NetworkError error )
{
    mDebug() << " Error when retrieving GraphhopperRouting route: " << error;
}

void GraphhopperRoutingRunner::get()
{
    QNetworkReply *reply = m_networkAccessManager.get( m_request );
    connect( reply, SIGNAL(error(QNetworkReply::NetworkError)),
             this, SLOT(handleError(QNetworkReply::NetworkError)), Qt::DirectConnection );
}

void GraphhopperRoutingRunner::append(QString *input, const QString &key, const QString &value)
{
    *input += '&' + key + '=' + value;
}

GeoDataLineString *GraphhopperRoutingRunner::decodePolyline( const QString &geometry )
{
    // See https://developers.google.com/maps/documentation/utilities/polylinealgorithm
    GeoDataLineString* lineString = new GeoDataLineString;
    int coordinates[2] = { 0, 0 };
    int const length = geometry.length();
    for( int i=0; i<length; /* increment happens below */ ) {
        for ( int j=0; j<2; ++j ) { // lat and lon
            int block( 0 ), shift( 0 ), result( 0 );
            do {
                block = geometry.at( i++ /* increment for outer loop */ ).toLatin1() - 63;
                result |= ( block & 0x1F ) << shift;
                shift += 5;
            } while ( block >= 0x20 );
            coordinates[j] += ( ( result & 1 ) != 0 ? ~( result >> 1 ) : ( result >> 1 ) );
        }
        lineString->append( GeoDataCoordinates( double( coordinates[1] ) / 1E5,
                                                double( coordinates[0] ) / 1E5,
                                                0.0, GeoDataCoordinates::Degree ) );
    }
    return lineString;
}

RoutingInstruction::TurnType GraphhopperRoutingRunner::parseTurnType( int graphHopperTurnType )
{   
    switch ( graphHopperTurnType )
    {
    case -3:
        return RoutingInstruction::SharpLeft;
    case -2:
        return RoutingInstruction::Left;
    case -1:
        return RoutingInstruction::SlightLeft;
    case 0:
        return RoutingInstruction::Straight;
    case 1:
        return RoutingInstruction::SlightRight;
    case 2:
        return RoutingInstruction::Right;
    case 3:
        return RoutingInstruction::SharpRight;
    case 6:
        return RoutingInstruction::RoundaboutExit;
    case 4:
    case 5:
    default:
        break;
    }

    return RoutingInstruction::Unknown;
}

GeoDataDocument *GraphhopperRoutingRunner::parse( const QByteArray &input ) const
{
    QString strInput = QString::fromUtf8( input );
    strInput = strInput.mid( strInput.indexOf( '(' ) );
    QScriptEngine engine;
    // Qt requires parentheses around json code
    const QScriptValue data = engine.evaluate( strInput );

    if ( !data.property( "paths" ).isArray() ) {
        return 0;
    }

    QScriptValueIterator pathsIterator( data.property( "paths" ) );
    if ( !pathsIterator.hasNext() ) {
        return 0;
    }

    pathsIterator.next();

    GeoDataLineString *const routeWaypoints = decodePolyline( pathsIterator.value().property( "points" ).toString() );

    QTime time;
    time = time.addSecs( pathsIterator.value().property( "time" ).toNumber() );
    qreal length = routeWaypoints->length( EARTH_RADIUS );
    const QString name = nameString( "Graphhopper", length, time );
    const GeoDataExtendedData extendedData = routeData( length, time );

    GeoDataPlacemark *const routePlacemark = new GeoDataPlacemark;
    routePlacemark->setName( "Route" );
    routePlacemark->setGeometry( routeWaypoints );
    routePlacemark->setExtendedData( extendedData );

    GeoDataDocument *const result = new GeoDataDocument();
    result->setName( "Graphhopper" );
    result->setName( name );
    result->append( routePlacemark );

    if ( pathsIterator.value().property( "instructions" ).isArray() ) {
        bool first = true;
        QScriptValueIterator iterator( pathsIterator.value().property( "instructions" ) );
        GeoDataPlacemark* instruction = new GeoDataPlacemark;
        int lastWaypointIndex = 0;
        while ( iterator.hasNext() ) {
            iterator.next();
            const QVariantMap details = iterator.value().toVariant().toMap();
            const QString text = details.value( "text" ).toString();
            const int graphHopperTurnType = details.value( "sign" ).toInt();
            qDebug() << details;

            const QVariantList interval = details.value( "interval" ).toList();
            if ( interval.size() != 2 ) {
                continue;
            }

            const int waypointIndex = interval.at( 0 ).toInt();

            if ( waypointIndex < routeWaypoints->size() ) {
                if ( iterator.hasNext() ) {
                    GeoDataLineString *lineString = new GeoDataLineString;
                    for ( int i=lastWaypointIndex; i<=waypointIndex; ++i ) {
                        lineString->append(routeWaypoints->at( i ) );
                    }
                    instruction->setGeometry( lineString );
                    result->append( instruction );
                    instruction = new GeoDataPlacemark;
                }
                lastWaypointIndex = waypointIndex;
                GeoDataExtendedData extendedData;
                GeoDataData turnTypeData;
                turnTypeData.setName( "turnType" );
                RoutingInstruction::TurnType turnType = parseTurnType( graphHopperTurnType );
                turnTypeData.setValue( turnType );
                extendedData.addValue( turnTypeData );

                if ( first ) {
                    turnType = RoutingInstruction::Continue;
                    first = false;
                }

                if ( turnType == RoutingInstruction::Unknown ) {
                    instruction->setName( text );
                }
                instruction->setExtendedData( extendedData );

                if ( !iterator.hasNext() && lastWaypointIndex > 0 ) {
                    GeoDataLineString *lineString = new GeoDataLineString;
                    for ( int i=lastWaypointIndex; i<waypointIndex; ++i ) {
                        lineString->append(routeWaypoints->at( i ) );
                    }
                    instruction->setGeometry( lineString );
                    result->append( instruction );
                }
            }
        }
    }

    return result;
}

} // namespace Marble

#include "GraphhopperRoutingRunner.moc"
