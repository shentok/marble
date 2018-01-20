//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Bastian Holst <bastianholst@gmx.de>
//

#include "KmlSouthTagHandler.h"

#include "MarbleDebug.h"

#include "KmlElementDictionary.h"
#include "GeoDataLatitude.h"
#include "GeoDataLatLonAltBox.h"
#include "GeoParser.h"

namespace Marble
{
namespace kml
{
KML_DEFINE_TAG_HANDLER( south )

GeoNode* KmlsouthTagHandler::parse( GeoParser& parser ) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_south)));

    GeoStackItem parentItem = parser.parentElement();

    const GeoDataLatitude south = GeoDataLatitude::fromDegrees(parser.readElementText().trimmed().toDouble());
    if( parentItem.represents( kmlTag_LatLonAltBox ) ) {
        const auto normalizedSouth = GeoDataNormalizedLatitude::fromLatitude(south);
        parentItem.nodeAs<GeoDataLatLonAltBox>()->setSouth(normalizedSouth);
    } else if ( parentItem.represents( kmlTag_LatLonBox ) ) {
        const auto normalizedSouth = GeoDataNormalizedLatitude::fromLatitude(south);
        parentItem.nodeAs<GeoDataLatLonBox>()->setSouth(normalizedSouth);
    }

    return nullptr;
}

}
}
