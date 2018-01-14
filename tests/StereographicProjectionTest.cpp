//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2014       Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include "StereographicProjection.h"
#include "ViewportParams.h"
#include "TestUtils.h"
#include <QDebug>

namespace Marble
{

class StereographicProjectionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void screenCoordinatesOfCenter_data();
    void screenCoordinatesOfCenter();
};

void StereographicProjectionTest::screenCoordinatesOfCenter_data()
{
    ViewportParams stereographic;
    stereographic.setProjection( Stereographic );

    QTest::addColumn<QPoint>( "screenCoordinates" );
    QTest::addColumn<GeoDataCoordinates>( "expected" );

    addRow() << QPoint( 5, 15 ) << GeoDataCoordinates(-GeoDataLongitude::fromDegrees(45), GeoDataLatitude::fromDegrees(72.1397));
    addRow() << QPoint( 15, 5 ) << GeoDataCoordinates(GeoDataLongitude::fromDegrees(135), GeoDataLatitude::fromDegrees(72.1397));
}

void StereographicProjectionTest::screenCoordinatesOfCenter()
{
    QFETCH( QPoint, screenCoordinates );
    QFETCH( GeoDataCoordinates, expected );

    ViewportParams viewport;
    viewport.setProjection( Stereographic );
    viewport.setRadius( 180 / 4 ); // for easy mapping of lon <-> x
    viewport.setSize( QSize( 20, 20 ) );
    viewport.centerOn(GeoDataLongitude::null, GeoDataLatitude::quaterCircle);

    {
        GeoDataLongitude lon;
        GeoDataLatitude lat;
        const bool retval = viewport.geoCoordinates(screenCoordinates.x(), screenCoordinates.y(), lon, lat);

        QVERIFY( retval ); // we want valid coordinates
        QFUZZYCOMPARE(lon, expected.longitude(), GeoDataLongitude::fromDegrees(0.0001));
        QFUZZYCOMPARE(lat, expected.latitude(), GeoDataLatitude::fromDegrees(0.0001));
    }
}

}

QTEST_MAIN( Marble::StereographicProjectionTest )

#include "StereographicProjectionTest.moc"
