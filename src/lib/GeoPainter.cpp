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
