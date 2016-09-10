//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011      Konstantin Oblaukhov <oblaukhov.konstantin@gmail.com>
//

#include "GeoPolygonGraphicsItem.h"

#include "BuildingGeoPolygonGraphicsItem.h"
#include "GeoDataPlacemark.h"
#include "StyleBuilder.h"

namespace Marble
{

AbstractGeoPolygonGraphicsItem *GeoPolygonGraphicsItem::createGraphicsItem(const GeoDataPlacemark *placemark, const GeoDataPolygon *polygon, const StyleBuilder &styleBuilder)
{
    if (placemark->visualCategory() == GeoDataPlacemark::Building) {
        return new BuildingGeoPolygonGraphicsItem(placemark, polygon);
    }
    return new GeoPolygonGraphicsItem(placemark, polygon, styleBuilder);
}

AbstractGeoPolygonGraphicsItem *GeoPolygonGraphicsItem::createGraphicsItem(const GeoDataPlacemark *placemark, const GeoDataLinearRing *ring, const StyleBuilder &styleBuilder)
{
    if (placemark->visualCategory() == GeoDataPlacemark::Building) {
        return new BuildingGeoPolygonGraphicsItem(placemark, ring);
    }
    return new GeoPolygonGraphicsItem(placemark, ring, styleBuilder);
}


GeoPolygonGraphicsItem::GeoPolygonGraphicsItem(const GeoDataPlacemark *placemark, const GeoDataPolygon *polygon, const StyleBuilder &styleBuilder) :
    AbstractGeoPolygonGraphicsItem(placemark, polygon)
{
    const int elevation = extractElevation(*placemark);
    setZValue(zValue() + elevation);

    const QString paintLayer = QLatin1String("Polygon/") + styleBuilder.visualCategoryName(*placemark);
    setPaintLayers(QStringList(paintLayer));
}

GeoPolygonGraphicsItem::GeoPolygonGraphicsItem(const GeoDataPlacemark *placemark, const GeoDataLinearRing *ring, const StyleBuilder &styleBuilder) :
    AbstractGeoPolygonGraphicsItem(placemark, ring)
{
    const int elevation = extractElevation(*placemark);
    setZValue(zValue() + elevation);

    const QString paintLayer = QLatin1String("Polygon/") + styleBuilder.visualCategoryName(*placemark);
    setPaintLayers(QStringList(paintLayer));
}

}
