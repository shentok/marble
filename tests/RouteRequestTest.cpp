//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2014      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include <QTest>

#include "routing/RouteRequest.h"
#include "GeoDataFolder.h"
#include "GeoDataPlacemark.h"
#include "GeoDataTreeModel.h"

namespace Marble
{

class RouteRequestTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void defaultConstructor();
    void reverse();
};

void RouteRequestTest::defaultConstructor()
{
    GeoDataTreeModel treeModel;
    GeoDataFolder folder;
    const RouteRequest request( &treeModel, &folder );

    QCOMPARE( request.size(), 0 );
    QCOMPARE( request.source(), GeoDataCoordinates() );
    QCOMPARE( request.destination(), GeoDataCoordinates() );
    QCOMPARE( request.routingProfile(), RoutingProfile() );
}

void RouteRequestTest::reverse()
{
    {
        GeoDataTreeModel treeModel;
        GeoDataFolder folder;
        RouteRequest request( &treeModel, &folder );
        request.reverse(); // should not crash
        QCOMPARE( request.size(), 0 );
    }

    {
        GeoDataTreeModel treeModel;
        GeoDataFolder folder;
        RouteRequest request( &treeModel, &folder );
        request.append( GeoDataCoordinates(), "A" );
        request.reverse();
        QCOMPARE( request.size(), 1 );
        QCOMPARE( request[0].name(), QString("A") );
    }

    {
        GeoDataTreeModel treeModel;
        GeoDataFolder folder;
        RouteRequest request( &treeModel, &folder );
        request.append( GeoDataCoordinates(), "A" );
        request.append( GeoDataCoordinates(), "B" );
        request.reverse();
        QCOMPARE( request.size(), 2 );
        QCOMPARE( request[0].name(), QString("B") );
        QCOMPARE( request[1].name(), QString("A") );
    }

    {
        GeoDataTreeModel treeModel;
        GeoDataFolder folder;
        RouteRequest request( &treeModel, &folder );
        request.append( GeoDataCoordinates(), "A" );
        request.append( GeoDataCoordinates(), "B" );
        request.append( GeoDataCoordinates(), "C" );
        request.reverse();
        QCOMPARE( request.size(), 3 );
        QCOMPARE( request[0].name(), QString("C") );
        QCOMPARE( request[1].name(), QString("B") );
        QCOMPARE( request[2].name(), QString("A") );
    }
}

}

QTEST_MAIN( Marble::RouteRequestTest )

#include "RouteRequestTest.moc"
