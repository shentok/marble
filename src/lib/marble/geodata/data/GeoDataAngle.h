//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2018       Bernhard Beschow   <shentey@gmail.com>
//

#ifndef MARBLE_GEODATA_ANGLE_H
#define MARBLE_GEODATA_ANGLE_H

#include "PrimitiveTypeWrapper.h"

#include "MarbleGlobal.h"

#include <QDataStream>
#include <QDebug>
#include <QMetaType>
#include <qglobal.h>

namespace Marble {

class GeoDataAngle : public PrimitiveTypeWrapper<qreal, GeoDataAngle>
{
public:
    struct Null {};
    struct FullCircle {};

    GeoDataAngle() :
        PrimitiveTypeWrapper()
    {}

    Q_DECL_CONSTEXPR GeoDataAngle(const PrimitiveTypeWrapper &other) :
        PrimitiveTypeWrapper(other)
    {}

    GeoDataAngle(Null) :
        PrimitiveTypeWrapper(fromRadians(0))
    {}

    GeoDataAngle(FullCircle) :
        PrimitiveTypeWrapper(fromRadians(2 * M_PI))
    {
    }

    inline GeoDataAngle operator*(qreal scale) const
    {
        return GeoDataAngle(m_value * scale);
    }

    inline GeoDataAngle operator/(qreal scale) const
    {
        return GeoDataAngle(m_value / scale);
    }

    inline qreal toRadian() const
    {
        return m_value;
    }

    inline qreal toDegree() const
    {
        return m_value * RAD2DEG;
    }

    inline static GeoDataAngle fromRadians(qreal radians)
    {
        return GeoDataAngle(radians);
    }

    inline static GeoDataAngle fromDegrees(qreal degrees)
    {
        return GeoDataAngle(degrees * DEG2RAD);
    }

    static const constexpr Null null = Null();
    static const constexpr FullCircle fullCircle = FullCircle();

protected:
    GeoDataAngle(qreal value) :
        PrimitiveTypeWrapper(value)
    {}
};

inline GeoDataAngle operator*(GeoDataAngle::FullCircle, qreal scale)
{
    return GeoDataAngle::fromRadians(2 * M_PI * scale);
}

inline GeoDataAngle operator/(GeoDataAngle::FullCircle, qreal scale)
{
    return GeoDataAngle::fromRadians(2 * M_PI / scale);
}

inline bool operator<(GeoDataAngle::Null, const GeoDataAngle angle)
{
    return 0 < angle.toRadian();
}

}

template <>
inline Marble::GeoDataAngle qAbs<Marble::GeoDataAngle>(const Marble::GeoDataAngle &t)
{
    return t >= Marble::GeoDataAngle::null ? t : -t;
}

inline QDataStream &operator<<(QDataStream &out, Marble::GeoDataAngle angle)
{
    double value = angle.toRadian();
    return out << value;
}

inline QDataStream &operator>>(QDataStream &in, Marble::GeoDataAngle &angle)
{
    double value = angle.toRadian();
    in >> value;
    angle = Marble::GeoDataAngle::fromRadians(value);
    return in;
}

inline QDebug &operator<<(QDebug &out, Marble::GeoDataAngle angle)
{
    return out << angle.toDegree();
}

Q_DECLARE_METATYPE(Marble::GeoDataAngle)

#endif // MARBLE_GEODATA_ANGLE_H
