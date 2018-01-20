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


#include "GeoDataNormalizedLatitude.h"


namespace Marble
{

GeoDataNormalizedLatitude GeoDataNormalizedLatitude::fromLatitude(GeoDataLatitude lat)
{
    const GeoDataLatitude halfCircle = 2 * GeoDataLatitude::quaterCircle;

    if ( lat > ( halfCircle / 2.0 ) ) {
        int cycles = (int)( ( lat + halfCircle ) / ( 2 * halfCircle ) );
        GeoDataLatitude temp;
        if (cycles == 0) { // pi/2 < lat < pi
            temp = halfCircle - lat;
        } else {
            temp = lat - ( cycles * 2 * halfCircle );
        }

        if ( temp > ( halfCircle / 2.0 ) ) {
            return GeoDataNormalizedLatitude((halfCircle - temp).toRadian());
        }

        if ( temp < ( -halfCircle / 2.0 ) ) {
            return GeoDataNormalizedLatitude((-halfCircle - temp).toRadian());
        }

        return GeoDataNormalizedLatitude(temp.toRadian());
    }

    if ( lat < ( -halfCircle / 2.0 ) ) {
        int cycles = (int)( ( lat - halfCircle ) / ( 2 * halfCircle ) );
        GeoDataLatitude temp;
        if( cycles == 0 ) {
            temp = -halfCircle - lat;
        } else {
            temp = lat - ( cycles * 2 * halfCircle );
        }

        if ( temp > ( +halfCircle / 2.0 ) ) {
            return GeoDataNormalizedLatitude((+halfCircle - temp).toRadian());
        }

        if ( temp < ( -halfCircle / 2.0 ) ) {
            return GeoDataNormalizedLatitude((-halfCircle - temp).toRadian());
        }

        return GeoDataNormalizedLatitude(temp.toRadian());
    }

    return GeoDataNormalizedLatitude(lat.toRadian());
}

GeoDataNormalizedLatitude GeoDataNormalizedLatitude::center(GeoDataNormalizedLatitude lhs, GeoDataNormalizedLatitude rhs)
{
    return GeoDataNormalizedLatitude::fromRadians((lhs + rhs).toRadian() / 2);
}

GeoDataLatitude GeoDataNormalizedLatitude::height(GeoDataNormalizedLatitude north, GeoDataNormalizedLatitude south)
{
    const GeoDataLatitude height = qAbs(south - north);

    return height;
}

}
