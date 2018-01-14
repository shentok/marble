//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2012       Thibaut Gridel <tgridel@free.fr>
// Copyright 2012,2013  Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include "TestUtils.h"

#include "ViewportParams.h"

#include "AbstractProjection.h"
#include "GeoDataCoordinates.h"
#include "GeoDataLineString.h"
#include "GeoDataLinearRing.h"

Q_DECLARE_METATYPE( Marble::GeoDataLinearRing )
Q_DECLARE_METATYPE( Marble::Projection )
Q_DECLARE_METATYPE( Marble::TessellationFlag )
Q_DECLARE_METATYPE( Marble::TessellationFlags )

namespace Marble
{

class ViewportParamsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void constructorDefaultValues();

    void constructorValues_data();
    void constructorValues();

    void screenCoordinates_GeoDataLineString_data();
    void screenCoordinates_GeoDataLineString();

    void screenCoordinates_GeoDataLineString2();

    void screenCoordinates_GeoDataLinearRing();

    void geoDataLinearRing_data();
    void geoDataLinearRing();

    void setInvalidRadius();

    void setFocusPoint();
};

void ViewportParamsTest::constructorDefaultValues()
{
    const ViewportParams viewport;

    QCOMPARE( viewport.projection(), Spherical );
    QCOMPARE( viewport.size(), QSize( 100, 100 ) );
    QCOMPARE( viewport.width(), 100 );
    QCOMPARE( viewport.height(), 100 );
    QCOMPARE( viewport.centerLongitude(), GeoDataLongitude::null );
    QCOMPARE( viewport.centerLatitude(), GeoDataLatitude::null );
    QCOMPARE( viewport.polarity(), 1 );
    QCOMPARE( viewport.radius(), 2000 );
    QCOMPARE( viewport.mapCoversViewport(), true );
    QCOMPARE( viewport.focusPoint(), GeoDataCoordinates(GeoDataLongitude::null, GeoDataLatitude::null, 0.) );

    // invariants:
    QVERIFY( viewport.radius() > 0 ); // avoids divisions by zero
    QVERIFY( viewport.viewLatLonAltBox() == viewport.latLonAltBox( QRect( 0, 0, 100, 100 ) ) );
    // FIXME QCOMPARE( viewport.viewLatLonAltBox().center().longitude(), viewport.centerLongitude() );
    // FIXME QCOMPARE( viewport.viewLatLonAltBox().center().latitude(), viewport.centerLatitude() );
}

void ViewportParamsTest::constructorValues_data()
{
    QTest::addColumn<Marble::Projection>( "projection" );
    QTest::addColumn<GeoDataLongitude>( "lon" );
    QTest::addColumn<GeoDataLatitude>( "lat" );
    QTest::addColumn<int>( "radius" );
    QTest::addColumn<QSize>( "size" );

    ViewportParams viewport;

    viewport.setProjection( Spherical );
    const AbstractProjection *const spherical = viewport.currentProjection();

    viewport.setProjection( Mercator);
    const AbstractProjection *const mercator = viewport.currentProjection();

    viewport.setProjection( Equirectangular );
    const AbstractProjection *const equirectangular = viewport.currentProjection();

    addRow() << Spherical << GeoDataLongitude::fromDegrees(0) << GeoDataLatitude::fromDegrees(0) << 2000 << QSize( 100, 100 );
    addRow() << Mercator << GeoDataLongitude::fromDegrees(0) << GeoDataLatitude::fromDegrees(0) << 2000 << QSize( 100, 100 );
    addRow() << Equirectangular << GeoDataLongitude::fromDegrees(0) << GeoDataLatitude::fromDegrees(0) << 2000 << QSize( 100, 100 );

    addRow() << Spherical << GeoDataLongitude::fromDegrees(205) << spherical->maxValidLat() + GeoDataLatitude::fromRadians(1.0) << 2000 << QSize( 100, 100 );
    addRow() << Mercator << GeoDataLongitude::fromDegrees(205) << mercator->maxValidLat() + GeoDataLatitude::fromRadians(1.0) << 2000 << QSize( 100, 100 );
    addRow() << Equirectangular << GeoDataLongitude::fromDegrees(205) << equirectangular->maxValidLat() + GeoDataLatitude::fromRadians(1.0) << 2000 << QSize( 100, 100 );
}

