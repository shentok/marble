//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010      Bastian Holst <bastianholst@gmx.de>
// Copyright 2011      Konstantin Oblaukhov <oblaukhov.konstantin@gmail.com>
//

#ifndef MARBLE_GEOGRAPHICSSCENE_H
#define MARBLE_GEOGRAPHICSSCENE_H

#include "marble_export.h"
#include "MarbleGlobal.h"

#include <QtCore/QObject>
#include <QtCore/QList>

class QAbstractItemModel;
class QModelIndex;

namespace Marble
{

class GeoGraphicsItem;
class GeoDataLatLonBox;
class ScreenOverlayGraphicsItem;

class GeoGraphicsScenePrivate;

/**
 * @short This is the home of all GeoGraphicsItems to be shown on the map.
 */
class MARBLE_EXPORT GeoGraphicsScene : public QObject
{
    Q_OBJECT

public:
    /**
     * Creates a new instance of GeoGraphicsScene
     * @param parent the QObject parent of the Scene
     */
    explicit GeoGraphicsScene( const QAbstractItemModel *model, QObject *parent = 0 );
    ~GeoGraphicsScene();

    int maximumZoomLevel() const;

    /**
     * @brief Remove all items from the GeoGraphicsScene
     * Removes all items from the GeoGraphicsScene
     */
    void clear();

    /**
     * @brief Get the list of items in the specified Box
     *
     * @param box The box around the items.
     * @param maxZoomLevel The max zoom level of tiling
     * @return The list of items in the specified box in no specific order.
     */
    QList<GeoGraphicsItem *> items( const GeoDataLatLonBox &box, int maxZoomLevel ) const;

    QList<ScreenOverlayGraphicsItem *> screenItems() const;

Q_SIGNALS:
    void repaintNeeded();

private Q_SLOTS:
    void addPlacemarks( const QModelIndex &index, int first, int last );
    void removePlacemarks( const QModelIndex &index, int first, int last );
    void resetCacheData();

private:
    GeoGraphicsScenePrivate * const d;
};
}
#endif // MARBLE_GEOGRAPHICSSCENE_H
