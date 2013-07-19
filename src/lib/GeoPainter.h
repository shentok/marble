//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008-2009 Torsten Rahn <tackat@kde.org>
//

#ifndef MARBLE_GEOPAINTER_H
#define MARBLE_GEOPAINTER_H


#include "marble_export.h"

#include <QtCore/QSize>
#include <QtGui/QRegion>

// Marble
#include "MarbleGlobal.h"
#include "ClipPainter.h"


class QImage;
class QPaintDevice;
class QPolygonF;
class QRect;
class QRectF;
class QString;


namespace Marble
{

class ViewportParams;
class GeoPainterPrivate;
class GeoDataCoordinates;
class GeoDataLineString;
class GeoDataLinearRing;
class GeoDataPoint;
class GeoDataPolygon;


/*!
    \class GeoPainter
    \brief A painter that allows to draw geometric primitives on the map.

    This class allows application developers to draw simple geometric shapes
    and objects onto the map.

    The API is modeled after the QPainter API.

    The GeoPainter provides a wide range of methods that are using geographic
    ("geodesic") coordinates to position the item.
    For example a point or the nodes of a polygon can fully be described in
    geographic coordinates.

    In all these cases the position of the object is specified in geographic
    coordinates.

    There are however some cases where there are two viable cases:
    \li the shape of the object could still use screen coordinates (like a label
    or an icon).
    \li Alternatively the shape of the object can get projected according
    to the current projection (e.g. a texture projected onto the spherical
    surface)
    
    If screen coordinates are used then e.g. width and height are assumed to be
    expressed in pixels, otherwise degrees are used.

    Painter transformations (e.g. translate) always happen in screen
    coordinates.

    Like in QPainter drawing objects onto a widget should always considered to
    be a volatile operation. This means that e.g. placemarks don't get added to
    the globe permanently.
    So the drawing needs to be done on every repaint to prevent that drawings
    will disappear during the next paint event.

    So if you want to add placemarks to your map widget permanently (i.e. you
    don't want to take care of repainting) then you need to use other solutions
    such as the KML import of the Marble framework or Marble's GeoGraphicsItems.

    \note By default the GeoPainter automatically filters geographical content
    in order to provide fast painting:
    \li Geographically positioned objects which are outside the viewport are not
    drawn at all.
    Parts of objects which are specified through geographic coordinates
    (like polygons, ellipses etc.) get cut off if they are not placed within the
    viewport.
    \li Objects which have a shape that is specified through geographic
    coordinates get filtered according to the viewport resolution:
    If the object is much smaller than a pixel then it won't get drawn at all.
*/


class MARBLE_EXPORT GeoPainter : public ClipPainter
{
 public:
     
/*!
    \brief Creates a new geo painter.

    To create a geo painter it's necessary to provide \a paintDevice
    as a canvas and the viewportParams to specify the map projection
    inside the viewport.
*/
    GeoPainter( QPaintDevice * paintDevice,
                const ViewportParams *viewportParams,
                MapQuality mapQuality = NormalQuality );

                
/*!
    \brief Destroys the geo painter.
*/
    ~GeoPainter();

    
/*!
    \brief Returns the map quality.
    \return The map quality that got assigned to the painter.
*/
    MapQuality mapQuality() const;


/*!
    \brief Creates a region for an ellipse at a given position

    A QRegion object is created that represents the area covered by
    GeoPainter::drawEllipse(). As such it can be used e.g. for input event
    handling for objects that have been painted using GeoPainter::drawEllipse().

    The \a strokeWidth allows to extrude the QRegion by half the amount of
    "stroke width" pixels. For input event handling it's always advisable to use
    a width that is slightly bigger than the width of the painter's pen.

    \see GeoDataCoordinates
*/
    QRegion regionFromEllipse ( const GeoDataCoordinates & centerPosition,
                                qreal width, qreal height,
                                qreal strokeWidth = 3 ) const;


/*!
    \brief Draws a pixmap at the given position.
    The pixmap is placed with its center located at the given \a centerPosition.

    The image rendering is performed in screen coordinates and is
    not subject to the current projection.

    \see GeoDataCoordinates
*/
    void drawPixmap ( const GeoDataCoordinates & centerPosition,
                      const QPixmap & pixmap /*, bool isGeoProjected = false */ );


/*!
    \brief Creates a region for a given line string (a "polyline").

    A QRegion object is created that represents the area covered by
    GeoPainter::drawPolyline( GeoDataLineString ). As such it can be used
    e.g. for input event handling for objects that have been painted using
    GeoPainter::drawPolyline( GeoDataLineString ).

    The \a strokeWidth allows to extrude the QRegion by half the amount of
    "stroke width" pixels. For input event handling it's always advisable to use
    a width that is slightly bigger than the width of the painter's pen.

    \see GeoDataLineString
*/
    QRegion regionFromPolyline ( const GeoDataLineString & lineString,
                                 qreal strokeWidth = 3 ) const;


/*!
    \brief Creates a region for a given linear ring (a "polygon without holes").

    A QRegion object is created that represents the area covered by
    GeoPainter::drawPolygon( GeoDataLinearRing ). As such it can be used
    e.g. for input event handling for objects that have been painted using
    GeoPainter::drawPolygon( GeoDataLinearRing ).

    Like in drawPolygon() the \a fillRule specifies the fill algorithm that is
    used to fill the polygon.

    The \a strokeWidth allows to extrude the QRegion by half the amount of
    "stroke width" pixels. For input event handling it's always advisable to use
    a width that is slightly bigger than the width of the painter's pen.

    For the polygon case a "cosmetic" strokeWidth of zero should provide the
    best performance.

    \see GeoDataLinearRing
*/
    QRegion regionFromPolygon ( const GeoDataLinearRing & linearRing,
                                Qt::FillRule fillRule, qreal strokeWidth = 3 ) const;


/*!
    \brief Creates a region for a rectangle at a given position.

    A QRegion object is created that represents the area covered by
    GeoPainter::drawRect(). This can be used e.g. for input event handling
    for objects that have been painted using GeoPainter::drawRect().

    The \a strokeWidth allows to extrude the QRegion by half the amount of
    "stroke width" pixels. For input event handling it's always advisable to use
    a width that is slightly bigger than the width of the painter's pen. This is
    especially true for small objects.

    \see GeoDataCoordinates
*/
    QRegion regionFromRect ( const GeoDataCoordinates & centerPosition,
                             qreal width, qreal height,
                             qreal strokeWidth = 3 ) const;



    // Reenabling QPainter+ClipPainter methods.
    using QPainter::drawPixmap;

 private:
    Q_DISABLE_COPY( GeoPainter )
    GeoPainterPrivate  * const d;
};

}

#endif
