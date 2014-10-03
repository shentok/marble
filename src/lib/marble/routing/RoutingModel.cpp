//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010      Dennis Nienhüser <earthwings@gentoo.org>
//

#include "RoutingModel.h"

#include "MarbleMath.h"
#include "Route.h"
#include "RouteRequest.h"
#include "PositionTracking.h"
#include "MarbleGlobal.h"

#include <QPixmap>

namespace Marble
{

class RoutingModelPrivate
{
public:
    enum RouteDeviation
    {
        Unknown,
        OnRoute,
        OffRoute
    };

    RoutingModelPrivate( RouteRequest* request );

    Route m_route;

    RouteDeviation m_deviation;
    PositionTracking* m_positionTracking;
    RouteRequest* const m_request;
#if QT_VERSION >= 0x050000
    QHash<int, QByteArray> m_roleNames;
#endif
};

RoutingModelPrivate::RoutingModelPrivate( RouteRequest* request )
    : m_deviation( Unknown ),
      m_positionTracking( 0 ),
      m_request( request )
{
    // nothing to do
}

RoutingModel::RoutingModel( RouteRequest* request, QObject *parent ) :
        QAbstractListModel( parent ), d( new RoutingModelPrivate( request ) )
{
   QHash<int, QByteArray> roles;
   roles.insert( Qt::DisplayRole, "display" );
   roles.insert( RoutingModel::TurnTypeIconRole, "turnTypeIcon" );
   roles.insert( RoutingModel::LongitudeRole, "longitude" );
   roles.insert( RoutingModel::LatitudeRole, "latitude" );
#if QT_VERSION < 0x050000
   setRoleNames( roles );
#else
   d->m_roleNames = roles;
#endif
}

RoutingModel::~RoutingModel()
{
    delete d;
}

int RoutingModel::rowCount ( const QModelIndex &parent ) const
{
    return parent.isValid() ? 0 : d->m_route.turnPoints().size();
}

QVariant RoutingModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
    if ( orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 0 ) {
        return QString( "Instruction" );
    }

    return QAbstractListModel::headerData( section, orientation, role );
}

QVariant RoutingModel::data ( const QModelIndex & index, int role ) const
{
    if ( !index.isValid() ) {
        return QVariant();
    }

    if ( index.row() < d->m_route.turnPoints().size() && index.column() == 0 ) {
        const RouteSegment &segment = d->m_route.at( index.row() );
        switch ( role ) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            return segment.maneuver().instructionText();
            break;
        case Qt::DecorationRole:
            {
                bool const smallScreen = MarbleGlobal::getInstance()->profiles() & MarbleGlobal::SmallScreen;
                if ( segment.maneuver().hasWaypoint() ) {
                    int const size = smallScreen ? 64 : 32;
                    return d->m_request->pixmap( segment.maneuver().waypointIndex(), size, size/4 );
                } else {
                    QPixmap const pixmap = segment.maneuver().directionPixmap();
                    return smallScreen ? pixmap : pixmap.scaled( 32, 32 );
                }
            }
            break;
        case Qt::SizeHintRole:
            {
                bool const smallScreen = MarbleGlobal::getInstance()->profiles() & MarbleGlobal::SmallScreen;
                int const size = smallScreen ? 64 : 32;
                return QSize( size, size );
            }
            break;
        case RoutingModel::CoordinateRole:
            return QVariant::fromValue( segment.maneuver().position() );
            break;
        case RoutingModel::LongitudeRole:
            return QVariant::fromValue( segment.maneuver().position().longitude( GeoDataCoordinates::Degree ) );
            break;
        case RoutingModel::LatitudeRole:
            return QVariant::fromValue( segment.maneuver().position().latitude( GeoDataCoordinates::Degree ) );
            break;
        case RoutingModel::TurnTypeIconRole:
            return segment.maneuver().directionPixmap();
            break;
        default:
            return QVariant();
        }
    }

    return QVariant();
}

#if QT_VERSION >= 0x050000
QHash<int, QByteArray> RoutingModel::roleNames() const
{
    return d->m_roleNames;
}
#endif

void RoutingModel::setRoute( const Route &route )
{
    d->m_route = route;
    d->m_deviation = RoutingModelPrivate::Unknown;

    beginResetModel();
    endResetModel();
    emit currentRouteChanged();
}

void RoutingModel::clear()
{
    d->m_route = Route();
    beginResetModel();
    endResetModel();
    emit currentRouteChanged();
}

void RoutingModel::updatePosition( GeoDataCoordinates location, qreal /*speed*/ )
{
    d->m_route.setPosition( location );

    // Mark via points visited after approaching them in a range of 500m or less
    for ( int i = 0; i < d->m_request->size(); ++i ) {
        if ( !d->m_request->visited( i ) ) {
            qreal const threshold = 500 / EARTH_RADIUS;
            if ( distanceSphere( location, d->m_request->at( i ) ) < threshold ) {
                d->m_request->setVisited( i, true );
            }
        }
    }

    qreal distance = EARTH_RADIUS * distanceSphere( location, d->m_route.positionOnRoute() );
    emit positionChanged();

    qreal deviation = 0.0;
    if ( d->m_positionTracking && d->m_positionTracking->accuracy().vertical > 0.0 ) {
        deviation = qMax<qreal>( d->m_positionTracking->accuracy().vertical, d->m_positionTracking->accuracy().horizontal );
    }
    qreal const threshold = deviation + 100.0;

    RoutingModelPrivate::RouteDeviation const deviated = distance < threshold ? RoutingModelPrivate::OnRoute : RoutingModelPrivate::OffRoute;
    if ( d->m_deviation != deviated ) {
        d->m_deviation = deviated;
        emit deviatedFromRoute( deviated == RoutingModelPrivate::OffRoute );
    }
}

bool RoutingModel::deviatedFromRoute() const
{
    return d->m_deviation == RoutingModelPrivate::OffRoute;
}

const Route & RoutingModel::route() const
{
    return d->m_route;
}

} // namespace Marble

#include "RoutingModel.moc"
