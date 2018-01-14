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

#include "TestUtils.h"

#include <GeoSceneEquirectTileProjection.h>
#include <GeoSceneMercatorTileProjection.h>
#include <GeoDataLatLonBox.h>
#include <TileId.h>


namespace Marble
{

class TileProjectionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testTypeEquirect();
    void testTypeMercator();

    void testLevelZeroColumnsRowsEquirect();
    void testLevelZeroColumnsRowsMercator();

    void testTileIndexesEquirect_data();
    void testTileIndexesEquirect();
    void testTileIndexesMercator_data();
    void testTileIndexesMercator();

    void testGeoCoordinatesEquirect_data();
    void testGeoCoordinatesEquirect();
    void testGeoCoordinatesMercator_data();
    void testGeoCoordinatesMercator();

private:
    void testLevelZeroColumnsRows(GeoSceneAbstractTileProjection& projection);
};


void TileProjectionTest::testLevelZeroColumnsRows(GeoSceneAbstractTileProjection& projection)
{
    // test default
    QCOMPARE(projection.levelZeroColumns(), 1);
    QCOMPARE(projection.levelZeroRows(), 1);

    // test setting a different value
    const int levelZeroColumns = 4;
    const int levelZeroRows = 6;

    projection.setLevelZeroColumns(levelZeroColumns);
    projection.setLevelZeroRows(levelZeroRows);

    QCOMPARE(projection.levelZeroColumns(), levelZeroColumns);
    QCOMPARE(projection.levelZeroRows(), levelZeroRows);
}

void TileProjectionTest::testLevelZeroColumnsRowsEquirect()
{
    GeoSceneEquirectTileProjection projection;
    testLevelZeroColumnsRows(projection);
}

void TileProjectionTest::testLevelZeroColumnsRowsMercator()
{
    GeoSceneMercatorTileProjection projection;
    testLevelZeroColumnsRows(projection);
}

void TileProjectionTest::testTypeEquirect()
{
    GeoSceneEquirectTileProjection projection;
    QCOMPARE(projection.type(), GeoSceneAbstractTileProjection::Equirectangular);
}

void TileProjectionTest::testTypeMercator()
{
    GeoSceneMercatorTileProjection projection;
    QCOMPARE(projection.type(), GeoSceneAbstractTileProjection::Mercator);
}


