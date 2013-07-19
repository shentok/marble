//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013      Rene Küttner <rene@bitkanal.net>
//

#include "GeoPixmapGraphicsItem.h"

#include "GeoGraphicsItem_p.h"
#include "GeoPainter.h"
#include "ViewportParams.h"

namespace Marble
{

GeoPixmapGraphicsItem::GeoPixmapGraphicsItem( const GeoDataFeature *feature,
                                              const QPixmap *pixmap,
                                              const GeoDataCoordinates &center )
    : GeoGraphicsItem( feature ),
      m_pixmap( pixmap ),
      m_centerPosition( center )
{
}

/*const GeoDataLatLonBox& GeoPixmapGraphicsItem::latLonBox() const
{
    return p()->m_latLonAltBox;
}*/


void GeoPixmapGraphicsItem::setCoordinates( const GeoDataCoordinates &coordinates )
{
    m_centerPosition = coordinates;
}

void GeoPixmapGraphicsItem::paint( GeoPainter* painter, const ViewportParams *viewport )
{
    Q_UNUSED( viewport )

    if ( !m_pixmap || m_pixmap->isNull() || !m_centerPosition.isValid() )
        return;

    int pointRepeatNum;
    qreal x[100];
    qreal y;
    bool globeHidesPoint;

//    if ( !isGeoProjected ) {
        // FIXME: Better visibility detection that takes the circle geometry into account
        bool visible = viewport->screenCoordinates( m_centerPosition, x, y, pointRepeatNum, m_pixmap->size(), globeHidesPoint );

        if ( visible ) {
            // Draw all the x-repeat-instances of the point on the screen
            for( int it = 0; it < pointRepeatNum; ++it ) {
                painter->drawPixmap( x[it] - ( m_pixmap->width() / 2 ), y - ( m_pixmap->height() / 2 ), *m_pixmap );
            }
        }
        //    }
}

} /* Marble */
