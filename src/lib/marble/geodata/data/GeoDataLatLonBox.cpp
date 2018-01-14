//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2007      Andrew Manson  <g.real.ate@gmail.com>
// Copyright 2008-2009 Torsten Rahn   <rahn@kde.org>
//


#include "GeoDataLatLonBox.h"

#include "MarbleDebug.h"
#include "GeoDataCoordinates.h"
#include "GeoDataLineString.h"

#include "GeoDataTypes.h"

#include <QDataStream>

namespace Marble
{

const GeoDataLatLonBox GeoDataLatLonBox::empty = GeoDataLatLonBox();

class GeoDataLatLonBoxPrivate
{
 public:
    GeoDataLatLonBoxPrivate() :
        m_north(GeoDataLatitude::null),
        m_south(GeoDataLatitude::null),
        m_east(GeoDataLongitude::null),
        m_west(GeoDataLongitude::null),
        m_rotation(GeoDataAngle::fromRadians(0.0))
    {
    }

    GeoDataLatitude m_north;
    GeoDataLatitude m_south;
    GeoDataLongitude m_east;
    GeoDataLongitude m_west;
    GeoDataAngle m_rotation; // NOT implemented yet!
};

bool operator==( GeoDataLatLonBox const& lhs, GeoDataLatLonBox const& rhs )
{
    return lhs.d->m_west == rhs.d->m_west &&
           lhs.d->m_east == rhs.d->m_east &&
           lhs.d->m_north == rhs.d->m_north &&
           lhs.d->m_south == rhs.d->m_south &&
           lhs.d->m_rotation == rhs.d->m_rotation;
}

bool operator!=( GeoDataLatLonBox const& lhs, GeoDataLatLonBox const& rhs )
{
    return !( lhs == rhs );
}

GeoDataLatLonBox::GeoDataLatLonBox() :
    GeoDataObject(),
    d(new GeoDataLatLonBoxPrivate)
{
}

GeoDataLatLonBox::GeoDataLatLonBox(GeoDataLatitude north, GeoDataLatitude south, GeoDataLongitude east, GeoDataLongitude west) :
    GeoDataObject(),
    d(new GeoDataLatLonBoxPrivate)
{
    setBoundaries(north, south, east, west);
}

GeoDataLatLonBox::GeoDataLatLonBox(const GeoDataLatLonBox &other) :
    GeoDataObject(other),
    d(new GeoDataLatLonBoxPrivate(*other.d))
{
}

GeoDataLatLonBox::~GeoDataLatLonBox()
{
    delete d;
}

const char* GeoDataLatLonBox::nodeType() const
{
    return GeoDataTypes::GeoDataLatLonBoxType;
}

GeoDataLatitude GeoDataLatLonBox::north() const
{
    return d->m_north;
}

void GeoDataLatLonBox::setNorth(GeoDataLatitude north)
{
    d->m_north = GeoDataCoordinates::normalizeLat(north);
}

GeoDataLatitude GeoDataLatLonBox::south() const
{
    return d->m_south;
}

void GeoDataLatLonBox::setSouth(GeoDataLatitude south)
{
    d->m_south = GeoDataCoordinates::normalizeLat(south);
}

GeoDataLongitude GeoDataLatLonBox::east() const
{
    return d->m_east;
}

void GeoDataLatLonBox::setEast(GeoDataLongitude east)
{
    d->m_east = GeoDataCoordinates::normalizeLon(east);
}

GeoDataLongitude GeoDataLatLonBox::west() const
{
    return d->m_west;
}

void GeoDataLatLonBox::setWest(GeoDataLongitude west)
{
    d->m_west = GeoDataCoordinates::normalizeLon(west);
}

void GeoDataLatLonBox::setRotation(GeoDataAngle rotation)
{
    d->m_rotation = rotation;
}

GeoDataAngle GeoDataLatLonBox::rotation() const
{
    return d->m_rotation;
}

void GeoDataLatLonBox::boundaries(GeoDataLatitude &north, GeoDataLatitude &south, GeoDataLongitude &east, GeoDataLongitude &west) const
{
    north = d->m_north;
    south = d->m_south;
    east  = d->m_east;
    west  = d->m_west;
}

void GeoDataLatLonBox::setBoundaries(GeoDataLatitude north, GeoDataLatitude south, GeoDataLongitude east, GeoDataLongitude west)
{
    d->m_north = GeoDataCoordinates::normalizeLat(north);
    d->m_south = GeoDataCoordinates::normalizeLat(south);
    d->m_east =  GeoDataCoordinates::normalizeLon(east);
    d->m_west =  GeoDataCoordinates::normalizeLon(west);
}

void GeoDataLatLonBox::scale(qreal verticalFactor, qreal horizontalFactor) const
{
    GeoDataCoordinates const middle = center();
    const GeoDataLatitude deltaY = 0.5 * height() * verticalFactor;
    const GeoDataLongitude deltaX = 0.5 * width() * horizontalFactor;
    d->m_north = qMin((middle.latitude() + deltaY), +GeoDataLatitude::quaterCircle);
    d->m_south = qMax((middle.latitude() - deltaY), -GeoDataLatitude::quaterCircle);
    if (deltaX > GeoDataLongitude::halfCircle) {
        d->m_east = GeoDataLongitude::halfCircle;
        d->m_west = -GeoDataLongitude::halfCircle;
    }
    else {
        d->m_east = GeoDataCoordinates::normalizeLon(middle.longitude() + deltaX);
        d->m_west = GeoDataCoordinates::normalizeLon(middle.longitude() - deltaX);
    }
}

GeoDataLatLonBox GeoDataLatLonBox::scaled(qreal verticalFactor, qreal horizontalFactor) const
{
    GeoDataLatLonBox result = *this;
    result.scale(verticalFactor, horizontalFactor);
    return result;
}

GeoDataLongitude GeoDataLatLonBox::width() const
{
    return GeoDataLatLonBox::width(d->m_east, d->m_west);
}

GeoDataLongitude GeoDataLatLonBox::width(GeoDataLongitude east, GeoDataLongitude west)
{
    GeoDataLongitude width = qAbs(GeoDataLatLonBox::crossesDateLine(east, west)
                                     ? 2 * GeoDataLongitude::halfCircle - west + east
                                     : east - west);

    // This also covers the case where this bounding box covers the whole
    // longitude range ( -180 <= lon <= + 180 ).
    if (width > 2 * GeoDataLongitude::halfCircle) {
        width = 2 * GeoDataLongitude::halfCircle;
    }

    return width;
}

GeoDataLatitude GeoDataLatLonBox::height() const
{
    return GeoDataLatLonBox::height(d->m_north, d->m_south);
}

GeoDataLatitude GeoDataLatLonBox::height(GeoDataLatitude north, GeoDataLatitude south)
{
    const GeoDataLatitude height = qAbs(south - north);

    return height;
}

bool GeoDataLatLonBox::crossesDateLine() const
{
    return GeoDataLatLonBox::crossesDateLine(d->m_east, d->m_west);
}

bool GeoDataLatLonBox::crossesDateLine(GeoDataLongitude east, GeoDataLongitude west)
{
    return east < west || (east == GeoDataLongitude::halfCircle && west == -GeoDataLongitude::halfCircle);
}

GeoDataCoordinates GeoDataLatLonBox::center() const
{
    if( isEmpty() )
        return GeoDataCoordinates();

    if( crossesDateLine() )
        return GeoDataCoordinates(GeoDataCoordinates::normalizeLon(east() + 2 * GeoDataLongitude::halfCircle - (east() + 2 * GeoDataLongitude::halfCircle - west() ) / 2) ,
                                north() - ( north() - south() ) / 2 );
    else
        return GeoDataCoordinates( east() - ( east() - west() ) / 2,
                                north() - ( north() - south() ) / 2 );
}

bool GeoDataLatLonBox::containsPole( Pole pole ) const
{
    switch ( pole ) {
      case NorthPole:
        return (north() == +GeoDataLatitude::quaterCircle);
      case SouthPole:
        return (south() == -GeoDataLatitude::quaterCircle);
      default:
      case AnyPole:
        return (   north() == +GeoDataLatitude::quaterCircle
                || south() == -GeoDataLatitude::quaterCircle);
    }

    mDebug() << Q_FUNC_INFO << "Invalid pole";
    return false;
}

bool GeoDataLatLonBox::contains(GeoDataLongitude lon, GeoDataLatitude lat) const
{
    if ( lat < d->m_south || lat > d->m_north ) {
        return false;
    }

    // We need to take care of the normal case ...
    if ( ( ( lon < d->m_west || lon > d->m_east ) && ( d->m_west < d->m_east ) ) ||
    // ... and the case where the bounding box crosses the date line:
         ( ( lon < d->m_west && lon > d->m_east ) && ( d->m_west > d->m_east ) ) )
        return false;

    return true;
}

bool GeoDataLatLonBox::contains( const GeoDataCoordinates &point ) const
{
    GeoDataLongitude lon;
    GeoDataLatitude lat;

    point.geoCoordinates( lon, lat );

    return contains(lon, lat);
}

bool GeoDataLatLonBox::contains( const GeoDataLatLonBox &other ) const
{
    // check the contain criterion for the latitude first as this is trivial:

    if ( d->m_north >= other.north() && d->m_south <= other.south() ) {

        if ( !crossesDateLine() ) {
            if ( !other.crossesDateLine() ) {
                // "Normal" case: both bounding boxes don't cross the date line
                if ( d->m_west <= other.west() && d->m_east >= other.east() ) {
                    return true;
                }                
            }
            else {
                // The other bounding box crosses the date line, "this" one does not:
                // So the date line splits the other bounding box in two parts.
                // Hence "this" bounding box could be fully contained by one of them. 
                // So for both cases we are able to ignore the "overhanging" portion 
                // and thereby basically reduce the problem to the "normal" case: 

                if ((other.west() <= d->m_west && d->m_east <= +GeoDataLongitude::halfCircle)
                  || (other.east() >= d->m_east && d->m_west >= -GeoDataLongitude::halfCircle)) {
                    return true;
                }
            }
        }
        else {
            if ( other.crossesDateLine() ) {
                // Other "Simple" case: both bounding boxes cross the date line
                if ( d->m_west <= other.west() && d->m_east >= other.east() ) {
                    return true;
                }                
            }
            else {
                // "This" bounding box crosses the date line, the other one does not.
                // So the date line splits "this" bounding box in two parts.
                // Hence the other bounding box could be fully contained by one of them. 
                // So for both cases we are able to ignore the "overhanging" portion 
                // and thereby basically reduce the problem to the "normal" case: 

                if ((d->m_west <= other.west() && other.east() <= +GeoDataLongitude::halfCircle)
                  || (d->m_east >= other.east() && other.west() >= -GeoDataLongitude::halfCircle)) {
                    return true;
                }

                // if this bounding box covers the whole longitude range  ( -180 <= lon <= + 180 )
                // then of course the "inner" bounding box is "inside"
                if (d->m_west == -GeoDataLongitude::halfCircle && d->m_east == +GeoDataLongitude::halfCircle) {
                    return true;
                }
            }

        }
    }

    return false;
}

bool GeoDataLatLonBox::intersects( const GeoDataLatLonBox &other ) const
{
    if ( isEmpty() || other.isEmpty() ) {
        return false;
    }

    // check the intersection criterion for the latitude first:

    // Case 1: northern boundary of other box intersects:
    if (   (d->m_north >= other.d->m_north && d->m_south <= other.d->m_north)
           // Case 2: northern boundary of this box intersects:
           || (other.d->m_north >= d->m_north && other.d->m_south <= d->m_north)
           // Case 3: southern boundary of other box intersects:
           || (d->m_north >= other.d->m_south && d->m_south <= other.d->m_south)
           // Case 4: southern boundary of this box intersects:
           || (other.d->m_north >= d->m_south && other.d->m_south <= d->m_south)) {

        if ( !crossesDateLine() ) {
            if ( !other.crossesDateLine() ) {
                // "Normal" case: both bounding boxes don't cross the date line
                // Case 1: eastern boundary of other box intersects:
                if (    (d->m_east >= other.d->m_east && d->m_west <= other.d->m_east)
                        // Case 2: eastern boundary of this box intersects:
                        || (other.d->m_east >= d->m_east && other.d->m_west <= d->m_east)
                        // Case 3: western boundary of other box intersects:
                        || (d->m_east >= other.d->m_west && d->m_west <= other.d->m_west)
                        // Case 4: western boundary of this box intersects:
                        || (other.d->m_east >= d->m_west && other.d->m_west <= d->m_west)) {
                    return true;
                }
            }
            else {
                // The other bounding box crosses the date line, "this" one does not:
                // So the date line splits the other bounding box in two parts.

                if ( d->m_west <= other.d->m_east || d->m_east >= other.d->m_west) {
                    return true;
                }
            }
        }
        else {
            if ( other.crossesDateLine() ) {
                // The trivial case: both bounding boxes cross the date line and intersect
                return true;
            }
            else {
                // "This" bounding box crosses the date line, the other one does not.
                // So the date line splits "this" bounding box in two parts.
                //
                // This also covers the case where this bounding box covers the whole
                // longitude range ( -180 <= lon <= + 180 ).
                if ( other.d->m_west <= d->m_east || other.d->m_east >= d->m_west ) {
                    return true;
                }
            }
        }
    }

    return false;
}

GeoDataLatLonBox GeoDataLatLonBox::united( const GeoDataLatLonBox& other ) const
{
    if ( isEmpty() ) {
        return other;
    }

    if ( other.isEmpty() ) {
        return *this;
    }

    GeoDataLatLonBox result;

    // use the position of the centers of the boxes to determine the "smallest"
    // box (i.e. should the total box go through IDL or not). this
    // determination does not depend on one box or the other crossing IDL too
    GeoDataCoordinates c1 = center();
    GeoDataCoordinates c2 = other.center();

    // do latitude first, quite simple
    result.setNorth(qMax( d->m_north, other.north() ) );
    result.setSouth( qMin( d->m_south, other.south() ) );

    GeoDataLongitude w1 = d->m_west;
    GeoDataLongitude w2 = other.west();
    GeoDataLongitude e1 = d->m_east;
    GeoDataLongitude e2 = other.east();

    bool const idl1 = d->m_east < d->m_west;
    bool const idl2 = other.d->m_east < other.d->m_west;

    if ( idl1 ) {
        w1 += 2 * GeoDataLongitude::halfCircle;
        e1 += 2 * GeoDataLongitude::halfCircle;
    }
    if ( idl2 ) {
        w2 += 2 * GeoDataLongitude::halfCircle;
        e2 += 2 * GeoDataLongitude::halfCircle;
    }

    // in the usual case, we take the maximum of east bounds, and
    // the minimum of west bounds. The exceptions are:
    // - centers of boxes are more than 180 apart
    //    (so the smallest box should go around the IDL)
    //
    // - 1 but not 2 boxes are crossing IDL
    if (qAbs(c2.longitude() - c1.longitude()) > GeoDataLongitude::halfCircle
         || ( idl1 ^ idl2 ) ) {
        // exceptions, we go the unusual way:
        // min of east, max of west
        result.setEast( qMin( e1, e2 ) );
        result.setWest( qMax( w1, w2 ) );
    }
    else {
        // normal case, max of east, min of west
        result.setEast( qMax( e1, e2 ) );
        result.setWest( qMin( w1, w2 ) );
    }
    return result;
}

GeoDataLatLonBox GeoDataLatLonBox::toCircumscribedRectangle() const
{
    QVector<GeoDataCoordinates> coordinates;
    coordinates.reserve(4);

    coordinates.append( GeoDataCoordinates( west(), north() ) );
    coordinates.append( GeoDataCoordinates( west(), south() ) );
    coordinates.append( GeoDataCoordinates( east() + ( crossesDateLine() ? 2 * GeoDataLongitude::halfCircle : GeoDataLongitude::null ), north() ) );
    coordinates.append( GeoDataCoordinates( east() + ( crossesDateLine() ? 2 * GeoDataLongitude::halfCircle : GeoDataLongitude::null ), south() ) );

    const qreal cosRotation = cos(rotation().toRadian());
    const qreal sinRotation = sin(rotation().toRadian());

    GeoDataLatitude centerLat = center().latitude();
    GeoDataLongitude centerLon = center().longitude();
    if (GeoDataLatLonBox(GeoDataLatitude::null, GeoDataLatitude::null, center().longitude(), west()).crossesDateLine()) {
        if (centerLon == GeoDataLongitude::null) {
            centerLon += GeoDataLongitude::halfCircle;
        }
        else {
            centerLon += 2 * GeoDataLongitude::halfCircle;
        }
    }

    GeoDataLatLonBox box;

    bool northSet = false;
    bool southSet = false;
    bool eastSet = false;
    bool westSet = false;

    for ( const GeoDataCoordinates& coord: coordinates ) {

        const GeoDataLongitude lon = coord.longitude();
        const GeoDataLatitude lat = coord.latitude();

        const GeoDataLongitude rotatedLon = (lon - centerLon) * cosRotation - GeoDataLongitude::fromRadians((lat - centerLat).toRadian() * sinRotation) + centerLon;
        const GeoDataLatitude rotatedLat = GeoDataLatitude::fromRadians((lon - centerLon).toRadian() * sinRotation) + (lat - centerLat) * cosRotation + centerLat;

        if ( !northSet || rotatedLat > box.north() ) {
            northSet = true;
            box.setNorth( rotatedLat );
        }

        if ( !southSet || rotatedLat < box.south() ) {
            southSet = true;
            box.setSouth( rotatedLat );
        }

        if ( !westSet || rotatedLon < box.west() ) {
            westSet = true;
            box.setWest( rotatedLon );
        }

        if ( !eastSet || rotatedLon > box.east() ) {
            eastSet = true;
            box.setEast( rotatedLon );
        }
    }

    box.setBoundaries(box.north(), box.south(), box.east(), box.west());

    return box;
}

GeoDataLatLonBox& GeoDataLatLonBox::operator=( const GeoDataLatLonBox &other )
{
    GeoDataObject::operator=( other );
    
    *d = *other.d;
    return *this;
}

GeoDataLatLonBox GeoDataLatLonBox::operator|( const GeoDataLatLonBox& other ) const
{
    return united( other );
}

GeoDataLatLonBox& GeoDataLatLonBox::operator|=( const GeoDataLatLonBox& other )
{
    *this = united( other );
    return *this;
}


void GeoDataLatLonBox::pack( QDataStream& stream ) const
{
    GeoDataObject::pack( stream );

    stream << d->m_north << d->m_south << d->m_east << d->m_west << d->m_rotation;
}

void GeoDataLatLonBox::unpack( QDataStream& stream )
{
    GeoDataObject::unpack( stream );

    stream >> d->m_north >> d->m_south >> d->m_east >> d->m_west >> d->m_rotation;
}

GeoDataLatLonBox GeoDataLatLonBox::fromLineString(  const GeoDataLineString& lineString  )
{
    // If the line string is empty return an empty boundingbox
    if ( lineString.isEmpty() ) {
        return GeoDataLatLonBox();
    }

    GeoDataLongitude lon;
    GeoDataLatitude lat;
    lineString.first().geoCoordinates( lon, lat );
    GeoDataCoordinates::normalizeLonLat( lon, lat );

    GeoDataLatitude north = lat;
    GeoDataLatitude south = lat;
    GeoDataLongitude west =  lon;
    GeoDataLongitude east =  lon;

    // If there's only a single node stored then the boundingbox only contains that point
    if ( lineString.size() == 1 )
        return GeoDataLatLonBox( north, south, east, west );

    // Specifies whether the polygon crosses the IDL
    bool idlCrossed = false;

    // "idlCrossState" specifies the state concerning IDL crossage.
    // This is needed in order to create optimal bounding boxes in case of covering the IDL 
    // Every time the IDL gets crossed from east to west the idlCrossState value gets 
    // increased by one.
    // Every time the IDL gets crossed from west to east the idlCrossState value gets 
    // decreased by one.
    
    int idlCrossState = 0;
    int idlMaxCrossState = 0;
    int idlMinCrossState = 0;

    // Holds values for east and west while idlCrossState != 0
    GeoDataLongitude otherWest = lon;
    GeoDataLongitude otherEast = lon;

    GeoDataLongitude previousLon = lon;

    int currentSign = (lon < GeoDataLongitude::null) ? -1 : +1;
    int previousSign = currentSign;

    QVector<GeoDataCoordinates>::ConstIterator it( lineString.constBegin() );
    QVector<GeoDataCoordinates>::ConstIterator itEnd( lineString.constEnd() );

    bool processingLastNode = false;

    while( it != itEnd ) {
        // Get coordinates and normalize them to the desired range.
        (it)->geoCoordinates( lon, lat );
        GeoDataCoordinates::normalizeLonLat( lon, lat );

        // Determining the maximum and minimum latitude
        if ( lat > north ) {
            north = lat;
        } else if ( lat < south ) {
            south = lat;
        }

        currentSign = (lon < GeoDataLongitude::null) ? -1 : +1;

        // Once the polyline crosses the dateline the covered bounding box
        // would cover the whole [-M_PI; M_PI] range.
        // When looking separately at the longitude range that gets covered
        // east and west from the IDL we get two bounding boxes (we prefix
        // the resulting longitude range on the "other side" with "other").
        // By picking the "inner" range values we get a more appropriate 
        // optimized single bounding box.

        // IDL check
        if ( previousSign != currentSign
             && qAbs(previousLon) + qAbs(lon) > GeoDataLongitude::halfCircle) {

            // Initialize values for otherWest and otherEast
            if ( idlCrossed == false ) {
                otherWest =  lon;
                otherEast =  lon;
                idlCrossed = true;
            }

            // Determine the new IDL Cross State
            if (previousLon < GeoDataLongitude::null) {
                idlCrossState++;
                if ( idlCrossState > idlMaxCrossState ) {
                    idlMaxCrossState = idlCrossState;
                }
            }
            else {
                idlCrossState--;
                if ( idlCrossState < idlMinCrossState ) {
                    idlMinCrossState = idlCrossState;
                }
            }
        }

        if ( idlCrossState == 0 ) {
            if ( lon > east ) east = lon;
            if ( lon < west ) west = lon;
        }
        else {
            if ( lon > otherEast ) otherEast = lon;
            if ( lon < otherWest ) otherWest = lon;
        }

        previousLon = lon;
        previousSign = currentSign;

        if ( processingLastNode ) {
            break;
        }
        ++it;

        if( lineString.isClosed() && it == itEnd ) {
                it = lineString.constBegin();
                processingLastNode = true;
        }
    }

    if ( idlCrossed ) {
        if ( idlMinCrossState < 0 ) {
            east = otherEast;
        }
        if ( idlMaxCrossState > 0 ) {
            west = otherWest;
        }
        if ( ( idlMinCrossState < 0 && idlMaxCrossState > 0 ) 
            || idlMinCrossState < -1  || idlMaxCrossState > 1 
            || west <= east ) {
            east = +GeoDataLongitude::halfCircle;
            west = -GeoDataLongitude::halfCircle;
            // if polygon fully in south hemisphere, contain south pole
            if (north < GeoDataLatitude::null) {
                south = -GeoDataLatitude::quaterCircle;
            } else {
                north = GeoDataLatitude::quaterCircle;
            }
        }
    }

    return GeoDataLatLonBox( north, south, east, west );
}

bool GeoDataLatLonBox::isNull() const
{
    return d->m_north == d->m_south && d->m_east == d->m_west;
}

bool GeoDataLatLonBox::isEmpty() const
{
    return *this == empty;
}

bool GeoDataLatLonBox::fuzzyCompare(const GeoDataLatLonBox& lhs,
                                           const GeoDataLatLonBox& rhs,
                                           const qreal factor)
{
    bool equal = true;

    // Check the latitude for approximate equality

    GeoDataLatitude latDelta = lhs.height() * factor;

    if (qAbs(lhs.north() - rhs.north()) > latDelta) equal = false;
    if (qAbs(lhs.south() - rhs.south()) > latDelta) equal = false;


    // Check the longitude for approximate equality

    GeoDataLongitude lonDelta = lhs.width() * factor;

    GeoDataLongitude lhsEast = lhs.east();
    GeoDataLongitude rhsEast = rhs.east();

    if (!GeoDataLatLonBox::crossesDateLine(lhsEast, rhsEast)) {
        if (qAbs(lhsEast - rhsEast) > lonDelta) equal = false;
    }
    else {
        if (lhsEast < GeoDataLongitude::null && rhsEast > GeoDataLongitude::null) {
            lhsEast += 2 * GeoDataLongitude::halfCircle;
            if (qAbs(lhsEast - rhsEast) > lonDelta) equal = false;
        }
        if (lhsEast > GeoDataLongitude::null && rhsEast < GeoDataLongitude::null) {
            rhsEast += 2 * GeoDataLongitude::halfCircle;
            if (qAbs(lhsEast - rhsEast) > lonDelta) equal = false;
        }
    }

    GeoDataLongitude lhsWest = lhs.west();
    GeoDataLongitude rhsWest = rhs.west();

    if (!GeoDataLatLonBox::crossesDateLine(lhsWest, rhsWest)) {
       if (qAbs(lhsWest - rhsWest) > lonDelta) equal = false;
    }
    else {
        if (lhsWest < GeoDataLongitude::null && rhsWest > GeoDataLongitude::null) {
            lhsWest += 2 * GeoDataLongitude::halfCircle;
            if (qAbs(lhsWest - rhsWest) > lonDelta) equal = false;
        }
        if (lhsWest > GeoDataLongitude::null && rhsWest < GeoDataLongitude::null) {
            rhsWest += 2 * GeoDataLongitude::halfCircle;
            if (qAbs(lhsWest - rhsWest) > lonDelta) equal = false;
        }
    }

    return equal;
}


void GeoDataLatLonBox::clear()
{
    *this = empty;
}
}
