//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2006-2007 Torsten Rahn <tackat@kde.org>
// Copyright 2007      Inge Wallin  <ingwa@kde.org>
// Copyright 2011      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include "Quaternion.h"

#include <cmath>
using namespace std;

#include <QString>
#include <QDebug>


using namespace Marble;

Quaternion::Quaternion()
{
//    like in libeigen we keep the quaternion uninitialized
//    set( 1.0, 0.0, 0.0, 0.0 );
}

Quaternion::Quaternion(qreal scalar, qreal x, qreal y, qreal z) :
    m_w(scalar),
    m_x(x),
    m_y(y),
    m_z(z)
{
}

Quaternion Quaternion::fromSpherical(qreal lon, qreal lat)
{
    const qreal w = 0.0;
    const qreal x = cos(lat) * sin(lon);
    const qreal y = sin(lat);
    const qreal z = cos(lat) * cos(lon);

    return Quaternion( w, x, y, z );
}

void Quaternion::getSpherical(qreal &lon, qreal &lat) const 
{
    qreal y = m_y;
    if ( y > 1.0 )
        y = 1.0;
    else if ( m_y < -1.0 )
        y = -1.0;

    lat = asin( y );

    if(m_x * m_x + m_z * m_z > 0.00005)
        lon = atan2(m_x, m_z);
    else
        lon = 0.0;
}

void Quaternion::normalize() 
{
    (*this) *= 1.0 / length();
}

qreal Quaternion::scalar() const
{
    return m_w;
}

qreal Quaternion::x() const
{
    return m_x;
}

qreal Quaternion::y() const
{
    return m_y;
}

qreal Quaternion::z() const
{
    return m_z;
}

qreal Quaternion::length() const
{
    return sqrt(m_w * m_w + m_x * m_x + m_y * m_y + m_z * m_z);
}

Quaternion& Quaternion::operator*=(qreal mult)
{
    (*this) = (*this) * mult;

    return *this;
}

Quaternion Quaternion::inverse() const
{
    Quaternion  inverse( m_w, -m_x, -m_y, -m_z );
    inverse.normalize();

    return inverse;
}

Quaternion Quaternion::log() const
{
    double const qlen = length();
    double const vlen = sqrt(m_x*m_x + m_y*m_y + m_z*m_z);
    double const a = acos(m_w/qlen) / vlen;
    return Quaternion(std::log(qlen), m_x * a, m_y * a, m_z * a);
}

Quaternion Quaternion::exp() const
{
    double const vlen = sqrt(m_x*m_x + m_y*m_y + m_z*m_z);
    double const s = std::exp(m_w);
    double const a = s * sin(vlen) / vlen;
    return Quaternion(s * cos(vlen), m_x * a, m_y * a, m_z * a);
}

Quaternion Quaternion::fromEuler(qreal pitch, qreal yaw, qreal roll)
{
    const qreal cPhi = cos(0.5 * pitch); // also: "heading"
    const qreal cThe = cos(0.5 * yaw);   // also: "attitude"
    const qreal cPsi = cos(0.5 * roll);  // also: "bank"

    const qreal sPhi = sin(0.5 * pitch);
    const qreal sThe = sin(0.5 * yaw);
    const qreal sPsi = sin(0.5 * roll);

    const qreal w = cPhi * cThe * cPsi + sPhi * sThe * sPsi;
    const qreal x = sPhi * cThe * cPsi - cPhi * sThe * sPsi;
    const qreal y = cPhi * sThe * cPsi + sPhi * cThe * sPsi;
    const qreal z = cPhi * cThe * sPsi - sPhi * sThe * cPsi;

    return Quaternion( w, x, y, z );
}

qreal Quaternion::pitch() const // "heading", phi
{
    return atan2(         2.0*(m_x*m_w-m_y*m_z),
                  ( 1.0 - 2.0*(m_x*m_x+m_z*m_z) ) );
}

qreal Quaternion::yaw() const // "attitude", theta
{
    return atan2(         2.0*(m_y*m_w-m_x*m_z),
                  ( 1.0 - 2.0*(m_y*m_y+m_z*m_z) ) );
}

qreal Quaternion::roll() const // "bank", psi 
{
    return asin(2.0*(m_x*m_y+m_z*m_w));
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug debug, const Quaternion &q)
{
    QString quatdisplay = QString("Quaternion: w= %1, x= %2, y= %3, z= %4, |q|= %5" )
        .arg(q.scalar()).arg(q.x()).arg(q.y()).arg(q.z()).arg(q.length());

    debug << quatdisplay;

    return debug;
}
#endif

Quaternion& Quaternion::operator*=(const Quaternion &q)
{
    (*this) = (*this) * q;

    return *this;
}

bool Quaternion::operator==(const Quaternion &q) const
{

    return ( m_w == q.m_w
         && m_x == q.m_x
         && m_y == q.m_y
         && m_z == q.m_z );
}

