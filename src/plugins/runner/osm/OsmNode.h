//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2015      Dennis Nienhüser <nienhueser@kde.org>
//

#ifndef MARBLE_OSMNODE
#define MARBLE_OSMNODE

#include <osm/OsmPlacemarkData.h>

#include <QString>

class QXmlStreamAttributes;

namespace Marble {

class GeoDataDocument;

class OsmNode {
public:
    void parseCoordinates(const QXmlStreamAttributes &attributes);
    void setCoordinates(const GeoDataCoordinates &coordinates);

    const GeoDataCoordinates coordinates() const;

    OsmPlacemarkData & osmData();
    const OsmPlacemarkData & osmData() const;

    static OsmNode fromOsmData(const OsmPlacemarkData &osmData);

    void create(GeoDataDocument* document) const;

private:
    int populationIndex(qint64 population) const;

    OsmPlacemarkData m_osmData;
};

typedef QHash<qint64,OsmNode> OsmNodes;

}

#endif
