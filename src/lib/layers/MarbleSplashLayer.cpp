//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008-2009 Torsten Rahn <tackat@kde.org>
// Copyright 2009      Jens-Michael Hoffmann <jensmh@gmx.de>
// Copyright 2011      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

// Own
#include "MarbleSplashLayer.h"

#include "GeoPainter.h"
#include "MarbleDirs.h"
#include "ViewportParams.h"

#include <QtGui/QPixmap>

namespace Marble
{

MarbleSplashLayer::MarbleSplashLayer() :
    m_viewportSize()
{
}

QStringList MarbleSplashLayer::renderPosition() const
{
    return QStringList() << "SURFACE";
}

bool MarbleSplashLayer::setViewport( const ViewportParams *viewport )
{
    m_viewportSize = viewport->size();

    return true;
}

bool MarbleSplashLayer::render( GeoPainter *painter, const QSize &viewportSize ) const
{
    Q_UNUSED( viewportSize )

    painter->save();

    QPixmap logoPixmap( MarbleDirs::path( "svg/marble-logo-inverted-72dpi.png" ) );

    if ( logoPixmap.width() > m_viewportSize.width() * 0.7
         || logoPixmap.height() > m_viewportSize.height() * 0.7 )
    {
        logoPixmap = logoPixmap.scaled( m_viewportSize * 0.7,
                                        Qt::KeepAspectRatio, Qt::SmoothTransformation );
    }

    QPoint logoPosition( ( m_viewportSize.width()  - logoPixmap.width() ) / 2,
                         ( m_viewportSize.height() - logoPixmap.height() ) / 2 );
    painter->drawPixmap( logoPosition, logoPixmap );

    QString message; // "Please assign a map theme!";

    painter->setPen( Qt::white );

    int yTop = logoPosition.y() + logoPixmap.height() + 10;
    QRect textRect( 0, yTop,
                    m_viewportSize.width(), m_viewportSize.height() - yTop );
    painter->drawText( textRect, Qt::AlignHCenter | Qt::AlignTop, message );

    painter->restore();

    return true;
}


}
