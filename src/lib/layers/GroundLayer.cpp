//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2012      Cezar Mocan <mocancezar@gmail.com>
//

#include "GroundLayer.h"

#include "GeoPainter.h"
#include "ViewportParams.h"

namespace Marble
{

GroundLayer::GroundLayer() :
    m_color( QColor( 153, 179, 204 ) ),
    m_mapShape()
{
}

GroundLayer::~GroundLayer()
{
}

QStringList GroundLayer::renderPosition() const
{
    return QStringList() << "SURFACE";
}

bool GroundLayer::setViewport( const ViewportParams *viewport )
{
    m_mapShape = viewport->mapShape();

    return true;
}

bool GroundLayer::render( GeoPainter *painter, const QSize &viewportSize ) const
{
    Q_UNUSED( viewportSize )

    QBrush backgroundBrush( m_color );
    QPen backgroundPen( Qt::NoPen );

    painter->setBrush( backgroundBrush );
    painter->setPen( backgroundPen );
    painter->drawPath( m_mapShape );

    return true;
}

qreal GroundLayer::zValue() const
{
    return -50.0;
}

void GroundLayer::setColor( const QColor &color )
{   
    m_color = color;
}

QColor GroundLayer::color() const
{
    return m_color;
}

}
