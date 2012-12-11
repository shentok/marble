//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2012      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#ifndef MARBLE_GLRENDERER_H
#define MARBLE_GLRENDERER_H

// Marble
#include "marble_export.h"

class QGLShaderProgram;
class QImage;

namespace Marble
{

class GeoDataLatLonBox;
class GeoDataLinearRing;
class GeoDataLineString;
class GeoDataPoint;
class GeoDataPolygon;
class GeoDataStyle;

class MARBLE_EXPORT GLRenderer
{
 public:
    GLRenderer();
    virtual ~GLRenderer();

    void addLineString( const GeoDataLineString &lineString, const GeoDataStyle &style );

    void addPolygon( const GeoDataPolygon &polygon, const GeoDataStyle &style );

    void addPolygon( const GeoDataLinearRing &linearRing, const GeoDataStyle &style );

    void addImage( const QImage &image, const GeoDataLatLonBox &boundingBox );

    void addPhoto( const QImage &image, const GeoDataPoint &point );

    virtual void paintGL( QGLShaderProgram *program );

 private:
    class Private;
    Private *const d;
};

} // namespace Marble

#endif
