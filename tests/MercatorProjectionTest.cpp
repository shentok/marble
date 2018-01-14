//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011       Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include "MercatorProjection.h"
#include "ViewportParams.h"
#include "TestUtils.h"

namespace Marble
{

class MercatorProjectionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void screenCoordinatesValidLat_data();
    void screenCoordinatesValidLat();

    void screenCoordinatesOfCenter_data();
    void screenCoordinatesOfCenter();

    void setInvalidRadius();
};

void MercatorProjectionTest::screenCoordinatesValidLat_data()
{
    ViewportParams mercator;
    mercator.setProjection( Mercator );

    QTest::addColumn<GeoDataLongitude>("lon");
    QTest::addColumn<GeoDataLatitude>("lat");
    QTest::addColumn<bool>( "validLat" );

    addRow() << GeoDataLongitude::fromDegrees(0.0) << GeoDataLatitude::fromDegrees(0.0) << true;

    addRow() << GeoDataLongitude::fromDegrees(-180.0) << GeoDataLatitude::fromDegrees(0.0) << true;
    addRow() <<  GeoDataLongitude::fromDegrees(180.0) << GeoDataLatitude::fromDegrees(0.0) << false;

    addRow() << GeoDataLongitude::fromDegrees(0.0) << mercator.currentProjection()->minValidLat() << true;
    addRow() << GeoDataLongitude::fromDegrees(0.0) << mercator.currentProjection()->maxValidLat() << true;

    addRow() << GeoDataLongitude::fromDegrees(0.0) << mercator.currentProjection()->minValidLat() - GeoDataLatitude::fromDegrees(0.0001) << false;
    addRow() << GeoDataLongitude::fromDegrees(0.0) << mercator.currentProjection()->maxValidLat() + GeoDataLatitude::fromDegrees(0.0001) << false;

    addRow() << GeoDataLongitude::fromDegrees(-180.0) << mercator.currentProjection()->minValidLat() << true;
    addRow() <<  GeoDataLongitude::fromDegrees(180.0) << mercator.currentProjection()->minValidLat() << false;

    addRow() << GeoDataLongitude::fromDegrees(-180.0) << mercator.currentProjection()->maxValidLat() << true;
    addRow() <<  GeoDataLongitude::fromDegrees(180.0) << mercator.currentProjection()->maxValidLat() << false;

    addRow() << GeoDataLongitude::fromDegrees(-180.0) << mercator.currentProjection()->minValidLat() - GeoDataLatitude::fromDegrees(0.0001) << false;
    addRow() <<  GeoDataLongitude::fromDegrees(180.0) << mercator.currentProjection()->minValidLat() - GeoDataLatitude::fromDegrees(0.0001) << false;

    addRow() << GeoDataLongitude::fromDegrees(-180.0) << mercator.currentProjection()->maxValidLat() + GeoDataLatitude::fromDegrees(0.0001) << false;
    addRow() <<  GeoDataLongitude::fromDegrees(180.0) << mercator.currentProjection()->maxValidLat() + GeoDataLatitude::fromDegrees(0.0001) << false;
}

void MercatorProjectionTest::screenCoordinatesValidLat()
{
    QFETCH(GeoDataLongitude, lon);
    QFETCH(GeoDataLatitude, lat);
    QFETCH( bool, validLat );

    const GeoDataCoordinates coordinates(lon, lat);

    ViewportParams viewport;
    viewport.setProjection( Mercator );
    viewport.setRadius( 360 / 4 ); // for easy mapping of lon <-> x
    viewport.centerOn(GeoDataLongitude::null, GeoDataLatitude::null);
    viewport.setSize( QSize( 360, 361 ) ); // TODO: check why height == 360 doesn't hold

    {
        qreal x;
        qreal y;

        const bool retval = viewport.screenCoordinates(lon, lat, x, y);

        QVERIFY( retval == validLat );
    }

    {
        qreal x;
        qreal y;
        bool globeHidesPoint = true;

        const bool retval = viewport.screenCoordinates( coordinates, x, y, globeHidesPoint );

        QVERIFY( retval == validLat );
        QVERIFY( !globeHidesPoint );
    }

    QVERIFY( viewport.currentProjection()->repeatableX() );

    {
        qreal x[2];
        qreal y;
        int pointRepeatNum = 1000;
        bool globeHidesPoint = true;

        const bool retval = viewport.screenCoordinates( coordinates, x, y, pointRepeatNum, QSizeF( 0, 0 ), globeHidesPoint );

        QVERIFY( retval == validLat );
        QCOMPARE( pointRepeatNum, 1 );
        QVERIFY( !globeHidesPoint );
    }
}

