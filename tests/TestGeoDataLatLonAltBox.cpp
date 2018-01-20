//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.

// Copyright 2009       Bastian Holst <bastianholst@gmx.de>
// Copyright 2010       Cezar Mocan <mocancezar@gmail.com>
// Copyright 2012       Bernhard Beschow <bbeschow@cs.tu-berlin.de>

#include "GeoDataCoordinates.h"
#include "GeoDataFolder.h"
#include "GeoDataLineString.h"
#include "GeoDataLinearRing.h"
#include "GeoDataLatLonAltBox.h"
#include "GeoDataLatLonBox.h"
#include "GeoDataPlacemark.h"

#include <QTest>
#include <QBuffer>

using namespace Marble;

class TestGeoDataLatLonAltBox : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testDefaultConstruction();
    void testCopyConstruction_data();
    void testCopyConstruction();
    void testConstructionFromLatLonBox_data();
    void testConstructionFromLatLonBox();
    void testAssignment_data();
    void testAssignment();
    void testContstructionFromCoordinates_data();
    void testContstructionFromCoordinates();
    void testAltitude_data();
    void testAltitude();
    void testSetNorthRadian_data();
    void testSetNorthRadian();
    void testSetSouthRadian_data();
    void testSetSouthRadian();
    void testContains();
    void testIntersects_data();
    void testIntersects();
    void testCrossesDateline_data();
    void testCrossesDateline();
    void testCenter_data();
    void testCenter();
    void testUnited_data();
    void testUnited();

    void testFromLineString_data();
    void testFromLineString();
    void testToString_data();
    void testToString();
    void testPack_data();
    void testPack();
    void testContainerBox_data();
    void testContainerBox();
    void testScale_data();
    void testScale();
};

void TestGeoDataLatLonAltBox::testDefaultConstruction()
{
    GeoDataLatLonBox const latLonBox;

    QCOMPARE( latLonBox.north(), GeoDataNormalizedLatitude::null );
    QCOMPARE( latLonBox.south(), GeoDataNormalizedLatitude::null );
    QCOMPARE( latLonBox.east(), GeoDataNormalizedLongitude::null );
    QCOMPARE( latLonBox.west(), GeoDataNormalizedLongitude::null );
    QCOMPARE( latLonBox.rotation(), GeoDataAngle::null );
    QCOMPARE( latLonBox.width(), GeoDataLongitude::null );
    QCOMPARE( latLonBox.height(), GeoDataLatitude::null );
    QVERIFY( !latLonBox.crossesDateLine() );
    QCOMPARE(latLonBox.center(), GeoDataCoordinates(GeoDataLongitude::null, GeoDataLatitude::null));
    QVERIFY( latLonBox.isNull() );
    QVERIFY( latLonBox.isEmpty() );

    QVERIFY( (latLonBox|latLonBox).isNull() );
    QVERIFY( (latLonBox|latLonBox).isEmpty() );
    QVERIFY( !latLonBox.intersects( latLonBox ) );


    GeoDataLatLonAltBox const latLonAltBox;

    QCOMPARE( latLonAltBox.north(), GeoDataNormalizedLatitude::null );
    QCOMPARE( latLonAltBox.south(), GeoDataNormalizedLatitude::null );
    QCOMPARE( latLonAltBox.east(), GeoDataNormalizedLongitude::null );
    QCOMPARE( latLonAltBox.west(), GeoDataNormalizedLongitude::null );
    QCOMPARE( latLonAltBox.rotation(), GeoDataAngle::null );
    QCOMPARE( latLonAltBox.width(), GeoDataLongitude::null );
    QCOMPARE( latLonAltBox.height(), GeoDataLatitude::null );
    QVERIFY( !latLonAltBox.crossesDateLine() );
    QCOMPARE(latLonAltBox.center(), GeoDataCoordinates(GeoDataLongitude::null, GeoDataLatitude::null, 0));
    QVERIFY( latLonAltBox.isNull() );
    QVERIFY( latLonAltBox.isEmpty() );
    QCOMPARE( latLonAltBox.minAltitude(), 0.0 );
    QCOMPARE( latLonAltBox.maxAltitude(), 0.0 );
    QCOMPARE( latLonAltBox.altitudeMode(), ClampToGround );

    QVERIFY( (latLonAltBox|latLonAltBox).isNull() );
    QVERIFY( (latLonAltBox|latLonAltBox).isEmpty() );
    QVERIFY( !latLonAltBox.intersects( latLonAltBox ) );
}

void TestGeoDataLatLonAltBox::testCopyConstruction_data()
{
    QTest::addColumn<GeoDataLatLonAltBox>("expected");

    QTest::newRow("null") << GeoDataLatLonAltBox();
}

void TestGeoDataLatLonAltBox::testCopyConstruction()
{
    QFETCH(GeoDataLatLonAltBox, expected);

    GeoDataLatLonAltBox const result(expected);

    QCOMPARE(result, expected);
}

