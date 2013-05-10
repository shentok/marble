//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010           Thibaut Gridel <tgridel@free.fr>
// Copyright 2011-2013      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include "ScreenLayer.h"

// Marble
#include "GeoGraphicsScene.h"
#include "GeoPainter.h"
#include "ViewportParams.h"
#include "ScreenOverlayGraphicsItem.h"

namespace Marble
{
class ScreenLayer::Private
{
public:
    Private( GeoGraphicsScene *scene );

    GeoGraphicsScene *const m_scene;
    QString m_runtimeTrace;
};

ScreenLayer::Private::Private( GeoGraphicsScene *scene ) :
    m_scene( scene )
{
}

ScreenLayer::ScreenLayer( GeoGraphicsScene *scene ) :
    d( new Private( scene ) )
{
    connect( d->m_scene, SIGNAL(repaintNeeded()), this, SIGNAL(repaintNeeded()) );
}

ScreenLayer::~ScreenLayer()
{
    delete d;
}

QStringList ScreenLayer::renderPosition() const
{
    return QStringList( "FLOAT_ITEM" );
}

bool ScreenLayer::render( GeoPainter *painter, ViewportParams *viewport,
                            const QString &renderPos, GeoSceneLayer *layer )
{
    Q_UNUSED( renderPos )
    Q_UNUSED( layer )

    painter->save();

    foreach( ScreenOverlayGraphicsItem *item, d->m_scene->screenItems() ) {
        item->paintEvent( painter, viewport );
    }

    painter->restore();

    d->m_runtimeTrace = QString( "Items: %1")
                .arg( d->m_scene->screenItems().size() );

    return true;
}

QString ScreenLayer::runtimeTrace() const
{
    return d->m_runtimeTrace;
}

}

#include "ScreenLayer.moc"
