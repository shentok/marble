//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2006-2007 Torsten Rahn <tackat@kde.org>
// Copyright 2007      Inge Wallin  <ingwa@kde.org>
// Copyright 2008      Carlos Licea <carlos.licea@kdemail.net>
//

#ifndef MARBLE_MARBLEMAP_P_H
#define MARBLE_MARBLEMAP_P_H

#include "ViewParams.h"


class QPainter;
class QRect;

namespace Marble
{

class MarbleMap;
class MarbleModel;
class TextureColorizer;


class MarbleMapPrivate
{
 public:
    explicit MarbleMapPrivate( MarbleMap *parent );

    void construct();

    void paintMarbleSplash( GeoPainter &painter, QRect &dirtyRect );

    void setBoundingBox();
    void doResize();

    MarbleMap       *m_parent;

    // The model we are showing.
    MarbleModel     *m_model;
    bool             m_modelIsOwned;

    int              m_width;
    int              m_height;
    ViewParams       m_viewParams;
    bool             m_justModified; // FIXME: Rename to isDirty

    // zoom related
    int              m_logzoom;

    TextureColorizer  *m_sealegend;

    const qreal      m_viewAngle;
};

}

#endif
