//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include "MarbleHeight.h"
#include "MarbleHeight.moc"

#include "MapThemeManager.h"
#include "TextureTile.h"
#include "TileLoaderHelper.h"
#include "global.h"
#include "MarbleDebug.h"

#include <GeoSceneDocument.h>
#include <GeoSceneHead.h>
#include <GeoSceneMap.h>
#include <GeoSceneLayer.h>
#include <GeoSceneTexture.h>

namespace Marble
{

MarbleHeight::MarbleHeight( HttpDownloadManager *downloadManager, QObject* parent )
    : QObject( parent )
    , m_tileLoader( downloadManager )
    , m_textureLayer( 0 )
    , m_level( 0 )
    , m_numXTiles( 1 )
    , m_numYTiles( 1 )
{
    const GeoSceneDocument *srtmTheme = MapThemeManager::loadMapTheme( "earth/srtm/srtm.dgml" );
    Q_ASSERT( srtmTheme );

    const GeoSceneHead *head = srtmTheme->head();
    Q_ASSERT( head );

    const GeoSceneMap *map = srtmTheme->map();
    Q_ASSERT( map );

    const GeoSceneLayer *sceneLayer = map->layer( head->theme() );
    Q_ASSERT( sceneLayer );

    const QString mapThemeId = head->target() + '/' + head->theme();
    m_hash = qHash( mapThemeId );

    m_textureLayer = dynamic_cast<GeoSceneTexture*>( sceneLayer->datasets().first() );
    Q_ASSERT( m_textureLayer );

    QHash<uint, const GeoSceneTexture *> layers;
    layers.insert( m_hash, m_textureLayer );

    m_tileLoader.setTextureLayers( layers );
}


void MarbleHeight::setRadius( int radius )
{
    m_radius = radius;
    m_level = 0;

    m_numXTiles = TileLoaderHelper::levelToColumn( m_textureLayer->levelZeroColumns(), m_level );
    m_numYTiles = TileLoaderHelper::levelToRow( m_textureLayer->levelZeroRows(), m_level );

    while ( m_numXTiles * m_textureLayer->tileSize().width() < radius * 2 && m_level < m_textureLayer->maximumTileLevel()) {
        ++m_level;
        m_numXTiles = TileLoaderHelper::levelToColumn( m_textureLayer->levelZeroColumns(), m_level );
        m_numYTiles = TileLoaderHelper::levelToRow( m_textureLayer->levelZeroRows(), m_level );
    };
}


qreal MarbleHeight::altitude( qreal lon, qreal lat )
{
    const int width = m_textureLayer->tileSize().width();
    const int height = m_textureLayer->tileSize().height();

    const qreal textureX = ( 0.5 + 0.5 * lon / M_PI ) * m_numXTiles * width;
    const qreal textureY = ( 0.5 -       lat / M_PI ) * m_numYTiles * height;

    qreal color = 0;

    for ( int i = 0; i < 4; ++i ) {
        const int x = ( static_cast<int>( textureX ) + ( i % 2 ) );
        const int y = ( static_cast<int>( textureY ) + ( i / 2 ) );

        const TileId id( m_hash, m_level, ( x  % ( m_numXTiles * width ) ) / width, ( y % ( m_numYTiles * height ) ) / height );

        const QImage *image = m_cache[id];
        if ( image == 0 )
        {
            QSharedPointer<TextureTile> tile = m_tileLoader.loadTile( id, id, DownloadBrowse );
            image = new QImage( *tile->image() );
            m_cache.insert( id, image );
        }
        Q_ASSERT( image );

        Q_ASSERT( width == image->width() );
        Q_ASSERT( height == image->height() );

        const qreal dx = ( textureX > (qreal)x ) ? textureX - (qreal)x : (qreal)x - textureX;
        const qreal dy = ( textureY > (qreal)y ) ? textureY - (qreal)y : (qreal)y - textureY;
        Q_ASSERT( 0 <= dx && dx <= 1 );
        Q_ASSERT( 0 <= dy && dy <= 1 );
        color += image->pixel( x % width, y % height ) * (1- dx) * (1-dy);
    }

    return color * m_radius / 6378137.0 / 1000;
}

}