void TileProjectionTest::testTileIndexesEquirect_data()
{
    QTest::addColumn<GeoDataLongitude>("westLon");
    QTest::addColumn<GeoDataLatitude>("northLat");
    QTest::addColumn<GeoDataLongitude>("eastLon");
    QTest::addColumn<GeoDataLatitude>("southLat");
    QTest::addColumn<int>("zoomLevel");
    QTest::addColumn<int>("expectedTileXWest");
    QTest::addColumn<int>("expectedTileYNorth");
    QTest::addColumn<int>("expectedTileXEast");
    QTest::addColumn<int>("expectedTileYSouth");

    // zoomlevel zero: 1 tile
    // bounds matching the tile map
    addRow() << GeoDataLongitude::fromRadians(-M_PI) << GeoDataLatitude::fromRadians(+M_PI * 0.5)
             << GeoDataLongitude::fromRadians(+M_PI) << GeoDataLatitude::fromRadians(-M_PI * 0.5)
             << 0
             << 0 << 0 << 0 << 0;
    // bounds inside the 1 tile
    addRow() << GeoDataLongitude::fromRadians(-M_PI*0.5) << GeoDataLatitude::fromRadians(+M_PI * 0.25)
             << GeoDataLongitude::fromRadians(+M_PI*0.5) << GeoDataLatitude::fromRadians(-M_PI * 0.25)
             << 0
             << 0 << 0 << 0 << 0;
    // bounds west and north on tile map borders, with normal border values
    addRow() << GeoDataLongitude::fromRadians(-M_PI)     << GeoDataLatitude::fromRadians(+M_PI * 0.5)
             << GeoDataLongitude::fromRadians(+M_PI*0.5) << GeoDataLatitude::fromRadians(-M_PI * 0.25)
             << 0
             << 0 << 0 << 0 << 0;
    // bounds west and north on tile map borders, with border values from other map border sides
    addRow() << GeoDataLongitude::fromRadians(+M_PI)     << GeoDataLatitude::fromRadians(-M_PI * 0.5)
             << GeoDataLongitude::fromRadians(+M_PI*0.5) << GeoDataLatitude::fromRadians(-M_PI * 0.25)
             << 0
             << 0 << 0 << 0 << 0;

    // zoomlevel 1: 2 tiles per dimension
    // bounds matching the tile map
    addRow() << GeoDataLongitude::fromRadians(-M_PI) << GeoDataLatitude::fromRadians(+M_PI * 0.5)
             << GeoDataLongitude::fromRadians(+M_PI) << GeoDataLatitude::fromRadians(-M_PI * 0.5)
             << 1
             << 0 << 0 << 1 << 1;
    // bounds inside the 4 tiles
    addRow() << GeoDataLongitude::fromRadians(-M_PI*0.5) << GeoDataLatitude::fromRadians(+M_PI * 0.25)
             << GeoDataLongitude::fromRadians(+M_PI*0.5) << GeoDataLatitude::fromRadians(-M_PI * 0.25)
             << 1
             << 0 << 0 << 1 << 1;
    // bounds matching the most north-west tile, with normal border values
    addRow() << GeoDataLongitude::fromRadians(-M_PI)     << GeoDataLatitude::fromRadians(+M_PI * 0.5)
             << GeoDataLongitude::fromRadians(0)         << GeoDataLatitude::fromRadians(0)
             << 1
             << 0 << 0 << 0 << 0;
    // bounds matching the most north-west tile, with border values from other map border sides
    addRow() << GeoDataLongitude::fromRadians(+M_PI)     << GeoDataLatitude::fromRadians(-M_PI * 0.5)
             << GeoDataLongitude::fromRadians(0)         << GeoDataLatitude::fromRadians(0)
             << 1
             << 0 << 0 << 0 << 0;
    // bounds matching the most south-east tile, with normal border values
    addRow() << GeoDataLongitude::fromRadians(0) << GeoDataLatitude::fromRadians(0)
             << GeoDataLongitude::fromRadians(+M_PI) << GeoDataLatitude::fromRadians(-M_PI * 0.5)
             << 1
             << 1 << 1 << 1 << 1;
    // bounds matching the most south-east tile, with border values from other map border sides
    addRow() << GeoDataLongitude::fromRadians(0) << GeoDataLatitude::fromRadians(0)
             << GeoDataLongitude::fromRadians(-M_PI) << GeoDataLatitude::fromRadians(+M_PI * 0.5)
             << 1
             << 1 << 1 << 1 << 1;

    // zoomlevel 9: 2^8==512 tiles per dimension
    // bounds matching the tile map
    addRow() << GeoDataLongitude::fromRadians(-M_PI) << GeoDataLatitude::fromRadians(+M_PI * 0.5)
             << GeoDataLongitude::fromRadians(+M_PI) << GeoDataLatitude::fromRadians(-M_PI * 0.5)
             << 9
             << 0 << 0 << 511 << 511;
    // bounds inside the outer tiles
    addRow() << GeoDataLongitude::fromRadians(-M_PI*(511/512.0)) << GeoDataLatitude::fromRadians(+M_PI * 0.5 * (511/512.0))
             << GeoDataLongitude::fromRadians(+M_PI*(511/512.0)) << GeoDataLatitude::fromRadians(-M_PI * 0.5 * (511/512.0))
             << 9
             << 0 << 0 << 511 << 511;
    // bounds matching the most north-west tile, with normal border values
    addRow() << GeoDataLongitude::fromRadians(-M_PI)             << GeoDataLatitude::fromRadians(+M_PI * 0.5)
             << GeoDataLongitude::fromRadians(-M_PI*(255/256.0)) << GeoDataLatitude::fromRadians(+M_PI * 0.5 *(255/256.0))
             << 9
             << 0 << 0 << 0 << 0;
    // bounds matching the most north-west tile, with border values from other map border sides
    addRow() << GeoDataLongitude::fromRadians(+M_PI)             << GeoDataLatitude::fromRadians(-M_PI * 0.5)
             << GeoDataLongitude::fromRadians(-M_PI*(255/256.0)) << GeoDataLatitude::fromRadians(+M_PI * 0.5 *(255/256.0))
             << 9
             << 0 << 0 << 0 << 0;
    // bounds matching the most south-east tile, with normal border values
    addRow() << GeoDataLongitude::fromRadians(+M_PI*(255/256.0)) << GeoDataLatitude::fromRadians(-M_PI * 0.5 *(255/256.0))
             << GeoDataLongitude::fromRadians(+M_PI)             << GeoDataLatitude::fromRadians(-M_PI * 0.5)
             << 9
             << 511 << 511 << 511 << 511;
    // bounds matching the most south-east tile, with border values from other map border sides
    addRow() << GeoDataLongitude::fromRadians(+M_PI*(255/256.0)) << GeoDataLatitude::fromRadians(-M_PI * 0.5 *(255/256.0))
             << GeoDataLongitude::fromRadians(-M_PI)             << GeoDataLatitude::fromRadians(+M_PI * 0.5)
             << 9
             << 511 << 511 << 511 << 511;
}


