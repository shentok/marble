//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013      Rene Kuettner <rene@bitkanal.net>
//

#include "GeoRectGraphicsItem.h"

#include "GeoDataLineStyle.h"
#include "GeoDataCoordinates.h"
#include "GeoPainter.h"
#include "ViewportParams.h"
#include "GeoDataStyle.h"

namespace Marble
{

GeoRectGraphicsItem::GeoRectGraphicsItem( const GeoDataFeature *feature,
                                          const GeoDataCoordinates &origin,
                                          qreal width, qreal height )
        : GeoGraphicsItem( feature ),
          m_latLonAltBox( origin ),
          m_origin( origin ),
          m_width( width ),
          m_height( height )
{
}

const GeoDataLatLonAltBox& GeoRectGraphicsItem::latLonAltBox() const
{
    // FIXME update latlonaltbox
    return m_latLonAltBox;
}

void GeoRectGraphicsItem::setOrigin( const GeoDataCoordinates &origin )
{
    m_origin = origin;
}

const GeoDataCoordinates GeoRectGraphicsItem::origin() const
{
    return m_origin;
}

void GeoRectGraphicsItem::setWidth( qreal width )
{
    m_width = width;
}

qreal GeoRectGraphicsItem::width() const
{
    return m_width;
}

void GeoRectGraphicsItem::setHeight( qreal height )
{
    m_height = height;
}

qreal GeoRectGraphicsItem::height() const
{
    return m_height;
}

void GeoRectGraphicsItem::paint( GeoPainter* painter, const ViewportParams *viewport )
{
    Q_UNUSED( viewport );

    painter->save();

    if ( !style() ) {
        painter->setPen( QPen() );
    }
    else {
        QPen currentPen = painter->pen();

        if ( currentPen.color() != style()->lineStyle().paintedColor() )
            currentPen.setColor( style()->lineStyle().paintedColor() );

        currentPen.setWidthF( style()->lineStyle().width() );

        if ( currentPen.capStyle() != style()->lineStyle().capStyle() )
            currentPen.setCapStyle( style()->lineStyle().capStyle() );

        if ( currentPen.style() != style()->lineStyle().penStyle() )
            currentPen.setStyle( style()->lineStyle().penStyle() );

        if ( style()->lineStyle().penStyle() == Qt::CustomDashLine )
            currentPen.setDashPattern( style()->lineStyle().dashPattern() );

        if ( painter->mapQuality() != Marble::HighQuality
                && painter->mapQuality() != Marble::PrintQuality ) {
            QColor penColor = currentPen.color();
            penColor.setAlpha( 255 );
            currentPen.setColor( penColor );
        }

        if ( painter->pen() != currentPen )
            painter->setPen( currentPen );

        if ( style()->lineStyle().background() ) {
            QBrush brush = painter->background();
            brush.setColor( style()->polyStyle().paintedColor() );
            painter->setBackground( brush );

            painter->setBackgroundMode( Qt::OpaqueMode );
        }
    }

    int pointRepeatNum;
    qreal x[100];
    qreal y;
    bool globeHidesPoint;

    bool visible = viewport->screenCoordinates( m_origin, x, y, pointRepeatNum, QSizeF( m_width, m_height ), globeHidesPoint );

    if ( visible ) {
        // Draw all the x-repeat-instances of the point on the screen
        for( int it = 0; it < pointRepeatNum; ++it ) {
            painter->drawRect( x[it] - ( m_width / 2.0 ), y - ( m_height / 2.0 ), m_width, m_height );
        }
    }

    painter->restore();
}

}
