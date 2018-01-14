//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Bastian Holst <bastianholst@gmx.de>
//

#include "KmlWestTagHandler.h"

#include "MarbleDebug.h"

#include "KmlElementDictionary.h"
#include "GeoDataLatLonAltBox.h"
#include "GeoDataLongitude.h"
#include "GeoParser.h"

namespace Marble
{
namespace kml
{
KML_DEFINE_TAG_HANDLER( west )

GeoNode* KmlwestTagHandler::parse( GeoParser& parser ) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_west)));

    GeoStackItem parentItem = parser.parentElement();

    const GeoDataLongitude west = GeoDataLongitude::fromDegrees(parser.readElementText().trimmed().toDouble());
    if( parentItem.represents( kmlTag_LatLonAltBox ) ) {
        parentItem.nodeAs<GeoDataLatLonAltBox>()->setWest(west);
    } else if( parentItem.represents( kmlTag_LatLonBox ) ) {
        parentItem.nodeAs<GeoDataLatLonBox>()->setWest(west);
    }

    return nullptr;
}

}
}