void TileProjectionTest::testTileIndexesEquirect()
{
    QFETCH(GeoDataLongitude, westLon);
    QFETCH(GeoDataLatitude, northLat);
    QFETCH(GeoDataLongitude, eastLon);
    QFETCH(GeoDataLatitude, southLat);
    QFETCH(int, zoomLevel);
    QFETCH(int, expectedTileXWest);
    QFETCH(int, expectedTileYNorth);
    QFETCH(int, expectedTileXEast);
    QFETCH(int, expectedTileYSouth);

    const GeoDataLatLonBox latLonBox(northLat, southLat, eastLon, westLon);

    const GeoSceneEquirectTileProjection projection;

    const QRect rect = projection.tileIndexes(latLonBox, zoomLevel);

    QCOMPARE(rect.left(), expectedTileXWest);
    QCOMPARE(rect.top(), expectedTileYNorth);
    QCOMPARE(rect.right(), expectedTileXEast);
    QCOMPARE(rect.bottom(), expectedTileYSouth);
}


void TileProjectionTest::testTileIndexesMercator_data()
{
    QTest::addColumn<GeoDataLongitude>("westLon");
    QTest::addColumn<GeoDataLatitude>("northLat");
    QTest::addColumn<GeoDataLongitude>("eastLon");
    QTest::addColumn<GeoDataLatitude>("southLat");
    QTest::addColumn<int>("zoomLevel");
    QTest::addColumn<int>("expectedTileXWest");
    QTest::addColumn<int>("expectedTileYNorth");
    QTest::addColumn<int>("expectedTileXEast");
    QTest::addColumn<int>("expectedTileYSouth");

    // zoomlevel zero: 1 tile
    // bounds matching the tile map up to 90 degree latitude
    addRow() << GeoDataLongitude::fromRadians(-M_PI) << GeoDataLatitude::fromRadians(+M_PI * 0.5)
             << GeoDataLongitude::fromRadians(+M_PI) << GeoDataLatitude::fromRadians(-M_PI * 0.5)
             << 0
             << 0 << 0 << 0 << 0;
    // bounds matching the tile map with 85 degree latitude limit
    addRow() << GeoDataLongitude::fromRadians(-M_PI) << GeoDataLatitude::fromRadians(85.0 * DEG2RAD)
             << GeoDataLongitude::fromRadians(+M_PI) << GeoDataLatitude::fromRadians(-85.0 * DEG2RAD)
             << 0
             << 0 << 0 << 0 << 0;
    // bounds inside the 1 tile
    addRow() << GeoDataLongitude::fromRadians(-M_PI*0.5) << GeoDataLatitude::fromRadians(+M_PI * 0.25)
             << GeoDataLongitude::fromRadians(+M_PI*0.5) << GeoDataLatitude::fromRadians(-M_PI * 0.25)
             << 0
             << 0 << 0 << 0 << 0;
    // bounds west and north on tile map borders, with normal border values
    addRow() << GeoDataLongitude::fromRadians(-M_PI)     << GeoDataLatitude::fromRadians(+M_PI * 0.5)
             << GeoDataLongitude::fromRadians(+M_PI*0.5) << GeoDataLatitude::fromRadians(-M_PI * 0.25)
             << 0
             << 0 << 0 << 0 << 0;
    // bounds west and north on tile map borders, with border values from other map border sides
    addRow() << GeoDataLongitude::fromRadians(+M_PI)     << GeoDataLatitude::fromRadians(-M_PI * 0.5)
             << GeoDataLongitude::fromRadians(+M_PI*0.5) << GeoDataLatitude::fromRadians(-M_PI * 0.25)
             << 0
             << 0 << 0 << 0 << 0;

    // zoomlevel 1: 2 tiles per dimension
    // bounds matching the tile map up to 90 degree latitude
    addRow() << GeoDataLongitude::fromRadians(-M_PI) << GeoDataLatitude::fromRadians(+M_PI * 0.5)
             << GeoDataLongitude::fromRadians(+M_PI) << GeoDataLatitude::fromRadians(-M_PI * 0.5)
             << 1
             << 0 << 0 << 1 << 1;
    // bounds matching the tile map with 85 degree latitude limit
    addRow() << GeoDataLongitude::fromRadians(-M_PI) << GeoDataLatitude::fromRadians(85.0 * DEG2RAD)
             << GeoDataLongitude::fromRadians(+M_PI) << GeoDataLatitude::fromRadians(-85.0 * DEG2RAD)
             << 1
             << 0 << 0 << 1 << 1;
    // bounds inside the 4 tiles
    addRow() << GeoDataLongitude::fromRadians(-M_PI*0.5) << GeoDataLatitude::fromRadians(+M_PI * 0.25)
             << GeoDataLongitude::fromRadians(+M_PI*0.5) << GeoDataLatitude::fromRadians(-M_PI * 0.25)
             << 1
             << 0 << 0 << 1 << 1;
    // bounds matching the most north-west tile, with normal border values
    addRow() << GeoDataLongitude::fromRadians(-M_PI)     << GeoDataLatitude::fromRadians(+M_PI * 0.5)
             << GeoDataLongitude::fromRadians(0)         << GeoDataLatitude::fromRadians(0)
             << 1
             << 0 << 0 << 0 << 0;
    // bounds matching the most north-west tile, with border values from other map border sides
    addRow() << GeoDataLongitude::fromRadians(+M_PI)     << GeoDataLatitude::fromRadians(-M_PI * 0.5)
             << GeoDataLongitude::fromRadians(0)         << GeoDataLatitude::fromRadians(0)
             << 1
             << 0 << 0 << 0 << 0;
    // bounds matching the most south-east tile, with normal border values
    addRow() << GeoDataLongitude::fromRadians(0) << GeoDataLatitude::fromRadians(0)
             << GeoDataLongitude::fromRadians(+M_PI) << GeoDataLatitude::fromRadians(-M_PI * 0.5)
             << 1
             << 1 << 1 << 1 << 1;
    // bounds matching the most south-east tile, with border values from other map border sides
    addRow() << GeoDataLongitude::fromRadians(0) << GeoDataLatitude::fromRadians(0)
             << GeoDataLongitude::fromRadians(-M_PI) << GeoDataLatitude::fromRadians(+M_PI * 0.5)
             << 1
             << 1 << 1 << 1 << 1;

    // zoomlevel 9: 2^8==512 tiles per dimension
    // GeoSceneMercatorTileProjection bounds latitude value at +/- 85.0 degree (so not at 85.05113),
    // which results in some tiles missed at the outer sides.
    // bounds matching the tile map up to 90 degree latitude
    addRow() << GeoDataLongitude::fromRadians(-M_PI) << GeoDataLatitude::fromRadians(+M_PI * 0.5)
             << GeoDataLongitude::fromRadians(+M_PI) << GeoDataLatitude::fromRadians(-M_PI * 0.5)
             << 9
             << 0 << 5 << 511 << 506;
    // bounds matching the tile map with 85 degree latitude limit
    addRow() << GeoDataLongitude::fromRadians(-M_PI) << GeoDataLatitude::fromRadians(85.0 * DEG2RAD)
             << GeoDataLongitude::fromRadians(+M_PI) << GeoDataLatitude::fromRadians(-85.0 * DEG2RAD)
             << 9
             << 0 << 5 << 511 << 506;
    // bounds inside the outer tiles
    addRow() << GeoDataLongitude::fromRadians(-M_PI*(511/512.0)) << GeoDataLatitude::fromRadians(+M_PI * 0.5 * (511/512.0))
             << GeoDataLongitude::fromRadians(+M_PI*(511/512.0)) << GeoDataLatitude::fromRadians(-M_PI * 0.5 * (511/512.0))
             << 9
             << 0 << 5 << 511 << 506;
    // bounds matching the most north-west tile, with normal border values
    addRow() << GeoDataLongitude::fromRadians(-M_PI)             << GeoDataLatitude::fromRadians(+M_PI * 0.5)
             << GeoDataLongitude::fromRadians(-M_PI*(255/256.0)) << GeoDataLatitude::fromRadians(+M_PI * 0.5 *(255/256.0))
             << 9
             << 0 << 5 << 0 << 5;
    // bounds matching the most north-west tile, with border values from other map border sides
    addRow() << GeoDataLongitude::fromRadians(+M_PI)             << GeoDataLatitude::fromRadians(-M_PI * 0.5)
             << GeoDataLongitude::fromRadians(-M_PI*(255/256.0)) << GeoDataLatitude::fromRadians(+M_PI * 0.5 *(255/256.0))
             << 9
             << 0 << 5 << 0 << 5;
    // bounds matching the most south-east tile, with normal border values
    addRow() << GeoDataLongitude::fromRadians(+M_PI*(255/256.0)) << GeoDataLatitude::fromRadians(-M_PI * 0.5 *(255/256.0))
             << GeoDataLongitude::fromRadians(+M_PI)             << GeoDataLatitude::fromRadians(-M_PI * 0.5)
             << 9
             << 511 << 506 << 511 << 506;
    // bounds matching the most south-east tile, with border values from other map border sides
    addRow() << GeoDataLongitude::fromRadians(+M_PI*(255/256.0)) << GeoDataLatitude::fromRadians(-M_PI * 0.5 *(255/256.0))
             << GeoDataLongitude::fromRadians(-M_PI)             << GeoDataLatitude::fromRadians(+M_PI * 0.5)
             << 9
             << 511 << 506 << 511 << 506;
}


