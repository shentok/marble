//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2018       Bernhard Beschow   <shentey@gmail.com>
//

#ifndef MARBLE_RADIANSLATITUDE_H
#define MARBLE_RADIANSLATITUDE_H

#include "PrimitiveTypeWrapper.h"

#include "MarbleGlobal.h"

#include <QDataStream>
#include <QMetaType>
#include <QtDebug>
#include <qglobal.h>
#include <qhashfunctions.h>

namespace Marble {

class GeoDataLatitude : public PrimitiveTypeWrapper<qreal, GeoDataLatitude>
{
public:
    struct Null {};
    struct QuaterCircle {};

    GeoDataLatitude() :
        PrimitiveTypeWrapper()
    {}

    GeoDataLatitude(const PrimitiveTypeWrapper &other) :
        PrimitiveTypeWrapper(other)
    {}

    GeoDataLatitude(Null) :
        PrimitiveTypeWrapper(fromRadians(0))
    {}

    GeoDataLatitude(QuaterCircle) :
        PrimitiveTypeWrapper(fromRadians(M_PI / 2))
    {}

    inline GeoDataLatitude operator*(qreal scale) const
    {
        return GeoDataLatitude(m_value * scale);
    }

    inline GeoDataLatitude &operator*=(qreal scale)
    {
        m_value *= scale;

        return *this;
    }

    inline GeoDataLatitude operator/(qreal divisor) const
    {
        return GeoDataLatitude(m_value / divisor);
    }

    inline GeoDataLatitude &operator/=(qreal divisor)
    {
        m_value /= divisor;

        return *this;
    }

    inline qreal operator/(GeoDataLatitude other) const
    {
        return m_value / other.m_value;
    }

    inline qreal toRadian() const
    {
        return m_value;
    }

    inline qreal toDegree() const
    {
        return m_value * RAD2DEG;
    }

    inline static GeoDataLatitude fromRadians(qreal radians)
    {
        return GeoDataLatitude(radians);
    }

    inline static GeoDataLatitude fromDegrees(qreal degrees)
    {
        return GeoDataLatitude(degrees * DEG2RAD);
    }

    static const constexpr Null null = Null();
    static const constexpr QuaterCircle quaterCircle = QuaterCircle();

protected:
    GeoDataLatitude(qreal value) :
        PrimitiveTypeWrapper(value)
    {}
};

inline GeoDataLatitude operator*(qreal scale, GeoDataLatitude latitude)
{
    return latitude * scale;
}

inline GeoDataLatitude operator+(GeoDataLatitude::QuaterCircle)
{
    return GeoDataLatitude(GeoDataLatitude::QuaterCircle());
}

inline GeoDataLatitude operator-(GeoDataLatitude::QuaterCircle)
{
    return -GeoDataLatitude(GeoDataLatitude::QuaterCircle());
}

inline GeoDataLatitude operator*(GeoDataLatitude::QuaterCircle, qreal scale)
{
    return GeoDataLatitude(GeoDataLatitude::QuaterCircle()) * scale;
}

inline GeoDataLatitude operator-(GeoDataLatitude::QuaterCircle, GeoDataLatitude latitude)
{
    return GeoDataLatitude(GeoDataLatitude::QuaterCircle()) - latitude;
}

inline qreal operator/(GeoDataLatitude::QuaterCircle, GeoDataLatitude latitude)
{
    return GeoDataLatitude(GeoDataLatitude::QuaterCircle()).toRadian() / latitude.toRadian();
}

inline uint qHash(const GeoDataLatitude lat, uint seed = 0)
{
    return ::qHash(lat.toRadian(), seed);
}

}

template <>
inline Marble::GeoDataLatitude qAbs<Marble::GeoDataLatitude>(const Marble::GeoDataLatitude &t)
{
    return t >= Marble::GeoDataLatitude::null ? t : -t;
}

inline QDataStream &operator<<(QDataStream &out, Marble::GeoDataLatitude lat)
{
    double value = lat.toRadian();
    return out << value;
}

inline QDataStream &operator>>(QDataStream &in, Marble::GeoDataLatitude &lat)
{
    double value = lat.toRadian();
    in >> value;
    lat = Marble::GeoDataLatitude::fromRadians(value);
    return in;
}

inline QDebug &operator<<(QDebug &out, Marble::GeoDataLatitude lat)
{
    return out << lat.toDegree();
}

Q_DECLARE_METATYPE(Marble::GeoDataLatitude)

#endif // MARBLE_RADIANSLATITUDE_H
