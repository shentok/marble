//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008 Torsten Rahn <tackat@kde.org>
// Copyright 2009 Jens-Michael Hoffmann <jensmh@gmx.de>
// Copyright 2011,2012 Bernahrd Beschow <bbeschow@cs.tu-berlin.de>
//


// Own
#include "LayerManager.h"

// Local dir
#include "MarbleDebug.h"
#include "AbstractDataPlugin.h"
#include "AbstractDataPluginItem.h"
#include "AbstractFloatItem.h"
#include "AbstractProjection.h"
#include "GeoPainter.h"
#include "GlLayerInterface.h"
#include "RenderPlugin.h"
#include "ViewportParams.h"
#include "LayerInterface.h"
#include "RenderState.h"

#include <QGLContext>
#include <QTime>
#include <QVector3D>

namespace Marble
{

/**
  * Returns true if the zValue of one is lower than that of two. Null must not be passed
  * as parameter.
  */
bool zValueLessThan( const LayerInterface * const one, const LayerInterface * const two )
{
    Q_ASSERT( one && two );
    return one->zValue() < two->zValue();
}

class LayerManager::Private
{
 public:
    Private( LayerManager *parent );
    ~Private();

    void updateVisibility( bool visible, const QString &nameId );

    LayerManager *const q;

    QList<RenderPlugin *> m_renderPlugins;
    QList<AbstractDataPlugin *> m_dataPlugins;
    QList<LayerInterface *> m_internalLayers;

    bool m_showBackground;

