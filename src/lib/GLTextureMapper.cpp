//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//


// local
#include"GLTextureMapper.h"

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
#include "Projections/AbstractProjection.h"
#include "GeoPainter.h"
#include "MarbleDebug.h"
#include "StackedTileLoader.h"
#include "ViewParams.h"
#include "ViewportParams.h"
#include "TextureColorizer.h"
#include "StackedTile.h"
#include "MathHelper.h"

using namespace Marble;



class GLTextureMapper::Private
{
public:
    Private( StackedTileLoader *tileLoader );

    StackedTileLoader *const m_tileLoader;

    QMap<QString, Tile> m_visibleTiles;
    QVector<QVector2D> m_texCoords;
    const int m_numLatitudes;
    const int m_numLongitudes;

    bool m_repaintNeeded;
    int m_previousLevel;
};


GLTextureMapper::Private::Private( StackedTileLoader *tileLoader )
    : m_tileLoader( tileLoader )
    , m_visibleTiles()
    , m_numLatitudes( 20 )
    , m_numLongitudes( 20 )
    , m_repaintNeeded( true )
    , m_previousLevel( -1 )
{
}


class GLTextureMapper::Tile
{
public:
    Tile();
    Tile( const TileId &id, GLuint glTexture, const QVector<QVector3D> &vertices );

    TileId id() const { return m_id; }
    GLuint glTexture() const { return m_glTexture; }
    QVector<QVector3D> vertices() const;
    bool operator==( const Tile &other );

private:
    TileId m_id;
    GLuint m_glTexture;
    QVector<QVector3D> m_vertices;
};


GLTextureMapper::Tile::Tile()
    : m_id()
    , m_glTexture( 0 )
{
}

GLTextureMapper::Tile::Tile( const Marble::TileId &id, GLuint glTexture , const QVector<QVector3D> &vertices )
    : m_id( id )
    , m_glTexture( glTexture )
    , m_vertices( vertices )
{
}

QVector<QVector3D> GLTextureMapper::Tile::vertices() const
{
    return m_vertices;
}

bool GLTextureMapper::Tile::operator==(const Tile &other)
{
    return m_id == other.id() && m_glTexture == other.glTexture();
}



GLTextureMapper::GLTextureMapper( StackedTileLoader *tileLoader )
    : TextureMapperInterface()
    , d( new Private( tileLoader ) )
{
    for (int row = 0; row < d->m_numLatitudes; row++) {
        for (int col = 0; col <= d->m_numLongitudes; col++){
            d->m_texCoords << QVector2D( col*1.0/d->m_numLatitudes, row*1.0/d->m_numLongitudes );
            if ( row > 0 && col == 0 ) {
                d->m_texCoords << d->m_texCoords.last();
            }

            d->m_texCoords << QVector2D( col*1.0/d->m_numLatitudes, (row+1)*1.0/d->m_numLongitudes );
            if ( row < d->m_numLatitudes - 1 && col == d->m_numLatitudes ) {
                d->m_texCoords << d->m_texCoords.last();
            }
        }
    }
}

GLTextureMapper::~GLTextureMapper()
{
    delete d;
}

void GLTextureMapper::mapTexture( GeoPainter *painter,
                                  const ViewportParams *viewport,
                                  const QRect &dirtyRect,
                                  TextureColorizer *texColorizer )
{
    if ( viewport->radius() <= 0 )
        return;

    painter->beginNativePainting();

    glEnable( GL_DEPTH_TEST );
    glEnable( GL_CULL_FACE );
    glEnable( GL_TEXTURE_2D );
    glFrontFace( GL_CCW );

    setViewport( viewport );

    glMatrixMode( GL_MODELVIEW );
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
        const qreal centerLon = viewport->centerLongitude();
        const qreal centerLat = viewport->centerLatitude();

        const QVector3D center = viewport->currentProjection()->vertexCoordinates( centerLon, centerLat );
        glTranslated( -center.x() * viewport->radius(),
                      -center.y() * viewport->radius(),
                      0 );
    }
    glScaled( viewport->radius(), viewport->radius(), viewport->radius() );

    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );

    foreach ( const Tile &tile, d->m_visibleTiles.values() ) {
        glBindTexture( GL_TEXTURE_2D, tile.glTexture() );
        glVertexPointer( 3, GL_FLOAT, 0, tile.vertices().data() );
        glTexCoordPointer( 2, GL_FLOAT, 0, d->m_texCoords.data() );
        glDrawArrays( GL_TRIANGLE_STRIP, 0, d->m_texCoords.size() );
    }

    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );

    glDisable( GL_CULL_FACE );
    painter->endNativePainting();
}

void GLTextureMapper::setRepaintNeeded()
{
    d->m_repaintNeeded = true;
}

