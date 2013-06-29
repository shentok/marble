//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011      Konstantin Oblaukhov <oblaukhov.konstantin@gmail.com>
//

#include "GeoPolygonGraphicsItem.h"

#include "GeoDataPolygon.h"
#include "GeoPainter.h"
#include "ViewportParams.h"
#include "GeoDataStyle.h"

namespace Marble
{

GeoPolygonGraphicsItem::GeoPolygonGraphicsItem( const GeoDataFeature *feature, const GeoDataPolygon* polygon )
        : GeoGraphicsItem( feature ),
          m_polygon( polygon )
{
}

const GeoDataLatLonAltBox& GeoPolygonGraphicsItem::latLonAltBox() const
{
    return m_polygon->latLonAltBox();
}

void GeoPolygonGraphicsItem::paint( GeoPainter* painter, const ViewportParams* viewport )
{
    Q_UNUSED( viewport );

    painter->save();

    if ( !style() ) {
        painter->setPen( QPen() );
    }
    else {
        QPen currentPen = painter->pen();

        if ( !style()->polyStyle().outline() ) {
            currentPen.setColor( Qt::transparent );
        }
        else {
            if ( painter->mapQuality() != Marble::HighQuality
                    && painter->mapQuality() != Marble::PrintQuality ) {
                QColor penColor = currentPen.color();
                penColor.setAlpha( 255 );
                currentPen.setColor( penColor );
            }
        }

        if ( painter->pen() != currentPen )
            painter->setPen( currentPen );

        if ( !style()->polyStyle().fill() ) {
            if ( painter->brush().color() != Qt::transparent )
                painter->setBrush( QColor( Qt::transparent ) );
        }
        else {
            if ( painter->brush().color() != style()->polyStyle().paintedColor() ) {
                painter->setBrush( style()->polyStyle().paintedColor() );
            }
        }
    }

    painter->drawPolygon( *m_polygon );

    painter->restore();
}

}
