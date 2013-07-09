//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008-2009      Patrick Spendrin  <ps_ml@gmx.de>
// Copyright 2010           Thibaut Gridel <tgridel@free.fr>
// Copyright 2011-2012      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include "GeometryLayer.h"

// Marble
#include "MarbleDebug.h"
#include "GeoPainter.h"
#include "ViewportParams.h"
#include "GeoGraphicsScene.h"
#include "GeoGraphicsItem.h"
#include "ScreenOverlayGraphicsItem.h"
#include "TileId.h"
#include "MarbleGraphicsItem.h"

// Qt
#include <QtCore/qmath.h>
#include <QtCore/QModelIndex>

namespace Marble
{
class GeometryLayerPrivate
{
public:
    GeometryLayerPrivate( GeoGraphicsScene *scene );

    GeoGraphicsScene *const m_scene;
    QString m_runtimeTrace;
};

GeometryLayerPrivate::GeometryLayerPrivate( GeoGraphicsScene *scene )
    : m_scene( scene )
{
}

GeometryLayer::GeometryLayer( GeoGraphicsScene *scene ) :
    d( new GeometryLayerPrivate( scene ) )
{
    connect( d->m_scene, SIGNAL(repaintNeeded()), this, SIGNAL(repaintNeeded()) );
}

GeometryLayer::~GeometryLayer()
{
    delete d;
}

QStringList GeometryLayer::renderPosition() const
{
    return QStringList( "HOVERS_ABOVE_SURFACE" );
}

bool GeometryLayer::render( GeoPainter *painter, ViewportParams *viewport,
                            const QString& renderPos, GeoSceneLayer * layer )
{
    Q_UNUSED( renderPos )
    Q_UNUSED( layer )

    painter->save();

    int maxZoomLevel = qMin<int>( qLn( viewport->radius() *4 / 256 ) / qLn( 2.0 ), d->m_scene->maximumZoomLevel() );

    QList<GeoGraphicsItem*> items = d->m_scene->items( viewport->viewLatLonAltBox(), maxZoomLevel );
    int painted = 0;
    foreach( GeoGraphicsItem* item, items )
    {
        if ( item->latLonAltBox().intersects( viewport->viewLatLonAltBox() ) ) {
            item->paint( painter, viewport );
            ++painted;
        }
    }

    foreach( ScreenOverlayGraphicsItem* item, d->m_scene->screenItems() ) {
        item->paintEvent( painter, viewport );
    }

    painter->restore();
    d->m_runtimeTrace = QString( "Items: %1 Drawn: %2 Zoom: %3")
                .arg( items.size() )
                .arg( painted )
                .arg( maxZoomLevel );
    return true;
}

QString GeometryLayer::runtimeTrace() const
{
    return d->m_runtimeTrace;
}

}

#include "GeometryLayer.moc"
