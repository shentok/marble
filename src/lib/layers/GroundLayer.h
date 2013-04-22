//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2012      Cezar Mocan <mocancezar@gmail.com>
//

#ifndef MARBLE_GROUNDLAYER_H
#define MARBLE_GROUNDLAYER_H

#include "LayerInterface.h"

#include <QtGui/QColor>
#include <QtGui/QPainterPath>

namespace Marble
{

class GroundLayer : public LayerInterface
{
 public:

    GroundLayer();

    ~GroundLayer();

    virtual QStringList renderPosition() const;

    bool setViewport( const ViewportParams *viewport );

    bool render( GeoPainter *painter, const QSize &viewportSize ) const;

    virtual qreal zValue() const;

    void setColor( const QColor &color );

    QColor color() const;

 private:
    QColor m_color;  // Gets the color specified via DGML's <map bgcolor="">
    QPainterPath m_mapShape;
};

}

#endif
