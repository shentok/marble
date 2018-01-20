//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2018       Bernhard Beschow   <shentey@gmail.com>
//

#ifndef MARBLE_GEODATANORMALIZEDLATITUDE_H
#define MARBLE_GEODATANORMALIZEDLATITUDE_H

#include "geodata_export.h"

#include "PrimitiveTypeWrapper.h"
#include "GeoDataLatitude.h"
#include "MarbleGlobal.h"

#include <QDataStream>
#include <QMetaType>
#include <QtDebug>
#include <qglobal.h>
#include <qhashfunctions.h>

namespace Marble {

class GEODATA_EXPORT GeoDataNormalizedLatitude : public PrimitiveTypeWrapper<qreal, GeoDataNormalizedLatitude>
{
public:
    struct Null {};
    struct QuaterCircle {};

    GeoDataNormalizedLatitude() :
        PrimitiveTypeWrapper()
    {}

    GeoDataNormalizedLatitude(const PrimitiveTypeWrapper &other) :
        PrimitiveTypeWrapper(other)
    {}

    GeoDataNormalizedLatitude(Null) :
        PrimitiveTypeWrapper(fromRadians(0))
    {}

    GeoDataNormalizedLatitude(QuaterCircle) :
        PrimitiveTypeWrapper(fromRadians(M_PI / 2))
    {}

    inline qreal toRadian() const
    {
        return m_value;
    }

    inline qreal toDegree() const
    {
        return m_value * RAD2DEG;
    }

    inline static GeoDataNormalizedLatitude fromRadians(qreal radians)
    {
        return GeoDataNormalizedLatitude::fromLatitude(GeoDataLatitude::fromRadians(radians));
    }

    inline static GeoDataNormalizedLatitude fromDegrees(qreal degrees)
    {
        return GeoDataNormalizedLatitude::fromRadians(degrees * DEG2RAD);
    }

    static GeoDataNormalizedLatitude fromLatitude(GeoDataLatitude lat);

    static GeoDataNormalizedLatitude center(GeoDataNormalizedLatitude lhs, GeoDataNormalizedLatitude rhs);

    /**
     * @brief Get the height of the latitude interval. North and south parameters are in radians.
     * @return the angle covered by the latitude range in given unit.
     */
    static GeoDataLatitude height(GeoDataNormalizedLatitude north, GeoDataNormalizedLatitude south);

    inline operator GeoDataLatitude() const
    {
        return GeoDataLatitude::fromRadians(m_value);
    }

    static const constexpr Null null = Null();
    static const constexpr QuaterCircle quaterCircle = QuaterCircle();

protected:
    GeoDataNormalizedLatitude(qreal value) :
        PrimitiveTypeWrapper(value)
    {}
};

inline GeoDataLatitude operator-(GeoDataNormalizedLatitude lhs, GeoDataNormalizedLatitude rhs)
{
    return GeoDataLatitude::fromRadians(lhs.toRadian() - rhs.toRadian());
}

inline GeoDataLatitude operator-(GeoDataNormalizedLatitude lhs, GeoDataLatitude rhs)
{
    return GeoDataLatitude::fromRadians(lhs.toRadian() - rhs.toRadian());
}

inline GeoDataLatitude operator+(GeoDataNormalizedLatitude lhs, GeoDataLatitude rhs)
{
    return GeoDataLatitude::fromRadians(lhs.toRadian() + rhs.toRadian());
}

inline GeoDataLatitude operator*(GeoDataNormalizedLatitude latitude, qreal scale)
{
    return GeoDataLatitude::fromRadians(latitude.toRadian() * scale);
}

inline GeoDataLatitude operator*(qreal scale, GeoDataNormalizedLatitude latitude)
{
    return latitude * scale;
}

inline GeoDataLatitude operator/(GeoDataNormalizedLatitude latitude, qreal divisor)
{
    return GeoDataLatitude::fromRadians(latitude.toRadian() / divisor);
}

inline qreal operator/(GeoDataNormalizedLatitude lhs, GeoDataNormalizedLatitude rhs)
{
    return lhs.toRadian() / rhs.toRadian();
}

inline GeoDataNormalizedLatitude operator+(GeoDataNormalizedLatitude::QuaterCircle)
{
    return GeoDataNormalizedLatitude(GeoDataNormalizedLatitude::QuaterCircle());
}

inline GeoDataNormalizedLatitude operator-(GeoDataNormalizedLatitude::QuaterCircle)
{
    return -GeoDataNormalizedLatitude(GeoDataNormalizedLatitude::QuaterCircle());
}

inline uint qHash(const GeoDataNormalizedLatitude lat, uint seed = 0)
{
    return ::qHash(lat.toRadian(), seed);
}

}

template <>
inline Marble::GeoDataNormalizedLatitude qAbs<Marble::GeoDataNormalizedLatitude>(const Marble::GeoDataNormalizedLatitude &t)
{
    return t >= Marble::GeoDataNormalizedLatitude::null ? t : -t;
}

inline QDataStream &operator<<(QDataStream &out, Marble::GeoDataNormalizedLatitude lat)
{
    double value = lat.toRadian();
    return out << value;
}

inline QDataStream &operator>>(QDataStream &in, Marble::GeoDataNormalizedLatitude &lat)
{
    double value = lat.toRadian();
    in >> value;
    lat = Marble::GeoDataNormalizedLatitude::fromRadians(value);
    return in;
}

inline QDebug &operator<<(QDebug &out, Marble::GeoDataNormalizedLatitude lat)
{
    return out << lat.toDegree();
}

Q_DECLARE_METATYPE(Marble::GeoDataNormalizedLatitude)

#endif // MARBLE_GEODATANORMALIZEDLATITUDE_H
