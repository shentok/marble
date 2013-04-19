//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2006-2009 Torsten Rahn <tackat@kde.org>


#include "GeoPainter.h"
#include "GeoPainter_p.h"

#include <QtCore/QList>
#include <QtGui/QPainterPath>
#include <QtGui/QRegion>

#include "MarbleDebug.h"

#include "GeoDataCoordinates.h"
#include "GeoDataLineString.h"
#include "GeoDataLinearRing.h"
#include "GeoDataPoint.h"
#include "GeoDataPolygon.h"

#include "MarbleGlobal.h"
#include "ViewportParams.h"

// #define MARBLE_DEBUG

using namespace Marble;

GeoPainterPrivate::GeoPainterPrivate( const ViewportParams *viewport, MapQuality mapQuality )
        : m_viewport( viewport ),
        m_mapQuality( mapQuality ),
        m_x( new qreal[100] )
{
}

GeoPainterPrivate::~GeoPainterPrivate()
{
    delete[] m_x;
}

bool GeoPainterPrivate::doClip( const ViewportParams *viewport )
{
    if ( viewport->projection() != Spherical )
        return true;

    return ( viewport->radius() > viewport->width() / 2 || viewport->radius() > viewport->height() / 2 );
}

// -------------------------------------------------------------------------------------------------

GeoPainter::GeoPainter( QPaintDevice* pd, const ViewportParams *viewport, MapQuality mapQuality )
    : ClipPainter( pd, GeoPainterPrivate::doClip( viewport ) ),
      d( new GeoPainterPrivate( viewport, mapQuality ) )
{
    const bool antialiased = mapQuality == HighQuality || mapQuality == PrintQuality;
    setRenderHint( QPainter::Antialiasing, antialiased );
}


GeoPainter::~GeoPainter()
{
    delete d;
}


MapQuality GeoPainter::mapQuality() const
{
    return d->m_mapQuality;
}


void GeoPainter::drawText ( const GeoDataCoordinates & position,
                            const QString & text )
{
    // Of course in theory we could have the "isGeoProjected" parameter used
    // for drawText as well. However this would require us to convert all
    // glyphs to PainterPaths / QPolygons. From QPolygons we could create
    // GeoDataPolygons which could get painted on screen. Any patches appreciated ;-)

    int pointRepeatNum;
    qreal y;
    bool globeHidesPoint;

    QSizeF textSize( fontMetrics().width( text ), fontMetrics().height() );  

    bool visible = d->m_viewport->screenCoordinates( position, d->m_x, y, pointRepeatNum, textSize, globeHidesPoint );

    if ( visible ) {
        // Draw all the x-repeat-instances of the point on the screen
        for( int it = 0; it < pointRepeatNum; ++it ) {
            QPainter::drawText( d->m_x[it], y, text );
        }
    }
}


void GeoPainter::drawEllipse ( const GeoDataCoordinates & centerPosition,
                               qreal width, qreal height )
{
        int pointRepeatNum;
        qreal y;
        bool globeHidesPoint;

        bool visible = d->m_viewport->screenCoordinates( centerPosition, d->m_x, y, pointRepeatNum, QSizeF( width, height ), globeHidesPoint );

        if ( visible ) {
            // Draw all the x-repeat-instances of the point on the screen
            for( int it = 0; it < pointRepeatNum; ++it ) {
                QPainter::drawEllipse(  d->m_x[it] - width / 2.0,
                                        y - height / 2.0, width, height  );
            }
        }
}


QRegion GeoPainter::regionFromEllipse ( const GeoDataCoordinates & centerPosition,
                                        qreal width, qreal height,
                                        qreal strokeWidth ) const
{
        int pointRepeatNum;
        qreal y;
        bool globeHidesPoint;

        bool visible = d->m_viewport->screenCoordinates( centerPosition, d->m_x, y, pointRepeatNum, QSizeF( width, height ), globeHidesPoint );

        QRegion regions;

        if ( visible ) {
            // Draw all the x-repeat-instances of the point on the screen
            for( int it = 0; it < pointRepeatNum; ++it ) {
                regions += QRegion( d->m_x[it] - width / 2.0,
                                    y - height / 2.0,
                                    width + strokeWidth,
                                    height + strokeWidth,
                                    QRegion::Ellipse );
            }
        }
        return regions;
}


