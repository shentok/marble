//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Bastian Holst <bastianholst@gmx.de>
//

#include "KmlNorthTagHandler.h"

#include "MarbleDebug.h"

#include "KmlElementDictionary.h"
#include "GeoDataLatitude.h"
#include "GeoDataLatLonAltBox.h"
#include "GeoParser.h"

namespace Marble
{
namespace kml
{
KML_DEFINE_TAG_HANDLER( north )

GeoNode* KmlnorthTagHandler::parse( GeoParser& parser ) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_north)));

    GeoStackItem parentItem = parser.parentElement();

    const GeoDataLatitude north = GeoDataLatitude::fromDegrees(parser.readElementText().trimmed().toDouble());
    if( parentItem.represents( kmlTag_LatLonAltBox ) ) {
        const auto normalizedNorth = GeoDataNormalizedLatitude::fromLatitude(north);
        parentItem.nodeAs<GeoDataLatLonAltBox>()->setNorth(normalizedNorth);
    } else if ( parentItem.represents( kmlTag_LatLonBox ) ) {
        const auto normalizedNorth = GeoDataNormalizedLatitude::fromLatitude(north);
        parentItem.nodeAs<GeoDataLatLonBox>()->setNorth(normalizedNorth);
    }

    return nullptr;
}

}
}
