//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013      Sanjiban Bairagya <sanjiban22393@gmail.com>
//

#include "GeoDataOrientation.h"
#include "GeoDataTypes.h"

namespace Marble {

class GeoDataOrientationPrivate
{
public:
    GeoDataAngle m_heading;

    GeoDataAngle m_tilt;

    GeoDataAngle m_roll;

    GeoDataOrientationPrivate();
};

GeoDataOrientationPrivate::GeoDataOrientationPrivate() :
    m_heading(GeoDataAngle::null),
    m_tilt(GeoDataAngle::null),
    m_roll(GeoDataAngle::null)
{
    // nothing to do
}

GeoDataOrientation::GeoDataOrientation() : d( new GeoDataOrientationPrivate )
{
    // nothing to do
}

GeoDataOrientation::GeoDataOrientation( const Marble::GeoDataOrientation &other ) :
    GeoDataObject( other ), d( new GeoDataOrientationPrivate( *other.d ) )
{
    // nothing to do
}

GeoDataOrientation &GeoDataOrientation::operator=( const GeoDataOrientation &other )
{
    GeoDataObject::operator=( other );
    *d = *other.d;
    return *this;
}


bool GeoDataOrientation::operator==( const GeoDataOrientation &other ) const
{
    return equals(other) &&
           d->m_heading == other.d->m_heading &&
           d->m_roll == other.d->m_roll &&
           d->m_tilt == other.d->m_tilt;
}

bool GeoDataOrientation::operator!=( const GeoDataOrientation &other ) const
{
    return !this->operator==( other );
}

GeoDataOrientation::~GeoDataOrientation()
{
    delete d;
}

const char *GeoDataOrientation::nodeType() const
{
    return GeoDataTypes::GeoDataOrientationType;
}

GeoDataAngle GeoDataOrientation::heading() const
{
    return d->m_heading;
}

void GeoDataOrientation::setHeading(GeoDataAngle heading)
{
    d->m_heading = heading;
}

GeoDataAngle GeoDataOrientation::tilt() const
{
    return d->m_tilt;
}

void GeoDataOrientation::setTilt(GeoDataAngle tilt)
{
    d->m_tilt = tilt;
}

GeoDataAngle GeoDataOrientation::roll() const
{
    return d->m_roll;
}

void GeoDataOrientation::setRoll(GeoDataAngle roll)
{
    d->m_roll = roll;
}

}