void GeoPainter::drawImage ( const GeoDataCoordinates & centerPosition,
                             const QImage & image /*, bool isGeoProjected */ )
{
    // isGeoProjected = true would project the image/pixmap onto the globe. This
    // requires to deal with the TextureMapping classes -> should get
    // implemented later on

    int pointRepeatNum;
    qreal y;
    bool globeHidesPoint;

//    if ( !isGeoProjected ) {
        bool visible = d->m_viewport->screenCoordinates( centerPosition, d->m_x, y, pointRepeatNum, image.size(), globeHidesPoint );

        if ( visible ) {
            // Draw all the x-repeat-instances of the point on the screen
            for( int it = 0; it < pointRepeatNum; ++it ) {
                QPainter::drawImage( d->m_x[it] - ( image.width() / 2 ), y - ( image.height() / 2 ), image );
            }
        }
//    }
}


void GeoPainter::drawPixmap ( const GeoDataCoordinates & centerPosition,
                              const QPixmap & pixmap /* , bool isGeoProjected */ )
{
    int pointRepeatNum;
    qreal y;
    bool globeHidesPoint;

//    if ( !isGeoProjected ) {
        // FIXME: Better visibility detection that takes the circle geometry into account
        bool visible = d->m_viewport->screenCoordinates( centerPosition, d->m_x, y, pointRepeatNum, pixmap.size(), globeHidesPoint );

        if ( visible ) {
            // Draw all the x-repeat-instances of the point on the screen
            for( int it = 0; it < pointRepeatNum; ++it ) {
                QPainter::drawPixmap( d->m_x[it] - ( pixmap.width() / 2 ),
                                      y - ( pixmap.height() / 2 ), pixmap );
            }
        }
//    }
}


void GeoPainter::drawPolyline ( const GeoDataLineString & lineString,
                                const QString& labelText,
                                LabelPositionFlags labelPositionFlags )
{
    // Immediately leave this method now if:
    // - the object is not visible in the viewport or if
    // - the size of the object is below the resolution of the viewport
    if ( ! d->m_viewport->viewLatLonAltBox().intersects( lineString.latLonAltBox() ) ||
         ! d->m_viewport->resolves( lineString.latLonAltBox() )
        )
    {
        // mDebug() << "LineString doesn't get displayed on the viewport";
        return;
    }

    QVector<QPolygonF*> polygons;
    d->m_viewport->screenCoordinates( lineString, polygons );

    if ( labelText.isEmpty() ) {
        foreach( QPolygonF* itPolygon, polygons ) {
            ClipPainter::drawPolyline( *itPolygon );
        }
    }
    else {
        int labelWidth = fontMetrics().width( labelText );
        int labelAscent = fontMetrics().ascent();

        QVector<QPointF> labelNodes;
        foreach( QPolygonF* itPolygon, polygons ) {
            labelNodes.clear();
            ClipPainter::drawPolyline( *itPolygon, labelNodes, labelPositionFlags );
            if ( !labelNodes.isEmpty() ) {
                foreach ( const QPointF& labelNode, labelNodes ) {
                    QPointF labelPosition = labelNode + QPointF( 3.0, -2.0 );

                    // FIXME: This is a Q&D fix.
                    qreal xmax = viewport().width() - 10.0 - labelWidth;
                    if ( labelPosition.x() > xmax ) labelPosition.setX( xmax ); 
                    qreal ymin = 10.0 + labelAscent;
                    if ( labelPosition.y() < ymin ) labelPosition.setY( ymin );
                    qreal ymax = viewport().height() - 10.0 - labelAscent;
                    if ( labelPosition.y() > ymax ) labelPosition.setY( ymax );
 
                    drawText( labelPosition, labelText );
                }
            }
        }
    }
    qDeleteAll( polygons );
}


