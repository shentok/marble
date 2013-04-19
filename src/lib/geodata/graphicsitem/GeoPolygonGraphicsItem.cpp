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

#include "GeoDataLinearRing.h"
#include "GeoDataPolygon.h"
#include "GeoPainter.h"
#include "ViewportParams.h"
#include "GeoDataStyle.h"

namespace Marble
{

GeoPolygonGraphicsItem::GeoPolygonGraphicsItem( const GeoDataFeature *feature, const GeoDataPolygon* polygon )
        : GeoGraphicsItem( feature ),
          m_polygon( polygon ),
          m_ring( 0 )
{
}

GeoPolygonGraphicsItem::GeoPolygonGraphicsItem( const GeoDataFeature *feature, const GeoDataLinearRing* ring )
        : GeoGraphicsItem( feature ),
          m_polygon( 0 ),
          m_ring( ring )
{
}

const GeoDataLatLonAltBox& GeoPolygonGraphicsItem::latLonAltBox() const
{
    if( m_polygon ) {
        return m_polygon->latLonAltBox();
    } else if ( m_ring ) {
        return m_ring->latLonAltBox();
    } else {
        return GeoGraphicsItem::latLonAltBox();
    }
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

    if ( m_polygon ) {
        // If the object is not visible in the viewport return
        if ( ! viewport->viewLatLonAltBox().intersects( m_polygon->outerBoundary().latLonAltBox() ) ||
        // If the size of the object is below the resolution of the viewport then return
             ! viewport->resolves( m_polygon->outerBoundary().latLonAltBox() )
            )
        {
            // mDebug() << "Polygon doesn't get displayed on the viewport";
            return;
        }
        // mDebug() << "Drawing Polygon";

        // Creating the outer screen polygons first
        QVector<QPolygonF*> outerPolygons;
        viewport->screenCoordinates( m_polygon->outerBoundary(), outerPolygons );

        // Now creating the "holes" by cutting away the inner boundaries:

        // In QPathClipper We Trust ...
        // ... and in the speed of a threesome of nested foreachs!

        QVector<QPolygonF> outline;
        QPen const oldPen = painter->pen();
        // When inner boundaries exist, the outline of the polygon must be painted
        // separately to avoid connections between the outer and inner boundaries
        // To avoid performance penalties the separate painting is only done when
        // it's really needed. See review 105019 for details.
        bool const needOutlineWorkaround = !m_polygon->innerBoundaries().isEmpty();
        if ( needOutlineWorkaround ) {
            foreach( QPolygonF* polygon, outerPolygons ) {
                outline << *polygon;
            }
            painter->setPen( QPen( Qt::NoPen ) );
        }


        QVector<GeoDataLinearRing> innerBoundaries = m_polygon->innerBoundaries();
        foreach( const GeoDataLinearRing& itInnerBoundary, innerBoundaries ) {
            QVector<QPolygonF*> innerPolygons;
            viewport->screenCoordinates( itInnerBoundary, innerPolygons );

            if ( needOutlineWorkaround ) {
                foreach( QPolygonF* polygon, innerPolygons ) {
                    outline << *polygon;
                }
            }

            foreach( QPolygonF* itOuterPolygon, outerPolygons ) {
                foreach( QPolygonF* itInnerPolygon, innerPolygons ) {
                    *itOuterPolygon = itOuterPolygon->subtracted( *itInnerPolygon );
                }
            }
            qDeleteAll( innerPolygons );
        }

        foreach( QPolygonF* itOuterPolygon, outerPolygons ) {
            painter->drawPolygon( *itOuterPolygon, Qt::OddEvenFill );
        }

        if ( needOutlineWorkaround ) {
            painter->setPen( oldPen );
            foreach( const QPolygonF &polygon, outline ) {
                painter->drawPolyline( polygon );
            }
        }

        qDeleteAll( outerPolygons );
    } else if ( m_ring ) {
        // Immediately leave this method now if:
        // - the object is not visible in the viewport or if
        // - the size of the object is below the resolution of the viewport
        if ( ! viewport->viewLatLonAltBox().intersects( m_ring->latLonAltBox() ) ||
             ! viewport->resolves( m_ring->latLonAltBox() )
            )
        {
            // mDebug() << "Polygon doesn't get displayed on the viewport";
            return;
        }

        if ( !m_ring->latLonAltBox().crossesDateLine() ) {
            QVector<QPolygonF*> polygons;
            viewport->screenCoordinates( *m_ring, polygons );

            foreach( QPolygonF* itPolygon, polygons ) {
                painter->drawPolygon( *itPolygon, Qt::OddEvenFill );
            }

            qDeleteAll( polygons );
        }
        else {
            QPen polygonPen = painter->pen();
            painter->setPen( Qt::NoPen );

            QVector<QPolygonF*> polygons;
            viewport->screenCoordinates( *m_ring, polygons );

            foreach( QPolygonF* itPolygon, polygons ) {
                painter->drawPolygon( *itPolygon, Qt::OddEvenFill );
            }

            qDeleteAll( polygons );

            painter->setPen( polygonPen );
            GeoDataLineString lineString( *m_ring );

            lineString << lineString.first();

            QVector<QPolygonF*> polylines;
            viewport->screenCoordinates( lineString, polylines );

            foreach( QPolygonF* itPolygon, polylines ) {
                painter->drawPolyline( *itPolygon );
            }

            qDeleteAll( polylines );
        }
    }

    painter->restore();
}

}
