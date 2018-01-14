/*
    Copyright 2016 Friedrich W. H. Kossebau  <kossebau@kde.org>

    This file is part of the KDE project

    This library is free software you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    aint with this library see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "GeoSceneMercatorTileProjection.h"

#include <GeoDataLatLonBox.h>
#include <MarbleMath.h>


namespace Marble
{

GeoSceneMercatorTileProjection::GeoSceneMercatorTileProjection()
{
}


GeoSceneMercatorTileProjection::~GeoSceneMercatorTileProjection()
{
}

GeoSceneAbstractTileProjection::Type GeoSceneMercatorTileProjection::type() const
{
    return Mercator;
}


static inline
unsigned int lowerBoundTileIndex(qreal baseTileIndex)
{
    const qreal floorBaseTileIndex = floor(baseTileIndex);
    unsigned int tileIndex = static_cast<unsigned int>(floorBaseTileIndex);
    return (baseTileIndex == floorBaseTileIndex) ? tileIndex-1 : tileIndex;
}

static inline
unsigned int upperBoundTileIndex(qreal baseTileIndex)
{
    return (unsigned int)floor(baseTileIndex);
}

static inline
qreal baseTileXFromLon(GeoDataLongitude lon, unsigned int tileCount)
{
    return 0.5 * (lon / GeoDataLongitude::halfCircle + 1.0) * tileCount;
}

static inline
qreal baseTileYFromLat(GeoDataLatitude latitude, unsigned int tileCount)
{
    // We need to calculate the tile position from the latitude
    // projected using the Mercator projection. This requires the inverse Gudermannian
    // function which is only defined between -85°S and 85°N. Therefore in order to
    // prevent undefined results we need to restrict our calculation.
    // Using 85.0 instead of some more correct 85.05113, to avoid running into NaN issues.
    GeoDataLatitude maxAbsLat = GeoDataLatitude::fromDegrees(85.0);
    GeoDataLatitude lat = (qAbs(latitude) > maxAbsLat) ? latitude/qAbs(latitude) * maxAbsLat : latitude;
    return (0.5 * (1.0 - gdInv(lat) / M_PI) * tileCount);
}

// on tile borders selects the tile to the east
static inline
unsigned int eastBoundTileXFromLon(GeoDataLongitude lon, unsigned int tileCount)
{
    // special casing tile-map end
    if (lon == GeoDataLongitude::halfCircle) {
        return 0;
    }
    return upperBoundTileIndex(baseTileXFromLon(lon, tileCount));
}

// on tile borders selects the tile to the west
static inline
unsigned int westBoundTileXFromLon(GeoDataLongitude lon, unsigned int tileCount)
{
    // special casing tile-map end
    if (lon == -GeoDataLongitude::halfCircle) {
        return tileCount-1;
    }
    return lowerBoundTileIndex(baseTileXFromLon(lon, tileCount));
}

// on tile borders selects the tile to the south
static inline
unsigned int southBoundTileYFromLat(GeoDataLatitude lat, unsigned int tileCount)
{
    // special casing tile-map end
    if (lat == -GeoDataLatitude::quaterCircle) {
        // calculate with normal lat value
        lat = GeoDataLatitude::quaterCircle;
    }
    return upperBoundTileIndex(baseTileYFromLat(lat, tileCount));
}

// on tile borders selects the tile to the north
static inline
unsigned int northBoundTileYFromLat(GeoDataLatitude lat, unsigned int tileCount)
{
    // special casing tile-map end
    if (lat == GeoDataLatitude::quaterCircle) {
        // calculate with normal lat value
        lat = -GeoDataLatitude::quaterCircle;
    }
    return lowerBoundTileIndex(baseTileYFromLat(lat, tileCount));
}


static inline
GeoDataLongitude lonFromTileX(unsigned int x, unsigned int tileCount)
{
    return (2 * GeoDataLongitude::halfCircle * x) / tileCount - GeoDataLongitude::halfCircle;
}

static inline
GeoDataLatitude latFromTileY(unsigned int y, unsigned int tileCount)
{
    return gd(M_PI * (1.0 - (2.0 * y) / tileCount));
}


QRect GeoSceneMercatorTileProjection::tileIndexes(const GeoDataLatLonBox &latLonBox, int zoomLevel) const
{
    const unsigned int xTileCount = (1 << zoomLevel) * levelZeroColumns();

    const int westX =   eastBoundTileXFromLon(latLonBox.west(),  xTileCount);
    const int eastX =   westBoundTileXFromLon(latLonBox.east(),  xTileCount);

    const unsigned int yTileCount = (1 << zoomLevel) * levelZeroRows();

    const int northY = southBoundTileYFromLat(latLonBox.north(), yTileCount);
    const int southY = northBoundTileYFromLat(latLonBox.south(), yTileCount);

    return QRect(QPoint(westX, northY), QPoint(eastX, southY));
}


GeoDataLatLonBox GeoSceneMercatorTileProjection::geoCoordinates(int zoomLevel, int x, int y) const
{
    const unsigned int xTileCount = (1 << zoomLevel) * levelZeroColumns();
    const unsigned int yTileCount = (1 << zoomLevel) * levelZeroRows();

    const GeoDataLongitude west = lonFromTileX(x, xTileCount);
    const GeoDataLatitude north = latFromTileY(y, yTileCount);

    const GeoDataLongitude east = lonFromTileX(x + 1, xTileCount);
    const GeoDataLatitude south = latFromTileY(y + 1, yTileCount);

    return GeoDataLatLonBox(north, south, east, west);
}

}
