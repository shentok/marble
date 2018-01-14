//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2018   Bernhard Beschow   <shentey@gmail.com>
//

#ifndef PRIMITIVE_TYPE_WRAPPER_H
#define PRIMITIVE_TYPE_WRAPPER_H

#include <qglobal.h>

template<typename T, typename Base>
class PrimitiveTypeWrapper
{
public:
    Q_DECL_CONSTEXPR inline Base operator+(const Base other) const
    {
        return PrimitiveTypeWrapper<T, Base>(m_value + other.m_value);
    }

    Q_DECL_CONSTEXPR inline Base operator+() const
    {
        return *this;
    }

    Q_DECL_CONSTEXPR inline Base operator-(const Base other) const
    {
        return PrimitiveTypeWrapper<T, Base>(m_value - other.m_value);
    }

    Q_DECL_CONSTEXPR inline Base operator-() const
    {
        return PrimitiveTypeWrapper<T, Base>(-m_value);
    }

    inline Base operator+=(const Base other)
    {
        m_value += other.m_value;

        return *this;
    }

    inline Base operator-=(const Base other)
    {
        m_value -= other.m_value;

        return *this;
    }

    Q_DECL_CONSTEXPR inline bool operator==(const Base other) const
    {
        return m_value == other.m_value;
    }

    Q_DECL_CONSTEXPR inline bool operator!=(const Base other) const
    {
        return !(*this == other);
    }

    Q_DECL_CONSTEXPR inline bool operator<(const Base other) const
    {
        return m_value < other.m_value;
    }

    Q_DECL_CONSTEXPR inline bool operator>(const Base other) const
    {
        return m_value > other.m_value;
    }

    Q_DECL_CONSTEXPR inline bool operator<=(const Base other) const
    {
        return m_value <= other.m_value;
    }

    Q_DECL_CONSTEXPR inline bool operator>=(const Base other) const
    {
        return m_value >= other.m_value;
    }

protected:
    PrimitiveTypeWrapper() :
        m_value()
    {}

    Q_DECL_CONSTEXPR inline PrimitiveTypeWrapper(T value) :
        m_value(value)
    {}

protected:
    T m_value;
};

#endif // PRIMITIVE_TYPE_WRAPPER_H
