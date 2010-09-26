//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#ifndef MARBLE_MARBLEHEIGHT_H
#define MARBLE_MARBLEHEIGHT_H

#include "TileLoader.h"
#include "TileId.h"

#include <QObject>
#include <QCache>

namespace Marble
{

class HttpDownloadManager;


class MarbleHeight : public QObject
{
    Q_OBJECT
public:
    explicit MarbleHeight( HttpDownloadManager *downloadManager, QObject* parent = 0 );

    void setRadius( int radius );

    qreal altitude( qreal lon, qreal lat );

private:
    TileLoader m_tileLoader;
    const GeoSceneTexture *m_textureLayer;
    uint m_hash;
    int m_radius;
    int m_level;
    int m_numXTiles;
    int m_numYTiles;
    QCache<TileId, const QImage> m_cache;
};

}

#endif
