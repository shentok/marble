//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010      Dennis Nienhüser <nienhueser@kde.org>
//

#ifndef MARBLE_ROUTINGPOINT_H
#define MARBLE_ROUTINGPOINT_H

#include "marble_export.h"

#include "GeoDataAngle.h"
#include "GeoDataLatitude.h"
#include "GeoDataLongitude.h"

#include <QtGlobal>

class QTextStream;

namespace Marble
{

/**
  * There are many Point classes, but this is mine.
  */
class MARBLE_EXPORT RoutingPoint
{
public:
    explicit RoutingPoint(GeoDataLongitude lon = GeoDataLongitude::null, GeoDataLatitude lat = GeoDataLatitude::null);

    /** Longitude of the point */
    GeoDataLongitude lon() const;

    /** Latitude of the point */
    GeoDataLatitude lat() const;

    /**
      * Calculates the bearing of the line defined by this point
      * and the given other point.
      * Code based on https://www.movable-type.co.uk/scripts/latlong.html
      */
    GeoDataAngle bearing(const RoutingPoint &other) const;

    /**
      * Calculates the distance in meter between this point and the
      * given other point.
      * Code based on https://www.movable-type.co.uk/scripts/latlong.html
      */
    qreal distance( const RoutingPoint &other ) const;

private:
    GeoDataLongitude m_lon;
    GeoDataLatitude m_lat;
};

QTextStream& operator<<( QTextStream& stream, const RoutingPoint &i );

} // namespace Marble

#endif // MARBLE_ROUTINGPOINT_H
