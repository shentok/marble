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
#include <QtCore/QLocale>
#include <QtCore/QTimer>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QVector2D>
#include <QtGui/QVector3D>
#include <QtOpenGL/QGLBuffer>
#include <QtOpenGL/QGLContext>
#include <QtOpenGL/QGLShaderProgram>

// Marble
#include "projections/AbstractProjection.h"
#include "MarbleDebug.h"
#include "MarbleDirs.h"
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

    QGLContext *m_glContext;
    QGLShaderProgram *m_program;
    QMap<TileId, GlTile *> m_visibleTiles;
    QGLBuffer m_texCoordsBuffer;
    QGLBuffer m_indexBuffer;
    const int m_numLatitudes;
    const int m_numLongitudes;
};


GLTextureMapper::Private::Private( StackedTileLoader *tileLoader )
    : m_tileLoader( tileLoader )
    , m_glContext( 0 )
    , m_program( 0 )
    , m_visibleTiles()
    , m_texCoordsBuffer( QGLBuffer::VertexBuffer )
    , m_indexBuffer( QGLBuffer::IndexBuffer )
    , m_numLatitudes( 20 )
    , m_numLongitudes( 20 )
{
}


class GLTextureMapper::GlTile
{
public:
    GlTile( GLuint textureId, QGLContext *glContext, const QVector<QVector3D> &vertices ) :
        m_textureId( textureId ),
        m_glContext( glContext ),
        m_vertexBuffer( QGLBuffer::VertexBuffer )
    {
        m_vertexBuffer.create();
        m_vertexBuffer.setUsagePattern( QGLBuffer::StaticDraw );
        m_vertexBuffer.bind();
        m_vertexBuffer.allocate( vertices.constData(), vertices.size() * sizeof( QVector3D ) );
    }

    ~GlTile();

    GLuint textureId() const { return m_textureId; }
    QGLBuffer *vertexBuffer() { return &m_vertexBuffer; }

private:
    const GLuint m_textureId;
    QGLContext *const m_glContext;
    QGLBuffer m_vertexBuffer;
};


GLTextureMapper::GlTile::~GlTile()
{
    m_glContext->makeCurrent();
    m_glContext->deleteTexture( m_textureId );
    m_vertexBuffer.destroy();
}


GLTextureMapper::GLTextureMapper( StackedTileLoader *tileLoader )
    : d( new Private( tileLoader ) )
{
    connect( tileLoader, SIGNAL( tileLoaded( TileId ) ),
             this, SLOT( updateTile( TileId ) ) );
}

GLTextureMapper::~GLTextureMapper()
{
    qDeleteAll( d->m_visibleTiles );
    delete d->m_program;
    delete d;
}

void GLTextureMapper::mapTexture( QGLContext *glContext, const ViewportParams *viewport, int tileZoomLevel )
{
    if ( viewport->radius() <= 0 )
        return;

    if ( !d->m_glContext ) {
        initializeGL( glContext );
    }

    loadVisibleTiles( glContext, viewport, tileZoomLevel );

    if ( !d->m_program->bind() )
        return;

    if ( !d->m_texCoordsBuffer.isCreated() ) {
        QVector<QVector2D> texCoords;

        for (int row = 0; row < d->m_numLatitudes; row++) {
            for (int col = 0; col < d->m_numLongitudes; col++) {
                texCoords << QVector2D( col/qreal(d->m_numLongitudes-1), row/qreal(d->m_numLatitudes-1) );
            }
        }

        d->m_texCoordsBuffer.create();
        d->m_texCoordsBuffer.setUsagePattern( QGLBuffer::StaticDraw );
        d->m_texCoordsBuffer.bind();
        d->m_texCoordsBuffer.allocate( texCoords.constData(), texCoords.size() * sizeof( QVector2D ) );
    }

    if ( !d->m_indexBuffer.isCreated() ) {
        QVector<GLushort> indices;

        for (int row = 1; row < d->m_numLatitudes; row++) {
            for (int col = 0; col < d->m_numLongitudes; col++) {
                indices << ((row-1)*d->m_numLongitudes + col);
                if ( col == 0 && row > 1 ) {
                    indices << indices.last();
                }

                indices << ((row  )*d->m_numLongitudes + col);
                if ( col == (d->m_numLongitudes-1) && row < (d->m_numLatitudes-1) ) {
                    indices << indices.last();
                }
            }
        }

        d->m_indexBuffer.create();
        d->m_indexBuffer.setUsagePattern( QGLBuffer::StaticDraw );
        d->m_indexBuffer.bind();
        d->m_indexBuffer.allocate( indices.constData(), indices.size() * sizeof( GLushort ) );
    }


    const QMatrix4x4 viewportMatrix = viewport->viewportMatrix();
    const QMatrix4x4 rotationMatrix = viewport->rotationMatrix();

    d->m_program->setUniformValue( "rotationMatrix", viewportMatrix * rotationMatrix );

    d->m_indexBuffer.bind();
    d->m_texCoordsBuffer.bind();

    // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
    d->m_program->enableAttributeArray( "texCoord" );
    d->m_program->setAttributeBuffer( "texCoord", GL_FLOAT, 0, 2 );

    d->m_program->setUniformValue( "texture", 0 );

    foreach ( GlTile *tile, d->m_visibleTiles.values() ) {
        glBindTexture(GL_TEXTURE_2D, tile->textureId());
        tile->vertexBuffer()->bind();

        // Tell OpenGL programmable pipeline how to locate vertex position data
        d->m_program->enableAttributeArray( "position" );
        d->m_program->setAttributeBuffer( "position", GL_FLOAT, 0, 3 );

        glDrawElements( GL_TRIANGLE_STRIP, (d->m_numLatitudes-1) * 2 * (d->m_numLongitudes+1) - 2, GL_UNSIGNED_SHORT, 0 );
    }

    d->m_program->release();
}

void GLTextureMapper::initializeGL( QGLContext *glContext )
{
    Q_ASSERT( d->m_glContext == 0 );

    d->m_glContext = glContext;

    d->m_program = new QGLShaderProgram( this );

    // Overriding system locale until shaders are compiled
    QLocale::setDefault( QLocale::c() );

    if ( !d->m_program->addShaderFromSourceFile( QGLShader::Vertex, MarbleDirs::path( "shaders/texturelayer.vertex.glsl" ) ) ) {
        qWarning() << d->m_program->log();
        return;
    }
    if ( !d->m_program->addShaderFromSourceFile( QGLShader::Fragment, MarbleDirs::path( "shaders/texturelayer.fragment.glsl" ) ) ) {
        qWarning() << d->m_program->log();
        return;
    }
    if ( !d->m_program->link() ) {
        qWarning() << d->m_program->log();
        return;
    }

    // Restore system locale
    QLocale::setDefault( QLocale::system() );
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