    RenderState m_renderState;
    bool m_showRuntimeTrace;
};

LayerManager::Private::Private( LayerManager *parent )
    : q( parent ),
      m_renderPlugins(),
      m_showBackground( true ),
      m_showRuntimeTrace( false )
{
}

LayerManager::Private::~Private()
{
}

void LayerManager::Private::updateVisibility( bool visible, const QString &nameId )
{
    emit q->visibilityChanged( nameId, visible );
}


LayerManager::LayerManager( QObject *parent ) :
    QObject( parent ),
    d( new Private( this ) )
{
}

LayerManager::~LayerManager()
{
    delete d;
}

bool LayerManager::showBackground() const
{
    return d->m_showBackground;
}

void LayerManager::addRenderPlugin( RenderPlugin *renderPlugin )
{
    d->m_renderPlugins.append( renderPlugin );

    QObject::connect( renderPlugin, SIGNAL(settingsChanged(QString)),
                      this, SIGNAL(pluginSettingsChanged()) );
    QObject::connect( renderPlugin, SIGNAL(repaintNeeded(QRegion)),
                      this, SIGNAL(repaintNeeded(QRegion)) );
    QObject::connect( renderPlugin, SIGNAL(visibilityChanged(bool,QString)),
                      this, SLOT(updateVisibility(bool,QString)) );

    AbstractDataPlugin * const dataPlugin =
        qobject_cast<AbstractDataPlugin *>( renderPlugin );
    if( dataPlugin )
        d->m_dataPlugins.append( dataPlugin );
}

void LayerManager::removeRenderPlugin( RenderPlugin *renderPlugin )
{
    if ( AbstractDataPlugin *dataPlugin = dynamic_cast<AbstractDataPlugin *>( renderPlugin ) ) {
        d->m_dataPlugins.removeAll( dataPlugin );
    }

    d->m_renderPlugins.removeAll( renderPlugin );
}

QList<AbstractDataPlugin *> LayerManager::dataPlugins() const
{
    return d->m_dataPlugins;
}

QList<AbstractDataPluginItem *> LayerManager::whichItemAt( const QPoint& curpos ) const
{
    QList<AbstractDataPluginItem *> itemList;

    foreach( AbstractDataPlugin *plugin, d->m_dataPlugins ) {
        itemList.append( plugin->whichItemAt( curpos ) );
    }
    return itemList;
}

void LayerManager::renderLayers( GeoPainter *painter, ViewportParams *viewport )
{
    d->m_renderState = RenderState( "Marble" );
    const QTime totalTime = QTime::currentTime();

    QGLContext *const glContext = const_cast<QGLContext *>( QGLContext::currentContext() );

    QStringList renderPositions;

    if ( d->m_showBackground ) {
        renderPositions << "STARS" << "BEHIND_TARGET";
    }

    renderPositions << "SURFACE" << "HOVERS_ABOVE_SURFACE" << "ATMOSPHERE"
                    << "ORBIT" << "ALWAYS_ON_TOP" << "FLOAT_ITEM" << "USER_TOOLS";

    QStringList traceList;
    foreach( const QString& renderPosition, renderPositions ) {
        QList<LayerInterface*> layers;

        // collect all RenderPlugins of current renderPosition
        foreach( RenderPlugin *renderPlugin, d->m_renderPlugins ) {
            if ( renderPlugin && renderPlugin->renderPosition().contains( renderPosition ) ) {
                if ( renderPlugin->enabled() && renderPlugin->visible() ) {
                    if ( !renderPlugin->isInitialized() ) {
                        renderPlugin->initialize();
                        emit renderPluginInitialized( renderPlugin );
                    }
                    layers.push_back( renderPlugin );
                }
            }
        }

        // collect all internal LayerInterfaces of current renderPosition
        foreach( LayerInterface *layer, d->m_internalLayers ) {
            if ( layer && layer->renderPosition().contains( renderPosition ) ) {
                layers.push_back( layer );
            }
        }

        // sort them according to their zValue()s
        qSort( layers.begin(), layers.end(), zValueLessThan );

        // render the layers of the current renderPosition
        QTime timer;
        foreach( LayerInterface *layer, layers ) {
            timer.start();

            GlLayerInterface *const glInterface = dynamic_cast<GlLayerInterface *>( layer );
            if ( glContext && glInterface ) {
                painter->beginNativePainting();
                glPushAttrib( GL_ALL_ATTRIB_BITS );
                glPushClientAttrib( GL_CLIENT_ALL_ATTRIB_BITS );
                glViewport( 0, 0, viewport->width(), viewport->height() );

                glMatrixMode( GL_PROJECTION );
                glPushMatrix();
                glLoadIdentity();
                glOrtho( 0, viewport->width(), viewport->height(), 0, -256000000/M_PI*80, 256/M_PI*32 );

                glMatrixMode( GL_MODELVIEW );
                glPushMatrix();
                glLoadIdentity();
                glTranslated( viewport->width() / 2, viewport->height() / 2, 0 );

                if ( viewport->projection() == Spherical ) {
                    const Quaternion axis = viewport->planetAxis();

                    const qreal angle = 2 * acos( axis.v[Q_W] ) * RAD2DEG;
                    const qreal ax = axis.v[Q_X];
                    const qreal ay = -axis.v[Q_Y];
                    const qreal az = axis.v[Q_Z];

                    glRotated( angle, ax, ay, az );
                } else {
                    // Calculate translation of center point
                    const GeoDataCoordinates coordinates = viewport->viewLatLonAltBox().center();
                    const QVector3D center = viewport->currentProjection()->vertexCoordinates( coordinates );
                    glTranslated( -center.x() * viewport->radius(),
                                  -center.y() * viewport->radius(),
                                  0 );
                }
                glScaled( viewport->radius(), viewport->radius(), viewport->radius() );

                glEnable( GL_DEPTH_TEST );
                glEnable( GL_CULL_FACE );
                glFrontFace( GL_CCW );

                glInterface->paintGL( glContext, viewport );

                glDisable( GL_CULL_FACE );
                glDisable( GL_DEPTH_TEST );

                glMatrixMode( GL_MODELVIEW );
                glPopMatrix();
                glMatrixMode( GL_PROJECTION );
                glPopMatrix();
                glPopClientAttrib();
                glPopAttrib();
                painter->endNativePainting();
            }
            else {
                layer->render( painter, viewport, renderPosition, 0 );
            }

            d->m_renderState.addChild( layer->renderState() );
            traceList.append( QString("%2 ms %3").arg( timer.elapsed(),3 ).arg( layer->runtimeTrace() ) );
        }
    }

    if ( d->m_showRuntimeTrace ) {
        const int totalElapsed = totalTime.elapsed();
        const int fps = 1000.0/totalElapsed;
        traceList.append( QString( "Total: %1 ms (%2 fps)" ).arg( totalElapsed, 3 ).arg( fps ) );

        painter->save();
        painter->setBackgroundMode( Qt::OpaqueMode );
        painter->setBackground( Qt::gray );
        painter->setFont( QFont( "Sans Serif", 10, QFont::Bold ) );

        int i=0;
        foreach ( const QString &text, traceList ) {
            painter->setPen( Qt::black );
            painter->drawText( QPoint(10,40+15*i), text );
            painter->setPen( Qt::white );
            painter->drawText( QPoint(9,39+15*i), text );
            ++i;
        }
        painter->restore();
    }
}

void LayerManager::setShowBackground( bool show )
{
    d->m_showBackground = show;
}

void LayerManager::setShowRuntimeTrace( bool show )
{
    d->m_showRuntimeTrace = show;
}

void LayerManager::addLayer(LayerInterface *layer)
{
    d->m_internalLayers.push_back(layer);
}

void LayerManager::removeLayer(LayerInterface *layer)
{
    d->m_internalLayers.removeAll(layer);
}

QList<LayerInterface *> LayerManager::internalLayers() const
{
    return d->m_internalLayers;
}

RenderState LayerManager::renderState() const
{
    return d->m_renderState;
}

}

#include "LayerManager.moc"
