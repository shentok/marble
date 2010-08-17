//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011-2012 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//


// local
#include "GLTextureMapper.h"

// posix
#include <cmath>

// Qt
#include <QtCore/qmath.h>
#include <QtCore/QTimer>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QVector2D>
#include <QtGui/QVector3D>
#include <QtOpenGL/QGLContext>

// Marble
#include "projections/AbstractProjection.h"
#include "MarbleDebug.h"
#include "MathHelper.h"
#include "StackedTile.h"
#include "StackedTileLoader.h"
#include "TextureColorizer.h"
#include "TileId.h"
#include "ViewportParams.h"

namespace Marble
{

class GLTextureMapper::Private
{
public:
    Private( StackedTileLoader *tileLoader );

    StackedTileLoader *const m_tileLoader;

    QMap<TileId, GlTile *> m_visibleTiles;
    QVector<QVector2D> m_texCoords;
    QVector<GLushort> m_indices;
    const int m_numLatitudes;
    const int m_numLongitudes;
};


GLTextureMapper::Private::Private( StackedTileLoader *tileLoader )
    : m_tileLoader( tileLoader )
    , m_visibleTiles()
    , m_numLatitudes( 20 )
    , m_numLongitudes( 20 )
{
}


class GLTextureMapper::GlTile
{
public:
    GlTile( GLuint textureId, QGLContext *glContext, const QVector<QVector3D> &vertices );
    ~GlTile();

    GLuint textureId() const { return m_textureId; }
    QVector<QVector3D> vertices() const { return m_vertices; }

private:
    const GLuint m_textureId;
    QGLContext *const m_glContext;
    const QVector<QVector3D> m_vertices;
};


GLTextureMapper::GlTile::GlTile( GLuint textureId, QGLContext *glContext, const QVector<QVector3D> &vertices )
    : m_textureId( textureId )
    , m_glContext( glContext )
    , m_vertices( vertices )
{
}

GLTextureMapper::GlTile::~GlTile()
{
    m_glContext->makeCurrent();
    m_glContext->deleteTexture( m_textureId );
}


GLTextureMapper::GLTextureMapper( StackedTileLoader *tileLoader )
    : d( new Private( tileLoader ) )
{
    for (int row = 0; row < d->m_numLatitudes; row++) {
        for (int col = 0; col < d->m_numLongitudes; col++) {
            d->m_texCoords << QVector2D( col/qreal(d->m_numLongitudes-1), row/qreal(d->m_numLatitudes-1) );
        }
    }

    for (int row = 1; row < d->m_numLatitudes; row++) {
        for (int col = 0; col < d->m_numLongitudes; col++) {
            d->m_indices << ((row-1)*d->m_numLongitudes + col);
            if ( col == 0 && row > 1 ) {
                d->m_indices << d->m_indices.last();
            }

            d->m_indices << ((row  )*d->m_numLongitudes + col);
            if ( col == (d->m_numLongitudes-1) && row < (d->m_numLatitudes-1) ) {
                d->m_indices << d->m_indices.last();
            }
        }
    }

    connect( tileLoader, SIGNAL( tileLoaded( TileId ) ),
             this, SLOT( updateTile( TileId ) ) );
}

GLTextureMapper::~GLTextureMapper()
{
    qDeleteAll( d->m_visibleTiles );
    delete d;
}

void GLTextureMapper::mapTexture( QGLContext *glContext, const ViewportParams *viewport, int tileZoomLevel )
{
    if ( viewport->radius() <= 0 )
        return;

    loadVisibleTiles( glContext, viewport, tileZoomLevel );

    glEnable( GL_TEXTURE_2D );

    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );

    foreach ( GlTile *tile, d->m_visibleTiles.values() ) {
        glBindTexture( GL_TEXTURE_2D, tile->textureId() );
        glVertexPointer( 3, GL_FLOAT, 0, tile->vertices().constData() );
        glTexCoordPointer( 2, GL_FLOAT, 0, d->m_texCoords.constData() );
        glDrawElements( GL_TRIANGLE_STRIP, (d->m_numLatitudes-1) * 2 * (d->m_numLongitudes+1) - 2, GL_UNSIGNED_SHORT, d->m_indices.constData() );
    }

    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );

    glDisable( GL_TEXTURE_2D );
}

