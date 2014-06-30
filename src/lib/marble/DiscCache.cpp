//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2007      Tobias Koenig  <tokoe@kde.org>
//


// Own
#include "DiscCache.h"

// Qt
#include <QtGlobal>
#include <QFile>
#include <QDirIterator>

using namespace Marble;

DiscCache::DiscCache( const QString &cacheDirectory )
    : m_CacheDirectory( cacheDirectory ),
      m_CacheLimit( 300 * 1024 * 1024 )
{
    Q_ASSERT( !m_CacheDirectory.isEmpty() && "Passed empty cache directory!" );
}

DiscCache::~DiscCache()
{
}

quint64 DiscCache::cacheLimit() const
{
    return m_CacheLimit;
}

void DiscCache::clear()
{
    QDirIterator it( m_CacheDirectory );

    // Remove all files from cache directory
    while ( it.hasNext() ) {
        it.next();

        QFile::remove( it.fileName() );
    }
}

bool DiscCache::exists( const QString &key ) const
{
    QFileInfo fi( keyToFileName( key ) );
    return fi.exists();
}

bool DiscCache::find( const QString &key, QByteArray &data )
{
    // If we can open the file, load all data and update access timestamp
    QFile file( keyToFileName( key ) );
    if ( file.open( QIODevice::ReadOnly ) ) {
        data = file.readAll();

        return true;
    }

    return false;
}

bool DiscCache::insert( const QString &key, const QByteArray &data )
{
    // If we can't open/create a file for this entry signal an error
    QFile file( keyToFileName( key ) );
    if ( !file.open( QIODevice::WriteOnly ) )
        return false;

    // Store the data on disc
    file.write( data );

    return true;
}

void DiscCache::remove( const QString &key )
{
    QFile::remove( keyToFileName( key ) );
}

void DiscCache::setCacheLimit( quint64 n )
{
    m_CacheLimit = n;
}

QString DiscCache::keyToFileName( const QString &key ) const
{
    QString fileName( key );
    fileName.replace( '/', '_' );

    return m_CacheDirectory + '/' + fileName;
}