void MercatorProjectionTest::screenCoordinatesOfCenter_data()
{
    ViewportParams mercator;
    mercator.setProjection( Mercator );

    QTest::addColumn<GeoDataLongitude>("lon");
    QTest::addColumn<GeoDataLatitude>("lat");

    addRow() << GeoDataLongitude::fromDegrees(0.0) << GeoDataLatitude::fromDegrees(0.0);

    addRow() << GeoDataLongitude::fromDegrees(-180.0) << GeoDataLatitude::fromDegrees(0.0);
    addRow() <<  GeoDataLongitude::fromDegrees(180.0) << GeoDataLatitude::fromDegrees(0.0);

    addRow() << GeoDataLongitude::fromDegrees(0.0) << mercator.currentProjection()->minValidLat();
    addRow() << GeoDataLongitude::fromDegrees(0.0) << mercator.currentProjection()->maxValidLat();

    addRow() << GeoDataLongitude::fromDegrees(-180.0) << mercator.currentProjection()->minValidLat();
    addRow() << GeoDataLongitude::fromDegrees(-180.0) << mercator.currentProjection()->maxValidLat();

    addRow() <<  GeoDataLongitude::fromDegrees(180.0) << mercator.currentProjection()->minValidLat();
    addRow() <<  GeoDataLongitude::fromDegrees(180.0) << mercator.currentProjection()->maxValidLat();
}

void MercatorProjectionTest::screenCoordinatesOfCenter()
{
    QFETCH(GeoDataLongitude, lon);
    QFETCH(GeoDataLatitude, lat);

    const GeoDataCoordinates coordinates(lon, lat);

    ViewportParams viewport;
    viewport.setProjection( Mercator );
    viewport.setRadius( 360 / 4 ); // for easy mapping of lon <-> x
    viewport.setSize( QSize( 2, 2 ) );
    viewport.centerOn( lon, lat );

    {
        qreal x;
        qreal y;

        const bool retval = viewport.screenCoordinates(lon, lat, x, y);

        QVERIFY( retval ); // FIXME: this should fail for lon < -180 || 180 < lon
        QCOMPARE( x, (lon - viewport.centerLongitude()).toDegree() + 1.0 );
        QCOMPARE( y, 1.0 );
    }

    {
        qreal x;
        qreal y;
        bool globeHidesPoint = true;

        const bool retval = viewport.screenCoordinates( coordinates, x, y, globeHidesPoint );

        QVERIFY( retval ); // FIXME: this should fail for lon < -180 || 180 < lon
        QVERIFY( !globeHidesPoint );
        QCOMPARE( x, (lon - viewport.centerLongitude()).toDegree() + 1.0 );
        QCOMPARE( y, 1.0 );
    }

    QVERIFY( viewport.currentProjection()->repeatableX() );

    {
        qreal x[2];
        qreal y;
        int pointRepeatNum = 1000;
        bool globeHidesPoint = true;

        const bool retval = viewport.screenCoordinates( coordinates, x, y, pointRepeatNum, QSizeF( 0, 0 ), globeHidesPoint );

        QVERIFY( retval );
        QCOMPARE( pointRepeatNum, 1 );
        QVERIFY( !globeHidesPoint );
        QCOMPARE( x[0], 1.0 );
        QCOMPARE( y, 1.0 );
    }
}

void MercatorProjectionTest::setInvalidRadius()
{
    ViewportParams viewport;
    viewport.setProjection( Mercator );
    viewport.setRadius( 0 );
    GeoDataLongitude lon = GeoDataLongitude::null;
    GeoDataLatitude lat = GeoDataLatitude::null;
    viewport.geoCoordinates(23, 42, lon, lat);
}

}

QTEST_MAIN( Marble::MercatorProjectionTest )

#include "MercatorProjectionTest.moc"
