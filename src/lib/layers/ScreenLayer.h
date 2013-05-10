//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#ifndef MARBLE_SCREENLAYER_H
#define MARBLE_SCREENLAYER_H

#include <QtCore/QObject>
#include "LayerInterface.h"

class QModelIndex;
class QAbstractItemModel;

namespace Marble
{

class ScreenLayer : public QObject, public LayerInterface
{
    Q_OBJECT

 public:
    explicit ScreenLayer( const QAbstractItemModel *model );
    ~ScreenLayer();

    QStringList renderPosition() const;

    bool render( GeoPainter *painter, ViewportParams *viewport,
                 const QString &renderPos, GeoSceneLayer *layer );

    QString runtimeTrace() const;

 Q_SIGNALS:
    void repaintNeeded();

 private:
    Q_PRIVATE_SLOT( d, void addPlacemarks( const QModelIndex &, int, int ) )
    Q_PRIVATE_SLOT( d, void removePlacemarks( const QModelIndex &, int, int ) )
    Q_PRIVATE_SLOT( d, void resetCacheData() )

    class Private;
    friend class Private;
    Private *d;
};

} // namespace Marble

#endif // MARBLE_SCREENLAYER_H
