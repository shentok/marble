//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013      Rene Kuettner <rene@bitkanal.net>
//

#ifndef MARBLE_GEOTEXTGRAPHICSITEM_H
#define MARBLE_GEOTEXTGRAPHICSITEM_H

#include <QString>

#include "GeoGraphicsItem.h"
#include "GeoDataLatLonAltBox.h"
#include "GeoDataCoordinates.h"
#include "marble_export.h"

namespace Marble
{

class GeoDataLineStyle;

class MARBLE_EXPORT GeoTextGraphicsItem : public GeoGraphicsItem
{
public:
    explicit GeoTextGraphicsItem( const GeoDataFeature *feature,
                                  const GeoDataCoordinates &location,
                                  const QString &text );

    virtual const GeoDataLatLonAltBox& latLonAltBox() const;

    void setLocation( const GeoDataCoordinates &location );

    const GeoDataCoordinates location() const;

    void setText( const QString &text );

    const QString text() const;

    void setViewport( const ViewportParams *viewport );

    void paint( GeoPainter* painter ) const;

protected:
    GeoDataCoordinates  m_location;
    QString             m_text;
    GeoDataLatLonAltBox m_latLonAltBox;
};

}

#endif
