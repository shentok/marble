//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Andrew Manson <g.real.ate@gmail.com>
//

#include "GeoPointGraphicsItem.h"
#include "GeoDataStyle.h"
#include "GeoDataIconStyle.h"

#include "GeoPainter.h"

namespace Marble
{

GeoPointGraphicsItem::GeoPointGraphicsItem( const GeoDataFeature *feature,
                                            const GeoDataPoint &point )
        : GeoGraphicsItem( feature ),
          m_point( point )
{
}

void GeoPointGraphicsItem::setPoint( const GeoDataPoint& point )
{
    m_point = point;
}

GeoDataPoint GeoPointGraphicsItem::point() const
{
    return m_point;
}

void GeoPointGraphicsItem::setViewport( const ViewportParams *viewport )
{
    Q_UNUSED( viewport );
}

void GeoPointGraphicsItem::paint( GeoPainter* painter ) const
{
    if ( style() && !style()->iconStyle().icon().isNull() ) {
        painter->drawImage( m_point.coordinates(), style()->iconStyle().icon() );
    } else {
        painter->drawPoint( m_point );
    }
}

const GeoDataLatLonAltBox& GeoPointGraphicsItem::latLonAltBox() const
{
    return m_point.latLonAltBox();
}

}