void GLTextureMapper::setViewport( const ViewportParams *viewport )
{
    // mark all tiles as unused
    d->m_tileLoader->resetTilehash();

    const int width = viewport->size().width();
    const int height = viewport->size().height();

    glViewport( 0, 0, width, height );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( 0, width, height, 0, -256000000/M_PI*80, 256/M_PI*32 );

    QGLContext *const glContext = const_cast<QGLContext *>( QGLContext::currentContext() );

    const GeoDataLatLonAltBox bbox = viewport->viewLatLonAltBox();

    const int numXTiles = d->m_tileLoader->tileColumnCount( tileZoomLevel() );
    const int numYTiles = d->m_tileLoader->tileRowCount( tileZoomLevel() );

    qreal topLeftX, topLeftY, botRightX, botRightY;
    projectionCoordinates( bbox.west(), bbox.north(), topLeftX, topLeftY );
    projectionCoordinates( bbox.east(), bbox.south(), botRightX, botRightY );
    const int startXTile =       numXTiles * topLeftX;
    const int startYTile =       numYTiles * topLeftY;
          int endXTile   = 1.5 + numXTiles * botRightX;
          int endYTile   = 1.5 + numYTiles * botRightY;

    if ( endXTile <= startXTile )
        endXTile += numXTiles;
    if ( endYTile <= startYTile )
        endYTile += numYTiles;

    QList<QString> invisibleTiles = d->m_visibleTiles.keys();

    for (int i = startXTile; i < endXTile; ++i) {
        for (int j = startYTile; j < endYTile; ++j) {
            const TileId id( 0, tileZoomLevel(), i % numXTiles, j % numYTiles );

            // load stacked tile into cache and mark it as used
            const StackedTile *stackedTile = d->m_tileLoader->loadTile( id );

            invisibleTiles.removeAll( id.toString() );
            if ( !d->m_visibleTiles.contains( id.toString() ) ) {
                const QImage image = *stackedTile->resultTile();
                const GLuint texture = glContext->bindTexture( image, GL_TEXTURE_2D, GL_RGBA, QGLContext::LinearFilteringBindOption | QGLContext::PremultipliedAlphaBindOption );

                const int numXTiles = d->m_tileLoader->tileColumnCount( id.zoomLevel() );
                const int numYTiles = d->m_tileLoader->tileRowCount( id.zoomLevel() );

                QVector<QVector3D> vertices;

                for (int row = 0; row < d->m_numLatitudes; row++) {
                    for (int col = 0; col <= d->m_numLongitudes; col++){
                        const qreal x  = (id.x() * d->m_numLongitudes + col    ) * 1.0 / (d->m_numLongitudes*numXTiles);
                        const qreal y1 = (id.y() * d->m_numLatitudes  + row    ) * 1.0 / (d->m_numLatitudes *numYTiles);
                        const qreal y2 = (id.y() * d->m_numLatitudes  + row + 1) * 1.0 / (d->m_numLatitudes *numYTiles);

                        qreal lon, lat;

                        geoCoordinates( x, y1, lon, lat );
                        vertices << viewport->currentProjection()->vertexCoordinates( lon, lat );
                        if ( row > 0 && col == 0 ) {
                            vertices << vertices.last();
                        }

                        geoCoordinates( x, y2, lon, lat );
                        vertices << viewport->currentProjection()->vertexCoordinates( lon, lat );
                        if ( row < d->m_numLatitudes - 1 && col == d->m_numLatitudes ) {
                            vertices << vertices.last();
                        }
                    }
                }

                d->m_visibleTiles.insert( id.toString(), Tile( id, texture, vertices ) );
            }
        }
    }

    foreach ( const QString &id, invisibleTiles ) {
        Tile tile = d->m_visibleTiles[id];
        d->m_visibleTiles.remove( id );
        glContext->deleteTexture( tile.glTexture() );
    }

    // clear unused tiles
    d->m_tileLoader->cleanupTilehash();
}

void GLTextureMapper::updateTile( const TileId &id )
{
    QGLContext *const glContext = const_cast<QGLContext *>( QGLContext::currentContext() );

    if ( d->m_visibleTiles.contains( id.toString() ) ) {
        Tile tile = d->m_visibleTiles[id.toString()];
        glContext->deleteTexture( tile.glTexture() );
        d->m_visibleTiles.remove( id.toString() );
    }

#warning this method gets not called
}

void GLTextureMapper::geoCoordinates( qreal normalizedX, qreal normalizedY,
                                      qreal& lon, qreal& lat ) const
{
    Q_ASSERT( 0 <= normalizedX && normalizedX <= 1 );
    Q_ASSERT( 0 <= normalizedY && normalizedY <= 1 );

    switch ( d->m_tileLoader->tileProjection() ) {
    case GeoSceneTexture::Mercator:
        lat = atan( sinh( ( 0.5 - normalizedY ) * 2 * M_PI ) );
        lon = ( normalizedX - 0.5 ) * 2 * M_PI;
        return;
    case GeoSceneTexture::Equirectangular:
        lat = ( 0.5 - normalizedY ) * M_PI;
        lon = ( normalizedX - 0.5 ) * 2 * M_PI;
        return;
    }

    Q_ASSERT( false ); // not reached

    return;
}

void GLTextureMapper::projectionCoordinates( qreal lon, qreal lat, qreal &x, qreal &y ) const
{
    switch ( d->m_tileLoader->tileProjection() ) {
    case GeoSceneTexture::Mercator:
        if ( lat < -85*DEG2RAD ) lat = -85*DEG2RAD;
        if ( lat >  85*DEG2RAD ) lat =  85*DEG2RAD;
        x = ( 0.5 + 0.5 * lon / M_PI );
        y = ( 0.5 - 0.5 * atanh( sin( lat ) ) / M_PI );
        return;
    case GeoSceneTexture::Equirectangular:
        x = ( 0.5 + 0.5 * lon / M_PI );
        y = ( 0.5 - lat / M_PI );
        return;
    }

    Q_ASSERT( false ); // not reached

    return;
}