void ViewportParamsTest::constructorValues()
{
    QFETCH( Projection, projection );
    QFETCH( GeoDataLongitude, lon );
    QFETCH( GeoDataLatitude, lat );
    QFETCH( int, radius );
    QFETCH( QSize, size );

    const ViewportParams byConstructor( projection, lon, lat, radius, size );

    ViewportParams bySetters;
    bySetters.setProjection( projection );
    bySetters.centerOn( lon, lat );
    bySetters.setRadius( radius );
    bySetters.setSize( size );

    QCOMPARE( byConstructor.projection(), bySetters.projection() );
    QCOMPARE( byConstructor.currentProjection(), bySetters.currentProjection() );
    QCOMPARE( byConstructor.centerLongitude(), bySetters.centerLongitude() );
    QCOMPARE( byConstructor.centerLatitude(), bySetters.centerLatitude() );
    QCOMPARE( byConstructor.planetAxis(), bySetters.planetAxis() );
    QCOMPARE( byConstructor.angularResolution(), bySetters.angularResolution() );
    QCOMPARE( byConstructor.radius(), bySetters.radius() );
    QCOMPARE( byConstructor.size(), bySetters.size() );
}

void ViewportParamsTest::screenCoordinates_GeoDataLineString_data()
{
    QTest::addColumn<Marble::Projection>( "projection" );
    QTest::addColumn<Marble::TessellationFlags>( "tessellation" );
    QTest::addColumn<GeoDataLineString>( "line" );
    QTest::addColumn<int>( "size" );

    GeoDataLineString longitudeLine;
    longitudeLine << GeoDataCoordinates(GeoDataLongitude::fromDegrees(185), GeoDataLatitude::fromDegrees(5))
                  << GeoDataCoordinates(GeoDataLongitude::fromDegrees(185), GeoDataLatitude::fromDegrees(15));

    GeoDataLineString diagonalLine;
    diagonalLine << GeoDataCoordinates(-GeoDataLongitude::fromDegrees(185), GeoDataLatitude::fromDegrees(5))
                 << GeoDataCoordinates(GeoDataLongitude::fromDegrees(185), GeoDataLatitude::fromDegrees(15));

    GeoDataLineString latitudeLine;
    latitudeLine << GeoDataCoordinates(-GeoDataLongitude::fromDegrees(185), GeoDataLatitude::fromDegrees(5))
                 << GeoDataCoordinates(GeoDataLongitude::fromDegrees(185), GeoDataLatitude::fromDegrees(5));

    Projection projection = Mercator;

    TessellationFlags flags = NoTessellation;
    QTest::newRow("Mercator NoTesselation Longitude")
            << projection << flags << longitudeLine << 2;

    QTest::newRow("Mercator NoTesselation Diagonal IDL")
            << projection << flags << diagonalLine << 2;

    QTest::newRow("Mercator NoTesselation Latitude IDL")
            << projection << flags << latitudeLine << 2;

    flags = Tessellate;
    QTest::newRow("Mercator Tesselate Longitude")
            << projection << flags << longitudeLine << 2;

    QTest::newRow("Mercator Tesselate Diagonal IDL")
            << projection << flags << diagonalLine << 2;

    QTest::newRow("Mercator Tesselate Latitude IDL")
            << projection << flags << latitudeLine << 2;

    flags = Tessellate | RespectLatitudeCircle;
    QTest::newRow("Mercator LatitudeCircle Longitude")
            << projection << flags << longitudeLine << 2;

    QTest::newRow("Mercator LatitudeCircle Diagonal IDL")
            << projection << flags << diagonalLine << 2;

    QTest::newRow("Mercator LatitudeCircle Latitude IDL")
            << projection << flags << latitudeLine << 2;

    projection = Equirectangular;

    flags = NoTessellation;
    QTest::newRow("Equirect NoTesselation Longitude")
            << projection << flags << longitudeLine << 2;

    QTest::newRow("Equirect NoTesselation Diagonal IDL")
            << projection << flags << diagonalLine << 2;

    QTest::newRow("Equirect NoTesselation Latitude IDL")
            << projection << flags << latitudeLine << 2;

    flags = Tessellate;
    QTest::newRow("Equirect Tesselate Longitude")
            << projection << flags << longitudeLine << 2;

    QTest::newRow("Equirect Tesselate Diagonal IDL")
            << projection << flags << diagonalLine << 2;

    QTest::newRow("Equirect Tesselate Latitude IDL")
            << projection << flags << latitudeLine << 2;

    flags = Tessellate | RespectLatitudeCircle;
    QTest::newRow("Equirect LatitudeCircle Longitude")
            << projection << flags << longitudeLine << 2;

    QTest::newRow("Equirect LatitudeCircle Diagonal IDL")
            << projection << flags << diagonalLine << 2;

    QTest::newRow("Equirect LatitudeCircle Latitude IDL")
            << projection << flags << latitudeLine << 2;


    projection = Spherical;

    flags = NoTessellation;
    QTest::newRow("Spherical NoTesselation Longitude")
            << projection << flags << longitudeLine << 1;

    QTest::newRow("Spherical NoTesselation Diagonal IDL")
            << projection << flags << diagonalLine << 1;

    QTest::newRow("Spherical NoTesselation Latitude IDL")
            << projection << flags << latitudeLine << 1;

    flags = Tessellate;
    QTest::newRow("Spherical Tesselate Longitude")
            << projection << flags << longitudeLine << 1;

    QTest::newRow("Spherical Tesselate Diagonal IDL")
            << projection << flags << diagonalLine << 1;

    QTest::newRow("Spherical Tesselate Latitude IDL")
            << projection << flags << latitudeLine << 1;

    flags = Tessellate | RespectLatitudeCircle;
    QTest::newRow("Spherical LatitudeCircle Longitude")
            << projection << flags << longitudeLine << 1;

    QTest::newRow("Spherical LatitudeCircle Diagonal IDL")
            << projection << flags << diagonalLine << 1;

    QTest::newRow("Spherical LatitudeCircle Latitude IDL")
            << projection << flags << latitudeLine << 1;

}