void TestGeoDataLatLonAltBox::testConstructionFromLatLonBox_data()
{
    QTest::addColumn<GeoDataLatLonBox>("latLonBox");
    QTest::addColumn<qreal>("minAltitude");
    QTest::addColumn<qreal>("maxAltitude");

    QTest::newRow("deg") << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(15.0), GeoDataNormalizedLatitude::fromDegrees(180.0), GeoDataNormalizedLongitude::fromDegrees(90.0), GeoDataNormalizedLongitude::fromDegrees(118.0)) << qreal(143.0) << qreal(356.0);
    QTest::newRow("rad") << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromRadians(1.0), GeoDataNormalizedLatitude::fromRadians(2.2), GeoDataNormalizedLongitude::fromDegrees(1.8), GeoDataNormalizedLongitude::fromDegrees(1.4)) << qreal(112.0) << qreal(120.0);
}

void TestGeoDataLatLonAltBox::testConstructionFromLatLonBox()
{
    QFETCH(GeoDataLatLonBox, latLonBox);
    QFETCH(qreal, minAltitude);
    QFETCH(qreal, maxAltitude);

    GeoDataLatLonAltBox const box(latLonBox, minAltitude, maxAltitude);

    QCOMPARE(box.west(), latLonBox.west());
    QCOMPARE(box.east(), latLonBox.east());
    QCOMPARE(box.north(), latLonBox.north());
    QCOMPARE(box.south(), latLonBox.south());
    QCOMPARE(box.rotation(), latLonBox.rotation());
    QCOMPARE(box.minAltitude(), minAltitude);
    QCOMPARE(box.maxAltitude(), maxAltitude);
}

void TestGeoDataLatLonAltBox::testAssignment_data()
{
    QTest::addColumn<GeoDataLatLonAltBox>("expected");

    QTest::newRow("deg") << GeoDataLatLonAltBox(GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(15.0), GeoDataNormalizedLatitude::fromDegrees(180.0), GeoDataNormalizedLongitude::fromDegrees(90.0), GeoDataNormalizedLongitude::fromDegrees(118.0)), 143.0, 356.0);
    QTest::newRow("rad") << GeoDataLatLonAltBox(GeoDataLatLonBox(GeoDataNormalizedLatitude::fromRadians(1.0), GeoDataNormalizedLatitude::fromRadians(2.2), GeoDataNormalizedLongitude::fromDegrees(1.8), GeoDataNormalizedLongitude::fromDegrees(1.4)), 112.0, 120.0);
}

void TestGeoDataLatLonAltBox::testAssignment()
{
    QFETCH(GeoDataLatLonAltBox, expected);

    GeoDataLatLonAltBox other = expected;

    QCOMPARE( expected, other );
}

void TestGeoDataLatLonAltBox::testContstructionFromCoordinates_data()
{
    QTest::addColumn<GeoDataCoordinates>("coordinates");

    QTest::newRow("deg") << GeoDataCoordinates(GeoDataLongitude::fromDegrees(90.0), GeoDataLatitude::fromDegrees(15.0), 10.0);
    QTest::newRow("rad") << GeoDataCoordinates(GeoDataLongitude::fromRadians(1.8), GeoDataLatitude::fromRadians(1.0), 61.0);
}

void TestGeoDataLatLonAltBox::testContstructionFromCoordinates()
{
    QFETCH(GeoDataCoordinates, coordinates);

    GeoDataLatLonAltBox const box(coordinates);

    QCOMPARE(box.east(), GeoDataNormalizedLongitude::fromLongitude(coordinates.longitude()));
    QCOMPARE(box.west(), GeoDataNormalizedLongitude::fromLongitude(coordinates.longitude()));
    QCOMPARE(box.north(), GeoDataNormalizedLatitude::fromLatitude(coordinates.latitude()));
    QCOMPARE(box.south(), GeoDataNormalizedLatitude::fromLatitude(coordinates.latitude()));
    QCOMPARE(box.minAltitude(), coordinates.altitude());
    QCOMPARE(box.maxAltitude(), coordinates.altitude());
}

void TestGeoDataLatLonAltBox::testAltitude_data() 
{
    QTest::addColumn<qreal>("alt");

    QTest::newRow("Altitude 1") << qreal(27.2);
    QTest::newRow("Altitude 2") << qreal(0.22);
}

void TestGeoDataLatLonAltBox::testAltitude() 
{
    QFETCH(qreal, alt);

    GeoDataLatLonAltBox box;
    box.setMinAltitude(alt);
    QCOMPARE(box.minAltitude(), alt);

    box.setMaxAltitude(alt);
    QCOMPARE(box.maxAltitude(), alt);
}

void TestGeoDataLatLonAltBox::testSetNorthRadian_data()
{
    QTest::addColumn<GeoDataLatLonAltBox>("box");
    QTest::addColumn<GeoDataNormalizedLatitude>("north");

    QTest::newRow("deg") << GeoDataLatLonAltBox(GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(15.0), GeoDataNormalizedLatitude::fromDegrees(180.0), GeoDataNormalizedLongitude::fromDegrees(90.0), GeoDataNormalizedLongitude::fromDegrees(118.0)), 143.0, 356.0) << GeoDataNormalizedLatitude::fromRadians(0.1);
    QTest::newRow("rad") << GeoDataLatLonAltBox(GeoDataLatLonBox(GeoDataNormalizedLatitude::fromRadians(1.0), GeoDataNormalizedLatitude::fromRadians(2.2), GeoDataNormalizedLongitude::fromDegrees(1.8), GeoDataNormalizedLongitude::fromDegrees(1.4)), 112.0, 120.0) << GeoDataNormalizedLatitude::fromRadians(0.1);
}

