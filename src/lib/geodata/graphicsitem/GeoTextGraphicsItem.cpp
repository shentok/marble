//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013      Rene Kuettner <rene@bitkanal.net>
//

#include "GeoTextGraphicsItem.h"

#include "GeoDataLineStyle.h"
#include "GeoDataCoordinates.h"
#include "GeoPainter.h"
#include "ViewportParams.h"
#include "GeoDataStyle.h"

#include <QtGui/QFontMetrics>

namespace Marble
{

GeoTextGraphicsItem::GeoTextGraphicsItem( const GeoDataFeature *feature,
                                          const GeoDataCoordinates &location,
                                          const QString &text )
    : GeoGraphicsItem( feature ),
      m_location( location ),
      m_text( text ),
      m_latLonAltBox( location )
{
}

const GeoDataLatLonAltBox& GeoTextGraphicsItem::latLonAltBox() const
{
    return m_latLonAltBox;
}

void GeoTextGraphicsItem::setLocation( const GeoDataCoordinates &location )
{
    m_location = location;
    m_latLonAltBox = GeoDataLatLonAltBox( location );
}

const GeoDataCoordinates GeoTextGraphicsItem::location() const
{
    return m_location;
}

void GeoTextGraphicsItem::setText( const QString &text )
{
    m_text = text;
}

const QString GeoTextGraphicsItem::text() const
{
    return m_text;
}

void GeoTextGraphicsItem::paint( GeoPainter* painter, const ViewportParams *viewport )
{
    Q_UNUSED( viewport );

    painter->save();

    if (style()) {
        // label styles
        painter->setFont( style()->labelStyle().font() );
        painter->setPen( style()->labelStyle().color() );
    }

    // Of course in theory we could have the "isGeoProjected" parameter used
    // for drawText as well. However this would require us to convert all
    // glyphs to PainterPaths / QPolygons. From QPolygons we could create
    // GeoDataPolygons which could get painted on screen. Any patches appreciated ;-)

    int pointRepeatNum;
    qreal x[100];
    qreal y;
    bool globeHidesPoint;

    const QSizeF textSize( painter->fontMetrics().width( m_text ), painter->fontMetrics().height() );

    bool visible = viewport->screenCoordinates( m_location, x, y, pointRepeatNum, textSize, globeHidesPoint );

    if ( visible ) {
        // Draw all the x-repeat-instances of the point on the screen
        for( int it = 0; it < pointRepeatNum; ++it ) {
            painter->drawText( x[it], y, m_text );
        }
    }

    painter->restore();
}

}
