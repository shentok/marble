//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Gaurav Gupta <1989.gaurav@googlemail.com>
// Copyright 2010      Bastian Holst <bastianholst@gmx.de>
//

//own
#include "GeoDataLookAt.h"
#include "GeoDataLookAt_p.h"

#include "GeoDataTypes.h"

#include "MarbleDebug.h"

namespace Marble
{

GeoDataLookAt::GeoDataLookAt() :
    GeoDataAbstractView(),
    d( new GeoDataLookAtPrivate )
{
}

GeoDataLookAt::GeoDataLookAt( const GeoDataLookAt& other ) :
    GeoDataAbstractView(),
    d( other.d )
{
    d->ref.ref();
}

GeoDataLookAt& GeoDataLookAt::operator=( const GeoDataLookAt &other )                                   
{
    GeoDataAbstractView::operator=( other );
    qAtomicAssign( d, other.d );
    return *this;
}

bool GeoDataLookAt::operator==(const GeoDataLookAt &other) const
{
    return equals(other) &&
           d->m_coordinates == other.d->m_coordinates &&
           d->m_range == other.d->m_range;
}

bool GeoDataLookAt::operator!=(const GeoDataLookAt &other) const
{
    return !this->operator==( other );
}

GeoDataLookAt::~GeoDataLookAt()
{
    if( !d->ref.deref() )
        delete d;
}

GeoDataAbstractView *GeoDataLookAt::copy() const
{
    return new GeoDataLookAt( *this );
}

void GeoDataLookAt::setCoordinates( const GeoDataCoordinates& coordinates )
{
    d->m_coordinates = coordinates;
}

const char* GeoDataLookAt::nodeType() const
{
    return GeoDataTypes::GeoDataLookAtType;
}

void GeoDataLookAt::setAltitude( qreal altitude )
{
    detach();
    d->m_coordinates.setAltitude( altitude );
}

qreal GeoDataLookAt::altitude() const
{
    return d->m_coordinates.altitude();
}

void GeoDataLookAt::setLatitude(GeoDataLatitude latitude)
{
    detach();
    d->m_coordinates.setLatitude(latitude);
}

GeoDataLatitude GeoDataLookAt::latitude() const
{
    return d->m_coordinates.latitude();
}

void GeoDataLookAt::setLongitude(GeoDataLongitude longitude)
{
    detach();
    d->m_coordinates.setLongitude(longitude);
}

GeoDataLongitude GeoDataLookAt::longitude() const
{
    return d->m_coordinates.longitude();
}

GeoDataCoordinates GeoDataLookAt::coordinates() const
{
    return d->m_coordinates;
}

void GeoDataLookAt::setRange( qreal range )
{
    detach();
    d->m_range = range;
}

qreal GeoDataLookAt::range() const
{
    return d->m_range;
}

void GeoDataLookAt::detach()
{
    qAtomicDetach( d );
}

}
