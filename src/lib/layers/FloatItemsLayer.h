//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2012      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#ifndef MARBLE_FLOATITEMSLAYER_H
#define MARBLE_FLOATITEMSLAYER_H

#include <QtCore/QObject>
#include "LayerInterface.h"

#include <QtCore/QList>

namespace Marble
{

class AbstractFloatItem;
class MarbleModel;

class FloatItemsLayer : public QObject, public LayerInterface
{
    Q_OBJECT

 public:
    explicit FloatItemsLayer( const MarbleModel *model, QObject *parent = 0 );

    ~FloatItemsLayer();

    virtual QStringList renderPosition() const;

    virtual bool render( GeoPainter *painter, ViewportParams *viewport,
       const QString& renderPos = "NONE", GeoSceneLayer * layer = 0 );

    void setVisible( const QString &nameId, bool visible );

    /**
     * @brief Returns a list of all FloatItems of the layer
     * @return the list of the floatItems
     */
    QList<AbstractFloatItem *> floatItems() const;

 Q_SIGNALS:
    void repaintNeeded();

 private:
    QList<AbstractFloatItem *> m_floatItems;
};

}

#endif