void ViewportParamsTest::screenCoordinates_GeoDataLineString()
{
    QFETCH( Marble::Projection, projection );
    QFETCH( Marble::TessellationFlags, tessellation );
    QFETCH( GeoDataLineString, line );
    QFETCH( int, size );

    ViewportParams viewport;
    viewport.setProjection( projection );
    viewport.setRadius( 360 / 4 ); // for easy mapping of lon <-> x
    viewport.centerOn(GeoDataLongitude::fromDegrees(185), GeoDataLatitude::null);

    line.setTessellationFlags( tessellation );
    QVector<QPolygonF*> polys;
    viewport.screenCoordinates(line, polys);

    foreach (QPolygonF* poly, polys) {
        // at least 2 points in one poly
        QVERIFY( poly->size() > 1 );
        QPointF oldCoord = poly->first();
        poly->pop_front();

        foreach(const QPointF &coord, *poly) {
            // no 2 same points
            QVERIFY( (coord-oldCoord) != QPointF() );

            // no 2 consecutive points should be more than 90° apart
            QVERIFY( (coord-oldCoord).manhattanLength() < viewport.radius() );
            oldCoord = coord;
        }
    }

    // check the provided number of polys
    QCOMPARE( polys.size(), size );
}

void ViewportParamsTest::screenCoordinates_GeoDataLineString2()
{
    const ViewportParams viewport(Spherical, 0.5 * GeoDataLongitude::halfCircle, GeoDataLatitude::fromDegrees(38), 256, QSize(1165, 833));

    const GeoDataCoordinates coordinates(-0.5 * GeoDataLongitude::halfCircle, GeoDataLatitude::fromDegrees(23.44));
    qreal x, y;
    bool globeHidesPoint;
    viewport.screenCoordinates( coordinates, x, y, globeHidesPoint );

    QCOMPARE( globeHidesPoint, true );

    GeoDataLineString line( Tessellate | RespectLatitudeCircle );
    line << GeoDataCoordinates( -GeoDataLongitude::halfCircle, GeoDataLatitude::fromDegrees(23.4400));
    line << GeoDataCoordinates( GeoDataLongitude::null, GeoDataLatitude::fromDegrees(23.4400));

    QVector<QPolygonF*> polys;
    viewport.screenCoordinates( line, polys );

    QCOMPARE( polys.size(), 2 );
}

