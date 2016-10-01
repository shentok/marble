//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2015      Dennis Nienhüser <nienhueser@kde.org>
//

#ifndef MARBLE_OSMRELATION
#define MARBLE_OSMRELATION

#include "OsmNode.h"
#include "OsmWay.h"
#include <osm/OsmPlacemarkData.h>
#include <GeoDataLinearRing.h>
#include <GeoDataPolygon.h>

#include <QString>
#include <QXmlStreamAttributes>
#include <QSet>

namespace Marble {

class GeoDataDocument;

class OsmRelation
{
public:
    void parseMember(const QXmlStreamAttributes &attributes);
    void addMember(qint64 reference, const QString &role, const QString &type);

    OsmPlacemarkData & osmData();
    const OsmPlacemarkData & osmData() const;

    QSet<GeoDataPolygon> polygons() const;

    QList<GeoDataLinearRing> rings(const QStringList &roles, const OsmWays &ways, const OsmNodes &nodes, QSet<qint64> &usedNodes, QSet<qint64> &usedWays) const;

    static OsmRelation fromOsmData(const OsmPlacemarkData &osmData);

private:
    struct OsmMember
    {
        QString type;
        QString role;
        qint64 reference;

        OsmMember();
    };

    OsmPlacemarkData m_osmData;
    QVector<OsmMember> m_members;
};

typedef QHash<qint64,OsmRelation> OsmRelations;

}

#endif