void TestGeoDataLatLonAltBox::testSetNorthRadian()
{
    QFETCH(GeoDataLatLonAltBox, box);
    QFETCH(GeoDataNormalizedLatitude, north);

    box.setNorth( north );

    QCOMPARE( box.north(), north );
}

void TestGeoDataLatLonAltBox::testSetSouthRadian_data()
{
    QTest::addColumn<GeoDataLatLonAltBox>("box");
    QTest::addColumn<GeoDataNormalizedLatitude>("south");

    QTest::newRow("deg") << GeoDataLatLonAltBox(GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(15.0), GeoDataNormalizedLatitude::fromDegrees(180.0), GeoDataNormalizedLongitude::fromDegrees(90.0), GeoDataNormalizedLongitude::fromDegrees(118.0)), 143.0, 356.0) << qreal(1.4);
    QTest::newRow("rad") << GeoDataLatLonAltBox(GeoDataLatLonBox(GeoDataNormalizedLatitude::fromRadians(1.0), GeoDataNormalizedLatitude::fromRadians(2.2), GeoDataNormalizedLongitude::fromDegrees(1.8), GeoDataNormalizedLongitude::fromDegrees(1.4)), 112.0, 120.0) << qreal(1.4);
}

void TestGeoDataLatLonAltBox::testSetSouthRadian()
{
    QFETCH(GeoDataLatLonAltBox, box);
    QFETCH(GeoDataNormalizedLatitude, south);

    box.setSouth( south );

    QCOMPARE( box.south(), south );
}

void TestGeoDataLatLonAltBox::testContains()
{
    GeoDataLatLonAltBox const largeBox = GeoDataLatLonAltBox::fromLineString( GeoDataLineString()
            << GeoDataCoordinates( GeoDataLongitude::fromDegrees(-20.0), GeoDataLatitude::fromDegrees(+10.0), 15.0)
            << GeoDataCoordinates( GeoDataLongitude::fromDegrees(+20.0), GeoDataLatitude::fromDegrees(-10.0), 25.0) );
    GeoDataLatLonAltBox const smallBox = GeoDataLatLonAltBox::fromLineString( GeoDataLineString()
            << GeoDataCoordinates( GeoDataLongitude::fromDegrees(-2.0), GeoDataLatitude::fromDegrees(+1.0), 18.0)
            << GeoDataCoordinates( GeoDataLongitude::fromDegrees(+2.0), GeoDataLatitude::fromDegrees(-1.0), 22.0) );

    QVERIFY( largeBox.contains( GeoDataCoordinates( GeoDataLongitude::fromDegrees(5.0), GeoDataLatitude::fromDegrees(5.0), 20.0) ) );
    QVERIFY( largeBox.contains( smallBox ) );
    QVERIFY( largeBox.contains( largeBox ) );
    QVERIFY( !smallBox.contains( largeBox ) );
    QVERIFY(  smallBox.contains( GeoDataCoordinates(   GeoDataLongitude::fromDegrees(0.0),   GeoDataLatitude::fromDegrees(0.0), 20.0) ) );
    QVERIFY( !largeBox.contains( GeoDataCoordinates(   GeoDataLongitude::fromDegrees(5.0),   GeoDataLatitude::fromDegrees(5.0), 30.0) ) );
    QVERIFY( !largeBox.contains( GeoDataCoordinates(   GeoDataLongitude::fromDegrees(5.0),   GeoDataLatitude::fromDegrees(5.0), 10.0) ) );
    QVERIFY( !largeBox.contains( GeoDataCoordinates(  GeoDataLongitude::fromDegrees(35.0),   GeoDataLatitude::fromDegrees(5.0), 20.0) ) );
    QVERIFY( !largeBox.contains( GeoDataCoordinates( GeoDataLongitude::fromDegrees(-35.0),   GeoDataLatitude::fromDegrees(5.0), 20.0) ) );
    QVERIFY( !largeBox.contains( GeoDataCoordinates(   GeoDataLongitude::fromDegrees(5.0),  GeoDataLatitude::fromDegrees(35.0), 20.0) ) );
    QVERIFY( !largeBox.contains( GeoDataCoordinates(   GeoDataLongitude::fromDegrees(5.0), GeoDataLatitude::fromDegrees(-35.0), 20.0) ) );
}