void ViewportParamsTest::screenCoordinates_GeoDataLinearRing()
{
    // Creates a Rectangle on the eastern southern hemisphere
    // with the planet rotated so that only the western half
    // of the rectangle is visible. As a result only a single
    // screen polygon should be rendered.

    const ViewportParams viewport(Spherical, -GeoDataLongitude::fromDegrees(15), GeoDataLatitude::null, 350, QSize(1000, 750));

    GeoDataLinearRing line( Tessellate );
    GeoDataCoordinates coord1 ( GeoDataLongitude::fromDegrees(30), -GeoDataLatitude::fromDegrees(10) );
    GeoDataCoordinates coord2 ( GeoDataLongitude::fromDegrees(30), -GeoDataLatitude::fromDegrees(45) );
    GeoDataCoordinates coord3 ( GeoDataLongitude::fromDegrees(100), -GeoDataLatitude::fromDegrees(45) );
    GeoDataCoordinates coord4 ( GeoDataLongitude::fromDegrees(100), -GeoDataLatitude::fromDegrees(10) );

    qreal x, y;
    bool globeHidesPoint;
    viewport.screenCoordinates( coord1, x, y, globeHidesPoint );
    QCOMPARE( globeHidesPoint, false );
    viewport.screenCoordinates( coord2, x, y, globeHidesPoint );
    QCOMPARE( globeHidesPoint, false );
    viewport.screenCoordinates( coord3, x, y, globeHidesPoint );
    QCOMPARE( globeHidesPoint, true );
    viewport.screenCoordinates( coord4, x, y, globeHidesPoint );
    QCOMPARE( globeHidesPoint, true );

    line << coord1 << coord2 << coord3 << coord4;

    QVector<QPolygonF*> polys;
    viewport.screenCoordinates( line, polys );

    QCOMPARE( polys.size(), 1 );
}

