//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011-2012 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#ifndef MARBLE_GLTEXTUREMAPPER_H
#define MARBLE_GLTEXTUREMAPPER_H

#include <QtCore/QObject>
#include <QtCore/qglobal.h>

#include "GeoDataCoordinates.h"

class QGLContext;
class QGLShaderProgram;

namespace Marble
{

class AbstractProjection;
class StackedTileLoader;
class TextureColorizer;
class TileId;
class ViewportParams;

class GLTextureMapper : public QObject
{
    Q_OBJECT
 public:
    GLTextureMapper( StackedTileLoader *tileLoader );
    ~GLTextureMapper();

    void mapTexture( QGLContext *glContext, const ViewportParams *viewport, int tileZoomLevel );

 private:
    static QGLShaderProgram *initializeGL( const AbstractProjection &projection );
    void loadVisibleTiles( QGLContext *glContext, const ViewportParams *viewport, int tileZoomLevel );
    GeoDataCoordinates geoCoordinates( const qreal x, const qreal y ) const;
    QPointF projectionCoordinates( qreal lon, qreal lat ) const;

 private Q_SLOTS:
    void updateTile( const TileId &id );

 private:
    class Private;
    Private *const d;

    class GlTile;
};

}

#endif
