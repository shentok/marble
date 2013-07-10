//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010 Wes Hardaker <hardaker@users.sourceforge.net>
//

#ifndef APRSOBJECT_H
#define APRSOBJECT_H

#include <QtCore/QObject>
#include <QtCore/QString>

#include "GeoAprsCoordinates.h"
#include "GeoDataStyle.h"

namespace Marble
{

    class GeoDataLineString;

    class AprsObject
    {

      public:
        AprsObject( const GeoAprsCoordinates &at, const QString &name );
        ~AprsObject();

        const QString name() const;

        const GeoDataLineString* trackLine() const;

        void setLocation( const GeoAprsCoordinates &location );
        void setPixmapId( QString &pixmap );
        GeoAprsCoordinates location() const;

        QString pixmapId() const;
        QPixmap* pixmap() const;

        int seenFrom() const;

        void update( int fadeTime = 10*60, int hideTime = 30*60 );

      private:
        QColor calculatePaintColor( int from, const QTime &time,
                                    int fadetime = 10*60*1000 ) const;
        void addPoint( const GeoAprsCoordinates at );

        QString                       m_name;
        int                           m_seenFrom;
        bool                          m_havePixmap;
        QString                       m_pixmapFilename;
        QPixmap                      *m_pixmap;
        GeoDataLineString            *m_trackLine;
        QList<GeoAprsCoordinates>     m_history;
    };

}

#endif /* APRSOBJECT_H */