void ViewportParamsTest::geoDataLinearRing_data()
{
    QTest::addColumn<Marble::Projection>( "projection" );
    QTest::addColumn<Marble::TessellationFlags>( "tessellation" );
    QTest::addColumn<GeoDataLinearRing>( "ring" );
    QTest::addColumn<int>( "size" );

    GeoDataLinearRing normalRing;
    normalRing << GeoDataCoordinates(GeoDataLongitude::fromDegrees(175), GeoDataLatitude::fromDegrees(5))
               << GeoDataCoordinates(GeoDataLongitude::fromDegrees(175), GeoDataLatitude::fromDegrees(15))
               << GeoDataCoordinates(GeoDataLongitude::fromDegrees(170), GeoDataLatitude::fromDegrees(15));

    GeoDataLinearRing acrossIDLRing;
    acrossIDLRing << GeoDataCoordinates(-GeoDataLongitude::fromDegrees(175), GeoDataLatitude::fromDegrees(5))
                  << GeoDataCoordinates(GeoDataLongitude::fromDegrees(175), GeoDataLatitude::fromDegrees(5))
                  << GeoDataCoordinates(GeoDataLongitude::fromDegrees(175), GeoDataLatitude::fromDegrees(15));

    GeoDataLinearRing aroundSPoleRing;
    aroundSPoleRing << GeoDataCoordinates(-GeoDataLongitude::fromDegrees(175), -GeoDataLatitude::fromDegrees(65))
                 << GeoDataCoordinates(-GeoDataLongitude::fromDegrees(55), -GeoDataLatitude::fromDegrees(70))
                    << GeoDataCoordinates(GeoDataLongitude::fromDegrees(65), -GeoDataLatitude::fromDegrees(75));

    Projection projection = Mercator;

    TessellationFlags flags = NoTessellation;
    QTest::newRow("Mercator NoTesselation normalRing")
            << projection << flags << normalRing << 2;

    QTest::newRow("Mercator NoTesselation acrossIDLRing")
            << projection << flags << acrossIDLRing << 2;

#ifdef BUG_357540_IS_FIXED
    QTest::newRow("Mercator NoTesselation aroundSPoleRing")
            << projection << flags << aroundSPoleRing << 2;
#endif

    flags = Tessellate;
    QTest::newRow("Mercator Tesselate normalRing")
            << projection << flags << normalRing << 2;

    QTest::newRow("Mercator Tesselate acrossIDLRing")
            << projection << flags << acrossIDLRing << 2;

#ifdef BUG_357540_IS_FIXED
    QTest::newRow("Mercator Tesselate aroundSPoleRing")
            << projection << flags << aroundSPoleRing << 2;
#endif

    flags = Tessellate | RespectLatitudeCircle;
    QTest::newRow("Mercator LatitudeCircle normalRing")
            << projection << flags << normalRing << 2;

    QTest::newRow("Mercator LatitudeCircle acrossIDLRing")
            << projection << flags << acrossIDLRing << 2;

#ifdef BUG_357540_IS_FIXED
    QTest::newRow("Mercator LatitudeCircle aroundSPoleRing")
            << projection << flags << aroundSPoleRing << 2;
#endif

    projection = Equirectangular;

    flags = NoTessellation;
    QTest::newRow("Equirect NoTesselation normalRing")
            << projection << flags << normalRing << 2;

    QTest::newRow("Equirect NoTesselation acrossIDLRing")
            << projection << flags << acrossIDLRing << 2;

#ifdef BUG_357540_IS_FIXED
    QTest::newRow("Equirect NoTesselation aroundSPoleRing")
            << projection << flags << aroundSPoleRing << 2;
#endif

    flags = Tessellate;
    QTest::newRow("Equirect Tesselate normalRing")
            << projection << flags << normalRing << 2;

    QTest::newRow("Equirect Tesselate acrossIDLRing")
            << projection << flags << acrossIDLRing << 2;

#ifdef BUG_357540_IS_FIXED
    QTest::newRow("Equirect Tesselate aroundSPoleRing")
            << projection << flags << aroundSPoleRing << 2;
#endif

    flags = Tessellate | RespectLatitudeCircle;
    QTest::newRow("Equirect LatitudeCircle normalRing")
            << projection << flags << normalRing << 2;

    QTest::newRow("Equirect LatitudeCircle acrossIDLRing")
            << projection << flags << acrossIDLRing << 2;

#ifdef BUG_357540_IS_FIXED
    QTest::newRow("Equirect LatitudeCircle aroundSPoleRing")
            << projection << flags << aroundSPoleRing << 2;
#endif

    projection = Spherical;

    flags = NoTessellation;
    QTest::newRow("Spherical NoTesselation normalRing")
            << projection << flags << normalRing << 1;

    QTest::newRow("Spherical NoTesselation acrossIDLRing")
            << projection << flags << acrossIDLRing << 1;

    QTest::newRow("Spherical NoTesselation aroundSPoleRing")
            << projection << flags << aroundSPoleRing << 1;

    flags = Tessellate;
    QTest::newRow("Spherical Tesselate normalRing")
            << projection << flags << normalRing << 1;

    QTest::newRow("Spherical Tesselate acrossIDLRing")
            << projection << flags << acrossIDLRing << 1;

/*    QTest::newRow("Spherical Tesselate aroundSPoleRing")
            << projection << flags << aroundSPoleRing << 1;*/

    flags = Tessellate | RespectLatitudeCircle;
    QTest::newRow("Spherical LatitudeCircle normalRing")
            << projection << flags << normalRing << 1;

    QTest::newRow("Spherical LatitudeCircle acrossIDLRing")
            << projection << flags << acrossIDLRing << 1;

/*    QTest::newRow("Spherical LatitudeCircle aroundSPoleRing")
            << projection << flags << aroundSPoleRing << 1;*/

}

