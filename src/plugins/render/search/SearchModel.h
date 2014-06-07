//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2014 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#ifndef SEARCHMODEL_H
#define SEARCHMODEL_H

#include "AbstractDataPluginModel.h"

#include "GeoDataLatLonBox.h"
#include "SearchRunnerManager.h"

#include <QList>

namespace Marble {

class MarbleModel;

class SearchModel : public AbstractDataPluginModel
{
    Q_OBJECT

public:
    explicit SearchModel( const MarbleModel *marbleModel, QObject *parent = 0 );
    ~SearchModel();

    void setSearchTerm( const QString &searchTerm );
    QString searchTerm() const;

protected:
    void getAdditionalItems( const GeoDataLatLonBox& box, qint32 number, const TileId &tileId );
    void parseFile( const QByteArray& file, const TileId &tileId );

private Q_SLOTS:
    void finished();

private:
    SearchRunnerManager m_runnerManager;
    QString m_searchTerm;
    int m_zoomLevel;
    struct Foo {
        Foo() : zoomLevel( 0 ) {}
        bool operator==( const Foo &other ) const { return box == other.box && zoomLevel == other.zoomLevel; }

        GeoDataLatLonBox box;
        int zoomLevel;
    };

    QList<Foo> m_queue;
};

}

#endif // SEARCHMODEL_H
