//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#ifndef MARBLE_MARBLESPLASHLAYER_H
#define MARBLE_MARBLESPLASHLAYER_H

#include "LayerInterface.h"

#include <QtCore/QSize>


namespace Marble
{

class MarbleSplashLayer : public LayerInterface
{
public:
    MarbleSplashLayer();

    virtual QStringList renderPosition() const;

    bool setViewport( const ViewportParams *viewport );

    bool render( GeoPainter *painter, const QSize &viewportSize ) const;

private:
    QSize m_viewportSize;
};

}

#endif