void ViewportParamsTest::geoDataLinearRing()
{
    QFETCH( Marble::Projection, projection );
    QFETCH( Marble::TessellationFlags, tessellation );
    QFETCH( GeoDataLinearRing, ring );
    QFETCH( int, size );

    ViewportParams viewport;
    viewport.setProjection( projection );
    viewport.setRadius( 360 / 4 ); // for easy mapping of lon <-> x
    viewport.centerOn(GeoDataLongitude::fromDegrees(175), GeoDataLatitude::null);

    ring.setTessellationFlags( tessellation );
    QVector<QPolygonF*> polys;
    viewport.screenCoordinates(ring, polys);

    foreach (QPolygonF* poly, polys) {
        // at least 3 points in one poly
        QVERIFY( poly->size() > 2 );
        QPointF oldCoord = poly->first();
        // polygon comes back to same point
        QVERIFY( poly->isClosed() );
        poly->pop_front();

        foreach(const QPointF &coord, *poly) {
            // no 2 same points
            QVERIFY( (coord-oldCoord) != QPointF() );

            // no 2 consecutive points should be more than 90° apart
//            QVERIFY( (coord-oldCoord).manhattanLength() < viewport.radius() );
            oldCoord = coord;
        }
    }

    // check the provided number of polys
    QCOMPARE( polys.size(), size );
}

void ViewportParamsTest::setInvalidRadius()
{
    ViewportParams viewport;

    // QVERIFY( viewport.radius() > 0 ); already verified above

    const int radius = viewport.radius();
    viewport.setRadius( 0 );

    QCOMPARE( viewport.radius(), radius );
}

void ViewportParamsTest::setFocusPoint()
{
    const GeoDataCoordinates focusPoint1( GeoDataLongitude::fromDegrees(10), GeoDataLatitude::fromDegrees(13) );
    const GeoDataCoordinates focusPoint2( GeoDataLongitude::fromDegrees(14.3), GeoDataLatitude::fromDegrees(20.5) );

    ViewportParams viewport;

    const GeoDataCoordinates center = viewport.focusPoint();

    QVERIFY( center != focusPoint1 );
    QVERIFY( center != focusPoint2 );

    viewport.setFocusPoint( focusPoint1 );
    QCOMPARE( viewport.focusPoint(), focusPoint1 );

    viewport.resetFocusPoint();
    QCOMPARE( viewport.focusPoint(), center );

    viewport.setFocusPoint( focusPoint2 );
    QCOMPARE( viewport.focusPoint(), focusPoint2 );

    viewport.setFocusPoint( focusPoint1 );
    QCOMPARE( viewport.focusPoint(), focusPoint1 );

    viewport.resetFocusPoint();
    QCOMPARE( viewport.focusPoint(), center );
}

}

QTEST_MAIN( Marble::ViewportParamsTest )

#include "ViewportParamsTest.moc"
