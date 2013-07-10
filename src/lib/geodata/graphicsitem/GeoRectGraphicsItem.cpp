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
          m_height( height ),
          m_isGeoProjected( false ),
          m_xRoundness( 0 ),
          m_yRoundness( 0 )
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

void GeoRectGraphicsItem::setRoundness( int xRoundness, int yRoundness )
{
    m_xRoundness = xRoundness;
    m_yRoundness = yRoundness;
}

int GeoRectGraphicsItem::xRoundness() const
{
    return m_xRoundness;
}

int GeoRectGraphicsItem::yRoundness() const
{
    return m_yRoundness;
}

void GeoRectGraphicsItem::setIsGeoProjected( bool isGeoProjected )
{
    m_isGeoProjected = isGeoProjected;
}

bool GeoRectGraphicsItem::isGeoProjected() const
{
    return m_isGeoProjected;
}

void GeoRectGraphicsItem::setViewport( const ViewportParams *viewport )
{
    Q_UNUSED( viewport );
}

void GeoRectGraphicsItem::paint( GeoPainter* painter ) const
{
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

    if ( m_xRoundness > 0 || m_yRoundness > 0) {
        // FIXME convert to pixels from width/height
        painter->drawRoundRect( m_origin, (int)m_width, (int)m_height,
                                m_xRoundness, m_yRoundness );
    } else {
        painter->drawRect( m_origin, m_width, m_height, m_isGeoProjected );
    }

    painter->restore();
}

}
