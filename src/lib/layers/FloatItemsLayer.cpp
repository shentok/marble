//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2012      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include "FloatItemsLayer.h"

#include "AbstractFloatItem.h"
#include "GeoPainter.h"
#include "MarbleModel.h"
#include "PluginManager.h"
#include "ViewportParams.h"

namespace Marble
{

FloatItemsLayer::FloatItemsLayer( const MarbleModel *model , QObject *parent ) :
    QObject( parent ),
    m_floatItems()
{
    foreach ( const AbstractFloatItem *plugin, model->pluginManager()->floatItemPlugins() ) {
        AbstractFloatItem *const item = plugin->newInstance( model );
        item->initialize();
        m_floatItems.append( item );
    }
}

FloatItemsLayer::~FloatItemsLayer()
{
    qDeleteAll( m_floatItems );
}

QStringList FloatItemsLayer::renderPosition() const
{
    return QStringList() << "FLOAT_ITEM";
}

bool FloatItemsLayer::render( GeoPainter *painter,
                              ViewportParams *viewport,
                              const QString &renderPos,
                              GeoSceneLayer *layer )
{
    foreach ( AbstractFloatItem *item, m_floatItems ) {
        if ( item->enabled() && item->visible() ) {
            item->paintEvent( painter, viewport, renderPos, layer );
        }
    }

    return true;
}

void FloatItemsLayer::setVisible( const QString &nameId, bool visible )
{
    bool visibiliyChanged = false;

    foreach ( AbstractFloatItem *item, m_floatItems ) {
        if ( item->nameId() == nameId ) {
            if ( item->visible() == visible )
                continue;

            item->setVisible( visible );
            visibiliyChanged = true;
        }
    }

    if ( visibiliyChanged ) {
        emit repaintNeeded();
    }
}

QList<AbstractFloatItem *> FloatItemsLayer::floatItems() const
{
    return m_floatItems;
}

}

#include "FloatItemsLayer.moc"
