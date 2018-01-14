//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013      Mayank Madan <maddiemadan@gmail.com>
//

#include "KmlLatLonQuadWriter.h"

#include "GeoDataLatitude.h"
#include "GeoDataLatLonQuad.h"
#include "GeoDataLongitude.h"
#include "GeoDataTypes.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"
#include "KmlObjectTagWriter.h"

namespace Marble
{

static GeoTagWriterRegistrar s_writerLatLonQuad(
    GeoTagWriter::QualifiedName( GeoDataTypes::GeoDataLatLonQuadType,
                                 kml::kmlTag_nameSpaceOgc22 ),
    new KmlLatLonQuadWriter );

bool KmlLatLonQuadWriter::write( const GeoNode *node,
                 GeoWriter& writer ) const
{
    const GeoDataLatLonQuad *latLonQuad = static_cast<const GeoDataLatLonQuad*>( node );

    if ( latLonQuad->isValid() ) {
        writer.writeStartElement( kml::kmlTag_nameSpaceGx22, kml::kmlTag_LatLonQuad );
        KmlObjectTagWriter::writeIdentifiers( writer, latLonQuad );

        writer.writeStartElement( kml::kmlTag_coordinates );

        writer.writeCharacters(QString::number(latLonQuad->bottomLeftLongitude().toDegree()));
        writer.writeCharacters( QString( ',' ) );
        writer.writeCharacters(QString::number(latLonQuad->bottomLeftLatitude().toDegree()));
        writer.writeCharacters( QString( ' ' ) );

        writer.writeCharacters(QString::number(latLonQuad->bottomRightLongitude().toDegree()));
        writer.writeCharacters( QString( ',' ) );
        writer.writeCharacters(QString::number(latLonQuad->bottomRightLatitude().toDegree()));
        writer.writeCharacters( QString( ' ' ) );

        writer.writeCharacters(QString::number(latLonQuad->topRightLongitude().toDegree()));
        writer.writeCharacters( QString( ',' ) );
        writer.writeCharacters(QString::number(latLonQuad->topRightLatitude().toDegree()));
        writer.writeCharacters( QString( ' ' ) );

        writer.writeCharacters(QString::number(latLonQuad->topLeftLongitude().toDegree()));
        writer.writeCharacters( QString( ',' ) );
        writer.writeCharacters(QString::number(latLonQuad->topLeftLatitude().toDegree()));

        writer.writeEndElement();

        writer.writeEndElement();
    }

    return true;
}

}
