//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011      Konstantin Oblaukhov <oblaukhov.konstantin@gmail.com>
//

#include "GeoLinearRingGraphicsItem.h"

#include "GeoDataLinearRing.h"
#include "GeoPainter.h"
#include "ViewportParams.h"
#include "GeoDataStyle.h"

namespace Marble
{

GeoLinearRingGraphicsItem::GeoLinearRingGraphicsItem( const GeoDataFeature *feature, const GeoDataLinearRing* ring )
        : GeoGraphicsItem( feature ),
          m_ring( ring )
{
}

const GeoDataLatLonAltBox& GeoLinearRingGraphicsItem::latLonAltBox() const
{
    return m_ring->latLonAltBox();
}

void GeoLinearRingGraphicsItem::paint( GeoPainter* painter, const ViewportParams* viewport )
{
    Q_UNUSED( viewport );

    painter->save();

    if ( !style() ) {
        painter->setPen( QPen() );
    }
    else {
        QPen currentPen = painter->pen();

        if ( currentPen.color() != style()->lineStyle().paintedColor() ||
                currentPen.widthF() != style()->lineStyle().width() ) {
            currentPen.setColor( style()->lineStyle().paintedColor() );
            currentPen.setWidthF( style()->lineStyle().width() );
        }

        if ( currentPen.capStyle() != style()->lineStyle().capStyle() )
            currentPen.setCapStyle( style()->lineStyle().capStyle() );

        if ( currentPen.style() != style()->lineStyle().penStyle() )
            currentPen.setStyle( style()->lineStyle().penStyle() );

        if ( painter->mapQuality() != Marble::HighQuality
                && painter->mapQuality() != Marble::PrintQuality ) {
            QColor penColor = currentPen.color();
            penColor.setAlpha( 255 );
            currentPen.setColor( penColor );
        }

        if ( painter->pen() != currentPen )
            painter->setPen( currentPen );
    }

    painter->drawPolygon( *m_ring );

    painter->restore();
}

}