Quaternion Quaternion::operator*(const Quaternion &q) const
{
    const qreal w = m_w * q.m_w - m_x * q.m_x - m_y * q.m_y - m_z * q.m_z;
    const qreal x = m_w * q.m_x + m_x * q.m_w + m_y * q.m_z - m_z * q.m_y;
    const qreal y = m_w * q.m_y - m_x * q.m_z + m_y * q.m_w + m_z * q.m_x;
    const qreal z = m_w * q.m_z + m_x * q.m_y - m_y * q.m_x + m_z * q.m_w;

    return Quaternion( w, x, y, z );
}

Quaternion Quaternion::operator+(const Quaternion &q) const
{
    return Quaternion(m_w + q.m_w,
                      m_x + q.m_x,
                      m_y + q.m_y,
                      m_z + q.m_z);
}

Quaternion Quaternion::operator*(qreal factor) const
{
    return Quaternion( m_w * factor, m_x * factor, m_y * factor, m_z * factor );
}

void Quaternion::rotateAroundAxis(const Quaternion &q)
{
    const qreal w = + m_x * q.m_x + m_y * q.m_y + m_z * q.m_z;
    const qreal x = + m_x * q.m_w - m_y * q.m_z + m_z * q.m_y;
    const qreal y = + m_x * q.m_z + m_y * q.m_w - m_z * q.m_x;
    const qreal z = - m_x * q.m_y + m_y * q.m_x + m_z * q.m_w;

    (*this) = q * Quaternion( w, x, y, z );
}

Quaternion Quaternion::slerp(const Quaternion &q1, const Quaternion &q2, qreal t)
{
    qreal  p1;
    qreal  p2;

    // Let alpha be the angle between the two quaternions.
    qreal  cosAlpha = ( q1.m_x * q2.m_x
                         + q1.m_y * q2.m_y
                         + q1.m_z * q2.m_z
                         + q1.m_w * q2.m_w );
    qreal  alpha    = acos( cosAlpha );
    qreal  sinAlpha = sin( alpha );

    if ( sinAlpha > 0.0 ) {
        p1 = sin( ( 1.0 - t ) * alpha ) / sinAlpha;
        p2 = sin( t           * alpha ) / sinAlpha;
    } else {
        // both Quaternions are equal
        p1 = 1.0;
        p2 = 0.0;
    }

    const qreal w = p1 * q1.m_w + p2 * q2.m_w;
    const qreal x = p1 * q1.m_x + p2 * q2.m_x;
    const qreal y = p1 * q1.m_y + p2 * q2.m_y;
    const qreal z = p1 * q1.m_z + p2 * q2.m_z;

    return Quaternion( w, x, y, z );
}

Quaternion Quaternion::nlerp(const Quaternion &q1, const Quaternion &q2, qreal t)
{
    const qreal p1 = 1.0 - t;

    const qreal w = p1 * q1.m_w + t * q2.m_w;
    const qreal x = p1 * q1.m_x + t * q2.m_x;
    const qreal y = p1 * q1.m_y + t * q2.m_y;
    const qreal z = p1 * q1.m_z + t * q2.m_z;

    Quaternion result( w, x, y, z );
    result.normalize();

    return result;
}

void Quaternion::toMatrix(matrix &m) const
{

    const qreal xy = m_x * m_y, xz = m_x * m_z;
    const qreal yy = m_y * m_y, yw = m_y * m_w;
    const qreal zw = m_z * m_w, zz = m_z * m_z;

    m[0][0] = 1.0 - 2.0 * (yy + zz);
    m[0][1] = 2.0 * (xy + zw);
    m[0][2] = 2.0 * (xz - yw);
    m[0][3] = 0.0;

    const qreal xx = m_x * m_x;
    const qreal xw = m_x * m_w;
    const qreal yz = m_y * m_z;

    m[1][0] = 2.0 * (xy - zw);
    m[1][1] = 1.0 - 2.0 * (xx + zz);
    m[1][2] = 2.0 * (yz + xw);
    m[1][3] = 0.0;

    m[2][0] = 2.0 * (xz + yw);
    m[2][1] = 2.0 * (yz - xw);
    m[2][2] = 1.0 - 2.0 * (xx + yy);
    m[2][3] = 0.0;
}

void Quaternion::rotateAroundAxis(const matrix &m)
{
    const qreal x =  m[0][0] * m_x + m[1][0] * m_y + m[2][0] * m_z;
    const qreal y =  m[0][1] * m_x + m[1][1] * m_y + m[2][1] * m_z;
    const qreal z =  m[0][2] * m_x + m[1][2] * m_y + m[2][2] * m_z;

    m_w = 1.0;
    m_x = x;
    m_y = y;
    m_z = z;
}
