//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010 Wes Hardaker <marble@ws6z.com>
//

#include "AprsObject.h"

#include <QtGui/QPixmap>

#include "MarbleDebug.h"
#include "MarbleDirs.h"
#include "GeoDataStyle.h"
#include "GeoDataLineString.h"
#include "GeoAprsCoordinates.h"

using namespace Marble;

AprsObject::AprsObject( const GeoAprsCoordinates &at, const QString &name )
    : m_myName( name ),
      m_seenFrom( at.seenFrom() ),
      m_havePixmap ( false ),
      m_pixmapFilename( ),
      m_pixmap( 0 ),
      m_trackLine( 0 )
{
    m_history.push_back( at );
}

AprsObject::~AprsObject()
{
    delete m_pixmap;
    delete m_trackLine;
}

const QString
AprsObject::name() const
{
    return m_myName;
}

const GeoDataLineString*
AprsObject::trackLine() const
{
    return m_trackLine;
}

GeoAprsCoordinates
AprsObject::location() const
{
    return m_history.last();
}

void
AprsObject::setLocation( const GeoAprsCoordinates &location )
{
    // Not ideal but it's unlikely they'll jump to the *exact* same spot again
    if ( !m_history.contains( location ) ) {
        m_history.push_back( location );
        mDebug() << "  moved: " << m_myName.toLocal8Bit().data();
    } else {
        int index = m_history.indexOf( location );
        QTime now;
        m_history[index].setTimestamp( now );
        m_history[index].addSeenFrom( location.seenFrom() );
    }

    m_seenFrom = ( m_seenFrom | location.seenFrom() );
}

void
AprsObject::setPixmapId( QString &pixmap )
{
    QString pixmapFilename = MarbleDirs::path( pixmap );
    if ( QFile( pixmapFilename ).exists() ) {
        m_havePixmap = true;
        m_pixmapFilename = pixmapFilename;
        // We can't load the pixmap here since it's used in a different thread
    }
    else {
        m_havePixmap = false;
    }
}

QString
AprsObject::pixmapId() const
{
    return m_pixmapFilename;
}

QPixmap*
AprsObject::pixmap() const
{
    if ( m_havePixmap ) {
        return m_pixmap;
    }

    return 0;
}

int
AprsObject::seenFrom() const
{
    return m_seenFrom;
}

void
AprsObject::update( int fadeTime, int hideTime )
{
    Q_UNUSED( fadeTime );

    //QColor baseColor = calculatePaintColor( m_seenFrom,
    //                                  m_history.last().timestamp(),
    //                                  fadeTime );

    if ( m_history.count() > 1 ) {
    
        QList<GeoAprsCoordinates>::iterator spot = m_history.begin();
        QList<GeoAprsCoordinates>::iterator endSpot = m_history.end();

        if ( !m_trackLine ) {
            m_trackLine = new GeoDataLineString();
        } else {
            m_trackLine->clear();
        }

        *m_trackLine << *spot;

        for( ++spot; spot != endSpot; ++spot ) {

            if ( hideTime > 0 && ( *spot ).timestamp().elapsed() > hideTime )
                // FIXME remove old points
                break;

            *m_trackLine << *spot;
        }
    } else {
        if ( m_trackLine ) {
            delete m_trackLine;
            m_trackLine = 0;
        }
    }
}
