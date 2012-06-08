//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2012      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#ifndef MARBLE_GLLAYERINTERFACE_H
#define MARBLE_GLLAYERINTERFACE_H

#include "marble_export.h"

class QGLContext;

namespace Marble {

class ViewportParams;

class MARBLE_EXPORT GlLayerInterface
{
public:

    /** Destructor */
    virtual ~GlLayerInterface();

    virtual void paintGL( QGLContext *glContext, const ViewportParams *viewport ) = 0;
};

} // namespace Marble

#endif
