//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013      Rene Kuettner <rene@bitkanal.net>
//

#ifndef MARBLE_GEORECTGRAPHICSITEM_H
#define MARBLE_GEORECTGRAPHICSITEM_H

#include "GeoGraphicsItem.h"
#include "GeoDataLatLonAltBox.h"
#include "GeoDataCoordinates.h"
#include "marble_export.h"

namespace Marble
{

class GeoDataLineStyle;

class MARBLE_EXPORT GeoRectGraphicsItem : public GeoGraphicsItem
{
public:
    explicit GeoRectGraphicsItem( const GeoDataFeature *feature,
                                  const GeoDataCoordinates &origin,
                                  const qreal width, const qreal height );

    virtual const GeoDataLatLonAltBox& latLonAltBox() const;

    void setOrigin( const GeoDataCoordinates &origin );

    const GeoDataCoordinates origin() const;

    void setWidth( qreal width );

    qreal width() const;

    void setHeight( qreal height );

    qreal height() const;

    void setRoundness( int xRoundness, int yRoundness );

    int xRoundness() const;

    int yRoundness() const;

    void setIsGeoProjected( bool isGeoProjected );

    bool isGeoProjected() const;

    void setViewport( const ViewportParams *viewport );

    void paint( GeoPainter* painter ) const;

protected:
    GeoDataLatLonAltBox m_latLonAltBox;
    GeoDataCoordinates m_origin;
    qreal m_width;
    qreal m_height;
    bool m_isGeoProjected;
    int m_xRoundness;
    int m_yRoundness;
};

}

#endif
