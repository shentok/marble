//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Andrew Manson <g.real.ate@gmail.com>
//

#include "GeoLineStringGraphicsItem.h"

#include "GeoDataLineString.h"
#include "GeoDataLineStyle.h"
#include "GeoPainter.h"
#include "ViewportParams.h"
#include "GeoDataStyle.h"

namespace Marble
{

GeoLineStringGraphicsItem::GeoLineStringGraphicsItem( const GeoDataFeature *feature, const GeoDataLineString* lineString )
        : GeoGraphicsItem( feature ),
          m_lineString( lineString )
{
}

void GeoLineStringGraphicsItem::setLineString( const GeoDataLineString* lineString )
{
    m_lineString = lineString;
}

const GeoDataLatLonAltBox& GeoLineStringGraphicsItem::latLonAltBox() const
{
    return m_lineString->latLonAltBox();
}

void GeoLineStringGraphicsItem::paint( GeoPainter* painter, const ViewportParams* viewport )
{
    LabelPositionFlags label_position_flags = NoLabel;

    painter->save();

    if ( !style() ) {
        painter->setPen( QPen() );
    }
    else {
        QPen currentPen = painter->pen();

        if ( currentPen.color() != style()->lineStyle().paintedColor() )
            currentPen.setColor( style()->lineStyle().paintedColor() );

        if ( currentPen.widthF() != style()->lineStyle().width() ||
                style()->lineStyle().physicalWidth() != 0.0 ) {
            if ( float( viewport->radius() ) / EARTH_RADIUS * style()->lineStyle().physicalWidth() < style()->lineStyle().width() )
                currentPen.setWidthF( style()->lineStyle().width() );
            else
                currentPen.setWidthF( float( viewport->radius() ) / EARTH_RADIUS * style()->lineStyle().physicalWidth() );
        }
        else if ( style()->lineStyle().width() != 0.0 ) {
            currentPen.setWidthF( style()->lineStyle().width() );
        }

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

        // label styles
        painter->setFont( style()->labelStyle().font() );
        if ( style()->labelStyle().alignment() == GeoDataLabelStyle::Corner )
            label_position_flags |= LineStart;
        if ( style()->labelStyle().alignment() == GeoDataLabelStyle::Center )
            label_position_flags |= LineCenter;
    }

    // Immediately leave this method now if:
    // - the object is not visible in the viewport or if
    // - the size of the object is below the resolution of the viewport
    if ( ! viewport->viewLatLonAltBox().intersects( m_lineString->latLonAltBox() ) ||
         ! viewport->resolves( m_lineString->latLonAltBox() )
        )
    {
        // mDebug() << "LineString doesn't get displayed on the viewport";
        return;
    }

    QVector<QPolygonF*> polygons;
    viewport->screenCoordinates( *m_lineString, polygons );

    if ( feature()->name().isEmpty() ) {
        foreach( QPolygonF* itPolygon, polygons ) {
            painter->drawPolyline( *itPolygon );
        }
    }
    else {
        int labelWidth = painter->fontMetrics().width( feature()->name() );
        int labelAscent = painter->fontMetrics().ascent();

        foreach( QPolygonF* itPolygon, polygons ) {
            QVector<QPointF> labelNodes;
            painter->drawPolyline( *itPolygon, labelNodes, label_position_flags );
            foreach ( const QPointF& labelNode, labelNodes ) {
                QPointF labelPosition = labelNode + QPointF( 3.0, -2.0 );

                // FIXME: This is a Q&D fix.
                qreal xmax = viewport->width() - 10.0 - labelWidth;
                if ( labelPosition.x() > xmax )
                    labelPosition.setX( xmax );
                qreal ymin = 10.0 + labelAscent;
                if ( labelPosition.y() < ymin )
                    labelPosition.setY( ymin );
                qreal ymax = viewport->height() - 10.0 - labelAscent;
                if ( labelPosition.y() > ymax )
                    labelPosition.setY( ymax );

                painter->drawText( labelPosition, feature()->name() );
            }
        }
    }

    qDeleteAll( polygons );

    painter->restore();
}

}
