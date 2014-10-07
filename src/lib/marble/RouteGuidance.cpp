//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
//

#include "RouteGuidance.h"

#include "MarbleDebug.h"
#include "MarbleDirs.h"
#include "MarbleMath.h"
#include "PositionProviderPlugin.h"
#include "PositionTracking.h"
#include "geodata/data/GeoDataCoordinates.h"
#include "routing/AlternativeRoutesModel.h"
#include "routing/Route.h"
#include "routing/RouteRequest.h"
#include "routing/RoutingManager.h"
#include "routing/RoutingModel.h"

#include <QMessageBox>
#include <QCheckBox>
#include <QDir>

namespace Marble
{

class RouteGuidance::Private
{
 public:
    enum RouteDeviation
    {
        Unknown,
        OnRoute,
        OffRoute
    };

    Private( RoutingManager *routingManager, PositionTracking *positionTracking, RouteGuidance *parent );

    void updatePosition( const GeoDataCoordinates &location, qreal speed );

    void recalculateRoute( bool deviated );

    void updateRoute();

    static QString stateFile();

    RouteGuidance *const q;

    RoutingManager *const m_routingManager;

    PositionTracking *const m_positionTracking;

    bool m_shutdownPositionTracking;

    bool m_guidanceModeEnabled;

    RouteDeviation m_deviation;

