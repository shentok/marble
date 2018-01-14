//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013      Mayank Madan <maddiemadan@gmail.com>
//

#include "GeoDataLatLonQuad.h"
#include "GeoDataObject.h"
#include "GeoDataTypes.h"

namespace Marble {

class GeoDataLatLonQuadPrivate
{
public:
    GeoDataCoordinates m_bottomLeft;
    GeoDataCoordinates m_bottomRight;
    GeoDataCoordinates m_topRight;
    GeoDataCoordinates m_topLeft;

    GeoDataLatLonQuadPrivate();
};

GeoDataLatLonQuadPrivate::GeoDataLatLonQuadPrivate() :
    m_bottomLeft(),m_bottomRight(),m_topRight(),m_topLeft()
{
    // nothing to do
}

GeoDataLatLonQuad::GeoDataLatLonQuad() : GeoDataObject(), d( new GeoDataLatLonQuadPrivate )
{
    // nothing to do
}

GeoDataLatLonQuad::GeoDataLatLonQuad( const Marble::GeoDataLatLonQuad &other ) :
    GeoDataObject( other ), d( new GeoDataLatLonQuadPrivate( *other.d ) )
{
    // nothing to do
}

GeoDataLatLonQuad &GeoDataLatLonQuad::operator=( const GeoDataLatLonQuad &other )
{
    *d = *other.d;
    return *this;
}

bool GeoDataLatLonQuad::operator==(const GeoDataLatLonQuad& other) const
{
    return equals( other )
           && d->m_bottomLeft == other.d->m_bottomLeft
           && d->m_bottomRight == other.d->m_bottomRight
           && d->m_topLeft == other.d->m_topLeft
           && d->m_topRight == other.d->m_topRight;
}

bool GeoDataLatLonQuad::operator!=(const GeoDataLatLonQuad& other) const
{
    return !this->operator==(other);
}

GeoDataLatLonQuad::~GeoDataLatLonQuad()
{
    delete d;
}

const char *GeoDataLatLonQuad::nodeType() const
{
    return GeoDataTypes::GeoDataLatLonQuadType;
}

GeoDataLatitude GeoDataLatLonQuad::bottomLeftLatitude() const
{
    return d->m_bottomLeft.latitude();
}

void GeoDataLatLonQuad::setBottomLeftLatitude(GeoDataLatitude latitude)
{
    d->m_bottomLeft.setLatitude(latitude);
}

GeoDataLongitude GeoDataLatLonQuad::bottomLeftLongitude() const
{
    return d->m_bottomLeft.longitude();
}

void GeoDataLatLonQuad::setBottomLeftLongitude(GeoDataLongitude longitude)
{
    d->m_bottomLeft.setLongitude(longitude);
}

GeoDataLatitude GeoDataLatLonQuad::bottomRightLatitude() const
{
    return d->m_bottomRight.latitude();
}

void GeoDataLatLonQuad::setBottomRightLatitude(GeoDataLatitude latitude)
{
    d->m_bottomRight.setLatitude(latitude);
}

GeoDataLongitude GeoDataLatLonQuad::bottomRightLongitude() const
{
    return d->m_bottomRight.longitude();
}

void GeoDataLatLonQuad::setBottomRightLongitude(GeoDataLongitude longitude)
{
    d->m_bottomRight.setLongitude(longitude);
}

GeoDataLatitude GeoDataLatLonQuad::topRightLatitude() const
{
    return d->m_topRight.latitude();
}

void GeoDataLatLonQuad::setTopRightLatitude(GeoDataLatitude latitude)
{
    d->m_topRight.setLatitude(latitude);
}

GeoDataLongitude GeoDataLatLonQuad::topRightLongitude() const
{
    return d->m_topRight.longitude();
}

void GeoDataLatLonQuad::setTopRightLongitude(GeoDataLongitude longitude)
{
    d->m_topRight.setLongitude(longitude);
}

GeoDataLatitude GeoDataLatLonQuad::topLeftLatitude() const
{
    return d->m_topLeft.latitude();
}

void GeoDataLatLonQuad::setTopLeftLatitude(GeoDataLatitude latitude)
{
    d->m_topLeft.setLatitude(latitude);
}

GeoDataLongitude GeoDataLatLonQuad::topLeftLongitude() const
{
    return d->m_topLeft.longitude();
}

void GeoDataLatLonQuad::setTopLeftLongitude(GeoDataLongitude longitude)
{
    d->m_topLeft.setLongitude(longitude);
}


GeoDataCoordinates &GeoDataLatLonQuad::bottomLeft() const
{
    return d->m_bottomLeft;
}

void GeoDataLatLonQuad::setBottomLeft(const GeoDataCoordinates &coordinates)
{
    d->m_bottomLeft = coordinates;
}
GeoDataCoordinates &GeoDataLatLonQuad::bottomRight() const
{
    return d->m_bottomRight;
}

void GeoDataLatLonQuad::setBottomRight(const GeoDataCoordinates &coordinates)
{
    d->m_bottomRight = coordinates;
}

GeoDataCoordinates &GeoDataLatLonQuad::topRight() const
{
    return d->m_topRight;
}

void GeoDataLatLonQuad::setTopRight(const GeoDataCoordinates &coordinates)
{
    d->m_topRight = coordinates;
}

GeoDataCoordinates &GeoDataLatLonQuad::topLeft() const
{
    return d->m_topLeft;
}

void GeoDataLatLonQuad::setTopLeft(const GeoDataCoordinates &coordinates)
{
    d->m_topLeft = coordinates;
}

bool GeoDataLatLonQuad::isValid() const
{
    return d->m_bottomLeft.isValid() && d->m_bottomRight.isValid()
           && d->m_topLeft.isValid() && d->m_topRight.isValid();
}

}