void TileProjectionTest::testTileIndexesMercator()
{
    QFETCH(GeoDataLongitude, westLon);
    QFETCH(GeoDataLatitude, northLat);
    QFETCH(GeoDataLongitude, eastLon);
    QFETCH(GeoDataLatitude, southLat);
    QFETCH(int, zoomLevel);
    QFETCH(int, expectedTileXWest);
    QFETCH(int, expectedTileYNorth);
    QFETCH(int, expectedTileXEast);
    QFETCH(int, expectedTileYSouth);

    GeoDataLatLonBox latLonBox(northLat, southLat, eastLon, westLon);

    const GeoSceneMercatorTileProjection projection;

    const QRect rect = projection.tileIndexes(latLonBox, zoomLevel);

    QCOMPARE(rect.left(), expectedTileXWest);
    QCOMPARE(rect.top(), expectedTileYNorth);
    QCOMPARE(rect.right(), expectedTileXEast);
    QCOMPARE(rect.bottom(), expectedTileYSouth);
}


void TileProjectionTest::testGeoCoordinatesEquirect_data()
{
    QTest::addColumn<int>("tileX");
    QTest::addColumn<int>("tileY");
    QTest::addColumn<int>("zoomLevel");
    QTest::addColumn<GeoDataLongitude>("expectedWesternTileEdgeLon");
    QTest::addColumn<GeoDataLatitude>("expectedNorthernTileEdgeLat");
    QTest::addColumn<GeoDataLongitude>("expectedEasternTileEdgeLon");
    QTest::addColumn<GeoDataLatitude>("expectedSouthernTileEdgeLat");

    // zoomlevel zero: 1 tile
    addRow() << 0 << 0 << 0 << GeoDataLongitude::fromRadians(-M_PI) << GeoDataLatitude::fromRadians(+M_PI * 0.5)
                            << GeoDataLongitude::fromRadians(+M_PI) << GeoDataLatitude::fromRadians(-M_PI * 0.5);

    // zoomlevel 1: 2 tiles per dimension
    addRow() << 0 << 0 << 1 << GeoDataLongitude::fromRadians(-M_PI) << GeoDataLatitude::fromRadians(+M_PI * 0.5)
                            << GeoDataLongitude::fromRadians(0)     << GeoDataLatitude::fromRadians(0);
    addRow() << 0 << 1 << 1 << GeoDataLongitude::fromRadians(-M_PI) << GeoDataLatitude::fromRadians(0)
                            << GeoDataLongitude::fromRadians(0)     << GeoDataLatitude::fromRadians(-M_PI * 0.5);
    addRow() << 1 << 0 << 1 << GeoDataLongitude::fromRadians(0)     << GeoDataLatitude::fromRadians(+M_PI * 0.5)
                            << GeoDataLongitude::fromRadians(+M_PI) << GeoDataLatitude::fromRadians(0);
    addRow() << 1 << 1 << 1 << GeoDataLongitude::fromRadians(0)     << GeoDataLatitude::fromRadians(0)
                            << GeoDataLongitude::fromRadians(+M_PI) << GeoDataLatitude::fromRadians(-M_PI * 0.5);

    // zoomlevel 9: 2^8==512 tiles per dimension
    addRow() <<   0 <<   0 << 9 << GeoDataLongitude::fromRadians(-M_PI)               << GeoDataLatitude::fromRadians(+M_PI * 0.5)
                                << GeoDataLongitude::fromRadians(-M_PI * (255/256.0)) << GeoDataLatitude::fromRadians(+M_PI * 0.5 * (255/256.0));
    addRow() <<   0 << 256 << 9 << GeoDataLongitude::fromRadians(-M_PI)               << GeoDataLatitude::fromRadians(0)
                                << GeoDataLongitude::fromRadians(-M_PI * (255/256.0)) << GeoDataLatitude::fromRadians(-M_PI * 0.5 * (1/256.0));
    addRow() << 256 <<   0 << 9 << GeoDataLongitude::fromRadians(0)                   << GeoDataLatitude::fromRadians(+M_PI * 0.5)
                                << GeoDataLongitude::fromRadians(M_PI * (1/256.0))    << GeoDataLatitude::fromRadians(+M_PI * 0.5 * (255/256.0));
    addRow() << 511 << 511 << 9 << GeoDataLongitude::fromRadians(M_PI * (255/256.0))  << GeoDataLatitude::fromRadians(-M_PI * 0.5 * (255/256.0))
                                << GeoDataLongitude::fromRadians(+M_PI)               << GeoDataLatitude::fromRadians(-M_PI * 0.5);
}