QRegion GeoPainter::regionFromPolyline ( const GeoDataLineString & lineString,
                                         qreal strokeWidth ) const
{
    // Immediately leave this method now if:
    // - the object is not visible in the viewport or if
    // - the size of the object is below the resolution of the viewport
    if ( ! d->m_viewport->viewLatLonAltBox().intersects( lineString.latLonAltBox() ) ||
         ! d->m_viewport->resolves( lineString.latLonAltBox() )
        )
    {
        // mDebug() << "LineString doesn't get displayed on the viewport";
        return QRegion();
    }

    QList<QRegion> regions;
    QPainterPath painterPath;

    QVector<QPolygonF*> polygons;
    d->m_viewport->screenCoordinates( lineString, polygons );

    foreach( QPolygonF* itPolygon, polygons ) {
        painterPath.addPolygon( *itPolygon );
    }

    qDeleteAll( polygons );

    QPainterPathStroker stroker;
    stroker.setWidth( strokeWidth );
    QPainterPath strokePath = stroker.createStroke( painterPath );

    return QRegion( strokePath.toFillPolygon().toPolygon(), Qt::WindingFill );
}


void GeoPainter::drawPolygon ( const GeoDataLinearRing & linearRing,
                               Qt::FillRule fillRule )
{
    // Immediately leave this method now if:
    // - the object is not visible in the viewport or if
    // - the size of the object is below the resolution of the viewport
    if ( ! d->m_viewport->viewLatLonAltBox().intersects( linearRing.latLonAltBox() ) ||
         ! d->m_viewport->resolves( linearRing.latLonAltBox() )
        )
    {
        // mDebug() << "Polygon doesn't get displayed on the viewport";
        return;
    }

    QVector<QPolygonF*> polygons;
    d->m_viewport->screenCoordinates( linearRing, polygons );

    foreach( QPolygonF* itPolygon, polygons ) {
        ClipPainter::drawPolygon( *itPolygon, fillRule );
    }

    qDeleteAll( polygons );
}


QRegion GeoPainter::regionFromPolygon ( const GeoDataLinearRing & linearRing,
                                        Qt::FillRule fillRule, qreal strokeWidth ) const
{
    // Immediately leave this method now if:
    // - the object is not visible in the viewport or if
    // - the size of the object is below the resolution of the viewport
    if ( ! d->m_viewport->viewLatLonAltBox().intersects( linearRing.latLonAltBox() ) ||
         ! d->m_viewport->resolves( linearRing.latLonAltBox() )
        )
    {
        return QRegion();
    }

    QRegion regions;

    QVector<QPolygonF*> polygons;
    d->m_viewport->screenCoordinates( linearRing, polygons );

    if ( strokeWidth == 0 ) {
        // This is the faster way
        foreach( QPolygonF* itPolygon, polygons ) {
            regions += QRegion ( (*itPolygon).toPolygon(), fillRule );
        }
    }
    else {
        QPainterPath painterPath;
        foreach( QPolygonF* itPolygon, polygons ) {
            painterPath.addPolygon( *itPolygon );
        }

        QPainterPathStroker stroker;
        stroker.setWidth( strokeWidth );
        QPainterPath strokePath = stroker.createStroke( painterPath );
        painterPath = painterPath.united( strokePath );
        regions = QRegion( painterPath.toFillPolygon().toPolygon() );
    }

    qDeleteAll( polygons );

    return regions;
}