    bool m_guidanceModeWarning;
};

RouteGuidance::Private::Private( RoutingManager *routingManager, PositionTracking *positionTracking, RouteGuidance *parent ) :
    q( parent ),
    m_routingManager( routingManager ),
    m_positionTracking( positionTracking ),
    m_shutdownPositionTracking( false ),
    m_guidanceModeEnabled( false ),
    m_deviation( Unknown ),
    m_guidanceModeWarning( true )
{
}

void RouteGuidance::Private::updatePosition( const GeoDataCoordinates &coordinates, qreal speed )
{
    m_routingManager->routingModel()->updatePosition( coordinates, speed );

    // Mark via points visited after approaching them in a range of 500m or less
    for ( int i = 0; i < m_routingManager->routeRequest()->size(); ++i ) {
        if ( !m_routingManager->routeRequest()->visited( i ) ) {
            qreal const threshold = 500 / EARTH_RADIUS;
            if ( distanceSphere( coordinates, m_routingManager->routeRequest()->at( i ) ) < threshold ) {
                m_routingManager->routeRequest()->setVisited( i, true );
            }
        }
    }

    qreal distance = EARTH_RADIUS * distanceSphere( coordinates, m_routingManager->routingModel()->route().positionOnRoute() );
    emit q->positionChanged();

    qreal deviation = 0.0;
    if ( m_positionTracking && m_positionTracking->accuracy().vertical > 0.0 ) {
        deviation = qMax<qreal>( m_positionTracking->accuracy().vertical, m_positionTracking->accuracy().horizontal );
    }
    qreal const threshold = deviation + 100.0;

    Private::RouteDeviation const deviated = distance < threshold ? Private::OnRoute : Private::OffRoute;
    if ( m_deviation != deviated ) {
        m_deviation = deviated;
        emit q->deviatedFromRoute( deviated == Private::OffRoute );
    }
}

void RouteGuidance::Private::recalculateRoute( bool deviated )
{
    if ( !m_guidanceModeEnabled ) {
        return;
    }

    if ( !deviated ) {
        return;
    }

    RouteRequest *const request = m_routingManager->routeRequest();

    for ( int i = request->size()-3; i>=0; --i ) {
        if ( request->visited( i ) ) {
            request->remove( i );
        }
    }

    if ( request->size() == 2 && request->visited( 0 ) && !request->visited( 1 ) ) {
        request->setPosition( 0, m_positionTracking->currentLocation(), QObject::tr( "Current Location" ) );
        m_routingManager->retrieveRoute();
    } else if ( request->size() != 0 && !request->visited( request->size()-1 ) ) {
        request->insert( 0, m_positionTracking->currentLocation(), QObject::tr( "Current Location" ) );
        m_routingManager->retrieveRoute();
    }
}

void RouteGuidance::Private::updateRoute()
{
    m_deviation = Unknown;
}

QString RouteGuidance::Private::stateFile()
{
    QString const subdir = "routing";
    QDir dir( MarbleDirs::localPath() );
    if ( !dir.exists( subdir ) ) {
        if ( !dir.mkdir( subdir ) ) {
            mDebug() << "Unable to create dir " << dir.absoluteFilePath( subdir );
            return dir.absolutePath();
        }
    }

    if ( !dir.cd( subdir ) ) {
        mDebug() << "Cannot change into " << dir.absoluteFilePath( subdir );
    }

    return dir.absoluteFilePath( "guidance.kml" );
}

RouteGuidance::RouteGuidance( RoutingManager *routingManager, PositionTracking *positionTracking, QObject *parent ) :
    QObject( parent ),
    d( new Private( routingManager, positionTracking, this ) )
{
    connect( this, SIGNAL(deviatedFromRoute(bool)),
             this, SLOT(recalculateRoute(bool)) );
    connect( d->m_positionTracking, SIGNAL(gpsLocation(GeoDataCoordinates,qreal)),
             this, SLOT(updatePosition(GeoDataCoordinates,qreal)) );
    connect( d->m_routingManager->alternativeRoutesModel(), SIGNAL(currentRouteChanged(GeoDataDocument*)),
             this, SLOT(updateRoute()) );
    connect( d->m_routingManager, SIGNAL(positionChanged()),
             this, SIGNAL(positionChanged()) );
}

RouteGuidance::~RouteGuidance()
{
    delete d;
}

bool RouteGuidance::guidanceModeEnabled() const
{
    return d->m_guidanceModeEnabled;
}

bool RouteGuidance::deviatedFromRoute() const
{
    return d->m_deviation == Private::OffRoute;
}

bool RouteGuidance::showGuidanceModeStartupWarning() const
{
    return d->m_guidanceModeWarning;
}

const RouteRequest *RouteGuidance::routeRequest() const
{
    return d->m_routingManager->routeRequest();
}

const Route &RouteGuidance::route() const
{
    return d->m_routingManager->route();
}

void RouteGuidance::setGuidanceModeEnabled( bool enabled )
{
    if ( d->m_guidanceModeEnabled == enabled ) {
        return;
    }

    d->m_guidanceModeEnabled = enabled;

    if ( enabled ) {
        d->m_routingManager->saveRoute( d->stateFile() );

        if ( d->m_guidanceModeWarning ) {
            QString text = "<p>" + tr( "Caution: Driving instructions may be incomplete or wrong." );
            text += ' ' + tr( "Road construction, weather and other unforeseen variables can result in the suggested route not to be the most expedient or safest route to your destination." );
            text += ' ' + tr( "Please use common sense while navigating." ) + "</p>";
            text += "<p>" + tr( "The Marble development team wishes you a pleasant and safe journey." ) + "</p>";
            QPointer<QMessageBox> messageBox = new QMessageBox( QMessageBox::Information, tr( "Guidance Mode - Marble" ), text, QMessageBox::Ok );
            QCheckBox *showAgain = new QCheckBox( tr( "Show again" ) );
            showAgain->setChecked( true );
            showAgain->blockSignals( true ); // otherwise it'd close the dialog
            messageBox->addButton( showAgain, QMessageBox::ActionRole );
            const bool smallScreen = MarbleGlobal::getInstance()->profiles() & MarbleGlobal::SmallScreen;
            messageBox->resize( 380, smallScreen ? 400 : 240 );
            messageBox->exec();
            if ( !messageBox.isNull() ) {
                d->m_guidanceModeWarning = showAgain->isChecked();
            }
            delete messageBox;
        }
    } else {
        d->m_routingManager->loadRoute( d->stateFile() );
    }

    const bool isEnabled = d->m_positionTracking->isEnabled();
    if ( !isEnabled && enabled ) {
        d->m_positionTracking->setEnabled( true );
        d->m_shutdownPositionTracking = true;
    } else if ( isEnabled && !enabled && d->m_shutdownPositionTracking ) {
        d->m_shutdownPositionTracking = false;
        d->m_positionTracking->setEnabled( false );
    }

    emit guidanceModeEnabledChanged( d->m_guidanceModeEnabled );
}

void RouteGuidance::setShowGuidanceModeStartupWarning( bool show )
{
    d->m_guidanceModeWarning = show;
}

void RouteGuidance::reverseRoute()
{
    d->m_routingManager->reverseRoute();
}

}

#include "RouteGuidance.moc"