void TestGeoDataLatLonAltBox::testIntersects_data()
{
    QTest::addColumn<GeoDataLatLonBox>( "latLonBox1" );
    QTest::addColumn<qreal>( "box1minAltitude" );
    QTest::addColumn<qreal>( "box1maxAltitude" );
    QTest::addColumn<GeoDataLatLonBox>( "latLonBox2" );
    QTest::addColumn<qreal>( "box2minAltitude" );
    QTest::addColumn<qreal>( "box2maxAltitude" );
    QTest::addColumn<bool>( "intersects" );

    QTest::newRow( "empty1" ) << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromRadians(0.5), GeoDataNormalizedLatitude::fromRadians(0.4), GeoDataNormalizedLongitude::fromRadians(0.3), GeoDataNormalizedLongitude::fromRadians(0.2)) << qreal(0.0) << qreal(0.0)
                              << GeoDataLatLonBox() << qreal(0.0) << qreal(0.0)
                              << false;
    QTest::newRow( "empty2" ) << GeoDataLatLonBox() << qreal(0.0) << qreal(0.0)
                              << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromRadians(0.5), GeoDataNormalizedLatitude::fromRadians(0.4), GeoDataNormalizedLongitude::fromRadians(0.3), GeoDataNormalizedLongitude::fromRadians(0.2)) << qreal(0.0) << qreal(0.0)
                              << false;
    QTest::newRow( "same" ) << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(56.0), GeoDataNormalizedLatitude::fromDegrees(40.0), GeoDataNormalizedLongitude::fromDegrees(11.0), GeoDataNormalizedLongitude::fromDegrees(0.0)) << qreal(10.0) << qreal(12.0)
                            << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(56.0), GeoDataNormalizedLatitude::fromDegrees(40.0), GeoDataNormalizedLongitude::fromDegrees(11.0), GeoDataNormalizedLongitude::fromDegrees(0.0)) << qreal(10.0) << qreal(12.0)
                            << true;
    QTest::newRow( "dateLineFalse" ) << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0),  GeoDataNormalizedLongitude::fromDegrees(170.0), GeoDataNormalizedLongitude::fromDegrees(-170.0)) << qreal(0.0) << qreal(0.0)
                                     << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(-171.0),  GeoDataNormalizedLongitude::fromDegrees(171.0)) << qreal(0.0) << qreal(0.0)
                                     << false;
    QTest::newRow( "dateLineTrue" ) << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(20.0),   GeoDataNormalizedLatitude::fromDegrees(0.0),  GeoDataNormalizedLongitude::fromDegrees(171.0), GeoDataNormalizedLongitude::fromDegrees(-171.0)) << qreal(0.0) << qreal(0.0)
                                    << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(-170.0),  GeoDataNormalizedLongitude::fromDegrees(170.0)) << qreal(0.0) << qreal(0.0)
                                    << true;
}

void TestGeoDataLatLonAltBox::testIntersects()
{
    QFETCH( GeoDataLatLonBox, latLonBox1 );
    QFETCH( qreal, box1minAltitude );
    QFETCH( qreal, box1maxAltitude );
    QFETCH( GeoDataLatLonBox, latLonBox2 );
    QFETCH( qreal, box2minAltitude );
    QFETCH( qreal, box2maxAltitude );
    QFETCH( bool, intersects );

    const GeoDataLatLonAltBox box1( latLonBox1, box1minAltitude, box1maxAltitude );
    const GeoDataLatLonAltBox box2( latLonBox2, box2minAltitude, box2maxAltitude );

    QCOMPARE( box1.intersects( box2 ), intersects );
}

void TestGeoDataLatLonAltBox::testCrossesDateline_data()
{
    QTest::addColumn<GeoDataLatLonBox>("box");
    QTest::addColumn<bool>("expected");

    QTest::newRow("all") << GeoDataLatLonBox(GeoDataNormalizedLatitude::quaterCircle, -GeoDataNormalizedLatitude::quaterCircle, GeoDataNormalizedLongitude::fromDegrees(179.999), -GeoDataNormalizedLongitude::halfCircle) << false;

    QTest::newRow("left")  << GeoDataLatLonBox(GeoDataNormalizedLatitude::quaterCircle, -GeoDataNormalizedLatitude::quaterCircle,  GeoDataNormalizedLongitude::null           , -GeoDataNormalizedLongitude::halfCircle     ) << false;
    QTest::newRow("front") << GeoDataLatLonBox(GeoDataNormalizedLatitude::quaterCircle, -GeoDataNormalizedLatitude::quaterCircle,  GeoDataNormalizedLongitude::fromDegrees(90), -GeoDataNormalizedLongitude::fromDegrees(90)) << false;
    QTest::newRow("right") << GeoDataLatLonBox(GeoDataNormalizedLatitude::quaterCircle, -GeoDataNormalizedLatitude::quaterCircle,  GeoDataNormalizedLongitude::halfCircle     ,  GeoDataNormalizedLongitude::null           ) << false;
    QTest::newRow("back")  << GeoDataLatLonBox(GeoDataNormalizedLatitude::quaterCircle, -GeoDataNormalizedLatitude::quaterCircle, -GeoDataNormalizedLongitude::fromDegrees(90),  GeoDataNormalizedLongitude::fromDegrees(90)) << true;
}

void TestGeoDataLatLonAltBox::testCrossesDateline()
{
    QFETCH(GeoDataLatLonBox, box);
    QFETCH(bool, expected);

    bool const result = box.crossesDateLine();

    QCOMPARE(result, expected);
}

