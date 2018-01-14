//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013      Dennis Nienhüser <nienhueser@kde.org>
//

#include <QObject>

#include "TestUtils.h"
#include <GeoDataDocument.h>
#include <MarbleDebug.h>
#include <GeoDataGroundOverlay.h>
#include <GeoDataLatLonQuad.h>

using namespace Marble;


class TestLatLonQuad : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void simpleParseTest();
};

void TestLatLonQuad::initTestCase()
{
    MarbleDebug::setEnabled( true );
}

void TestLatLonQuad::simpleParseTest()
{
    QString const centerContent (
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    "<kml xmlns=\"http://www.opengis.net/kml/2.2\" xmlns:gx=\"http://www.google.com/kml/ext/2.2\">"
        "<Document>"
         "<GroundOverlay id=\"overlayID\">"
          "<gx:LatLonQuad>"
            "<coordinates>1,2 3,4 5,6 7,8</coordinates>"
          "</gx:LatLonQuad>"
         "</GroundOverlay>"
        "</Document>"
    "</kml>");

    GeoDataDocument* dataDocument = parseKml( centerContent  );
    QCOMPARE( dataDocument->size(), 1 );
    GeoDataGroundOverlay *overlay = dynamic_cast<GeoDataGroundOverlay*>( dataDocument->child( 0 ) );
    QVERIFY( overlay != nullptr );

    QVERIFY( overlay->latLonBox().isEmpty() );
    QVERIFY( overlay->latLonQuad().isValid() );
    QFUZZYCOMPARE(overlay->latLonQuad().bottomLeft().longitude(), GeoDataLongitude::fromDegrees(1.0), GeoDataLongitude::fromDegrees(0.0001));
    QFUZZYCOMPARE(overlay->latLonQuad().bottomLeft().latitude(), GeoDataLatitude::fromDegrees(2.0), GeoDataLatitude::fromDegrees(0.0001));
    QFUZZYCOMPARE(overlay->latLonQuad().bottomRight().longitude(), GeoDataLongitude::fromDegrees(3.0), GeoDataLongitude::fromDegrees(0.0001));
    QFUZZYCOMPARE(overlay->latLonQuad().bottomRight().latitude(), GeoDataLatitude::fromDegrees(4.0), GeoDataLatitude::fromDegrees(0.0001));
    QFUZZYCOMPARE(overlay->latLonQuad().topRight().longitude(), GeoDataLongitude::fromDegrees(5.0), GeoDataLongitude::fromDegrees(0.0001));
    QFUZZYCOMPARE(overlay->latLonQuad().topRight().latitude(), GeoDataLatitude::fromDegrees(6.0), GeoDataLatitude::fromDegrees(0.0001));
    QFUZZYCOMPARE(overlay->latLonQuad().topLeft().longitude(), GeoDataLongitude::fromDegrees(7.0), GeoDataLongitude::fromDegrees(0.0001));
    QFUZZYCOMPARE(overlay->latLonQuad().topLeft().latitude(), GeoDataLatitude::fromDegrees(8.0), GeoDataLatitude::fromDegrees(0.0001));
}

QTEST_MAIN( TestLatLonQuad )

#include "TestLatLonQuad.moc"

