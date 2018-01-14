//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010      Gaurav Gupta <1989.gaurav@googlemail.com>
// Copyright 2013      Sanjiban Bairagya <sanjiban22393@gmail.com>
//

#include "KmlLatitudeTagHandler.h"

#include "MarbleDebug.h"

#include "KmlElementDictionary.h"
#include "MarbleGlobal.h"

#include "GeoDataLookAt.h"
#include "GeoDataCamera.h"
#include "GeoParser.h"
#include "GeoDataLatitude.h"
#include "GeoDataLocation.h"

namespace Marble
{
namespace kml
{
    KML_DEFINE_TAG_HANDLER( latitude )
    GeoNode *KmllatitudeTagHandler::parse( GeoParser & parser ) const
    {
        Q_ASSERT(parser.isStartElement() && parser.isValidElement(QLatin1String(kmlTag_latitude)));

        GeoStackItem parentItem = parser.parentElement();

        if ( parentItem.is<GeoDataLookAt>() ) {
            const GeoDataLatitude latitude = GeoDataLatitude::fromDegrees(parser.readElementText().trimmed().toDouble());
            parentItem.nodeAs<GeoDataLookAt>()->setLatitude(latitude);
        } else if ( parentItem.is<GeoDataCamera>() ) {
            const GeoDataLatitude latitude = GeoDataLatitude::fromDegrees(parser.readElementText().trimmed().toDouble());
            parentItem.nodeAs<GeoDataCamera>()->setLatitude(latitude);
        } else if ( parentItem.is<GeoDataLocation>() ) {
            const GeoDataLatitude latitude = GeoDataLatitude::fromDegrees(parser.readElementText().trimmed().toDouble());
            parentItem.nodeAs<GeoDataLocation>()->setLatitude(latitude);
	}
        return nullptr;
    }
  }
}