void TestGeoDataLatLonAltBox::testCenter_data()
{
    QTest::addColumn<GeoDataLatLonBox>( "box" );
    QTest::addColumn<GeoDataCoordinates>( "center" );

    QTest::newRow( "N-E" ) << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(60.0), GeoDataNormalizedLatitude::fromDegrees(40.0), GeoDataNormalizedLongitude::fromDegrees(30.0), GeoDataNormalizedLongitude::fromDegrees(10.0))
                           << GeoDataCoordinates( GeoDataLongitude::fromDegrees(20.0), GeoDataLatitude::fromDegrees(50.0), 0);

    QTest::newRow( "N-GW" ) << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(60.0), GeoDataNormalizedLatitude::fromDegrees(40.0), GeoDataNormalizedLongitude::fromDegrees(10.0), GeoDataNormalizedLongitude::fromDegrees(-30.0))
                            << GeoDataCoordinates( GeoDataLongitude::fromDegrees(-10.0), GeoDataLatitude::fromDegrees(50.0), 0);

    QTest::newRow( "N-W" ) << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(60.0), GeoDataNormalizedLatitude::fromDegrees(40.0), GeoDataNormalizedLongitude::fromDegrees(-10.0), GeoDataNormalizedLongitude::fromDegrees(-30.0))
                           << GeoDataCoordinates( GeoDataLongitude::fromDegrees(-20.0), GeoDataLatitude::fromDegrees(50.0), 0);

    QTest::newRow( "NS-W" ) << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(-10.0), GeoDataNormalizedLongitude::fromDegrees(-30.0))
                            << GeoDataCoordinates( GeoDataLongitude::fromDegrees(-20.0), GeoDataLatitude::fromDegrees(0.0), 0);

    QTest::newRow( "N-IDL" ) << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(-150.0), GeoDataNormalizedLongitude::fromDegrees(170.0))
                             << GeoDataCoordinates( GeoDataLongitude::fromDegrees(-170.0), GeoDataLatitude::fromDegrees(0.0), 0);
}

void TestGeoDataLatLonAltBox::testCenter()
{
    QFETCH( GeoDataLatLonBox, box );
    QFETCH( GeoDataCoordinates, center );

    QCOMPARE( box.center().latitude(), center.latitude() );
    QCOMPARE( box.center().longitude(), center.longitude() );
}

