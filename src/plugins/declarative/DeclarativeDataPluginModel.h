//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2012      Anton Chernov <chernov.anton.mail@gmail.com>
// Copyright 2012      "LOTES TM" LLC <lotes.sis@gmail.com>
// Copyright 2012      Dennis Nienhüser <earthwings@gentoo.org>
//


#ifndef DECLARATIVEDATAPLUGINMODEL_H
#define DECLARATIVEDATAPLUGINMODEL_H

#include "AbstractDataPluginModel.h"

namespace Marble{

class DeclarativeDataPluginModel: public AbstractDataPluginModel
{
    Q_OBJECT
public:

    explicit DeclarativeDataPluginModel( const MarbleModel *marbleModel, QObject *parent=0 );

    virtual ~DeclarativeDataPluginModel();

Q_SIGNALS:
    /** Additional items for the given bound box (in degrees) are requested */
    void dataRequest( qreal north, qreal south, qreal east, qreal west, const TileId& tileId );

protected:
    void getAdditionalItems( const GeoDataLatLonBox& box, qint32 number, const TileId& tileId );
    void parseFile( const QByteArray& file, const TileId &tileId );
};

}
#endif // DECLARATIVEDATAPLUGINMODEL_H
