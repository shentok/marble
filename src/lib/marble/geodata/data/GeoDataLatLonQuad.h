//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013      Mayank Madan <maddiemadan@gmail.com>
//

#ifndef GEODATALATLONQUAD_H
#define GEODATALATLONQUAD_H

#include "GeoDataObject.h"

#include "GeoDataCoordinates.h"
#include "GeoDataLatitude.h"
#include "GeoDataLongitude.h"

#include "MarbleGlobal.h"

namespace Marble {

class GeoDataLatLonQuadPrivate;

/**
 */
class GEODATA_EXPORT GeoDataLatLonQuad: public GeoDataObject
{
public:
    GeoDataLatLonQuad();

    GeoDataLatLonQuad( const GeoDataLatLonQuad &other );

    GeoDataLatLonQuad& operator=( const GeoDataLatLonQuad &other );
    bool operator==( const GeoDataLatLonQuad &other ) const;
    bool operator!=( const GeoDataLatLonQuad &other ) const;

    ~GeoDataLatLonQuad() override;

    /** Provides type information for downcasting a GeoNode */
    const char* nodeType() const override;

    GeoDataLatitude bottomLeftLatitude() const;
    GeoDataLatitude bottomRightLatitude() const;
    GeoDataLatitude topRightLatitude() const;
    GeoDataLatitude topLeftLatitude() const;

    GeoDataLongitude bottomLeftLongitude() const;
    GeoDataLongitude bottomRightLongitude() const;
    GeoDataLongitude topRightLongitude() const;
    GeoDataLongitude topLeftLongitude() const;

    void setBottomLeftLatitude(GeoDataLatitude latitude);
    void setBottomRightLatitude(GeoDataLatitude latitude);
    void setTopRightLatitude(GeoDataLatitude latitude);
    void setTopLeftLatitude(GeoDataLatitude latitude);

    void setBottomLeftLongitude(GeoDataLongitude longitude);
    void setBottomRightLongitude(GeoDataLongitude longitude);
    void setTopRightLongitude(GeoDataLongitude longitude);
    void setTopLeftLongitude(GeoDataLongitude longitude);

    GeoDataCoordinates& bottomLeft() const;
    void setBottomLeft( const GeoDataCoordinates &coordinates );

    GeoDataCoordinates& bottomRight() const;
    void setBottomRight( const GeoDataCoordinates &coordinates );

    GeoDataCoordinates& topRight() const;
    void setTopRight( const GeoDataCoordinates &coordinates );

    GeoDataCoordinates& topLeft() const;
    void setTopLeft( const GeoDataCoordinates &coordinates );

    bool isValid() const;

private:
    GeoDataLatLonQuadPrivate* const d;
};

}

#endif