void TestGeoDataLatLonAltBox::testUnited_data()
{
    QTest::addColumn<GeoDataLatLonBox>( "box1" );
    QTest::addColumn<GeoDataLatLonBox>( "box2" );
    QTest::addColumn<GeoDataLatLonBox>( "expected" );

    QTest::newRow( "emptyRight" ) << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromRadians(0.5), GeoDataNormalizedLatitude::fromRadians(0.4), GeoDataNormalizedLongitude::fromRadians(0.3), GeoDataNormalizedLongitude::fromRadians(0.2))
                                  << GeoDataLatLonBox()
                                  << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromRadians(0.5), GeoDataNormalizedLatitude::fromRadians(0.4), GeoDataNormalizedLongitude::fromRadians(0.3), GeoDataNormalizedLongitude::fromRadians(0.2));

    QTest::newRow( "emptyLeft" ) << GeoDataLatLonBox()
                                 << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromRadians(0.5), GeoDataNormalizedLatitude::fromRadians(0.4), GeoDataNormalizedLongitude::fromRadians(0.3), GeoDataNormalizedLongitude::fromRadians(0.2))
                                 << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromRadians(0.5), GeoDataNormalizedLatitude::fromRadians(0.4), GeoDataNormalizedLongitude::fromRadians(0.3), GeoDataNormalizedLongitude::fromRadians(0.2));

    QTest::newRow( "same" ) << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(56.0), GeoDataNormalizedLatitude::fromDegrees(40.0), GeoDataNormalizedLongitude::fromDegrees(11.0), GeoDataNormalizedLongitude::fromDegrees(0.0))
                            << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(56.0), GeoDataNormalizedLatitude::fromDegrees(40.0), GeoDataNormalizedLongitude::fromDegrees(11.0), GeoDataNormalizedLongitude::fromDegrees(0.0))
                            << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(56.0), GeoDataNormalizedLatitude::fromDegrees(40.0), GeoDataNormalizedLongitude::fromDegrees(11.0), GeoDataNormalizedLongitude::fromDegrees(0.0));

    // 2 boxes in West, result stays west
    QTest::newRow( "bigWest" ) << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0),  GeoDataNormalizedLongitude::fromDegrees(-10.0),  GeoDataNormalizedLongitude::fromDegrees(-30.0))
                               << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(-150.0), GeoDataNormalizedLongitude::fromDegrees(-170.0))
                               << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0),  GeoDataNormalizedLongitude::fromDegrees(-10.0), GeoDataNormalizedLongitude::fromDegrees(-170.0));

    // 2 boxes each side of greenwich, result crosses greenwich
    QTest::newRow( "aroundGreenwich" ) << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(-10.0), GeoDataNormalizedLongitude::fromDegrees(-30.0))
                                       << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0),  GeoDataNormalizedLongitude::fromDegrees(30.0),  GeoDataNormalizedLongitude::fromDegrees(10.0))
                                       << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0),  GeoDataNormalizedLongitude::fromDegrees(30.0), GeoDataNormalizedLongitude::fromDegrees(-30.0));

    // 2 boxes crossing greenwich, result crosses greenwich
    QTest::newRow( "aroundGreenwich" ) << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(10.0), GeoDataNormalizedLongitude::fromDegrees(-30.0))
                                       << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(30.0), GeoDataNormalizedLongitude::fromDegrees(-10.0))
                                       << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(30.0), GeoDataNormalizedLongitude::fromDegrees(-30.0));

    // 2 boxes each side of IDL, result crosses IDL as smaller box
    QTest::newRow( "aroundIDL" ) << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(-150.0), GeoDataNormalizedLongitude::fromDegrees(-170.0))
                                 << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0),  GeoDataNormalizedLongitude::fromDegrees(170.0),  GeoDataNormalizedLongitude::fromDegrees(150.0))
                                 << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(-150.0),  GeoDataNormalizedLongitude::fromDegrees(150.0));

    // reciprocical, so independent of side
    QTest::newRow( "aroundIDL2" ) << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0),  GeoDataNormalizedLongitude::fromDegrees(170.0),  GeoDataNormalizedLongitude::fromDegrees(150.0))
                                  << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(-150.0), GeoDataNormalizedLongitude::fromDegrees(-170.0))
                                  << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(-150.0),  GeoDataNormalizedLongitude::fromDegrees(150.0));

    // 1 box crossing IDL, the 2 centers are close together, result crosses IDL
    QTest::newRow( "crossingIDLclose" ) << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(-130.0), GeoDataNormalizedLongitude::fromDegrees(-150.0))
                                        << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(-150.0),  GeoDataNormalizedLongitude::fromDegrees(170.0))
                                        << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(-130.0),  GeoDataNormalizedLongitude::fromDegrees(170.0));

    // reciprocical
    QTest::newRow( "crossingIDLclose2" ) << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(-160.0),  GeoDataNormalizedLongitude::fromDegrees(170.0))
                                         << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(-140.0), GeoDataNormalizedLongitude::fromDegrees(-150.0))
                                         << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(-140.0),  GeoDataNormalizedLongitude::fromDegrees(170.0));

    // 1 box crossing IDL, the 2 centers are across IDL, result crosses IDL
    QTest::newRow( "crossingIDLfar" ) << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(-150.0), GeoDataNormalizedLongitude::fromDegrees(-170.0))
                                      << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(-170.0),  GeoDataNormalizedLongitude::fromDegrees(150.0))
                                      << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(-150.0),  GeoDataNormalizedLongitude::fromDegrees(150.0));

    // reciprocical
    QTest::newRow( "crossingIDLfar2" ) << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(-170.0),  GeoDataNormalizedLongitude::fromDegrees(150.0))
                                       << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(-150.0), GeoDataNormalizedLongitude::fromDegrees(-170.0))
                                       << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(-150.0),  GeoDataNormalizedLongitude::fromDegrees(150.0));

    // 2 box crossing IDL, the 2 centers are close together, result crosses IDL
    QTest::newRow( "crossingsIDLclose" ) << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(-140.0), GeoDataNormalizedLongitude::fromDegrees(160.0))
                                         << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(-160.0), GeoDataNormalizedLongitude::fromDegrees(170.0))
                                         << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(-140.0), GeoDataNormalizedLongitude::fromDegrees(160.0));

    // 2 box crossing IDL, the 2 centers are across IDL, result crosses IDL
    QTest::newRow( "crossingsIDLfar" ) << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(-150.0), GeoDataNormalizedLongitude::fromDegrees(-170.0))
                                       << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(-170.0),  GeoDataNormalizedLongitude::fromDegrees(150.0))
                                       << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-30.0), GeoDataNormalizedLongitude::fromDegrees(-150.0),  GeoDataNormalizedLongitude::fromDegrees(150.0));

    QTest::newRow( "bug299959" ) << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(90.0), GeoDataNormalizedLatitude::fromDegrees(-90.0), GeoDataNormalizedLongitude::fromDegrees(180.0), GeoDataNormalizedLongitude::fromDegrees(-180.0))
                                 << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(18.0), GeoDataNormalizedLatitude::fromDegrees(-18.0),  GeoDataNormalizedLongitude::fromDegrees(30.0),   GeoDataNormalizedLongitude::fromDegrees(20.0))
                                 << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(90.0), GeoDataNormalizedLatitude::fromDegrees(-90.0), GeoDataNormalizedLongitude::fromDegrees(180.0), GeoDataNormalizedLongitude::fromDegrees(-180.0));
}

void TestGeoDataLatLonAltBox::testUnited()
{
    QFETCH( GeoDataLatLonBox, box1 );
    QFETCH( GeoDataLatLonBox, box2 );
    QFETCH( GeoDataLatLonBox, expected );

    GeoDataLatLonBox const result = box1 | box2;

    QCOMPARE( result.north(), expected.north() );
    QCOMPARE( result.south(), expected.south() );
    QCOMPARE( result.west(), expected.west() );
    QCOMPARE( result.east(), expected.east() );
}

