//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
//

#ifndef MARBLE_ROUTEGUIDANCE_H
#define MARBLE_ROUTEGUIDANCE_H

#include "marble_export.h"

#include <QObject>

namespace Marble
{

class PositionTracking;
class Route;
class RouteRequest;
class RoutingManager;

class MARBLE_EXPORT RouteGuidance : public QObject
{
    Q_OBJECT
    Q_PROPERTY( bool guidanceModeEnabled READ guidanceModeEnabled WRITE setGuidanceModeEnabled NOTIFY guidanceModeEnabledChanged )
    Q_PROPERTY( bool deviatedFromRoute READ deviatedFromRoute NOTIFY deviatedFromRoute )

 public:
    explicit RouteGuidance( RoutingManager *routingManager, PositionTracking *positionTracking, QObject *parent = 0 );
    ~RouteGuidance();

    bool guidanceModeEnabled() const;

    /**
     * @brief Returns whether the GPS location is on route
     */
    bool deviatedFromRoute() const;

    /**
     * Returns true (default) if a warning is shown to the user when starting guidance
     * mode.
     */
    bool showGuidanceModeStartupWarning() const;

    const RouteRequest *routeRequest() const;

    const Route &route() const;

 public Q_SLOTS:
    /**
     * Toggle turn by turn navigation mode
     */
    void setGuidanceModeEnabled( bool enabled );

    /**
     * Set whether a warning message should be shown to the user before
     * starting guidance mode.
     */
    void setShowGuidanceModeStartupWarning( bool show );

    void reverseRoute();

 Q_SIGNALS:
    void guidanceModeEnabledChanged( bool enabled );

    /**
     * emits a signal regarding information about total time( seconds ) and distance( metres ) remaining to reach destination
     */
     void positionChanged();
     void deviatedFromRoute( bool deviated );

 private:
    Q_PRIVATE_SLOT( d, void updatePosition( const GeoDataCoordinates &, qreal ) )
    Q_PRIVATE_SLOT( d, void recalculateRoute( bool deviated ) )
    Q_PRIVATE_SLOT( d, void updateRoute() )

    class Private;
    Private *const d;
};

}

#endif
