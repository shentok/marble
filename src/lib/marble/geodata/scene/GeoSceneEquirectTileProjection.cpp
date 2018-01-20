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

#include "GeoSceneEquirectTileProjection.h"

#include <GeoDataLatLonBox.h>

namespace Marble
{

GeoSceneEquirectTileProjection::GeoSceneEquirectTileProjection()
{
}


GeoSceneEquirectTileProjection::~GeoSceneEquirectTileProjection()
{
}

GeoSceneAbstractTileProjection::Type GeoSceneEquirectTileProjection::type() const
{
    return Equirectangular;
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
qreal baseTileYFromLat(GeoDataLatitude lat, unsigned int tileCount)
{
    return (0.5 - 0.5 * lat / GeoDataLatitude::quaterCircle) * tileCount;
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
        return 0;
    }
    return upperBoundTileIndex(baseTileYFromLat(lat, tileCount));
}

// on tile borders selects the tile to the north
static inline
unsigned int northBoundTileYFromLat(GeoDataLatitude lat, unsigned int tileCount)
{
    // special casing tile-map end
    if (lat == GeoDataLatitude::quaterCircle) {
        return tileCount-1;
    }
    return lowerBoundTileIndex(baseTileYFromLat(lat, tileCount));
}


QRect GeoSceneEquirectTileProjection::tileIndexes(const GeoDataLatLonBox &latLonBox, int zoomLevel) const
{
    const unsigned int xTileCount = (1 << zoomLevel) * levelZeroColumns();

    const int westX =   eastBoundTileXFromLon(latLonBox.west(),  xTileCount);
    const int eastX =   westBoundTileXFromLon(latLonBox.east(),  xTileCount);

    const unsigned int yTileCount = (1 << zoomLevel) * levelZeroRows();

    const int northY = southBoundTileYFromLat(latLonBox.north(), yTileCount);
    const int southY = northBoundTileYFromLat(latLonBox.south(), yTileCount);

    return QRect(QPoint(westX, northY), QPoint(eastX, southY));
}

GeoDataLatLonBox GeoSceneEquirectTileProjection::geoCoordinates(int zoomLevel, int x, int y) const
{
    const qreal radiusX = (1 << zoomLevel) * levelZeroColumns() / 2.0;
    const qreal radiusY = (1 << zoomLevel) * levelZeroRows() / 2.0;

    const auto west  = GeoDataNormalizedLongitude::fromLongitude((x - radiusX) / radiusX * GeoDataLongitude::halfCircle);
    const auto north = GeoDataNormalizedLatitude::fromLatitude((radiusY - y) / radiusY * GeoDataLatitude::quaterCircle);

    const auto east  = GeoDataNormalizedLongitude::fromLongitude(((x + 1) - radiusX) / radiusX * GeoDataLongitude::halfCircle);
    const auto south = GeoDataNormalizedLatitude::fromLatitude((radiusY - (y + 1)) / radiusY * GeoDataLatitude::quaterCircle);

    return GeoDataLatLonBox(north, south, east, west);
}

}