void TestGeoDataLatLonAltBox::testFromLineString_data() 
{
    QTest::addColumn<GeoDataLineString>("string");
    QTest::addColumn<GeoDataLatLonBox>("expected");

    QTest::newRow("empty") << GeoDataLineString() << GeoDataLatLonBox(GeoDataNormalizedLatitude::null, GeoDataNormalizedLatitude::null, GeoDataNormalizedLongitude::null, GeoDataNormalizedLongitude::null);

    QTest::newRow("bug 299527")
            << (GeoDataLineString()
                 << GeoDataCoordinates(       GeoDataLongitude::null, GeoDataLatitude::null, 200)
                 << GeoDataCoordinates( GeoDataLongitude::halfCircle, GeoDataLatitude::null, 200)
                 << GeoDataCoordinates(-GeoDataLongitude::halfCircle, GeoDataLatitude::null, 200)
                 << GeoDataCoordinates(       GeoDataLongitude::null, GeoDataLatitude::null, 200))
            << GeoDataLatLonBox(GeoDataNormalizedLatitude::quaterCircle, GeoDataNormalizedLatitude::null, GeoDataNormalizedLongitude::halfCircle, -GeoDataNormalizedLongitude::halfCircle);

    QTest::newRow("around south pole")
            << (GeoDataLineString()
                 << GeoDataCoordinates(       GeoDataLongitude::null, -GeoDataLatitude::fromDegrees(10), 200)
                 << GeoDataCoordinates( GeoDataLongitude::halfCircle, -GeoDataLatitude::fromDegrees(10), 200)
                 << GeoDataCoordinates(-GeoDataLongitude::halfCircle, -GeoDataLatitude::fromDegrees(10), 200)
                 << GeoDataCoordinates(       GeoDataLongitude::null, -GeoDataLatitude::fromDegrees(10), 200))
            << GeoDataLatLonBox(-GeoDataNormalizedLatitude::fromDegrees(10.0), -GeoDataNormalizedLatitude::quaterCircle, GeoDataNormalizedLongitude::halfCircle, -GeoDataNormalizedLongitude::halfCircle);
}

void TestGeoDataLatLonAltBox::testFromLineString() {
    QFETCH(GeoDataLineString, string);
    QFETCH(GeoDataLatLonBox, expected);

    GeoDataLatLonAltBox const result = GeoDataLatLonAltBox::fromLineString(string);

    QCOMPARE(result.north(), expected.north());
    QCOMPARE(result.south(), expected.south());
    QCOMPARE(result.east(), expected.east());
    QCOMPARE(result.west(), expected.west());
}

void TestGeoDataLatLonAltBox::testToString_data() {
    
}

void TestGeoDataLatLonAltBox::testToString() {
    
}

void TestGeoDataLatLonAltBox::testPack_data() {
    QTest::addColumn<GeoDataCoordinates>("coordinates");

    QTest::newRow("rad1") << GeoDataCoordinates(GeoDataLongitude::fromRadians(1.0), GeoDataLatitude::fromRadians(2.2), 1.8);
    QTest::newRow("rad2") << GeoDataCoordinates(GeoDataLongitude::fromRadians(0.2), GeoDataLatitude::fromRadians(3.1), 2.9);
}

void TestGeoDataLatLonAltBox::testPack() {
    QFETCH(GeoDataCoordinates, coordinates);

    GeoDataLatLonAltBox const original = GeoDataLatLonAltBox(coordinates);

    QBuffer buffer;
    bool const isOpenForWriting = buffer.open(QBuffer::WriteOnly);

    QVERIFY(isOpenForWriting);

    QDataStream out(&buffer);
    original.pack(out);
    buffer.close();

    bool const isOpenForReading = buffer.open(QBuffer::ReadOnly);

    QVERIFY(isOpenForReading);

    QDataStream in(&buffer);

    GeoDataLatLonAltBox unpacked;
    unpacked.unpack(in);

    buffer.close();

#if 0
    QCOMPARE(unpacked.north(), original.north());
    QCOMPARE(unpacked.south(), original.south());
    QCOMPARE(unpacked.east(), original.east());
    QCOMPARE(unpacked.west(), original.west());
#endif

    QCOMPARE(unpacked.maxAltitude(), original.maxAltitude());
    QCOMPARE(unpacked.minAltitude(), original.minAltitude());
    QCOMPARE(unpacked.altitudeMode(), original.altitudeMode());
}

void TestGeoDataLatLonAltBox::testContainerBox_data() {
    QTest::addColumn<GeoDataNormalizedLongitude>("lon1");
    QTest::addColumn<GeoDataNormalizedLatitude>("lat1");
    QTest::addColumn<GeoDataNormalizedLongitude>("lon2");
    QTest::addColumn<GeoDataNormalizedLatitude>("lat2");
    QTest::addColumn<GeoDataNormalizedLongitude>("lon3");
    QTest::addColumn<GeoDataNormalizedLatitude>("lat3");

    QTest::newRow("rad1") << GeoDataNormalizedLongitude::fromRadians(2.4) << GeoDataNormalizedLatitude::fromRadians(0.1) << GeoDataNormalizedLongitude::fromRadians(1.0) << GeoDataNormalizedLatitude::fromRadians(1.2) << GeoDataNormalizedLongitude::fromRadians(-1.8) << GeoDataNormalizedLatitude::fromRadians(-0.7) ;
    QTest::newRow("rad2") << GeoDataNormalizedLongitude::fromRadians(-1.3) << GeoDataNormalizedLatitude::fromRadians(-0.1) << GeoDataNormalizedLongitude::fromRadians(0.2) << GeoDataNormalizedLatitude::fromRadians(1.1) << GeoDataNormalizedLongitude::fromRadians(2.9) << GeoDataNormalizedLatitude::fromRadians(0.9) ;
}

