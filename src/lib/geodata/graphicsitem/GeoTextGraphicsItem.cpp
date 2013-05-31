//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013      Rene Kuettner <rene@bitkanal.net>
//

#include "GeoTextGraphicsItem.h"

#include "GeoDataLineStyle.h"
#include "GeoDataCoordinates.h"
#include "GeoPainter.h"
#include "ViewportParams.h"
#include "GeoDataStyle.h"

namespace Marble
{

GeoTextGraphicsItem::GeoTextGraphicsItem( const GeoDataFeature *feature,
                                          const GeoDataCoordinates &location,
                                          const QString &text )
    : GeoGraphicsItem( feature ),
      m_location( location ),
      m_text( text ),
      m_latLonAltBox( location )
{
}

const GeoDataLatLonAltBox& GeoTextGraphicsItem::latLonAltBox() const
{
    return m_latLonAltBox;
}

void GeoTextGraphicsItem::setLocation( const GeoDataCoordinates &location )
{
    m_location = location;
    m_latLonAltBox = GeoDataLatLonAltBox( location );
}

const GeoDataCoordinates GeoTextGraphicsItem::location() const
{
    return m_location;
}

void GeoTextGraphicsItem::setText( const QString &text )
{
    m_text = text;
}

const QString GeoTextGraphicsItem::text() const
{
    return m_text;
}

void GeoTextGraphicsItem::setViewport( const ViewportParams *viewport )
{
    Q_UNUSED( viewport );
}

void GeoTextGraphicsItem::paint( GeoPainter* painter ) const
{
    painter->save();

    if (style()) {
        // label styles
        painter->setFont( style()->labelStyle().font() );
        painter->setPen( style()->labelStyle().color() );
    }

    painter->drawText( m_location, m_text );

    painter->restore();
}

}