void GLTextureMapper::loadVisibleTiles( QGLContext *glContext, const ViewportParams *viewport, int tileZoomLevel )
{
    // mark all tiles as unused
    d->m_tileLoader->resetTilehash();

    const GeoDataLatLonAltBox bbox = viewport->viewLatLonAltBox();

    const int numXTiles = d->m_tileLoader->tileColumnCount( tileZoomLevel );
    const int numYTiles = d->m_tileLoader->tileRowCount( tileZoomLevel );

    const QPointF topLeft = projectionCoordinates( bbox.west(), bbox.north() );
    const QPointF botRight = projectionCoordinates( bbox.east(), bbox.south() );
    const int startXTile =       numXTiles * topLeft.x();
    const int startYTile =       numYTiles * topLeft.y();
          int endXTile   = 1.5 + numXTiles * botRight.x();
          int endYTile   = 1.5 + numYTiles * botRight.y();

    if ( endXTile <= startXTile )
        endXTile += numXTiles;
    if ( endYTile <= startYTile )
        endYTile += numYTiles;

    QMap<TileId, GlTile *> visibleTiles;

    for (int i = startXTile; i < endXTile; ++i) {
        for (int j = startYTile; j < endYTile; ++j) {
            const TileId id( 0, tileZoomLevel, i % numXTiles, j % numYTiles );

            // load stacked tile into cache and mark it as used
            const StackedTile *stackedTile = d->m_tileLoader->loadTile( id );

            if ( d->m_visibleTiles.contains( id ) ) {
                visibleTiles.insert( id, d->m_visibleTiles.take( id ) );
            }
            else {
                const QImage image = *stackedTile->resultImage();
                const GLuint texture = glContext->bindTexture( image, GL_TEXTURE_2D, GL_RGBA, QGLContext::LinearFilteringBindOption | QGLContext::PremultipliedAlphaBindOption );

                const qreal numXTiles = d->m_tileLoader->tileColumnCount( id.zoomLevel() );
                const qreal numYTiles = d->m_tileLoader->tileRowCount( id.zoomLevel() );

                QVector<QVector3D> vertices;

                for (int row = 0; row < d->m_numLatitudes; row++) {
                    for (int col = 0; col < d->m_numLongitudes; col++){
                        const qreal x = (id.x() + col/qreal(d->m_numLongitudes-1)) / numXTiles;
                        const qreal y = (id.y() + row/qreal(d->m_numLatitudes -1)) / numYTiles;

                        const GeoDataCoordinates coordinates = geoCoordinates( x, y );
                        vertices << viewport->currentProjection()->vertexCoordinates( coordinates );
                    }
                }

                visibleTiles.insert( id, new GlTile( texture, glContext, vertices ) );
            }
        }
    }

    qDeleteAll( d->m_visibleTiles );
    d->m_visibleTiles = visibleTiles;

    // clear unused tiles
    d->m_tileLoader->cleanupTilehash();
}

void GLTextureMapper::updateTile( const TileId &id )
{
    if ( d->m_visibleTiles.contains( id ) ) {
        GlTile *tile = d->m_visibleTiles.take( id );
        if ( tile ) {
            delete tile;
        }
    }
}

GeoDataCoordinates GLTextureMapper::geoCoordinates( qreal normalizedX, qreal normalizedY ) const
{
    Q_ASSERT( 0 <= normalizedX && normalizedX <= 1 );
    Q_ASSERT( 0 <= normalizedY && normalizedY <= 1 );

    switch ( d->m_tileLoader->tileProjection() ) {
    case GeoSceneTiled::Mercator:
        return GeoDataCoordinates(             ( normalizedX - 0.5 ) * 2 * M_PI,
                                   atan( sinh( ( 0.5 - normalizedY ) * 2 * M_PI ) ) );
    case GeoSceneTiled::Equirectangular:
        return GeoDataCoordinates( ( normalizedX - 0.5 ) * 2 * M_PI,
                                   ( 0.5 - normalizedY ) * M_PI );
    }

    Q_ASSERT( false ); // not reached

    return GeoDataCoordinates();
}

QPointF GLTextureMapper::projectionCoordinates( qreal lon, qreal lat ) const
{
    const qreal x = 0.5 + 0.5 * lon / M_PI;

    switch ( d->m_tileLoader->tileProjection() ) {
    case GeoSceneTiled::Mercator:
        if ( lat < -85*DEG2RAD ) lat = -85*DEG2RAD;
        if ( lat >  85*DEG2RAD ) lat =  85*DEG2RAD;
        return QPointF( x, 0.5 - 0.5 * atanh( sin( lat ) ) / M_PI );
    case GeoSceneTiled::Equirectangular:
        return QPointF( x, 0.5 - lat / M_PI );
    }

    Q_ASSERT( false ); // not reached

    return QPointF();
}

}

#include "GLTextureMapper.moc"