void TileProjectionTest::testGeoCoordinatesEquirect()
{
    QFETCH(int, tileX);
    QFETCH(int, tileY);
    QFETCH(int, zoomLevel);
    QFETCH(GeoDataLongitude, expectedWesternTileEdgeLon);
    QFETCH(GeoDataLatitude, expectedNorthernTileEdgeLat);
    QFETCH(GeoDataLongitude, expectedEasternTileEdgeLon);
    QFETCH(GeoDataLatitude, expectedSouthernTileEdgeLat);

    const GeoSceneEquirectTileProjection projection;

    // method variants with GeoDataLatLonBox
    const GeoDataLatLonBox latLonBox = projection.geoCoordinates(zoomLevel, tileX, tileY);

    QCOMPARE(latLonBox.west(), expectedWesternTileEdgeLon);
    QCOMPARE(latLonBox.north(), expectedNorthernTileEdgeLat);
    QCOMPARE(latLonBox.east(), expectedEasternTileEdgeLon);
    QCOMPARE(latLonBox.south(), expectedSouthernTileEdgeLat);

    TileId tileId(QStringLiteral("testmap"), zoomLevel, tileX, tileY);
    const GeoDataLatLonBox latLonBox2 = projection.geoCoordinates(tileId);

    QCOMPARE(latLonBox2.west(), expectedWesternTileEdgeLon);
    QCOMPARE(latLonBox2.north(), expectedNorthernTileEdgeLat);
    QCOMPARE(latLonBox2.east(), expectedEasternTileEdgeLon);
    QCOMPARE(latLonBox2.south(), expectedSouthernTileEdgeLat);
}

