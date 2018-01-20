//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2018   Bernhard Beschow   <shentey@gmail.com>
//

#ifndef MARBLE_GEODATANORMALIZEDLONGITUDE_H
#define MARBLE_GEODATANORMALIZEDLONGITUDE_H

#include "geodata_export.h"

#include "PrimitiveTypeWrapper.h"
#include "GeoDataLongitude.h"
#include "MarbleGlobal.h"

#include <QDataStream>
#include <QMetaType>
#include <QtDebug>
#include <qglobal.h>
#include <qhashfunctions.h>

namespace Marble {

class GEODATA_EXPORT GeoDataNormalizedLongitude : public PrimitiveTypeWrapper<qreal, GeoDataNormalizedLongitude>
{
public:
    struct Null {};
    struct HalfCircle {};

    GeoDataNormalizedLongitude() :
        PrimitiveTypeWrapper()
    {}

    GeoDataNormalizedLongitude(const PrimitiveTypeWrapper &other) :
        PrimitiveTypeWrapper(other)
    {}

    GeoDataNormalizedLongitude(Null) :
        PrimitiveTypeWrapper(fromRadians(0))
    {}

    GeoDataNormalizedLongitude(HalfCircle) :
        PrimitiveTypeWrapper(fromRadians(M_PI))
    {}

    inline qreal toRadian() const
    {
        return m_value;
    }

    inline qreal toDegree() const
    {
        return m_value * RAD2DEG;
    }

    static GeoDataNormalizedLongitude fromRadians(qreal radians)
    {
        return GeoDataNormalizedLongitude::fromLongitude(GeoDataLongitude::fromRadians(radians));
    }

    inline static GeoDataNormalizedLongitude fromDegrees(qreal degrees)
    {
        return GeoDataNormalizedLongitude::fromRadians(degrees * DEG2RAD);
    }

    static GeoDataNormalizedLongitude fromLongitude(GeoDataLongitude lon);

    static GeoDataNormalizedLongitude center(GeoDataNormalizedLongitude west, GeoDataNormalizedLongitude east);

    /**
     * @brief Get the width of the longitude interval. East and west parameters are in radians.
     * @return the angle covered by the longitude range in given unit.
     */
    static GeoDataLongitude width(GeoDataNormalizedLongitude east, GeoDataNormalizedLongitude west);

    /**
     * @brief Detect whether the bounding box crosses the IDL.
     * @param east radians east.
     * @param west radians west.
     * @return @c true  the bounding box crosses the +/-180 deg longitude.
     *         @c false the bounding box doesn't cross the +/-180 deg longitude.
     */
    static bool crossesDateLine(GeoDataNormalizedLongitude east, GeoDataNormalizedLongitude west);

    inline operator GeoDataLongitude() const
    {
        return GeoDataLongitude::fromRadians(m_value);
    }

    static const constexpr Null null = Null();
    static const constexpr HalfCircle halfCircle = HalfCircle();

protected:
    GeoDataNormalizedLongitude(qreal value) :
        PrimitiveTypeWrapper(value)
    {}
};

inline GeoDataLongitude operator+(GeoDataNormalizedLongitude lhs, GeoDataNormalizedLongitude rhs)
{
    return GeoDataLongitude::fromRadians(lhs.toRadian() + rhs.toRadian());
}

inline GeoDataLongitude operator+(GeoDataNormalizedLongitude normalizedLongitude, GeoDataLongitude longitude)
{
    return GeoDataLongitude::fromRadians(normalizedLongitude.toRadian() + longitude.toRadian());
}

inline GeoDataLongitude operator-(GeoDataNormalizedLongitude lhs, GeoDataNormalizedLongitude rhs)
{
    return GeoDataLongitude::fromRadians(lhs.toRadian() - rhs.toRadian());
}

inline GeoDataLongitude operator-(GeoDataNormalizedLongitude lhs, GeoDataLongitude rhs)
{
    return GeoDataLongitude::fromRadians(lhs.toRadian() - rhs.toRadian());
}

inline GeoDataLongitude operator*(GeoDataNormalizedLongitude longitude, qreal scale)
{
    return GeoDataLongitude::fromRadians(longitude.toRadian() * scale);
}

inline GeoDataLongitude operator*(qreal scale, GeoDataNormalizedLongitude longitude)
{
    return longitude * scale;
}

inline GeoDataLongitude operator/(GeoDataNormalizedLongitude longitude, qreal divisor)
{
    return GeoDataLongitude::fromRadians(longitude.toRadian() / divisor);
}

inline qreal operator/(GeoDataNormalizedLongitude lhs, GeoDataNormalizedLongitude rhs)
{
    return lhs.toRadian() / rhs.toRadian();
}

inline GeoDataNormalizedLongitude operator+(GeoDataNormalizedLongitude::HalfCircle)
{
    return GeoDataNormalizedLongitude(GeoDataNormalizedLongitude::HalfCircle());
}

inline GeoDataNormalizedLongitude operator-(GeoDataNormalizedLongitude::HalfCircle)
{
    return -GeoDataNormalizedLongitude(GeoDataNormalizedLongitude::HalfCircle());
}

inline uint qHash(const GeoDataNormalizedLongitude lon, uint seed = 0)
{
    return ::qHash(lon.toRadian(), seed);
}

}

template <>
inline Marble::GeoDataNormalizedLongitude qAbs<Marble::GeoDataNormalizedLongitude>(const Marble::GeoDataNormalizedLongitude &t)
{
    return t >= Marble::GeoDataNormalizedLongitude::null ? t : -t;
}

inline QDataStream &operator<<(QDataStream &out, Marble::GeoDataNormalizedLongitude lon)
{
    double value = lon.toRadian();
    return out << value;
}

inline QDataStream &operator>>(QDataStream &in, Marble::GeoDataNormalizedLongitude &lon)
{
    double value = lon.toRadian();
    in >> value;
    lon = Marble::GeoDataNormalizedLongitude::fromRadians(value);
    return in;
}

inline QDebug &operator<<(QDebug &out, Marble::GeoDataNormalizedLongitude lon)
{
    return out << lon.toDegree();
}

Q_DECLARE_METATYPE(Marble::GeoDataNormalizedLongitude)

#endif // MARBLE_GEODATANORMALIZEDLONGITUDE_H
