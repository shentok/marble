//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2015      Constantin Mihalache <mihalache.c94@gmail.com>
//

//self
#include "OpenLocationCodeSearchRunner.h"

#include "GeoDataPlacemark.h"
#include "GeoDataLinearRing.h"
#include "GeoDataPolygon.h"
#include "GeoDataStyle.h"
#include "GeoDataLineStyle.h"
#include "GeoDataPolyStyle.h"

//Qt
#include <QColor>
#include <QVector>

namespace Marble
{

OpenLocationCodeSearchRunner::OpenLocationCodeSearchRunner( QObject *parent ):
    SearchRunner( parent )
{
    // initialize the charIndex map
    QString const acceptedChars = "23456789CFGHJMPQRVWX";
    for( int index = 0; index < acceptedChars.size(); index++ ) {
        charIndex[ acceptedChars[index] ] = index;
    }
}

void OpenLocationCodeSearchRunner::search( const QString &searchTerm, const GeoDataLatLonBox &preferred )
{
    Q_UNUSED( preferred );

    QVector<GeoDataPlacemark*> result;

    if( isValidOLC( searchTerm.toUpper() ) ) {
        GeoDataLatLonBox boundingBox = decodeOLC( searchTerm.toUpper() );
        if( !boundingBox.isEmpty() ) {
            GeoDataPlacemark *placemark = new GeoDataPlacemark( searchTerm );

            GeoDataPolygon *geometry = new GeoDataPolygon( polygonFromLatLonBox( boundingBox ) );
            placemark->setGeometry( geometry );

            GeoDataStyle::Ptr style = GeoDataStyle::Ptr(new GeoDataStyle());
            GeoDataLineStyle lineStyle;
            GeoDataPolyStyle polyStyle;
            lineStyle.setColor( QColor( Qt::GlobalColor::red ) );
            lineStyle.setWidth( 2 );
            polyStyle.setFill( false );
            style->setLineStyle( lineStyle );
            style->setPolyStyle( polyStyle );
            placemark->setStyle( style );

            result.append( placemark );
        }
    }

    emit searchFinished( result );
}

GeoDataPolygon OpenLocationCodeSearchRunner::polygonFromLatLonBox( const GeoDataLatLonBox& boundingBox ) const
{
    if( boundingBox.isEmpty() ) {
        return GeoDataPolygon();
    }

    GeoDataPolygon poly;
    GeoDataLinearRing outerBoundry;
    // north-west corner
    outerBoundry.append(GeoDataCoordinates(boundingBox.west(), boundingBox.north()));
    // north-east corner
    outerBoundry.append(GeoDataCoordinates(boundingBox.east(), boundingBox.north()));
    // south-east corner
    outerBoundry.append(GeoDataCoordinates(boundingBox.east(), boundingBox.south()));
    // south-west corner
    outerBoundry.append(GeoDataCoordinates(boundingBox.west(), boundingBox.south()));

    poly.setOuterBoundary( outerBoundry );

    return poly;
}

GeoDataLatLonBox OpenLocationCodeSearchRunner::decodeOLC( const QString &olc ) const
{
    if( !isValidOLC( olc ) ) {
        return GeoDataLatLonBox();
    }

    // remove padding
    QString decoded = olc;
    decoded = decoded.remove( QRegExp("[0+]") );
    GeoDataLatitude southLatitude = GeoDataLatitude::null;
    GeoDataLongitude westLongitude = GeoDataLongitude::null;

    int digit = 0;
    GeoDataLatitude latitudeResolution = GeoDataLatitude::fromDegrees(400);
    GeoDataLongitude longitudeResolution = GeoDataLongitude::fromDegrees(400);

    while( digit < decoded.size() ) {
        if( digit < 10 ) {
            latitudeResolution /= 20;
            longitudeResolution /= 20;
            southLatitude += latitudeResolution * charIndex[ decoded[digit] ];
            westLongitude += longitudeResolution * charIndex[ decoded[digit+1] ];
            digit += 2;
        }
        else {
            latitudeResolution /= 5;
            longitudeResolution /= 4;
            southLatitude += latitudeResolution *( charIndex[ decoded[digit] ] / 4 );
            westLongitude += longitudeResolution *( charIndex[ decoded[digit] ] % 4 );
            digit += 1;
        }
    }

    return GeoDataLatLonBox(GeoDataNormalizedLatitude::fromLatitude(southLatitude - GeoDataLatitude::quaterCircle + latitudeResolution),
                            GeoDataNormalizedLatitude::fromLatitude(southLatitude - GeoDataLatitude::quaterCircle),
                            GeoDataNormalizedLongitude::fromLongitude(westLongitude - GeoDataLongitude::halfCircle+ longitudeResolution),
                            GeoDataNormalizedLongitude::fromLongitude(westLongitude - GeoDataLongitude::halfCircle));
}

bool OpenLocationCodeSearchRunner::isValidOLC( const QString& olc ) const
{
    // It must have only one SEPARATOR located at an even index in
    // the string.
    QChar const separator(QLatin1Char('+'));
    int separatorPos = olc.indexOf(separator);
    if( separatorPos == -1
        || separatorPos != olc.lastIndexOf(separator)
        || separatorPos % 2 != 0 )
    {
        return false;
    }
    int const separatorPosition = 8;
    // It must be a full open location code.
    if( separatorPos != separatorPosition ) {
        return false;
    }

    // Test the first two characters as only some characters of the
    // ACCEPTED_CHARS are allowed.
    //
    // First latitude character can only take one of the first 9 values.
    int index0 = charIndex.value( olc[0], -1 );
    if( index0 == -1 || index0 > 8 ) {
        return false;
    }
    // First longitude character can only take one of the first 18 values.
    int index1 = charIndex.value( olc[1], -1 );
    if( index1 == -1 || index1 > 17) {
        return false;
    }

    // Test the characters before the SEPARATOR.
    QChar const suffixPadding(QLatin1Char('0'));
    bool paddingBegun = false;
    for( int index = 0; index < separatorPos; index++ ) {
        if( paddingBegun ) {
        // Once padding has begun, there should be only padding.
        if( olc[index] != suffixPadding ) {
            return false;
        }
        continue;
        }
        if( charIndex.contains( olc[index] ) ) {
            continue;
        }
        if( olc[index] == suffixPadding ) {
            paddingBegun = true;
            // Padding can start only at an even index.
            if( index % 2 != 0 ) {
                return false;
            }
            continue;
        }
    return false;
    }

    // Test the characters after the SEPARATOR.
    if( olc.size() > separatorPos +1 ) {
        if( paddingBegun ) {
            return false;
        }
        // Only one character after the SEPARATOR is not allowed.
        if( olc.size() == separatorPos + 2 ) {
            return false;
        }
        for( int index = separatorPos + 1; index < olc.size(); index++ ) {
            if( !charIndex.contains( olc[index] ) ) {
                return false;
            }
        }
    }

    return true;
}

}
