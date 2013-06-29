//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011      Konstantin Oblaukhov <oblaukhov.konstantin@gmail.com>
//

#ifndef MARBLE_GEOLINEARRINGGRAPHICSITEM_H
#define MARBLE_GEOLINEARRINGGRAPHICSITEM_H

#include "GeoGraphicsItem.h"
#include "marble_export.h"

namespace Marble
{

class GeoDataLinearRing;

class MARBLE_EXPORT GeoLinearRingGraphicsItem : public GeoGraphicsItem
{
public:
    explicit GeoLinearRingGraphicsItem( const GeoDataFeature *feature, const GeoDataLinearRing* ring );

    virtual const GeoDataLatLonAltBox& latLonAltBox() const;

    virtual void paint( GeoPainter* painter, const ViewportParams *viewport );

protected:
    const GeoDataLinearRing *const m_ring;
};

}

#endif
