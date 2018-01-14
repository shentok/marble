//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011       Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include "GeoPainter.h"
#include "MarbleMap.h"
#include "MarbleModel.h"
#include "TestUtils.h"

#include <QThreadPool>

namespace Marble
{

class MarbleMapTest : public QObject
{
    Q_OBJECT

 private Q_SLOTS:
    void centerOnSpherical_data();
    void centerOnSpherical();

    void centerOnEquirectangular_data();
    void centerOnEquirectangular();

    void centerOnMercator_data();
    void centerOnMercator();

    void centerOnSphericalMinLat_data();
    void centerOnSphericalMinLat();

    void centerOnSphericalMaxLat_data();
    void centerOnSphericalMaxLat();

    void centerOnEquirectangularMinLat_data();
    void centerOnEquirectangularMinLat();

    void centerOnEquirectangularMaxLat_data();
    void centerOnEquirectangularMaxLat();

    void centerOnMercatorMinLat_data();
    void centerOnMercatorMinLat();

    void centerOnMercatorMaxLat_data();
    void centerOnMercatorMaxLat();

    void rotateBySpherical_data();
    void rotateBySpherical();

    void setMapTheme_data();
    void setMapTheme();

    void switchMapThemes();

    void setMapThemeIdTwoMaps_data();
    void setMapThemeIdTwoMaps();

    void switchMapThemesTwoMaps();

    void paint_data();
    void paint();

