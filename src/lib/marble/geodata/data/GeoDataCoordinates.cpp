//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2004-2007 Torsten Rahn <tackat@kde.org>
// Copyright 2007-2008 Inge Wallin  <ingwa@kde.org>
// Copyright 2008      Patrick Spendrin <ps_ml@gmx.de>
// Copyright 2011      Friedrich W. H. Kossebau <kossebau@kde.org>
// Copyright 2011      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
// Copyright 2015      Alejandro Garcia Montoro <alejandro.garciamontoro@gmail.com>
//


#include "GeoDataCoordinates.h"
#include "GeoDataCoordinates_p.h"
#include "LonLatParser_p.h"

#include <qmath.h>
#include <QDataStream>
#include <QPointF>

#include "MarbleGlobal.h"
#include "MarbleDebug.h"
#include "MarbleMath.h"

#include "Quaternion.h"

namespace Marble
{

const qreal GeoDataCoordinatesPrivate::sm_semiMajorAxis = 6378137.0;
const qreal GeoDataCoordinatesPrivate::sm_semiMinorAxis = 6356752.314;
const qreal GeoDataCoordinatesPrivate::sm_eccentricitySquared = 6.69437999013e-03;
const qreal GeoDataCoordinatesPrivate::sm_utmScaleFactor = 0.9996;
GeoDataCoordinates::Notation GeoDataCoordinates::s_notation = GeoDataCoordinates::DMS;

const GeoDataCoordinates GeoDataCoordinates::null = GeoDataCoordinates(GeoDataLongitude::null, GeoDataLatitude::null, 0 ); // don't use default constructor!

GeoDataCoordinates::GeoDataCoordinates(GeoDataLongitude lon, GeoDataLatitude lat, qreal alt, int detail) :
    d(new GeoDataCoordinatesPrivate(lon, lat, alt, detail))
{
    d->ref.ref();
}

/* simply copy the d pointer
* it will be replaced in the detach function instead
*/
GeoDataCoordinates::GeoDataCoordinates( const GeoDataCoordinates& other )
  : d( other.d )
{
    d->ref.ref();
}

/* simply copy null's d pointer
 * it will be replaced in the detach function
 */
GeoDataCoordinates::GeoDataCoordinates()
  : d( null.d )
{
    d->ref.ref();
}

/*
 * only delete the private d pointer if the number of references is 0
 * remember that all copies share the same d pointer!
 */
GeoDataCoordinates::~GeoDataCoordinates()
{
    delete d->m_q;
    d->m_q = nullptr;

    if (!d->ref.deref())
        delete d;
#ifdef DEBUG_GEODATA
//    mDebug() << "delete coordinates";
#endif
}

bool GeoDataCoordinates::isValid() const
{
    return d != null.d;
}

/*
 * if only one copy exists, return
 * else make a new private d pointer object and assign the values of the current
 * one to it
 * at the end, if the number of references thus reaches 0 delete it
 * this state shouldn't happen, but if it does, we have to clean up behind us.
 */
void GeoDataCoordinates::detach()
{    
    delete d->m_q;
    d->m_q = nullptr;

    if(d->ref.load() == 1) {
        return;
    }

    GeoDataCoordinatesPrivate *new_d = new GeoDataCoordinatesPrivate( *d );

    if (!d->ref.deref()) {
        delete d;
    }

    d = new_d;
    d->ref.ref();
}

/*
 * call detach() at the start of all non-static, non-const functions
 */
void GeoDataCoordinates::set(GeoDataLongitude lon, GeoDataLatitude lat, qreal alt)
{
    detach();
    d->m_altitude = alt;
    d->m_lon = lon;
    d->m_lat = lat;
}

/*
 * call detach() at the start of all non-static, non-const functions
 */
void GeoDataCoordinates::setLongitude(GeoDataLongitude lon)
{
    detach();
    d->m_lon = lon;
}


/*
 * call detach() at the start of all non-static, non-const functions
 */
void GeoDataCoordinates::setLatitude(GeoDataLatitude lat)
{
    detach();
    d->m_lat = lat;
}

void GeoDataCoordinates::geoCoordinates(GeoDataLongitude &lon, GeoDataLatitude &lat) const
{
    lon = d->m_lon;
    lat = d->m_lat;
}

void GeoDataCoordinates::geoCoordinates(GeoDataLongitude &lon, GeoDataLatitude &lat, qreal &alt) const
{
    geoCoordinates(lon, lat);
    alt = d->m_altitude;
}

GeoDataLongitude GeoDataCoordinates::longitude() const
{
    return d->m_lon;
}

GeoDataLatitude GeoDataCoordinates::latitude() const
{
    return d->m_lat;
}

//static
GeoDataCoordinates::Notation GeoDataCoordinates::defaultNotation()
{
    return s_notation;
}

//static
void GeoDataCoordinates::setDefaultNotation( GeoDataCoordinates::Notation notation )
{
    s_notation = notation;
}

//static
GeoDataLongitude GeoDataCoordinates::normalizeLon(GeoDataLongitude lon)
{
    const GeoDataLongitude halfCircle = GeoDataLongitude::halfCircle;

    if ( lon > halfCircle ) {
        int cycles = (int)( ( lon + halfCircle ) / ( 2 * halfCircle ) );
        return lon - ( cycles * 2 * halfCircle );
    }

    if ( lon < -halfCircle ) {
        int cycles = (int)( ( lon - halfCircle ) / ( 2 * halfCircle ) );
        return lon - ( cycles * 2 * halfCircle );
    }

    return lon;
}

//static
GeoDataLatitude GeoDataCoordinates::normalizeLat(GeoDataLatitude lat)
{
    const GeoDataLatitude halfCircle = 2 * GeoDataLatitude::quaterCircle;

    if ( lat > ( halfCircle / 2.0 ) ) {
        int cycles = (int)( ( lat + halfCircle ) / ( 2 * halfCircle ) );
        GeoDataLatitude temp;
        if (cycles == 0) { // pi/2 < lat < pi
            temp = halfCircle - lat;
        } else {
            temp = lat - ( cycles * 2 * halfCircle );
        }

        if ( temp > ( halfCircle / 2.0 ) ) {
            return ( halfCircle - temp );
        }

        if ( temp < ( -halfCircle / 2.0 ) ) {
            return ( -halfCircle - temp );
        }

        return temp;
    }

    if ( lat < ( -halfCircle / 2.0 ) ) {
        int cycles = (int)( ( lat - halfCircle ) / ( 2 * halfCircle ) );
        GeoDataLatitude temp;
        if( cycles == 0 ) { 
            temp = -halfCircle - lat;
        } else {
            temp = lat - ( cycles * 2 * halfCircle );
        }

        if ( temp > ( +halfCircle / 2.0 ) ) {
            return ( +halfCircle - temp );
        }

        if ( temp < ( -halfCircle / 2.0 ) ) {
            return ( -halfCircle - temp );
        }

        return temp;
    }

    return lat;
}

//static
void GeoDataCoordinates::normalizeLonLat(GeoDataLongitude &lon, GeoDataLatitude &lat)
{
    lon = normalizeLon(lon);
    lat = normalizeLat(lat);
}

GeoDataCoordinates GeoDataCoordinates::fromString( const QString& string, bool& successful )
{
    LonLatParser parser;
    successful = parser.parse(string);
    if (successful) {
        return GeoDataCoordinates(parser.lon(), parser.lat(), 0);
    } else {
        return GeoDataCoordinates();
    }
}


QString GeoDataCoordinates::toString() const
{
    return GeoDataCoordinates::toString( s_notation );
}

QString GeoDataCoordinates::toString( GeoDataCoordinates::Notation notation, int precision ) const
{
        QString coordString;

        if( notation == GeoDataCoordinates::UTM ){
            int zoneNumber = GeoDataCoordinatesPrivate::lonLatToZone(d->m_lon, d->m_lat);

            // Handle lack of UTM zone number in the poles
            const QString zoneString = (zoneNumber > 0) ? QString::number(zoneNumber) : QString();

            QString bandString = GeoDataCoordinatesPrivate::lonLatToLatitudeBand(d->m_lon, d->m_lat);

            QString eastingString  = QString::number(GeoDataCoordinatesPrivate::lonLatToEasting(d->m_lon, d->m_lat), 'f', 2);
            QString northingString = QString::number(GeoDataCoordinatesPrivate::lonLatToNorthing(d->m_lon, d->m_lat), 'f', 2);

            return QString("%1%2 %3 m E, %4 m N").arg(zoneString, bandString, eastingString, northingString);
        }
        else{
            coordString = lonToString( d->m_lon, notation, precision )
                        + QLatin1String(", ")
                        + latToString( d->m_lat, notation, precision );
        }

        return coordString;
}

QString GeoDataCoordinates::lonToString(GeoDataLongitude lon, GeoDataCoordinates::Notation notation,
                                                    int precision,
                                                    char format )
{
    if( notation == GeoDataCoordinates::UTM ){
        /**
         * @FIXME: UTM needs lon + lat to know zone number and easting
         * By now, this code returns the zone+easting of the point
         * (lon, equator), but this can differ a lot at different locations
         * See bug 347536 https://bugs.kde.org/show_bug.cgi?id=347536
         */
        const int zoneNumber = GeoDataCoordinatesPrivate::lonLatToZone(lon, GeoDataLatitude::null);

        // Handle lack of UTM zone number in the poles
        QString result = (zoneNumber > 0) ? QString::number(zoneNumber) : QString();

        if(precision > 0){
            const QString eastingString = QString::number(GeoDataCoordinatesPrivate::lonLatToEasting(lon, GeoDataLatitude::null), 'f', 2);
            result += QString(" %1 m E").arg(eastingString);
        }

        return result;
    }

    const QString weString = (lon < GeoDataLongitude::null) ? tr("W") : tr("E");

    QString lonString;

    // Take care of -1 case
    precision = ( precision < 0 ) ? 5 : precision;
    
    if ( notation == DMS || notation == DM ) {
        qreal lonDegF = qAbs(lon.toDegree());
        int lonDeg = (int) lonDegF;
        qreal lonMinF = 60 * (lonDegF - lon.toDegree());
        int lonMin = (int) lonMinF;
        qreal lonSecF = 60 * (lonMinF - lonMin);
        int lonSec = (int) lonSecF;

        // Adjustment for fuzziness (like 49.999999999999999999999)
        if ( precision == 0 ) {
            lonDeg = qRound( lonDegF );
        } else if ( precision <= 2 ) {
            lonMin = qRound( lonMinF );
        } else if ( precision <= 4 && notation == DMS ) {
            lonSec = qRound( lonSecF );
        } else {
            if ( notation == DMS ) {
                lonSec = lonSecF = qRound( lonSecF * qPow( 10, precision - 4 ) ) / qPow( 10, precision - 4 );
            }
            else {
                lonMin = lonMinF = qRound( lonMinF * qPow( 10, precision - 2 ) ) / qPow( 10, precision - 2 );
            }
        }

        if (lonSec > 59 && notation == DMS ) {
            lonSec = lonSecF = 0;
            lonMin = lonMinF = lonMinF + 1;
        }
        if (lonMin > 59) {
            lonMin = lonMinF = 0;
            lonDeg = lonDegF = lonDegF + 1;
        }

        // Evaluate the string
        lonString = QString::fromUtf8("%1\xc2\xb0").arg(lonDeg, 3, 10, QLatin1Char(' '));

        if ( precision == 0 ) {
            return lonString + weString;
        }

        if ( notation == DMS || precision < 3 ) {
            lonString += QString(" %2\'").arg(lonMin, 2, 10, QLatin1Char('0'));
        }

        if ( precision < 3 ) {
            return lonString + weString;
        }

        if ( notation == DMS ) {
            // Includes -1 case!
            if ( precision < 5 ) {
                lonString += QString(" %3\"").arg(lonSec, 2, 'f', 0, QLatin1Char('0'));
                return lonString + weString;
            }

            lonString += QString(" %L3\"").arg(lonSecF, precision - 1, 'f', precision - 4, QLatin1Char('0'));
        }
        else {
            lonString += QString(" %L3'").arg(lonMinF, precision + 1, 'f', precision - 2, QLatin1Char('0'));
        }
    }
    else if ( notation == GeoDataCoordinates::Decimal )
    {
        lonString = QString::fromUtf8("%L1\xc2\xb0").arg(qAbs(lon).toDegree(), 4 + precision, format, precision, QLatin1Char(' '));
    }
    else if ( notation == GeoDataCoordinates::Astro )
    {
        if (lon < GeoDataLongitude::null) {
            lon += 2 * GeoDataLongitude::halfCircle;
        }

        qreal lonHourF = qAbs(lon).toDegree() / 15.0;
        int lonHour = (int) lonHourF;
        qreal lonMinF = 60 * (lonHourF - lonHour);
        int lonMin = (int) lonMinF;
        qreal lonSecF = 60 * (lonMinF - lonMin);
        int lonSec = (int) lonSecF;

        // Adjustment for fuzziness (like 49.999999999999999999999)
        if ( precision == 0 ) {
            lonHour = qRound( lonHourF );
        } else if ( precision <= 2 ) {
            lonMin = qRound( lonMinF );
        } else if ( precision <= 4 ) {
            lonSec = qRound( lonSecF );
        } else {
            lonSec = lonSecF = qRound( lonSecF * qPow( 10, precision - 4 ) ) / qPow( 10, precision - 4 );
        }

        if (lonSec > 59 ) {
            lonSec = lonSecF = 0;
            lonMin = lonMinF = lonMinF + 1;
        }
        if (lonMin > 59) {
            lonMin = lonMinF = 0;
            lonHour = lonHourF = lonHourF + 1;
        }

        // Evaluate the string
        lonString = QString::fromUtf8("%1h").arg(lonHour, 3, 10, QLatin1Char(' '));

        if ( precision == 0 ) {
            return lonString;
        }

        lonString += QString(" %2\'").arg(lonMin, 2, 10, QLatin1Char('0'));

        if ( precision < 3 ) {
            return lonString;
        }

        // Includes -1 case!
        if ( precision < 5 ) {
            lonString += QString(" %3\"").arg(lonSec, 2, 'f', 0, QLatin1Char('0'));
            return lonString;
        }

        lonString += QString(" %L3\"").arg(lonSecF, precision - 1, 'f', precision - 4, QLatin1Char('0'));
        return lonString;
    }

    return lonString + weString;
}

QString GeoDataCoordinates::lonToString() const
{
    return GeoDataCoordinates::lonToString( d->m_lon , s_notation );
}

QString GeoDataCoordinates::latToString(GeoDataLatitude lat, GeoDataCoordinates::Notation notation,
                                                    int precision,
                                                    char format )
{
    if( notation == GeoDataCoordinates::UTM ){
        /**
         * @FIXME: UTM needs lon + lat to know latitude band and northing
         * By now, this code returns the band+northing of the point
         * (meridian, lat), but this can differ a lot at different locations
         * See bug 347536 https://bugs.kde.org/show_bug.cgi?id=347536
         */

        QString result = GeoDataCoordinatesPrivate::lonLatToLatitudeBand(GeoDataLongitude::null, lat);

        if ( precision > 0 ){
            const QString northingString = QString::number( GeoDataCoordinatesPrivate::lonLatToNorthing(GeoDataLongitude::null, lat), 'f', 2 );
            result += QString(" %1 m N").arg(northingString);
        }

        return result;
    }

    QString pmString;
    QString nsString;

    if ( notation == GeoDataCoordinates::Astro ) {
        pmString = (lat > GeoDataLatitude::null) ? "+" : "-";
    }
    else {
        nsString = (lat > GeoDataLatitude::null) ? tr("N") : tr("S");
    }

    QString latString;

    // Take care of -1 case
    precision = ( precision < 0 ) ? 5 : precision;
    
    if ( notation == DMS || notation == DM || notation == Astro) {
        qreal latDegF = qAbs(lat).toDegree();
        int latDeg = (int) latDegF;
        qreal latMinF = 60 * (latDegF - latDeg);
        int latMin = (int) latMinF;
        qreal latSecF = 60 * (latMinF - latMin);
        int latSec = (int) latSecF;

        // Adjustment for fuzziness (like 49.999999999999999999999)
        if ( precision == 0 ) {
            latDeg = qRound( latDegF );
        } else if ( precision <= 2 ) {
            latMin = qRound( latMinF );
        } else if ( precision <= 4 && notation == DMS ) {
            latSec = qRound( latSecF );
        } else {
            if ( notation == DMS || notation == Astro ) {
                latSec = latSecF = qRound( latSecF * qPow( 10, precision - 4 ) ) / qPow( 10, precision - 4 );
            }
            else {
                latMin = latMinF = qRound( latMinF * qPow( 10, precision - 2 ) ) / qPow( 10, precision - 2 );
            }
        }

        if (latSec > 59 && ( notation == DMS || notation == Astro )) {
            latSecF = 0;
            latSec = latSecF;
            latMin = latMin + 1;
        }
        if (latMin > 59) {
            latMinF = 0;
            latMin = latMinF;
            latDeg = latDeg + 1;
        }

        // Evaluate the string
        latString = QString::fromUtf8("%1\xc2\xb0").arg(latDeg, 3, 10, QLatin1Char(' '));

        if ( precision == 0 ) {
            return pmString + latString + nsString;
        }

        if ( notation == DMS || notation == Astro || precision < 3 ) {
            latString += QString(" %2\'").arg(latMin, 2, 10, QLatin1Char('0'));
        }

        if ( precision < 3 ) {
            return pmString + latString + nsString;
        }

        if ( notation == DMS || notation == Astro ) {
            // Includes -1 case!
            if ( precision < 5 ) {
                latString += QString(" %3\"").arg(latSec, 2, 'f', 0, QLatin1Char('0'));
                return latString + nsString;
            }

            latString += QString(" %L3\"").arg(latSecF, precision - 1, 'f', precision - 4, QLatin1Char('0'));
        }
        else {
            latString += QString(" %L3'").arg(latMinF, precision + 1, 'f', precision - 2, QLatin1Char('0'));
        }
    }
    else // notation = GeoDataCoordinates::Decimal
    {
        latString = QString::fromUtf8("%L1\xc2\xb0").arg(qAbs(lat).toDegree(), 4 + precision, format, precision, QLatin1Char(' '));
    }
    return pmString + latString + nsString;
}

QString GeoDataCoordinates::latToString() const
{
    return GeoDataCoordinates::latToString( d->m_lat, s_notation );
}

bool GeoDataCoordinates::operator==( const GeoDataCoordinates &rhs ) const
{
    return *d == *rhs.d;
}

bool GeoDataCoordinates::operator!=( const GeoDataCoordinates &rhs ) const
{
    return *d != *rhs.d;
}

void GeoDataCoordinates::setAltitude( const qreal altitude )
{
    detach();
    d->m_altitude = altitude;
}

qreal GeoDataCoordinates::altitude() const
{
    return d->m_altitude;
}

int GeoDataCoordinates::utmZone() const{
    return GeoDataCoordinatesPrivate::lonLatToZone(d->m_lon, d->m_lat);
}

qreal GeoDataCoordinates::utmEasting() const{
    return GeoDataCoordinatesPrivate::lonLatToEasting(d->m_lon, d->m_lat);
}

QString GeoDataCoordinates::utmLatitudeBand() const{
    return GeoDataCoordinatesPrivate::lonLatToLatitudeBand(d->m_lon, d->m_lat);
}

qreal GeoDataCoordinates::utmNorthing() const{
    return GeoDataCoordinatesPrivate::lonLatToNorthing(d->m_lon, d->m_lat);
}

quint8 GeoDataCoordinates::detail() const
{
    return d->m_detail;
}

void GeoDataCoordinates::setDetail(quint8 detail)
{
    detach();
    d->m_detail = detail;
}

GeoDataCoordinates GeoDataCoordinates::rotateAround(const GeoDataCoordinates &axis, GeoDataAngle angle) const
{
    const Quaternion quatAxis = Quaternion::fromEuler(-axis.latitude().toRadian(), axis.longitude().toRadian(), 0);
    const Quaternion rotationAmount = Quaternion::fromEuler(0, 0, angle.toRadian());
    const Quaternion resultAxis = quatAxis * rotationAmount * quatAxis.inverse();

    return rotateAround(resultAxis);
}

GeoDataCoordinates GeoDataCoordinates::rotateAround(const Quaternion &rotAxis) const
{
    Quaternion rotatedQuat = quaternion();
    rotatedQuat.rotateAroundAxis(rotAxis);

    qreal rotatedLon, rotatedLat;
    rotatedQuat.getSpherical(rotatedLon, rotatedLat);

    return GeoDataCoordinates(GeoDataLongitude::fromRadians(rotatedLon), GeoDataLatitude::fromRadians(rotatedLat), d->m_altitude);
}

GeoDataAngle GeoDataCoordinates::bearing(const GeoDataCoordinates &other, BearingType type) const
{
    if ( type == FinalBearing ) {
        const GeoDataAngle offset = GeoDataAngle::fromRadians(M_PI);
        return offset + other.bearing(*this, InitialBearing);
    }

    const GeoDataLongitude delta = other.d->m_lon - d->m_lon;
    const double bearing =
            atan2(sin(delta.toRadian()) * cos(other.d->m_lat.toRadian()),
                  cos(d->m_lat.toRadian()) * sin(other.d->m_lat.toRadian()) - sin(d->m_lat.toRadian()) * cos(other.d->m_lat.toRadian()) * cos(delta.toRadian()));
    return GeoDataAngle::fromRadians(bearing);
}

GeoDataCoordinates GeoDataCoordinates::moveByBearing(GeoDataAngle bearing, qreal distance) const
{
    const GeoDataLatitude newLat = GeoDataLatitude::fromRadians(
                asin(sin(d->m_lat.toRadian()) * cos(distance) +
                     cos(d->m_lat.toRadian()) * sin(distance) * cos(bearing.toRadian())));
    const GeoDataLongitude newLon = d->m_lon + GeoDataLongitude::fromRadians(
                atan2(sin(bearing.toRadian()) * sin(distance) * cos(d->m_lat.toRadian()),
                      cos(distance) - sin(d->m_lat.toRadian()) * sin(newLat.toRadian())));

    return GeoDataCoordinates( newLon, newLat );
}

const Quaternion& GeoDataCoordinates::quaternion() const
{
    if (d->m_q == nullptr) {
        d->m_q = new Quaternion(Quaternion::fromSpherical(d->m_lon.toRadian(), d->m_lat.toRadian()));
    }
    return *d->m_q;
}

GeoDataCoordinates GeoDataCoordinates::interpolate( const GeoDataCoordinates &target, double t_ ) const
{
    double const t = qBound( 0.0, t_, 1.0 );
    Quaternion const quat = Quaternion::slerp( quaternion(), target.quaternion(), t );
    qreal lon, lat;
    quat.getSpherical( lon, lat );
    double const alt = (1.0-t) * d->m_altitude + t * target.d->m_altitude;
    return GeoDataCoordinates(GeoDataLongitude::fromRadians(lon), GeoDataLatitude::fromRadians(lat), alt);
}

GeoDataCoordinates GeoDataCoordinates::nlerp(const GeoDataCoordinates &target, double t) const
{
    qreal  lon = 0.0;
    qreal  lat = 0.0;

    const Quaternion itpos = Quaternion::nlerp(quaternion(), target.quaternion(), t);
    itpos.getSpherical(lon, lat);

    const qreal altitude = 0.5 * (d->m_altitude + target.altitude());

    return GeoDataCoordinates(GeoDataLongitude::fromRadians(lon), GeoDataLatitude::fromRadians(lat), altitude);
}

GeoDataCoordinates GeoDataCoordinates::interpolate( const GeoDataCoordinates &before, const GeoDataCoordinates &target, const GeoDataCoordinates &after, double t_ ) const
{
    double const t = qBound( 0.0, t_, 1.0 );
    Quaternion const b1 = GeoDataCoordinatesPrivate::basePoint( before.quaternion(), quaternion(), target.quaternion() );
    Quaternion const a2 = GeoDataCoordinatesPrivate::basePoint( quaternion(), target.quaternion(), after.quaternion() );
    Quaternion const a = Quaternion::slerp( quaternion(), target.quaternion(), t );
    Quaternion const b = Quaternion::slerp( b1, a2, t );
    Quaternion c = Quaternion::slerp( a, b, 2 * t * (1.0-t) );
    qreal lon, lat;
    c.getSpherical( lon, lat );
    // @todo spline interpolation of altitude?
    double const alt = (1.0-t) * d->m_altitude + t * target.d->m_altitude;
    return GeoDataCoordinates(GeoDataLongitude::fromRadians(lon), GeoDataLatitude::fromRadians(lat), alt);
}

bool GeoDataCoordinates::isPole( Pole pole ) const
{
    // Evaluate the most likely case first:
    // The case where we haven't hit the pole and where our latitude is normalized
    // to the range of 90 deg S ... 90 deg N
    if (qAbs(d->m_lat) < GeoDataLatitude::quaterCircle) {
        return false;
    }
    else {
        if (qAbs(d->m_lat) == GeoDataLatitude::quaterCircle) {
            // Ok, we have hit a pole. Now let's check whether it's the one we've asked for:
            if ( pole == AnyPole ){
                return true;
            }
            else {
                if (pole == NorthPole && d->m_lat == +GeoDataLatitude::quaterCircle) {
                    return true;
                }
                if (pole == SouthPole && d->m_lat == -GeoDataLatitude::quaterCircle) {
                    return true;
                }
                return false;
            }
        }
        // 
        else {
            // FIXME: Should we just normalize latitude and longitude and be done?
            //        While this might work well for persistent data it would create some 
            //        possible overhead for temporary data, so this needs careful thinking.
            mDebug() << "GeoDataCoordinates not normalized!";

            // Only as a last resort we cover the unlikely case where
            // the latitude is not normalized to the range of 
            // 90 deg S ... 90 deg N
            if (qAbs(normalizeLat(d->m_lat)) < GeoDataLatitude::quaterCircle) {
                return false;
            }
            else {
                // Ok, we have hit a pole. Now let's check whether it's the one we've asked for:
                if ( pole == AnyPole ){
                    return true;
                }
                else {
                    if (pole == NorthPole && d->m_lat == +GeoDataLatitude::quaterCircle) {
                        return true;
                    }
                    if (pole == SouthPole && d->m_lat == -GeoDataLatitude::quaterCircle) {
                        return true;
                    }
                    return false;
                }
            }            
        }
    }
}

qreal GeoDataCoordinates::sphericalDistanceTo(const GeoDataCoordinates &other) const
{
    GeoDataLongitude lon2;
    GeoDataLatitude lat2;
    other.geoCoordinates( lon2, lat2 );

    // FIXME: Take the altitude into account!

    return distanceSphere(d->m_lon, d->m_lat, lon2, lat2);
}

GeoDataCoordinates& GeoDataCoordinates::operator=( const GeoDataCoordinates &other )
{
    qAtomicAssign(d, other.d);
    return *this;
}

void GeoDataCoordinates::pack( QDataStream& stream ) const
{
    stream << d->m_lon;
    stream << d->m_lat;
    stream << d->m_altitude;
}

void GeoDataCoordinates::unpack( QDataStream& stream )
{
    // call detach even though it shouldn't be needed - one never knows
    detach();
    stream >> d->m_lon;
    stream >> d->m_lat;
    stream >> d->m_altitude;
}

Quaternion GeoDataCoordinatesPrivate::basePoint( const Quaternion &q1, const Quaternion &q2, const Quaternion &q3 )
{
    Quaternion const a = (q2.inverse() * q3).log();
    Quaternion const b = (q2.inverse() * q1).log();
    return q2 * ((a+b)*-0.25).exp();
}



qreal GeoDataCoordinatesPrivate::arcLengthOfMeridian( qreal phi )
{
    // Precalculate n
    qreal const n = (GeoDataCoordinatesPrivate::sm_semiMajorAxis - GeoDataCoordinatesPrivate::sm_semiMinorAxis)
                    / (GeoDataCoordinatesPrivate::sm_semiMajorAxis + GeoDataCoordinatesPrivate::sm_semiMinorAxis);

    // Precalculate alpha
    qreal const alpha = ( (GeoDataCoordinatesPrivate::sm_semiMajorAxis + GeoDataCoordinatesPrivate::sm_semiMinorAxis) / 2.0)
                        * (1.0 + (qPow (n, 2.0) / 4.0) + (qPow (n, 4.0) / 64.0) );

    // Precalculate beta
    qreal const beta = (-3.0 * n / 2.0)
                        + (9.0 * qPow (n, 3.0) / 16.0)
                        + (-3.0 * qPow (n, 5.0) / 32.0);

    // Precalculate gamma
    qreal const gamma = (15.0 * qPow (n, 2.0) / 16.0)
                        + (-15.0 * qPow (n, 4.0) / 32.0);

    // Precalculate delta
    qreal const delta = (-35.0 * qPow (n, 3.0) / 48.0)
                        + (105.0 * qPow (n, 5.0) / 256.0);

    // Precalculate epsilon
    qreal const epsilon = (315.0 * qPow (n, 4.0) / 512.0);

    // Now calculate the sum of the series and return
    qreal const result = alpha * (phi + (beta * qSin (2.0 * phi))
                        + (gamma * qSin (4.0 * phi))
                        + (delta * qSin (6.0 * phi))
                        + (epsilon * qSin (8.0 * phi)));

    return result;
}

GeoDataLongitude GeoDataCoordinatesPrivate::centralMeridianUTM(qreal zone)
{
    return GeoDataLongitude::fromDegrees(-183.0 + (zone * 6.0));
}

qreal GeoDataCoordinatesPrivate::footpointLatitude( qreal northing )
{
    // Precalculate n (Eq. 10.18)
    qreal const n = (GeoDataCoordinatesPrivate::sm_semiMajorAxis - GeoDataCoordinatesPrivate::sm_semiMinorAxis)
                    / (GeoDataCoordinatesPrivate::sm_semiMajorAxis + GeoDataCoordinatesPrivate::sm_semiMinorAxis);

    // Precalculate alpha (Eq. 10.22)
    // (Same as alpha in Eq. 10.17)
    qreal const alpha = ((GeoDataCoordinatesPrivate::sm_semiMajorAxis + GeoDataCoordinatesPrivate::sm_semiMinorAxis) / 2.0)
                        * (1 + (qPow (n, 2.0) / 4) + (qPow (n, 4.0) / 64));

    // Precalculate y (Eq. 10.23)
    qreal const y = northing / alpha;

    // Precalculate beta (Eq. 10.22)
    qreal const beta = (3.0 * n / 2.0) + (-27.0 * qPow (n, 3.0) / 32.0)
                        + (269.0 * qPow (n, 5.0) / 512.0);

    // Precalculate gamma (Eq. 10.22)
    qreal const gamma = (21.0 * qPow (n, 2.0) / 16.0)
                        + (-55.0 * qPow (n, 4.0) / 32.0);

    // Precalculate delta (Eq. 10.22)
    qreal const delta = (151.0 * qPow (n, 3.0) / 96.0)
                        + (-417.0 * qPow (n, 5.0) / 128.0);

    // Precalculate epsilon (Eq. 10.22)
    qreal const epsilon = (1097.0 * qPow (n, 4.0) / 512.0);

    // Now calculate the sum of the series (Eq. 10.21)
    qreal const result = y + (beta * qSin (2.0 * y))
                        + (gamma * qSin (4.0 * y))
                        + (delta * qSin (6.0 * y))
                        + (epsilon * qSin (8.0 * y));

    return result;
}

QPointF GeoDataCoordinatesPrivate::mapLonLatToXY(GeoDataLongitude lambda, GeoDataLatitude phi, GeoDataLongitude lambda0)
{
    // Equation (10.15)

    // Precalculate second numerical eccentricity
    const qreal ep2 = (qPow(GeoDataCoordinatesPrivate::sm_semiMajorAxis, 2.0) - qPow(GeoDataCoordinatesPrivate::sm_semiMinorAxis, 2.0))
                    / qPow(GeoDataCoordinatesPrivate::sm_semiMinorAxis, 2.0);

    // Precalculate the square of nu, just an auxilar quantity
    const qreal nu2 = ep2 * qPow(qCos(phi.toRadian()), 2.0);

    // Precalculate the radius of curvature in prime vertical
    const qreal N = qPow(GeoDataCoordinatesPrivate::sm_semiMajorAxis, 2.0) / (GeoDataCoordinatesPrivate::sm_semiMinorAxis * qSqrt(1 + nu2));

    // Precalculate the tangent of phi and its square
    const qreal t = qTan(phi.toRadian());
    const qreal t2 = t * t;

    // Precalculate longitude difference
    const GeoDataLongitude l = lambda - lambda0;

    /*
     * Precalculate coefficients for l**n in the equations below
     * so a normal human being can read the expressions for easting
     * and northing
     * -- l**1 and l**2 have coefficients of 1.0
     *
     * The actual used coefficients starts at coef[1], just to
     * follow the meaningful nomenclature in equation 10.15
     * (coef[n] corresponds to qPow(l,n) factor)
     */

    const qreal coef1 = 1;
    const qreal coef2 = 1;
    const qreal coef3 = 1.0 - t2 + nu2;
    const qreal coef4 = 5.0 - t2 + 9 * nu2 + 4.0 * (nu2 * nu2);
    const qreal coef5 = 5.0 - 18.0 * t2 + (t2 * t2) + 14.0 * nu2 - 58.0 * t2 * nu2;
    const qreal coef6 = 61.0 - 58.0 * t2 + (t2 * t2) + 270.0 * nu2 - 330.0 * t2 * nu2;
    const qreal coef7 = 61.0 - 479.0 * t2 + 179.0 * (t2 * t2) - (t2 * t2 * t2);
    const qreal coef8 = 1385.0 - 3111.0 * t2 + 543.0 * (t2 * t2) - (t2 * t2 * t2);

    // Calculate easting (x)
    const qreal easting = N * qCos(phi.toRadian()) * coef1 * l.toRadian()
        + (N / 6.0 * qPow(qCos(phi.toRadian()), 3.0) * coef3 * qPow(l.toRadian(), 3.0))
        + (N / 120.0 * qPow(qCos(phi.toRadian()), 5.0) * coef5 * qPow(l.toRadian(), 5.0))
        + (N / 5040.0 * qPow(qCos(phi.toRadian()), 7.0) * coef7 * qPow(l.toRadian(), 7.0));

    // Calculate northing (y)
    const qreal northing = arcLengthOfMeridian (phi.toRadian())
        + (t / 2.0 * N * qPow(qCos(phi.toRadian()), 2.0) * coef2 * qPow(l.toRadian(), 2.0))
        + (t / 24.0 * N * qPow(qCos(phi.toRadian()), 4.0) * coef4 * qPow(l.toRadian(), 4.0))
        + (t / 720.0 * N * qPow(qCos(phi.toRadian()), 6.0) * coef6 * qPow(l.toRadian(), 6.0))
        + (t / 40320.0 * N * qPow(qCos(phi.toRadian()), 8.0) * coef8 * qPow(l.toRadian(), 8.0));

    return QPointF(easting, northing);
}

int GeoDataCoordinatesPrivate::lonLatToZone(GeoDataLongitude lon, GeoDataLatitude lat)
{
    // Converts lon and lat to degrees
    qreal lonDeg = lon.toDegree();
    qreal latDeg = lat.toDegree();

    /* Round the value of the longitude when the distance to the nearest integer
     * is less than 0.0000001. This avoids fuzzy values such as -114.0000000001, which
     * can produce a misbehaviour when calculating the zone associated at the borders
     * of the zone intervals (for example, the interval [-114, -108[ is associated with
     * zone number 12; if the following rounding is not done, the value returned by
     * lonLatToZone(114,0) is 11 instead of 12, as the function actually receives
     * -114.0000000001, which is in the interval [-120,-114[, associated to zone 11
     */
    qreal precision = 0.0000001;

    if ( qAbs(lonDeg - qFloor(lonDeg)) < precision || qAbs(lonDeg - qCeil(lonDeg)) < precision ){
        lonDeg = qRound(lonDeg);
    }

    // There is no numbering associated to the poles, special value 0 is returned.
    if ( latDeg < -80 || latDeg > 84 ) {
        return 0;
    }

    // Obtains the zone number handling all the so called "exceptions"
    // See problem: https://en.wikipedia.org/wiki/Universal_Transverse_Mercator_coordinate_system#Exceptions
    // See solution: https://gis.stackexchange.com/questions/13291/computing-utm-zone-from-lat-long-point

    // General
    int zoneNumber = static_cast<int>( (lonDeg+180) / 6.0 ) + 1;

    // Southwest Norway
    if ( latDeg >= 56 && latDeg < 64 && lonDeg >= 3 && lonDeg < 12 ) {
        zoneNumber = 32;
    }

    // Svalbard
    if ( latDeg >= 72 && latDeg < 84 ) {
        if ( lonDeg >= 0 && lonDeg < 9 ) {
            zoneNumber = 31;
        } else if ( lonDeg >= 9 && lonDeg < 21 ) {
            zoneNumber = 33;
        } else if ( lonDeg >= 21 && lonDeg < 33 ) {
            zoneNumber = 35;
        } else if ( lonDeg >= 33 && lonDeg < 42 ) {
            zoneNumber = 37;
        }
    }

    return zoneNumber;
}

qreal GeoDataCoordinatesPrivate::lonLatToEasting(GeoDataLongitude lon, GeoDataLatitude lat)
{
    int zoneNumber = lonLatToZone( lon, lat );

    if ( zoneNumber == 0 ){
        zoneNumber = static_cast<int>((lon.toDegree()+180) / 6.0) + 1;
    }

    QPointF coordinates = GeoDataCoordinatesPrivate::mapLonLatToXY( lon, lat, GeoDataCoordinatesPrivate::centralMeridianUTM(zoneNumber) );

    // Adjust easting and northing for UTM system
    qreal easting = coordinates.x() * GeoDataCoordinatesPrivate::sm_utmScaleFactor + 500000.0;

    return easting;
}

QString GeoDataCoordinatesPrivate::lonLatToLatitudeBand(GeoDataLongitude lon, GeoDataLatitude lat)
{
    // Obtains the latitude bands handling all the so called "exceptions"

    // Regular latitude bands between 80 S and 80 N (that is, between 10 and 170 in the [0,180] interval)
    int bandLetterIndex = 24; //Avoids "may be used uninitialized" warning

    if (lat < GeoDataLatitude::fromDegrees(-80)) {
        // South pole (A for zones 1-30, B for zones 31-60)
        bandLetterIndex = (lon < GeoDataLongitude::fromDegrees(6*31) - GeoDataLongitude::halfCircle) ? 0 : 1;
    } else if (lat >= GeoDataLatitude::fromDegrees(-80) && lat <= GeoDataLatitude::fromDegrees(80)) {
        // General (+2 because the general lettering starts in C)
        bandLetterIndex = static_cast<int>((lat.toDegree() + 80.0) / 8.0) + 2;
    } else if (lat >= GeoDataLatitude::fromDegrees(80) && lat < GeoDataLatitude::fromDegrees(84)) {
        // Band X is extended 4 more degrees
        bandLetterIndex = 21;
    } else if (lat >= GeoDataLatitude::fromDegrees(84)) {
        // North pole (Y for zones 1-30, Z for zones 31-60)
        bandLetterIndex = (lon < GeoDataLongitude::fromDegrees(6*31) - GeoDataLongitude::halfCircle) ? 22 : 23;
    }

    return QString( "ABCDEFGHJKLMNPQRSTUVWXYZ?" ).at( bandLetterIndex );
}

qreal GeoDataCoordinatesPrivate::lonLatToNorthing(GeoDataLongitude lon, GeoDataLatitude lat )
{
    int zoneNumber = lonLatToZone(lon, lat);

    if (zoneNumber == 0) {
        zoneNumber = static_cast<int>((lon.toDegree() + 180) / 6.0) + 1;
    }

    QPointF coordinates = GeoDataCoordinatesPrivate::mapLonLatToXY( lon, lat, GeoDataCoordinatesPrivate::centralMeridianUTM(zoneNumber) );

    qreal northing = coordinates.y() * GeoDataCoordinatesPrivate::sm_utmScaleFactor;

    if ( northing < 0.0 ) {
        northing += 10000000.0;
    }

    return northing;
}

uint qHash(const GeoDataCoordinates &coordinates)
{
    uint seed = ::qHash(coordinates.altitude());
    seed = ::qHash(coordinates.latitude(), seed);

    return ::qHash(coordinates.longitude(), seed);
}

}
