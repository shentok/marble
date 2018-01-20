//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2007      Andrew Manson  <g.real.ate@gmail.com>
// Copyright 2008-2009 Torsten Rahn   <rahn@kde.org>
//


#include "GeoDataNormalizedLongitude.h"


namespace Marble
{

GeoDataNormalizedLongitude GeoDataNormalizedLongitude::fromLongitude(GeoDataLongitude lon)
{
    const GeoDataLongitude halfCircle = GeoDataLongitude::halfCircle;

    if ( lon > halfCircle ) {
        int cycles = (int)( ( lon + halfCircle ) / ( 2 * halfCircle ) );
        return GeoDataNormalizedLongitude((lon - ( cycles * 2 * halfCircle )).toRadian());
    }

    if ( lon < -halfCircle ) {
        int cycles = (int)( ( lon - halfCircle ) / ( 2 * halfCircle ) );
        return GeoDataNormalizedLongitude((lon - ( cycles * 2 * halfCircle )).toRadian());
    }

    return GeoDataNormalizedLongitude(lon.toRadian());
}

GeoDataNormalizedLongitude GeoDataNormalizedLongitude::center(GeoDataNormalizedLongitude west, GeoDataNormalizedLongitude east)
{
    if (crossesDateLine(east, west)) {
        return fromLongitude(east + 2 * halfCircle - (east + 2 * halfCircle - west) / 2);
    }

    return GeoDataNormalizedLongitude::fromRadians((west + east).toRadian() / 2);
}

bool GeoDataNormalizedLongitude::crossesDateLine(GeoDataNormalizedLongitude east, GeoDataNormalizedLongitude west)
{
    return east < west || (east == halfCircle && west == -halfCircle);
}

GeoDataLongitude GeoDataNormalizedLongitude::width(GeoDataNormalizedLongitude east, GeoDataNormalizedLongitude west)
{
    GeoDataLongitude width = qAbs(GeoDataNormalizedLongitude::crossesDateLine(east, west)
                                     ? 2 * GeoDataLongitude::halfCircle - west + east
                                     : east - west);

    // This also covers the case where this bounding box covers the whole
    // longitude range ( -180 <= lon <= + 180 ).
    if (width > 2 * GeoDataLongitude::halfCircle) {
        width = 2 * GeoDataLongitude::halfCircle;
    }

    return width;
}

}