 private:
    MarbleModel m_model;
};

void MarbleMapTest::centerOnSpherical_data()
{
    QTest::addColumn<GeoDataLongitude>("lon");
    QTest::addColumn<GeoDataLatitude>("lat");

    addRow() << GeoDataLongitude::fromDegrees(0.0) << GeoDataLatitude::fromDegrees(0.0);

    addRow() << GeoDataLongitude::fromDegrees(-180.0) << GeoDataLatitude::fromDegrees(0.0);
    addRow() <<  GeoDataLongitude::fromDegrees(-90.0) << GeoDataLatitude::fromDegrees(0.0);
    addRow() <<   GeoDataLongitude::fromDegrees(90.0) << GeoDataLatitude::fromDegrees(0.0);
    addRow() <<  GeoDataLongitude::fromDegrees(180.0) << GeoDataLatitude::fromDegrees(0.0);

    addRow() << GeoDataLongitude::fromDegrees(-180.0) << GeoDataLatitude::fromDegrees(90.0);
    addRow() <<  GeoDataLongitude::fromDegrees(-90.0) << GeoDataLatitude::fromDegrees(90.0);
    addRow() <<    GeoDataLongitude::fromDegrees(0.0) << GeoDataLatitude::fromDegrees(90.0);
    addRow() <<   GeoDataLongitude::fromDegrees(90.0) << GeoDataLatitude::fromDegrees(90.0);
    addRow() <<  GeoDataLongitude::fromDegrees(180.0) << GeoDataLatitude::fromDegrees(90.0);

    addRow() << GeoDataLongitude::fromDegrees(-180.0) << GeoDataLatitude::fromDegrees(-90.0);
    addRow() <<  GeoDataLongitude::fromDegrees(-90.0) << GeoDataLatitude::fromDegrees(-90.0);
    addRow() <<    GeoDataLongitude::fromDegrees(0.0) << GeoDataLatitude::fromDegrees(-90.0);
    addRow() <<   GeoDataLongitude::fromDegrees(90.0) << GeoDataLatitude::fromDegrees(-90.0);
    addRow() <<  GeoDataLongitude::fromDegrees(180.0) << GeoDataLatitude::fromDegrees(-90.0);

    addRow() << GeoDataLongitude::fromDegrees(-180.0) << GeoDataLatitude::fromDegrees(180.0);
    addRow() <<  GeoDataLongitude::fromDegrees(-90.0) << GeoDataLatitude::fromDegrees(180.0);
    addRow() <<    GeoDataLongitude::fromDegrees(0.0) << GeoDataLatitude::fromDegrees(180.0);
    addRow() <<   GeoDataLongitude::fromDegrees(90.0) << GeoDataLatitude::fromDegrees(180.0);
    addRow() <<  GeoDataLongitude::fromDegrees(180.0) << GeoDataLatitude::fromDegrees(180.0);

    addRow() << GeoDataLongitude::fromDegrees(-180.0) << GeoDataLatitude::fromDegrees(-180.0);
    addRow() <<  GeoDataLongitude::fromDegrees(-90.0) << GeoDataLatitude::fromDegrees(-180.0);
    addRow() <<    GeoDataLongitude::fromDegrees(0.0) << GeoDataLatitude::fromDegrees(-180.0);
    addRow() <<   GeoDataLongitude::fromDegrees(90.0) << GeoDataLatitude::fromDegrees(-180.0);
    addRow() <<  GeoDataLongitude::fromDegrees(180.0) << GeoDataLatitude::fromDegrees(-180.0);
}

void MarbleMapTest::centerOnSpherical()
{
    QFETCH(GeoDataLongitude, lon);
    QFETCH(GeoDataLatitude, lat);

    MarbleMap map( &m_model );
    map.setProjection( Spherical );

    map.centerOn( lon, lat );
    QFUZZYCOMPARE(map.centerLongitude(), lon, GeoDataLongitude::fromDegrees(0.0001));
    QFUZZYCOMPARE(map.centerLatitude(), lat, GeoDataLatitude::fromDegrees(0.0001));
}

void MarbleMapTest::centerOnEquirectangular_data()
{
    QTest::addColumn<GeoDataLongitude>("lon");
    QTest::addColumn<GeoDataLatitude>("lat");

    addRow() << GeoDataLongitude::fromDegrees(0.0) << GeoDataLatitude::fromDegrees(0.0);

    addRow() << GeoDataLongitude::fromDegrees(-180.0) << GeoDataLatitude::fromDegrees(0.0);
    addRow() <<  GeoDataLongitude::fromDegrees(-90.0) << GeoDataLatitude::fromDegrees(0.0);
    addRow() <<   GeoDataLongitude::fromDegrees(90.0) << GeoDataLatitude::fromDegrees(0.0);
    addRow() <<  GeoDataLongitude::fromDegrees(180.0) << GeoDataLatitude::fromDegrees(0.0);

    addRow() << GeoDataLongitude::fromDegrees(-180.0) << GeoDataLatitude::fromDegrees(90.0);
    addRow() <<  GeoDataLongitude::fromDegrees(-90.0) << GeoDataLatitude::fromDegrees(90.0);
    addRow() <<    GeoDataLongitude::fromDegrees(0.0) << GeoDataLatitude::fromDegrees(90.0);
    addRow() <<   GeoDataLongitude::fromDegrees(90.0) << GeoDataLatitude::fromDegrees(90.0);
    addRow() <<  GeoDataLongitude::fromDegrees(180.0) << GeoDataLatitude::fromDegrees(90.0);

    addRow() << GeoDataLongitude::fromDegrees(-180.0) << GeoDataLatitude::fromDegrees(-90.0);
    addRow() <<  GeoDataLongitude::fromDegrees(-90.0) << GeoDataLatitude::fromDegrees(-90.0);
    addRow() <<    GeoDataLongitude::fromDegrees(0.0) << GeoDataLatitude::fromDegrees(-90.0);
    addRow() <<   GeoDataLongitude::fromDegrees(90.0) << GeoDataLatitude::fromDegrees(-90.0);
    addRow() <<  GeoDataLongitude::fromDegrees(180.0) << GeoDataLatitude::fromDegrees(-90.0);
}

void MarbleMapTest::centerOnEquirectangular()
{
    QFETCH(GeoDataLongitude, lon);
    QFETCH(GeoDataLatitude, lat);

    MarbleMap map( &m_model );
    map.setProjection( Equirectangular );

    map.centerOn( lon, lat );
    QFUZZYCOMPARE(map.centerLongitude(), lon, GeoDataLongitude::fromDegrees(0.0001));
    QFUZZYCOMPARE(map.centerLatitude(), lat, GeoDataLatitude::fromDegrees(0.0001));
}

void MarbleMapTest::centerOnMercator_data()
{
    QTest::addColumn<GeoDataLongitude>("lon");
    QTest::addColumn<GeoDataLatitude>("lat");

    addRow() << GeoDataLongitude::fromDegrees(0.0) << GeoDataLatitude::fromDegrees(0.0);

    addRow() << GeoDataLongitude::fromDegrees(-180.0) << GeoDataLatitude::fromDegrees(0.0);
    addRow() <<  GeoDataLongitude::fromDegrees(-90.0) << GeoDataLatitude::fromDegrees(0.0);
    addRow() <<   GeoDataLongitude::fromDegrees(90.0) << GeoDataLatitude::fromDegrees(0.0);
    addRow() <<  GeoDataLongitude::fromDegrees(180.0) << GeoDataLatitude::fromDegrees(0.0);

    addRow() << GeoDataLongitude::fromDegrees(-180.0) << GeoDataLatitude::fromDegrees(-85.0511);
    addRow() <<  GeoDataLongitude::fromDegrees(-90.0) << GeoDataLatitude::fromDegrees(-85.0511);
    addRow() <<    GeoDataLongitude::fromDegrees(0.0) << GeoDataLatitude::fromDegrees(-85.0511);
    addRow() <<   GeoDataLongitude::fromDegrees(90.0) << GeoDataLatitude::fromDegrees(-85.0511);
    addRow() <<  GeoDataLongitude::fromDegrees(180.0) << GeoDataLatitude::fromDegrees(-85.0511);

    addRow() << GeoDataLongitude::fromDegrees(-180.0) << GeoDataLatitude::fromDegrees(85.0511);
    addRow() <<  GeoDataLongitude::fromDegrees(-90.0) << GeoDataLatitude::fromDegrees(85.0511);
    addRow() <<    GeoDataLongitude::fromDegrees(0.0) << GeoDataLatitude::fromDegrees(85.0511);
    addRow() <<   GeoDataLongitude::fromDegrees(90.0) << GeoDataLatitude::fromDegrees(85.0511);
    addRow() <<  GeoDataLongitude::fromDegrees(180.0) << GeoDataLatitude::fromDegrees(85.0511);
}

void MarbleMapTest::centerOnMercator()
{
    MarbleMap map( &m_model );

    map.setProjection( Mercator );

    QFETCH(GeoDataLongitude, lon);
    QFETCH(GeoDataLatitude, lat);

    map.centerOn( lon, lat );

    QFUZZYCOMPARE(map.centerLongitude(), lon, GeoDataLongitude::fromDegrees(0.0001));
    QFUZZYCOMPARE(map.centerLatitude(), lat, GeoDataLatitude::fromDegrees(0.0001));
}

void MarbleMapTest::centerOnSphericalMinLat_data()
{
    QTest::addColumn<GeoDataLongitude>("lon");
    QTest::addColumn<GeoDataLatitude>("lat");

    addRow() << GeoDataLongitude::fromDegrees(-180.0) << GeoDataLatitude::fromDegrees(-180.001);
    addRow() <<  GeoDataLongitude::fromDegrees(-90.0) << GeoDataLatitude::fromDegrees(-180.001);
    addRow() <<    GeoDataLongitude::fromDegrees(0.0) << GeoDataLatitude::fromDegrees(-180.001);
    addRow() <<   GeoDataLongitude::fromDegrees(90.0) << GeoDataLatitude::fromDegrees(-180.001);
    addRow() <<  GeoDataLongitude::fromDegrees(180.0) << GeoDataLatitude::fromDegrees(-180.001);
}

void MarbleMapTest::centerOnSphericalMinLat()
{
    MarbleMap map( &m_model );

    map.setProjection( Spherical );

    QFETCH(GeoDataLongitude, lon);
    QFETCH(GeoDataLatitude, lat);

    map.centerOn( lon, lat );

    QFUZZYCOMPARE(map.centerLongitude(), lon, GeoDataLongitude::fromDegrees(0.0001));
    QFUZZYCOMPARE(map.centerLatitude(), GeoDataLatitude::fromDegrees(179.999), GeoDataLatitude::fromDegrees(0.0001));
}

void MarbleMapTest::centerOnSphericalMaxLat_data()
{
    QTest::addColumn<GeoDataLongitude>("lon");
    QTest::addColumn<GeoDataLatitude>("lat");

    addRow() << GeoDataLongitude::fromDegrees(-180.0) << GeoDataLatitude::fromDegrees(180.001);
    addRow() <<  GeoDataLongitude::fromDegrees(-90.0) << GeoDataLatitude::fromDegrees(180.001);
    addRow() <<    GeoDataLongitude::fromDegrees(0.0) << GeoDataLatitude::fromDegrees(180.001);
    addRow() <<   GeoDataLongitude::fromDegrees(90.0) << GeoDataLatitude::fromDegrees(180.001);
    addRow() <<  GeoDataLongitude::fromDegrees(180.0) << GeoDataLatitude::fromDegrees(180.001);
}

void MarbleMapTest::centerOnSphericalMaxLat()
{
    MarbleMap map( &m_model );

    map.setProjection( Spherical );

    QFETCH(GeoDataLongitude, lon);
    QFETCH(GeoDataLatitude, lat);

    map.centerOn( lon, lat );

    QFUZZYCOMPARE(map.centerLongitude(), lon, GeoDataLongitude::fromDegrees(0.0001));
    QFUZZYCOMPARE(map.centerLatitude(), -GeoDataLatitude::fromDegrees(179.999), GeoDataLatitude::fromDegrees(0.0001));
}

void MarbleMapTest::centerOnEquirectangularMinLat_data()
{
    QTest::addColumn<GeoDataLongitude>("lon");
    QTest::addColumn<GeoDataLatitude>("lat");

    addRow() << GeoDataLongitude::fromDegrees(-180.0) << GeoDataLatitude::fromDegrees(-90.001);
    addRow() <<  GeoDataLongitude::fromDegrees(-90.0) << GeoDataLatitude::fromDegrees(-90.001);
    addRow() <<    GeoDataLongitude::fromDegrees(0.0) << GeoDataLatitude::fromDegrees(-90.001);
    addRow() <<   GeoDataLongitude::fromDegrees(90.0) << GeoDataLatitude::fromDegrees(-90.001);
    addRow() <<  GeoDataLongitude::fromDegrees(180.0) << GeoDataLatitude::fromDegrees(-90.001);
}

void MarbleMapTest::centerOnEquirectangularMinLat()
{
    MarbleMap map( &m_model );

    map.setProjection( Equirectangular );

    QFETCH(GeoDataLongitude, lon);
    QFETCH(GeoDataLatitude, lat);

    map.centerOn( lon, lat );

    QFUZZYCOMPARE(map.centerLongitude(), lon, GeoDataLongitude::fromDegrees(0.0001));
    QFUZZYCOMPARE(map.centerLatitude(), -GeoDataLatitude::fromDegrees(90.0), GeoDataLatitude::fromDegrees(0.0001));
}

void MarbleMapTest::centerOnEquirectangularMaxLat_data()
{
    QTest::addColumn<GeoDataLongitude>("lon");
    QTest::addColumn<GeoDataLatitude>("lat");

    addRow() << GeoDataLongitude::fromDegrees(-180.0) << GeoDataLatitude::fromDegrees(90.001);
    addRow() <<  GeoDataLongitude::fromDegrees(-90.0) << GeoDataLatitude::fromDegrees(90.001);
    addRow() <<    GeoDataLongitude::fromDegrees(0.0) << GeoDataLatitude::fromDegrees(90.001);
    addRow() <<   GeoDataLongitude::fromDegrees(90.0) << GeoDataLatitude::fromDegrees(90.001);
    addRow() <<  GeoDataLongitude::fromDegrees(180.0) << GeoDataLatitude::fromDegrees(90.001);
}

void MarbleMapTest::centerOnEquirectangularMaxLat()
{
    MarbleMap map( &m_model );

    map.setProjection( Equirectangular );

    QFETCH(GeoDataLongitude, lon);
    QFETCH(GeoDataLatitude, lat);

    map.centerOn( lon, lat );

    QFUZZYCOMPARE(map.centerLongitude(), lon, GeoDataLongitude::fromDegrees(0.0001));
    QFUZZYCOMPARE(map.centerLatitude(), GeoDataLatitude::fromDegrees(90.0), GeoDataLatitude::fromDegrees(0.0001));
}

void MarbleMapTest::centerOnMercatorMinLat_data()
{
    QTest::addColumn<GeoDataLongitude>("lon");
    QTest::addColumn<GeoDataLatitude>("lat");

    addRow() << GeoDataLongitude::fromDegrees(-180.0) << GeoDataLatitude::fromDegrees(-87.0);
    addRow() <<  GeoDataLongitude::fromDegrees(-90.0) << GeoDataLatitude::fromDegrees(-87.0);
    addRow() <<    GeoDataLongitude::fromDegrees(0.0) << GeoDataLatitude::fromDegrees(-87.0);
    addRow() <<   GeoDataLongitude::fromDegrees(90.0) << GeoDataLatitude::fromDegrees(-87.0);
    addRow() <<  GeoDataLongitude::fromDegrees(180.0) << GeoDataLatitude::fromDegrees(-87.0);

    addRow() << GeoDataLongitude::fromDegrees(-180.0) << GeoDataLatitude::fromDegrees(-90.0);
    addRow() <<  GeoDataLongitude::fromDegrees(-90.0) << GeoDataLatitude::fromDegrees(-90.0);
    addRow() <<    GeoDataLongitude::fromDegrees(0.0) << GeoDataLatitude::fromDegrees(-90.0);
    addRow() <<   GeoDataLongitude::fromDegrees(90.0) << GeoDataLatitude::fromDegrees(-90.0);
    addRow() <<  GeoDataLongitude::fromDegrees(180.0) << GeoDataLatitude::fromDegrees(-90.0);
}

void MarbleMapTest::centerOnMercatorMinLat()
{
    MarbleMap map( &m_model );

    map.setProjection( Mercator );

    QFETCH(GeoDataLongitude, lon);
    QFETCH(GeoDataLatitude, lat);

    map.centerOn( lon, lat );

    QFUZZYCOMPARE(map.centerLongitude(), lon, GeoDataLongitude::fromDegrees(0.0001));
    QFUZZYCOMPARE(map.centerLatitude(), -GeoDataLatitude::fromDegrees(85.0511), GeoDataLatitude::fromDegrees(0.0001)); // clip to minLat
}

void MarbleMapTest::centerOnMercatorMaxLat_data()
{
    QTest::addColumn<GeoDataLongitude>("lon");
    QTest::addColumn<GeoDataLatitude>("lat");

    addRow() << GeoDataLongitude::fromDegrees(-180.0) << GeoDataLatitude::fromDegrees(87.0);
    addRow() <<  GeoDataLongitude::fromDegrees(-90.0) << GeoDataLatitude::fromDegrees(87.0);
    addRow() <<    GeoDataLongitude::fromDegrees(0.0) << GeoDataLatitude::fromDegrees(87.0);
    addRow() <<   GeoDataLongitude::fromDegrees(90.0) << GeoDataLatitude::fromDegrees(87.0);
    addRow() <<  GeoDataLongitude::fromDegrees(180.0) << GeoDataLatitude::fromDegrees(87.0);

    addRow() << GeoDataLongitude::fromDegrees(-180.0) << GeoDataLatitude::fromDegrees(90.0);
    addRow() <<  GeoDataLongitude::fromDegrees(-90.0) << GeoDataLatitude::fromDegrees(90.0);
    addRow() <<    GeoDataLongitude::fromDegrees(0.0) << GeoDataLatitude::fromDegrees(90.0);
    addRow() <<   GeoDataLongitude::fromDegrees(90.0) << GeoDataLatitude::fromDegrees(90.0);
    addRow() <<  GeoDataLongitude::fromDegrees(180.0) << GeoDataLatitude::fromDegrees(90.0);
}

void MarbleMapTest::centerOnMercatorMaxLat()
{
    MarbleMap map( &m_model );

    map.setProjection( Mercator );

    QFETCH(GeoDataLongitude, lon);
    QFETCH(GeoDataLatitude, lat);

    map.centerOn( lon, lat );

    QFUZZYCOMPARE(map.centerLongitude(), lon, GeoDataLongitude::fromDegrees(0.0001));
    QFUZZYCOMPARE(map.centerLatitude(), GeoDataLatitude::fromDegrees(85.0511), GeoDataLatitude::fromDegrees(0.0001)); // clip to maxLat
}

void MarbleMapTest::rotateBySpherical_data()
{
    QTest::addColumn<GeoDataLongitude>("lon");
    QTest::addColumn<GeoDataLatitude>("lat");
    QTest::addColumn<GeoDataLongitude>("deltaLon");
    QTest::addColumn<GeoDataLatitude>("deltaLat");

    addRow() << GeoDataLongitude::fromDegrees(0) << GeoDataLatitude::fromDegrees(0) << GeoDataLongitude::fromDegrees(0) << GeoDataLatitude::fromDegrees(0);

    addRow() << GeoDataLongitude::fromDegrees(0) << GeoDataLatitude::fromDegrees(0) << GeoDataLongitude::fromDegrees(-180) << GeoDataLatitude::fromDegrees(0);
    addRow() << GeoDataLongitude::fromDegrees(0) << GeoDataLatitude::fromDegrees(0) <<  GeoDataLongitude::fromDegrees(-90) << GeoDataLatitude::fromDegrees(0);
    addRow() << GeoDataLongitude::fromDegrees(0) << GeoDataLatitude::fromDegrees(0) <<   GeoDataLongitude::fromDegrees(-5) << GeoDataLatitude::fromDegrees(0);
    addRow() << GeoDataLongitude::fromDegrees(0) << GeoDataLatitude::fromDegrees(0) <<    GeoDataLongitude::fromDegrees(5) << GeoDataLatitude::fromDegrees(0);
    addRow() << GeoDataLongitude::fromDegrees(0) << GeoDataLatitude::fromDegrees(0) <<   GeoDataLongitude::fromDegrees(90) << GeoDataLatitude::fromDegrees(0);
    addRow() << GeoDataLongitude::fromDegrees(0) << GeoDataLatitude::fromDegrees(0) <<  GeoDataLongitude::fromDegrees(180) << GeoDataLatitude::fromDegrees(0);

    addRow() << GeoDataLongitude::fromDegrees(0) << GeoDataLatitude::fromDegrees(0) << GeoDataLongitude::fromDegrees(0) << GeoDataLatitude::fromDegrees(-180);
    addRow() << GeoDataLongitude::fromDegrees(0) << GeoDataLatitude::fromDegrees(0) << GeoDataLongitude::fromDegrees(0) <<  GeoDataLatitude::fromDegrees(-90);
    addRow() << GeoDataLongitude::fromDegrees(0) << GeoDataLatitude::fromDegrees(0) << GeoDataLongitude::fromDegrees(0) <<   GeoDataLatitude::fromDegrees(-5);
    addRow() << GeoDataLongitude::fromDegrees(0) << GeoDataLatitude::fromDegrees(0) << GeoDataLongitude::fromDegrees(0) <<    GeoDataLatitude::fromDegrees(5);
    addRow() << GeoDataLongitude::fromDegrees(0) << GeoDataLatitude::fromDegrees(0) << GeoDataLongitude::fromDegrees(0) <<   GeoDataLatitude::fromDegrees(90);
    addRow() << GeoDataLongitude::fromDegrees(0) << GeoDataLatitude::fromDegrees(0) << GeoDataLongitude::fromDegrees(0) <<  GeoDataLatitude::fromDegrees(180);

    addRow() << GeoDataLongitude::fromDegrees(0) << GeoDataLatitude::fromDegrees(0) << GeoDataLongitude::fromDegrees(-180) << GeoDataLatitude::fromDegrees(-180);
    addRow() << GeoDataLongitude::fromDegrees(0) << GeoDataLatitude::fromDegrees(0) <<  GeoDataLongitude::fromDegrees(-90) <<  GeoDataLatitude::fromDegrees(-90);
    addRow() << GeoDataLongitude::fromDegrees(0) << GeoDataLatitude::fromDegrees(0) <<   GeoDataLongitude::fromDegrees(-5) <<   GeoDataLatitude::fromDegrees(-5);
    addRow() << GeoDataLongitude::fromDegrees(0) << GeoDataLatitude::fromDegrees(0) <<    GeoDataLongitude::fromDegrees(5) <<    GeoDataLatitude::fromDegrees(5);
    addRow() << GeoDataLongitude::fromDegrees(0) << GeoDataLatitude::fromDegrees(0) <<   GeoDataLongitude::fromDegrees(90) <<   GeoDataLatitude::fromDegrees(90);
    addRow() << GeoDataLongitude::fromDegrees(0) << GeoDataLatitude::fromDegrees(0) <<  GeoDataLongitude::fromDegrees(180) <<  GeoDataLatitude::fromDegrees(180);

    addRow() << GeoDataLongitude::fromDegrees(0) <<  GeoDataLatitude::fromDegrees(160) << GeoDataLongitude::fromDegrees(0) << GeoDataLatitude::fromDegrees(-20);
    addRow() << GeoDataLongitude::fromDegrees(0) <<  GeoDataLatitude::fromDegrees(160) << GeoDataLongitude::fromDegrees(0) <<  GeoDataLatitude::fromDegrees(20);
    addRow() << GeoDataLongitude::fromDegrees(0) <<   GeoDataLatitude::fromDegrees(80) << GeoDataLongitude::fromDegrees(0) << GeoDataLatitude::fromDegrees(-20);
    addRow() << GeoDataLongitude::fromDegrees(0) <<   GeoDataLatitude::fromDegrees(80) << GeoDataLongitude::fromDegrees(0) <<  GeoDataLatitude::fromDegrees(20);
    addRow() << GeoDataLongitude::fromDegrees(0) <<  GeoDataLatitude::fromDegrees(-80) << GeoDataLongitude::fromDegrees(0) << GeoDataLatitude::fromDegrees(-20);
    addRow() << GeoDataLongitude::fromDegrees(0) <<  GeoDataLatitude::fromDegrees(-80) << GeoDataLongitude::fromDegrees(0) <<  GeoDataLatitude::fromDegrees(20);
    addRow() << GeoDataLongitude::fromDegrees(0) << GeoDataLatitude::fromDegrees(-160) << GeoDataLongitude::fromDegrees(0) << GeoDataLatitude::fromDegrees(-20);
    addRow() << GeoDataLongitude::fromDegrees(0) << GeoDataLatitude::fromDegrees(-160) << GeoDataLongitude::fromDegrees(0) <<  GeoDataLatitude::fromDegrees(20);

    addRow() <<  GeoDataLongitude::fromDegrees(150) << GeoDataLatitude::fromDegrees(0) << GeoDataLongitude::fromDegrees(-30) << GeoDataLatitude::fromDegrees(0);
    addRow() <<  GeoDataLongitude::fromDegrees(150) << GeoDataLatitude::fromDegrees(0) <<  GeoDataLongitude::fromDegrees(30) << GeoDataLatitude::fromDegrees(0);
    addRow() <<   GeoDataLongitude::fromDegrees(50) << GeoDataLatitude::fromDegrees(0) << GeoDataLongitude::fromDegrees(-30) << GeoDataLatitude::fromDegrees(0);
    addRow() <<   GeoDataLongitude::fromDegrees(50) << GeoDataLatitude::fromDegrees(0) <<  GeoDataLongitude::fromDegrees(30) << GeoDataLatitude::fromDegrees(0);
    addRow() <<  GeoDataLongitude::fromDegrees(-50) << GeoDataLatitude::fromDegrees(0) << GeoDataLongitude::fromDegrees(-30) << GeoDataLatitude::fromDegrees(0);
    addRow() <<  GeoDataLongitude::fromDegrees(-50) << GeoDataLatitude::fromDegrees(0) <<  GeoDataLongitude::fromDegrees(30) << GeoDataLatitude::fromDegrees(0);
    addRow() << GeoDataLongitude::fromDegrees(-150) << GeoDataLatitude::fromDegrees(0) << GeoDataLongitude::fromDegrees(-30) << GeoDataLatitude::fromDegrees(0);
    addRow() << GeoDataLongitude::fromDegrees(-150) << GeoDataLatitude::fromDegrees(0) <<  GeoDataLongitude::fromDegrees(30) << GeoDataLatitude::fromDegrees(0);

    addRow() <<  GeoDataLongitude::fromDegrees(150) <<  GeoDataLatitude::fromDegrees(160) << GeoDataLongitude::fromDegrees(-30) << GeoDataLatitude::fromDegrees(-20);
    addRow() <<  GeoDataLongitude::fromDegrees(150) <<  GeoDataLatitude::fromDegrees(160) <<  GeoDataLongitude::fromDegrees(30) <<  GeoDataLatitude::fromDegrees(20);
    addRow() <<   GeoDataLongitude::fromDegrees(50) <<   GeoDataLatitude::fromDegrees(80) << GeoDataLongitude::fromDegrees(-30) << GeoDataLatitude::fromDegrees(-20);
    addRow() <<   GeoDataLongitude::fromDegrees(50) <<   GeoDataLatitude::fromDegrees(80) <<  GeoDataLongitude::fromDegrees(30) <<  GeoDataLatitude::fromDegrees(20);
    addRow() <<  GeoDataLongitude::fromDegrees(-50) <<  GeoDataLatitude::fromDegrees(-80) << GeoDataLongitude::fromDegrees(-30) << GeoDataLatitude::fromDegrees(-20);
    addRow() <<  GeoDataLongitude::fromDegrees(-50) <<  GeoDataLatitude::fromDegrees(-80) <<  GeoDataLongitude::fromDegrees(30) <<  GeoDataLatitude::fromDegrees(20);
    addRow() << GeoDataLongitude::fromDegrees(-150) << GeoDataLatitude::fromDegrees(-160) << GeoDataLongitude::fromDegrees(-30) << GeoDataLatitude::fromDegrees(-20);
    addRow() << GeoDataLongitude::fromDegrees(-150) << GeoDataLatitude::fromDegrees(-160) <<  GeoDataLongitude::fromDegrees(30) <<  GeoDataLatitude::fromDegrees(20);
}

void MarbleMapTest::rotateBySpherical()
{
    MarbleMap map( &m_model );

    map.setProjection( Spherical );

    QFETCH(GeoDataLongitude, lon);
    QFETCH(GeoDataLatitude, lat);

    map.centerOn( lon, lat );

    QFUZZYCOMPARE(map.centerLongitude(), lon, GeoDataLongitude::fromDegrees(0.0001));
    QFUZZYCOMPARE(map.centerLatitude(), lat, GeoDataLatitude::fromDegrees(0.0001));

    QFETCH(GeoDataLongitude, deltaLon);
    QFETCH(GeoDataLatitude, deltaLat);

    map.rotateBy( deltaLon, deltaLat );

    const GeoDataLongitude expectedLon = lon + deltaLon;
    const GeoDataLatitude expectedLat = lat + deltaLat;

    QFUZZYCOMPARE(map.centerLongitude(), expectedLon, GeoDataLongitude::fromDegrees(0.0001));
    QFUZZYCOMPARE(map.centerLatitude(), expectedLat, GeoDataLatitude::fromDegrees(0.0001));
}

void MarbleMapTest::setMapTheme_data()
{
    QTest::addColumn<QString>( "mapThemeId" );

    addRow() << "earth/plain/plain.dgml";
    addRow() << "earth/srtm/srtm.dgml";
    addRow() << "earth/openstreetmap/openstreetmap.dgml";
}

void MarbleMapTest::setMapTheme()
{
    QFETCH( QString, mapThemeId );

    MarbleMap map;

    map.setMapThemeId( mapThemeId );

    QCOMPARE( map.mapThemeId(), mapThemeId );

    QThreadPool::globalInstance()->waitForDone();  // wait for all runners to terminate
}

void MarbleMapTest::switchMapThemes()
{
    QImage image( QSize( 143, 342 ), QImage::Format_ARGB32_Premultiplied );

    MarbleMap map;
    map.setSize( image.size() );
    map.setRadius( 114003 );
    map.setViewContext( Animation );

    GeoPainter painter( &image, map.viewport() );

    map.setMapThemeId( "earth/plain/plain.dgml" );
    QCOMPARE( map.mapThemeId(), QString( "earth/plain/plain.dgml" ) );
    QCOMPARE( map.preferredRadiusCeil( 1000 ), 1000 );
    QCOMPARE( map.preferredRadiusFloor( 1000 ), 1000 );

    map.setMapThemeId( "earth/srtm/srtm.dgml" );
    QCOMPARE( map.mapThemeId(), QString( "earth/srtm/srtm.dgml" ) );
    QCOMPARE( map.preferredRadiusCeil( 1000 ), 1348 );
    QCOMPARE( map.preferredRadiusFloor( 1000 ), 674 );

    map.setMapThemeId( "earth/openstreetmap/openstreetmap.dgml" );
    QCOMPARE( map.mapThemeId(), QString( "earth/openstreetmap/openstreetmap.dgml" ) );
    QCOMPARE( map.preferredRadiusCeil( 1000 ), 1024 );
    QCOMPARE( map.preferredRadiusFloor( 1000 ), 512 );
    map.paint( painter, QRect() ); // loads tiles

    map.setMapThemeId( "earth/plain/plain.dgml" );
    QCOMPARE( map.mapThemeId(), QString( "earth/plain/plain.dgml" ) );
    QCOMPARE( map.preferredRadiusCeil( 1000 ), 1000 );
    QCOMPARE( map.preferredRadiusFloor( 1000 ), 1000 );
    map.reload(); // don't crash, please

    QThreadPool::globalInstance()->waitForDone();  // wait for all runners to terminate
}

void MarbleMapTest::setMapThemeIdTwoMaps_data()
{
    QTest::addColumn<QString>( "mapThemeId" );

    addRow() << "earth/plain/plain.dgml";
    addRow() << "earth/srtm/srtm.dgml";
    addRow() << "earth/openstreetmap/openstreetmap.dgml";
}

void MarbleMapTest::setMapThemeIdTwoMaps()
{
    QFETCH( QString, mapThemeId );

    MarbleModel model;
    MarbleMap map1( &model );
    MarbleMap map2( &model );

    map1.setMapThemeId( mapThemeId );

    QCOMPARE( map1.mapThemeId(), mapThemeId );
    QCOMPARE( map2.mapThemeId(), mapThemeId );

    QThreadPool::globalInstance()->waitForDone();  // wait for all runners to terminate
}

void MarbleMapTest::switchMapThemesTwoMaps()
{
    MarbleModel model;
    MarbleMap map1( &model );
    MarbleMap map2( &model );

    map1.setMapThemeId( "earth/plain/plain.dgml" );
    QCOMPARE( map1.mapThemeId(), QString( "earth/plain/plain.dgml" ) );
    QCOMPARE( map2.mapThemeId(), QString( "earth/plain/plain.dgml" ) );

    map2.setMapThemeId( "earth/srtm/srtm.dgml" );
    QCOMPARE( map1.mapThemeId(), QString( "earth/srtm/srtm.dgml" ) );
    QCOMPARE( map2.mapThemeId(), QString( "earth/srtm/srtm.dgml" ) );

    map1.setMapThemeId( "earth/openstreetmap/openstreetmap.dgml" );
    QCOMPARE( map1.mapThemeId(), QString( "earth/openstreetmap/openstreetmap.dgml" ) );
    QCOMPARE( map2.mapThemeId(), QString( "earth/openstreetmap/openstreetmap.dgml" ) );

    map2.setMapThemeId( "earth/plain/plain.dgml" );
    QCOMPARE( map1.mapThemeId(), QString( "earth/plain/plain.dgml" ) );
    QCOMPARE( map2.mapThemeId(), QString( "earth/plain/plain.dgml" ) );

    QThreadPool::globalInstance()->waitForDone();  // wait for all runners to terminate
}

void MarbleMapTest::paint_data()
{
    QTest::addColumn<QString>( "mapThemeId" );

    addRow() << "earth/plain/plain.dgml";
    addRow() << "earth/srtm/srtm.dgml";
    addRow() << "earth/openstreetmap/openstreetmap.dgml";
}

void MarbleMapTest::paint()
{
    QFETCH( QString, mapThemeId );

    MarbleMap map;

    map.setMapThemeId( mapThemeId );
    map.setSize( 200, 200 );

    QCOMPARE( map.mapThemeId(), mapThemeId );

    QPixmap paintDevice( map.size() );

    QVERIFY( map.projection() == Spherical );

    GeoPainter painter1( &paintDevice, map.viewport(), map.mapQuality() );
    map.paint( painter1, QRect() );

    QThreadPool::globalInstance()->waitForDone();  // wait for all runners to terminate
}

}

QTEST_MAIN( Marble::MarbleMapTest )

#include "MarbleMapTest.moc"
