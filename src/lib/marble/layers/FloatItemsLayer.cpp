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
#include "ViewportParams.h"

#include <QGLFramebufferObject>

namespace Marble
{

FloatItemsLayer::FloatItemsLayer( QObject *parent ) :
    QObject( parent ),
    m_floatItems(),
    m_frameBuffer( 0 )
{
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
    Q_UNUSED( renderPos )
    Q_UNUSED( layer )

    foreach ( AbstractFloatItem *item, m_floatItems ) {
        if ( !item->enabled() )
            continue;

        if ( !item->isInitialized() ) {
            item->initialize();
            emit renderPluginInitialized( item );
        }

        if ( item->visible() ) {
            item->paintEvent( painter, viewport );
        }
    }

    return true;
}

void FloatItemsLayer::paintGL( QGLContext *glContext, const ViewportParams *viewport )
{
    if ( m_frameBuffer == 0 || m_frameBuffer->size() != viewport->size() ) {
        delete m_frameBuffer;
        m_frameBuffer = new QGLFramebufferObject( viewport->size() );
        m_frameBuffer->bind();
        QPainter painter( m_frameBuffer );
        painter.setPen( Qt::transparent );
        painter.drawRect( 0, 0, viewport->size().width(), viewport->size().height() );
        painter.end();
        m_frameBuffer->release();
    }

    {
        m_frameBuffer->bind();
        QPainter painter( m_frameBuffer );
        foreach ( AbstractFloatItem *item, m_floatItems ) {
            item->paintEvent( &painter, viewport );
        }
        painter.end();
        m_frameBuffer->release();
    }

    m_frameBuffer->bind();
    glContext->drawTexture( QPointF( 0, 0 ), m_frameBuffer->texture() );
}

void FloatItemsLayer::addFloatItem( AbstractFloatItem *floatItem )
{
    Q_ASSERT( floatItem );

    connect( floatItem, SIGNAL(settingsChanged(QString)),
             this,      SIGNAL(pluginSettingsChanged()) );
    connect( floatItem, SIGNAL(repaintNeeded(QRegion)),
             this,      SIGNAL(repaintNeeded(QRegion)) );
    connect( floatItem, SIGNAL(visibilityChanged(bool,QString)),
             this,      SLOT(updateVisibility(bool,QString)) );

    m_floatItems.append( floatItem );
}

QList<AbstractFloatItem *> FloatItemsLayer::floatItems() const
{
    return m_floatItems;
}

void FloatItemsLayer::updateVisibility( bool visible, const QString &nameId )
{
    emit visibilityChanged( nameId, visible );
}

}

#include "FloatItemsLayer.moc"