void GeoPainter::drawPolygon ( const GeoDataPolygon & polygon,
                               Qt::FillRule fillRule )
{
    // If the object is not visible in the viewport return 
    if ( ! d->m_viewport->viewLatLonAltBox().intersects( polygon.outerBoundary().latLonAltBox() ) ||
    // If the size of the object is below the resolution of the viewport then return
         ! d->m_viewport->resolves( polygon.outerBoundary().latLonAltBox() )
        )
    {
        // mDebug() << "Polygon doesn't get displayed on the viewport";
        return;
    }
    // mDebug() << "Drawing Polygon";

    // Creating the outer screen polygons first
    QVector<QPolygonF*> outerPolygons;
    d->m_viewport->screenCoordinates( polygon.outerBoundary(), outerPolygons );

    // Now creating the "holes" by cutting away the inner boundaries:

    // In QPathClipper We Trust ...
    // ... and in the speed of a threesome of nested foreachs!

    QVector<QPolygonF> outline;
    QPen const oldPen = pen();
    // When inner boundaries exist, the outline of the polygon must be painted
    // separately to avoid connections between the outer and inner boundaries
    // To avoid performance penalties the separate painting is only done when
    // it's really needed. See review 105019 for details.
    bool const needOutlineWorkaround = !polygon.innerBoundaries().isEmpty();
    if ( needOutlineWorkaround ) {
        foreach( QPolygonF* polygon, outerPolygons ) {
            outline << *polygon;
        }
        setPen( QPen( Qt::NoPen ) );
    }


    QVector<GeoDataLinearRing> innerBoundaries = polygon.innerBoundaries(); 
    foreach( const GeoDataLinearRing& itInnerBoundary, innerBoundaries ) {
        QVector<QPolygonF*> innerPolygons;
        d->m_viewport->screenCoordinates( itInnerBoundary, innerPolygons );

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
        ClipPainter::drawPolygon( *itOuterPolygon, fillRule );
    }

    if ( needOutlineWorkaround ) {
        setPen( oldPen );
        foreach( const QPolygonF &polygon, outline ) {
            ClipPainter::drawPolyline( polygon );
        }
    }

    qDeleteAll( outerPolygons );    
}


void GeoPainter::drawRect ( const GeoDataCoordinates & centerCoordinates,
                            qreal width, qreal height )
{
        int pointRepeatNum;
        qreal y;
        bool globeHidesPoint;

        bool visible = d->m_viewport->screenCoordinates( centerCoordinates,
                       d->m_x, y, pointRepeatNum, QSizeF( width, height ), globeHidesPoint );

        if ( visible ) {
            // Draw all the x-repeat-instances of the point on the screen
            for( int it = 0; it < pointRepeatNum; ++it ) {
                QPainter::drawRect( d->m_x[it] - ( width / 2.0 ), y - ( height / 2.0 ), width, height );
            }
        }
}


QRegion GeoPainter::regionFromRect ( const GeoDataCoordinates & centerCoordinates,
                                     qreal width, qreal height,
                                     qreal strokeWidth ) const
{
        int pointRepeatNum;
        qreal y;
        bool globeHidesPoint;

        bool visible = d->m_viewport->screenCoordinates( centerCoordinates,
                       d->m_x, y, pointRepeatNum, QSizeF( width, height ), globeHidesPoint );

        QRegion regions;

        if ( visible ) {
            // Draw all the x-repeat-instances of the point on the screen
            for( int it = 0; it < pointRepeatNum; ++it ) {
                regions += QRegion( d->m_x[it] - ( ( width + strokeWidth ) / 2.0 ),
                                    y - ( ( height + strokeWidth ) / 2.0 ),
                                    width + strokeWidth,
                                    height + strokeWidth );
            }
        }
        return regions;
}


void GeoPainter::drawRoundRect ( const GeoDataCoordinates &centerPosition,
                                 int width, int height,
                                 int xRnd, int yRnd )
{
        int pointRepeatNum;
        qreal y;
        bool globeHidesPoint;

        // FIXME: Better visibility detection that takes the circle geometry into account
        bool visible = d->m_viewport->screenCoordinates( centerPosition, d->m_x, y, pointRepeatNum, QSizeF( width, height ), globeHidesPoint );

        if ( visible ) {
            // Draw all the x-repeat-instances of the point on the screen
            for( int it = 0; it < pointRepeatNum; ++it ) {
                QPainter::drawRoundRect( d->m_x[it] - ( width / 2 ), y - ( height / 2 ), width, height, xRnd, yRnd );
            }
        }
}
