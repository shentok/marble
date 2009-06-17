//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Andrew Manson <g.real.ate@gmail.com>
//

#include "TmpGraphicsItem.h"

namespace Marble{

TmpGraphicsItem::TmpGraphicsItem()
{
}

TmpGraphicsItem::~TmpGraphicsItem()
{

}

GeoDataLatLonBox TmpGraphicsItem::geoBounding()
{
    return m_geoBoundCache;
}

}



