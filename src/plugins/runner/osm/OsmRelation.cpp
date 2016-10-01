//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2015      Dennis Nienhüser <nienhueser@kde.org>
//

#include <OsmRelation.h>
#include <MarbleDebug.h>
#include <GeoDataPlacemark.h>
#include <GeoDataDocument.h>
#include <GeoDataPolygon.h>
#include <StyleBuilder.h>
#include <osm/OsmObjectManager.h>

namespace Marble {

OsmRelation::OsmMember::OsmMember() :
    reference(0)
{
    // nothing to do
}

OsmPlacemarkData &OsmRelation::osmData()
{
    return m_osmData;
}

const OsmPlacemarkData &OsmRelation::osmData() const
{
    return m_osmData;
}

QSet<GeoDataPolygon> OsmRelation::polygons() const
{
#warning implement me
    return QSet<GeoDataPolygon>();
}

void OsmRelation::parseMember(const QXmlStreamAttributes &attributes)
{
    addMember(attributes.value(QLatin1String("ref")).toLongLong(),
              attributes.value(QLatin1String("role")).toString(),
              attributes.value(QLatin1String("type")).toString());
}

void OsmRelation::addMember(qint64 reference, const QString &role, const QString &type)
{
    OsmMember member;
    member.reference = reference;
    member.role = role;
    member.type = type;
    m_members << member;
}

OsmRelation OsmRelation::fromOsmData(const OsmPlacemarkData &osmData)
{
    OsmRelation result;

    result.osmData() = osmData;

    return result;
}

QList<GeoDataLinearRing> OsmRelation::rings(const QStringList &roles, const OsmWays &ways, const OsmNodes &nodes, QSet<qint64> &usedNodes, QSet<qint64> &usedWays) const
{
    QSet<qint64> currentWays;
    QSet<qint64> currentNodes;
    QList<qint64> roleMembers;
    foreach(const OsmMember &member, m_members) {
        if (roles.contains(member.role)) {
            if (!ways.contains(member.reference)) {
                // A way is missing. Return nothing.
                return QList<GeoDataLinearRing>();
            }
            roleMembers << member.reference;
        }
    }

    QList<GeoDataLinearRing> result;
    QList<OsmWay> unclosedWays;
    foreach(qint64 wayId, roleMembers) {
        GeoDataLinearRing ring;
        OsmWay const & way = ways[wayId];
        if (way.references().first() != way.references().last()) {
            unclosedWays.append(way);
            continue;
        }
        foreach(qint64 id, way.references()) {
            if (!nodes.contains(id)) {
                // A node is missing. Return nothing.
                return QList<GeoDataLinearRing>();
            }
            ring << nodes[id].coordinates();
        }
        Q_ASSERT(ways.contains(wayId));
        currentWays << wayId;
        result << ring;
    }

    if( !unclosedWays.isEmpty() ) {
        //mDebug() << "Trying to merge non-trivial polygon boundary in relation " << m_osmData.id();
        while( unclosedWays.length() > 0 ) {
            GeoDataLinearRing ring;
            qint64 firstReference = unclosedWays.first().references().first();
            qint64 lastReference = firstReference;
            bool ok = true;
            while( ok ) {
                ok = false;
                for(int i = 0; i<unclosedWays.length(); ) {
                    const OsmWay &nextWay = unclosedWays.at(i);
                    if( nextWay.references().first() == lastReference
                            || nextWay.references().last() == lastReference ) {

                        bool isReversed = nextWay.references().last() == lastReference;
                        QVector<qint64> v = nextWay.references();
                        while( !v.isEmpty() ) {
                            qint64 id = isReversed ? v.takeLast() : v.takeFirst();
                            if (!nodes.contains(id)) {
                                // A node is missing. Return nothing.
                                return QList<GeoDataLinearRing>();
                            }
                            if ( id != lastReference ) {
                                ring << nodes[id].coordinates();
                                currentNodes << id;
                            }
                        }
                        lastReference = isReversed ? nextWay.references().first()
                                                   : nextWay.references().last();
                        Q_ASSERT(ways.contains(nextWay.osmData().id()));
                        currentWays << nextWay.osmData().id();
                        unclosedWays.removeAt(i);
                        ok = true;
                        break;
                    } else {
                        ++i;
                    }
                }
            }

            if(lastReference != firstReference) {
                return QList<GeoDataLinearRing>();
            } else {
                result << ring;
            }
        }
    }

    usedWays |= currentWays;
    usedNodes |= currentNodes;
    return result;
}

}