void TileProjectionTest::testGeoCoordinatesMercator_data()
{
    QTest::addColumn<int>("tileX");
    QTest::addColumn<int>("tileY");
    QTest::addColumn<int>("zoomLevel");
    QTest::addColumn<GeoDataLongitude>("expectedWesternTileEdgeLon");
    QTest::addColumn<GeoDataLatitude>("expectedNorthernTileEdgeLat");
    QTest::addColumn<GeoDataLongitude>("expectedEasternTileEdgeLon");
    QTest::addColumn<GeoDataLatitude>("expectedSouthernTileEdgeLat");

    const qreal absMaxLat = DEG2RAD * 85.05113;

    // zoomlevel zero: 1 tile
    addRow() << 0 << 0 << 0 << GeoDataLongitude::fromRadians(-M_PI) << GeoDataLatitude::fromRadians(+absMaxLat)
                            << GeoDataLongitude::fromRadians(+M_PI) << GeoDataLatitude::fromRadians(-absMaxLat);

    // zoomlevel 1: 2 tiles per dimension
    addRow() << 0 << 0 << 1 << GeoDataLongitude::fromRadians(-M_PI) << GeoDataLatitude::fromRadians(+absMaxLat)
                            << GeoDataLongitude::fromRadians(0)     << GeoDataLatitude::fromRadians(0);
    addRow() << 0 << 1 << 1 << GeoDataLongitude::fromRadians(-M_PI) << GeoDataLatitude::fromRadians(0)
                            << GeoDataLongitude::fromRadians(0)     << GeoDataLatitude::fromRadians(-absMaxLat);
    addRow() << 1 << 0 << 1 << GeoDataLongitude::fromRadians(0)     << GeoDataLatitude::fromRadians(+absMaxLat)
                            << GeoDataLongitude::fromRadians(+M_PI) << GeoDataLatitude::fromRadians(0);
    addRow() << 1 << 1 << 1 << GeoDataLongitude::fromRadians(0)     << GeoDataLatitude::fromRadians(0)
                            << GeoDataLongitude::fromRadians(+M_PI) << GeoDataLatitude::fromRadians(-absMaxLat);

    // zoomlevel 9: 2^8==512 tiles per dimension
    addRow() <<   0 <<   0 << 9 << GeoDataLongitude::fromRadians(-M_PI)               << GeoDataLatitude::fromRadians(+absMaxLat)
                                << GeoDataLongitude::fromRadians(-M_PI * (255/256.0)) << GeoDataLatitude::fromRadians(+1.48336);
    addRow() <<   0 << 256 << 9 << GeoDataLongitude::fromRadians(-M_PI)               << GeoDataLatitude::fromRadians(0)
                                << GeoDataLongitude::fromRadians(-M_PI * (255/256.0)) << GeoDataLatitude::fromRadians(-0.0122715);
    addRow() << 256 <<   0 << 9 << GeoDataLongitude::fromRadians(0)                   << GeoDataLatitude::fromRadians(+absMaxLat)
                                << GeoDataLongitude::fromRadians(M_PI * (1/256.0))    << GeoDataLatitude::fromRadians(+1.48336);
    addRow() << 511 << 511 << 9 << GeoDataLongitude::fromRadians(M_PI * (255/256.0))  << GeoDataLatitude::fromRadians(-1.48336)
                                << GeoDataLongitude::fromRadians(+M_PI)               << GeoDataLatitude::fromRadians(-absMaxLat);
}


