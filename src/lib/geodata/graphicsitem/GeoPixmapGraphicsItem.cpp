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

const QPixmap* GeoPixmapGraphicsItem::pixmap() const
{
    return m_pixmap;
}

void GeoPixmapGraphicsItem::setViewport( const ViewportParams *viewport )
{
}

void GeoPixmapGraphicsItem::paint( GeoPainter* painter ) const
{
    if ( !m_pixmap || m_pixmap->isNull() || !m_centerPosition.isValid() )
        return;

    painter->drawPixmap( m_centerPosition, *m_pixmap );
}

} /* Marble */