void TestGeoDataLatLonAltBox::testContainerBox()
{
    QFETCH(GeoDataNormalizedLongitude, lon1);
    QFETCH(GeoDataNormalizedLatitude, lat1);
    QFETCH(GeoDataNormalizedLongitude, lon2);
    QFETCH(GeoDataNormalizedLatitude, lat2);
    QFETCH(GeoDataNormalizedLongitude, lon3);
    QFETCH(GeoDataNormalizedLatitude, lat3);

    GeoDataPlacemark p1, p2, p3;
    p1.setCoordinate(lon1, lat1);
    p2.setCoordinate(lon2, lat2);
    p3.setCoordinate(lon3, lat3);
    GeoDataFolder f1, f2;
    f1.append(new GeoDataPlacemark(p1));
    f2.append(new GeoDataPlacemark(p2));
    f2.append(new GeoDataPlacemark(p3));
    f1.append(new GeoDataFolder(f2));
    GeoDataLatLonAltBox box = f1.latLonAltBox();

    QCOMPARE(box.north(), qMax(qMax(lat1, lat2), lat3));
    QCOMPARE(box.east(), qMax(qMax(lon1, lon2), lon3));
    QCOMPARE(box.south(), qMin(qMin(lat1, lat2), lat3));
    QCOMPARE(box.west(), qMin(qMin(lon1, lon2), lon3));
}

void TestGeoDataLatLonAltBox::testScale_data()
{
    QTest::addColumn<GeoDataLatLonBox>( "box" );
    QTest::addColumn<qreal>( "verticalScale" );
    QTest::addColumn<qreal>( "horizontalScale" );
    QTest::addColumn<GeoDataLatLonBox>( "expected" );

    QTest::newRow( "void" ) << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromRadians(0.5), GeoDataNormalizedLatitude::fromRadians(0.4), GeoDataNormalizedLongitude::fromRadians(0.3), GeoDataNormalizedLongitude::fromRadians(0.2))
                            << qreal(1.0)
                            << qreal(1.0)
                            << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromRadians(0.5), GeoDataNormalizedLatitude::fromRadians(0.4), GeoDataNormalizedLongitude::fromRadians(0.3), GeoDataNormalizedLongitude::fromRadians(0.2));
    QTest::newRow( "simple vertical" ) << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromRadians(0.5), GeoDataNormalizedLatitude::fromRadians(0.4), GeoDataNormalizedLongitude::fromRadians(0.3), GeoDataNormalizedLongitude::fromRadians(0.2))
                                       << qreal(1.5)
                                       << qreal(1.0)
                                       << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromRadians(0.525), GeoDataNormalizedLatitude::fromRadians(0.375), GeoDataNormalizedLongitude::fromRadians(0.3), GeoDataNormalizedLongitude::fromRadians(0.2));
    QTest::newRow( "simple horizontal" ) << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromRadians(0.7), GeoDataNormalizedLatitude::fromRadians(0.6), GeoDataNormalizedLongitude::fromRadians(0.3), GeoDataNormalizedLongitude::fromRadians(0.2))
                                         << qreal(1.0)
                                         << qreal(2.0)
                                         << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromRadians(0.7), GeoDataNormalizedLatitude::fromRadians(0.6), GeoDataNormalizedLongitude::fromRadians(0.35), GeoDataNormalizedLongitude::fromRadians(0.15));
    QTest::newRow( "crosses dateline" )  << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(20.0),   GeoDataNormalizedLatitude::fromDegrees(0.0), GeoDataNormalizedLongitude::fromDegrees(-170.0), GeoDataNormalizedLongitude::fromDegrees(170.0))
                                         << qreal(2.0)
                                         << qreal(2.0)
                                         << GeoDataLatLonBox(GeoDataNormalizedLatitude::fromDegrees(30.0), GeoDataNormalizedLatitude::fromDegrees(-10.0), GeoDataNormalizedLongitude::fromDegrees(-160.0), GeoDataNormalizedLongitude::fromDegrees(160.0));
}

void TestGeoDataLatLonAltBox::testScale()
{
    QFETCH(GeoDataLatLonBox, box);
    QFETCH(qreal, verticalScale);
    QFETCH(qreal, horizontalScale);
    QFETCH(GeoDataLatLonBox, expected);
    GeoDataLatLonBox const scaled = box.scaled(verticalScale, horizontalScale);
    QCOMPARE(scaled.west(), expected.west());
    QCOMPARE(scaled.north(), expected.north());
    QCOMPARE(scaled.south(), expected.south());
    QCOMPARE(scaled.east(), expected.east());
    QCOMPARE(scaled.rotation(), expected.rotation());
    QCOMPARE(scaled.center(), expected.center());
}

QTEST_MAIN(TestGeoDataLatLonAltBox)
#include "TestGeoDataLatLonAltBox.moc"

