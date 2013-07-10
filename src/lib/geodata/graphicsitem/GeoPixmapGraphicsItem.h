//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013      Rene Küttner <rene@bitkanal.net>
//

#ifndef MARBLE_GEOPIXMAPGRAPHICSITEM_H
#define MARBLE_GEOPIXMAPGRAPHICSITEM_H

#include "GeoDataCoordinates.h"
#include "GeoGraphicsItem.h"
#include "marble_export.h"

#include <QPixmap>

namespace Marble
{

class MARBLE_EXPORT GeoPixmapGraphicsItem : public GeoGraphicsItem
{
public:
    explicit GeoPixmapGraphicsItem( const GeoDataFeature *feature,
                                    const QPixmap *pixmap,
                                    const GeoDataCoordinates &center );

    const QPixmap* pixmap() const;

    void setViewport( const ViewportParams *viewport );

    void paint( GeoPainter* painter ) const;

protected:
    const QPixmap *m_pixmap;
    GeoDataCoordinates m_centerPosition;
};

}

#endif /* MARBLE_GEOPIXMAPGRAPHICSITEM_H */