void TileProjectionTest::testGeoCoordinatesMercator()
{
    QFETCH(int, tileX);
    QFETCH(int, tileY);
    QFETCH(int, zoomLevel);
    QFETCH(GeoDataLongitude, expectedWesternTileEdgeLon);
    QFETCH(GeoDataLatitude, expectedNorthernTileEdgeLat);
    QFETCH(GeoDataLongitude, expectedEasternTileEdgeLon);
    QFETCH(GeoDataLatitude, expectedSouthernTileEdgeLat);

    const GeoSceneMercatorTileProjection projection;

    // method variants with GeoDataLatLonBox
    const GeoDataLatLonBox latLonBox = projection.geoCoordinates(zoomLevel, tileX, tileY);

    QCOMPARE(latLonBox.west(), expectedWesternTileEdgeLon);
    QFUZZYCOMPARE(latLonBox.north().toRadian(), expectedNorthernTileEdgeLat.toRadian(), 0.00001);
    QCOMPARE(latLonBox.east(), expectedEasternTileEdgeLon);
    QFUZZYCOMPARE(latLonBox.south().toRadian(), expectedSouthernTileEdgeLat.toRadian(), 0.00001);

    TileId tileId(QStringLiteral("testmap"), zoomLevel, tileX, tileY);
    const GeoDataLatLonBox latLonBox2 = projection.geoCoordinates(tileId);

    QCOMPARE(latLonBox2.west(), expectedWesternTileEdgeLon);
    QFUZZYCOMPARE(latLonBox2.north().toRadian(), expectedNorthernTileEdgeLat.toRadian(), 0.00001);
    QCOMPARE(latLonBox2.east(), expectedEasternTileEdgeLon);
    QFUZZYCOMPARE(latLonBox2.south().toRadian(), expectedSouthernTileEdgeLat.toRadian(), 0.00001);
}

} // namespace Marble

QTEST_MAIN(Marble::TileProjectionTest)

#include "TestTileProjection.moc"
