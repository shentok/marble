//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2018   Bernhard Beschow   <shentey@gmail.com>
//

#ifndef MARBLE_RADIANSLONGITUDE_H
#define MARBLE_RADIANSLONGITUDE_H

#include "PrimitiveTypeWrapper.h"
#include "MarbleGlobal.h"

#include <QDataStream>
#include <QMetaType>
#include <QtDebug>
#include <qglobal.h>
#include <qhashfunctions.h>

namespace Marble {

class GeoDataLongitude : public PrimitiveTypeWrapper<qreal, GeoDataLongitude>
{
public:
    struct Null {};
    struct HalfCircle {};

    GeoDataLongitude() :
        PrimitiveTypeWrapper()
    {}

    GeoDataLongitude(const PrimitiveTypeWrapper &other) :
        PrimitiveTypeWrapper(other)
    {}

    GeoDataLongitude(Null) :
        PrimitiveTypeWrapper(fromRadians(0))
    {}

    GeoDataLongitude(HalfCircle) :
        PrimitiveTypeWrapper(fromRadians(M_PI))
    {}

    inline GeoDataLongitude operator*(qreal scale) const
    {
        return GeoDataLongitude(m_value * scale);
    }

    inline qreal operator/(GeoDataLongitude other) const
    {
        return m_value / other.m_value;
    }

    inline GeoDataLongitude operator/(qreal divisor) const
    {
        return GeoDataLongitude(m_value / divisor);
    }

    inline GeoDataLongitude &operator*=(qreal scale)
    {
        m_value *= scale;

        return *this;
    }

    inline GeoDataLongitude &operator/=(qreal divisor)
    {
        m_value /= divisor;

        return *this;
    }

    inline qreal toRadian() const
    {
        return m_value;
    }

    inline qreal toDegree() const
    {
        return m_value * RAD2DEG;
    }

    inline static GeoDataLongitude fromRadians(qreal radians)
    {
        return GeoDataLongitude(radians);
    }

    inline static GeoDataLongitude fromDegrees(qreal degrees)
    {
        return GeoDataLongitude(degrees * DEG2RAD);
    }

    static const constexpr Null null = Null();
    static const constexpr HalfCircle halfCircle = HalfCircle();

protected:
    GeoDataLongitude(qreal value) :
        PrimitiveTypeWrapper(value)
    {}
};

inline GeoDataLongitude operator*(qreal scale, GeoDataLongitude longitude)
{
    return longitude * scale;
}

inline GeoDataLongitude operator+(GeoDataLongitude::HalfCircle)
{
    return GeoDataLongitude(GeoDataLongitude::HalfCircle());
}

inline GeoDataLongitude operator-(GeoDataLongitude::HalfCircle)
{
    return -GeoDataLongitude(GeoDataLongitude::HalfCircle());
}

inline GeoDataLongitude operator*(GeoDataLongitude::HalfCircle, qreal scale)
{
    return GeoDataLongitude(GeoDataLongitude::HalfCircle()) * scale;
}

inline GeoDataLongitude operator/(GeoDataLongitude::HalfCircle, qreal scale)
{
    return GeoDataLongitude::fromRadians(M_PI / scale);
}

inline qreal operator/(GeoDataLongitude::HalfCircle, GeoDataLongitude longitude)
{
    return GeoDataLongitude(GeoDataLongitude::HalfCircle()).toRadian() / longitude.toRadian();
}

inline uint qHash(const GeoDataLongitude lon, uint seed = 0)
{
    return ::qHash(lon.toRadian(), seed);
}

}

template <>
inline Marble::GeoDataLongitude qAbs<Marble::GeoDataLongitude>(const Marble::GeoDataLongitude &t)
{
    return t >= Marble::GeoDataLongitude::null ? t : -t;
}

inline QDataStream &operator<<(QDataStream &out, Marble::GeoDataLongitude lon)
{
    double value = lon.toRadian();
    return out << value;
}

inline QDataStream &operator>>(QDataStream &in, Marble::GeoDataLongitude &lon)
{
    double value = lon.toRadian();
    in >> value;
    lon = Marble::GeoDataLongitude::fromRadians(value);
    return in;
}

inline QDebug &operator<<(QDebug &out, Marble::GeoDataLongitude lon)
{
    return out << lon.toDegree();
}

Q_DECLARE_METATYPE(Marble::GeoDataLongitude)

#endif // MARBLE_RADIANSLONGITUDE_H
