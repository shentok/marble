//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Bastian Holst <bastianholst@gmx.de>
//

#include "KmlEastTagHandler.h"

#include "MarbleDebug.h"

#include "KmlElementDictionary.h"
#include "GeoDataLatLonAltBox.h"
#include "GeoDataLongitude.h"
#include "GeoParser.h"

namespace Marble
{
namespace kml
{
KML_DEFINE_TAG_HANDLER( east )

GeoNode* KmleastTagHandler::parse( GeoParser& parser ) const
{
    Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_east)));

    GeoStackItem parentItem = parser.parentElement();

    GeoDataLongitude const east = GeoDataLongitude::fromDegrees(parser.readElementText().trimmed().toDouble());
    if( parentItem.represents( kmlTag_LatLonAltBox ) ) {
        parentItem.nodeAs<GeoDataLatLonAltBox>()->setEast(east);
    } else if( parentItem.represents( kmlTag_LatLonBox ) ) {
        parentItem.nodeAs<GeoDataLatLonBox>()->setEast(east);
    }

    return nullptr;
}

}
}
