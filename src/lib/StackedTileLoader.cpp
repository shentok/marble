/*
 * This file is part of the Marble Virtual Globe.
 *
 * Copyright 2005-2007 Torsten Rahn <tackat@kde.org>
 * Copyright 2007      Inge Wallin  <ingwa@kde.org>
 * Copyright 2008, 2009, 2010 Jens-Michael Hoffmann <jensmh@gmx.de>
 * Copyright 2010-2012 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "StackedTileLoader.h"

#include "GeoSceneTiled.h"
#include "MarbleDebug.h"
#include "MergedLayerDecorator.h"
#include "StackedTile.h"
#include "TileLoader.h"
#include "TileLoaderHelper.h"
#include "MarbleGlobal.h"

#include <QtCore/QCache>
#include <QtCore/QHash>
#include <QtCore/QReadWriteLock>
#include <QtGui/QImage>


namespace Marble
{

class StackedTileLoaderPrivate
{
public:
    StackedTileLoaderPrivate( const MergedLayerDecorator *mergedLayerDecorator )
        : m_layerDecorator( mergedLayerDecorator )
    {
        m_tileCache.setMaxCost( 20000 * 1024 ); // Cache size measured in bytes
    }

    const MergedLayerDecorator *const m_layerDecorator;
    QHash <TileId, StackedTile*>  m_tilesOnDisplay;
    QCache <TileId, StackedTile>  m_tileCache;
    QReadWriteLock m_cacheLock;
};

StackedTileLoader::StackedTileLoader( const MergedLayerDecorator *mergedLayerDecorator, QObject *parent )
    : QObject( parent ),
      d( new StackedTileLoaderPrivate( mergedLayerDecorator ) )
{
}

StackedTileLoader::~StackedTileLoader()
{
    qDeleteAll( d->m_tilesOnDisplay );
    delete d;
}

int StackedTileLoader::tileColumnCount( int level ) const
{
    return d->m_layerDecorator->tileColumnCount( level );
}

int StackedTileLoader::tileRowCount( int level ) const
{
    return d->m_layerDecorator->tileRowCount( level );
}

GeoSceneTiled::Projection StackedTileLoader::tileProjection() const
{
    return d->m_layerDecorator->tileProjection();
}

QSize StackedTileLoader::tileSize() const
{
    return d->m_layerDecorator->tileSize();
}

void StackedTileLoader::resetTilehash()
{
    QHash<TileId, StackedTile*>::const_iterator it = d->m_tilesOnDisplay.constBegin();
    QHash<TileId, StackedTile*>::const_iterator const end = d->m_tilesOnDisplay.constEnd();
    for (; it != end; ++it ) {
        Q_ASSERT( it.value()->used() && "contained in m_tilesOnDisplay should imply used()" );
        StackedTile *const tile = it.value();
        if ( tile->id().zoomLevel() != 0 ) { // keep level zero tiles in the hash
            tile->setUsed( false );
        }
    }
}

void StackedTileLoader::cleanupTilehash()
{
    // Make sure that tiles which haven't been used during the last
    // rendering of the map at all get removed from the tile hash.

    QHashIterator<TileId, StackedTile*> it( d->m_tilesOnDisplay );
    while ( it.hasNext() ) {
        it.next();
        if ( !it.value()->used() ) {
            Q_ASSERT( it.value()->id().zoomLevel() != 0 ); // keep level zero tiles in the hash
            // If insert call result is false then the cache is too small to store the tile
            // but the item will get deleted nevertheless and the pointer we have
            // doesn't get set to zero (so don't delete it in this case or it will crash!)
            d->m_tileCache.insert( it.key(), it.value(), it.value()->numBytes() );
            d->m_tilesOnDisplay.remove( it.key() );
        }
    }
}

const StackedTile* StackedTileLoader::object( TileId const & stackedTileId )
{
    // check if the tile is in the hash
    d->m_cacheLock.lockForRead();
    StackedTile * stackedTile = d->m_tilesOnDisplay.value( stackedTileId, 0 );
    d->m_cacheLock.unlock();
    if ( stackedTile ) {
        stackedTile->setUsed( true );
        return stackedTile;
    }

    // here ends the performance critical section of this method

    d->m_cacheLock.lockForWrite();

    // has another thread loaded our tile due to a race condition?
    stackedTile = d->m_tilesOnDisplay.value( stackedTileId, 0 );
    if ( stackedTile ) {
        Q_ASSERT( stackedTile->used() && "other thread should have marked tile as used" );
        d->m_cacheLock.unlock();
        return stackedTile;
    }

    Q_ASSERT( stackedTileId.zoomLevel() != 0 ); // level zero tiles should always be in the cache

    // the tile was not in the hash so check if it is in the cache
    stackedTile = d->m_tileCache.take( stackedTileId );
    Q_ASSERT( stackedTile && "tile should be loaded into cache prior to using it" );
    Q_ASSERT( !stackedTile->used() && "tiles in m_tileCache are invisible and should thus be marked as unused" );
    stackedTile->setUsed( true );
    d->m_tilesOnDisplay[ stackedTileId ] = stackedTile;
    d->m_cacheLock.unlock();

    return stackedTile;
}

bool StackedTileLoader::contains( const TileId &id ) const
{
    return d->m_tilesOnDisplay.contains( id ) || d->m_tileCache.contains( id );
}

void StackedTileLoader::insert( const TileId &stackedTileId, StackedTile *stackedTile )
{
    mDebug() << Q_FUNC_INFO << stackedTileId;

    Q_ASSERT( stackedTile );
    stackedTile->setUsed( false );

    d->m_tileCache.insert( stackedTileId, stackedTile );

    emit tileLoaded( stackedTileId );
}

void StackedTileLoader::remove( const TileId &stackedTileId )
{
    delete d->m_tilesOnDisplay.take( stackedTileId );
    d->m_tileCache.remove( stackedTileId );
}

quint64 StackedTileLoader::volatileCacheLimit() const
{
    return d->m_tileCache.maxCost() / 1024;
}

QList<TileId> StackedTileLoader::visibleTiles() const
{
    return d->m_tilesOnDisplay.keys();
}

int StackedTileLoader::tileCount() const
{
    return d->m_tileCache.count() + d->m_tilesOnDisplay.count();
}

void StackedTileLoader::setVolatileCacheLimit( quint64 kiloBytes )
{
    mDebug() << QString("Setting tile cache to %1 kilobytes.").arg( kiloBytes );
    d->m_tileCache.setMaxCost( kiloBytes * 1024 );
}

void StackedTileLoader::clear( MergedLayerDecorator *layerDecorator )
{
    mDebug() << Q_FUNC_INFO;

    qDeleteAll( d->m_tilesOnDisplay );
    d->m_tilesOnDisplay.clear();
    d->m_tileCache.clear(); // clear the tile cache in physical memory

    if ( d->m_layerDecorator->textureLayersSize() > 0 ) {
        for ( int row = 0; row < layerDecorator->tileRowCount( 0 ); ++row ) {
            for ( int column = 0; column < layerDecorator->tileColumnCount( 0 ); ++column ) {
                const TileId id = TileId( 0, 0, column, row );
                StackedTile *const levelZeroTile = layerDecorator->loadTile( id );
                levelZeroTile->setUsed( true );
                d->m_tilesOnDisplay.insert( id, levelZeroTile );
            }
        }
    }

    emit cleared();
}

}

#include "StackedTileLoader.moc"
