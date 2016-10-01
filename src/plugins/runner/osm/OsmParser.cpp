//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011      Thibaut Gridel <tgridel@free.fr>
// Copyright 2011      Konstantin Oblaukhov <oblaukhov.konstantin@gmail.com>
// Copyright 2014      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
// Copyright 2015      Dennis Nienhüser <nienhueser@kde.org>
//

#include "OsmParser.h"
#include "OsmElementDictionary.h"
#include "osm/OsmObjectManager.h"
#include "GeoDataDocument.h"
#include "GeoDataPlacemark.h"
#include "GeoDataPoint.h"
#include "GeoDataTypes.h"
#include "GeoDataStyle.h"
#include "GeoDataPolyStyle.h"
#include <MarbleZipReader.h>
#include "o5mreader.h"

#include <QFile>
#include <QFileInfo>
#include <QBuffer>
#include <QSet>

namespace Marble {

GeoDataDocument *OsmParser::parse(const QString &filename, QString &error)
{
    QFileInfo const fileInfo(filename);
    if (!fileInfo.exists() || !fileInfo.isReadable()) {
        error = QString("Cannot read file %1").arg(filename);
        return 0;
    }

    if (fileInfo.completeSuffix() == QLatin1String("o5m")) {
        return parseO5m(filename, error);
    } else {
        return parseXml(filename, error);
    }
}

GeoDataDocument* OsmParser::parseO5m(const QString &filename, QString &error)
{
    O5mreader* reader;
    O5mreaderDataset data;
    O5mreaderIterateRet outerState, innerState;
    char *key, *value;
    // share string data on the heap at least for this file
    QSet<QString> stringPool;

    OsmNodes nodes;
    OsmWays ways;
    OsmRelations relations;
    QHash<uint8_t, QString> relationTypes;
    relationTypes[O5MREADER_DS_NODE] = QStringLiteral("node");
    relationTypes[O5MREADER_DS_WAY] = QStringLiteral("way");
    relationTypes[O5MREADER_DS_REL] = QStringLiteral("relation");

    auto file = fopen(filename.toStdString().c_str(), "rb");
    o5mreader_open(&reader, file);

    while( (outerState = o5mreader_iterateDataSet(reader, &data)) == O5MREADER_ITERATE_RET_NEXT) {
        switch (data.type) {
        case O5MREADER_DS_NODE:
        {
            OsmNode& node = nodes[data.id];
            node.osmData().setId(data.id);
            node.setCoordinates(GeoDataCoordinates(data.lon*1.0e-7, data.lat*1.0e-7,
                                                   0.0, GeoDataCoordinates::Degree));
            while ((innerState = o5mreader_iterateTags(reader, &key, &value)) == O5MREADER_ITERATE_RET_NEXT) {
                const QString keyString = *stringPool.insert(QString::fromUtf8(key));
                const QString valueString = *stringPool.insert(QString::fromUtf8(value));
                node.osmData().insertTag(keyString, valueString);
            }
        }
            break;
        case O5MREADER_DS_WAY:
        {
            OsmWay &way = ways[data.id];
            way.osmData().setId(data.id);
            uint64_t nodeId;
            while ((innerState = o5mreader_iterateNds(reader, &nodeId)) == O5MREADER_ITERATE_RET_NEXT) {
                way.addReference(nodeId);
            }
            while ((innerState = o5mreader_iterateTags(reader, &key, &value)) == O5MREADER_ITERATE_RET_NEXT) {
                const QString keyString = *stringPool.insert(QString::fromUtf8(key));
                const QString valueString = *stringPool.insert(QString::fromUtf8(value));
                way.osmData().insertTag(keyString, valueString);
            }
        }
            break;
        case O5MREADER_DS_REL:
        {
            OsmRelation &relation = relations[data.id];
            relation.osmData().setId(data.id);
            char *role;
            uint8_t type;
            uint64_t refId;
            while ((innerState = o5mreader_iterateRefs(reader, &refId, &type, &role)) == O5MREADER_ITERATE_RET_NEXT) {
                const QString roleString = *stringPool.insert(QString::fromUtf8(role));
                relation.addMember(refId, roleString, relationTypes[type]);
            }
            while ((innerState = o5mreader_iterateTags(reader, &key, &value)) == O5MREADER_ITERATE_RET_NEXT) {
                const QString keyString = *stringPool.insert(QString::fromUtf8(key));
                const QString valueString = *stringPool.insert(QString::fromUtf8(value));
                relation.osmData().insertTag(keyString, valueString);
            }
        }
            break;
        }
    }

    fclose(file);
    error = reader->errMsg;
    o5mreader_close(reader);
    return createDocument(nodes, ways, relations);
}

GeoDataDocument* OsmParser::parseXml(const QString &filename, QString &error)
{
    QXmlStreamReader parser;
    QFile file;
    QBuffer buffer;
    QFileInfo fileInfo(filename);
    if (fileInfo.completeSuffix() == QLatin1String("osm.zip")) {
        MarbleZipReader zipReader(filename);
        if (zipReader.fileInfoList().size() != 1) {
            int const fileNumber = zipReader.fileInfoList().size();
            error = QStringLiteral("Unexpected number of files (%1) in %2").arg(fileNumber).arg(filename);
            return nullptr;
        }
        QByteArray const data = zipReader.fileData(zipReader.fileInfoList().first().filePath);
        buffer.setData(data);
        buffer.open(QBuffer::ReadOnly);
        parser.setDevice(&buffer);
    } else {
        file.setFileName(filename);
        if (!file.open(QFile::ReadOnly)) {
            error = QStringLiteral("Cannot open file %1").arg(filename);
            return nullptr;
        }
        parser.setDevice(&file);
    }

    OsmPlacemarkData* osmData(0);
    QString parentTag;
    qint64 parentId(0);
    // share string data on the heap at least for this file
    QSet<QString> stringPool;

    OsmNodes m_nodes;
    OsmWays m_ways;
    OsmRelations m_relations;

    while (!parser.atEnd()) {
        parser.readNext();
        if (!parser.isStartElement()) {
            continue;
        }

        QStringRef const tagName = parser.name();
        if (tagName == osm::osmTag_node || tagName == osm::osmTag_way || tagName == osm::osmTag_relation) {
            parentTag = parser.name().toString();
            parentId = parser.attributes().value(QLatin1String("id")).toLongLong();

            if (tagName == osm::osmTag_node) {
                m_nodes[parentId].osmData() = OsmPlacemarkData::fromParserAttributes(parser.attributes());
                m_nodes[parentId].parseCoordinates(parser.attributes());
                osmData = &m_nodes[parentId].osmData();
            } else if (tagName == osm::osmTag_way) {
                m_ways[parentId].osmData() = OsmPlacemarkData::fromParserAttributes(parser.attributes());
                osmData = &m_ways[parentId].osmData();
            } else {
                Q_ASSERT(tagName == osm::osmTag_relation);
                m_relations[parentId].osmData() = OsmPlacemarkData::fromParserAttributes(parser.attributes());
                osmData = &m_relations[parentId].osmData();
            }
        } else if (tagName == osm::osmTag_tag) {
            const QXmlStreamAttributes &attributes = parser.attributes();
            const QString keyString = *stringPool.insert(attributes.value(QLatin1String("k")).toString());
            const QString valueString = *stringPool.insert(attributes.value(QLatin1String("v")).toString());
            osmData->insertTag(keyString, valueString);
        } else if (tagName == osm::osmTag_nd && parentTag == osm::osmTag_way) {
            m_ways[parentId].addReference(parser.attributes().value(QLatin1String("ref")).toLongLong());
        } else if (tagName == osm::osmTag_member && parentTag == osm::osmTag_relation) {
            m_relations[parentId].parseMember(parser.attributes());
        } // other tags like osm, bounds ignored
    }

    if (parser.hasError()) {
        error = parser.errorString();
        return nullptr;
    }

    return createDocument(m_nodes, m_ways, m_relations);
}

GeoDataDocument *OsmParser::createDocument(OsmNodes &nodes, OsmWays &ways, OsmRelations &relations)
{
    GeoDataDocument* document = new GeoDataDocument;
    GeoDataPolyStyle backgroundPolyStyle;
    backgroundPolyStyle.setFill( true );
    backgroundPolyStyle.setOutline( false );
    backgroundPolyStyle.setColor(QStringLiteral("#f1eee8"));
    GeoDataStyle::Ptr backgroundStyle(new GeoDataStyle);
    backgroundStyle->setPolyStyle( backgroundPolyStyle );
    backgroundStyle->setId(QStringLiteral("background"));
    document->addStyle( backgroundStyle );

    QSet<qint64> usedNodes, usedWays;
    foreach(OsmRelation const &relation, relations) {
        createRelation(document, relation, ways, nodes, usedNodes, usedWays);
    }
    foreach(qint64 id, usedWays) {
        ways.remove(id);
    }

    foreach(OsmWay const &way, ways) {
        createWay(document, way, nodes, usedNodes);
    }

    foreach(qint64 id, usedNodes) {
        if (nodes[id].osmData().isEmpty()) {
            nodes.remove(id);
        }
    }

    foreach(OsmNode const &node, nodes) {
        createNode(document, node);
    }

    return document;
}


void OsmParser::createNode(GeoDataDocument *document, const OsmNode &node)
{
    GeoDataFeature::GeoDataVisualCategory const category = StyleBuilder::determineVisualCategory(node.osmData());

    if (category == GeoDataFeature::None) {
        return;
    }

    GeoDataPlacemark* placemark = new GeoDataPlacemark;
    placemark->setOsmData(node.osmData());
    placemark->setCoordinate(node.coordinates());

    QHash<QString, QString>::const_iterator tagIter;
    if ((category == GeoDataFeature::TransportCarShare || category == GeoDataFeature::MoneyAtm)
            && (tagIter = node.osmData().findTag(QStringLiteral("operator"))) != node.osmData().tagsEnd()) {
        placemark->setName(tagIter.value());
    } else {
        placemark->setName(node.osmData().tagValue(QStringLiteral("name")));
    }
    if (placemark->name().isEmpty()) {
        placemark->setName(node.osmData().tagValue(QStringLiteral("ref")));
    }
    placemark->setVisualCategory(category);
    placemark->setStyle( GeoDataStyle::Ptr() );

    placemark->setZoomLevel( 18 );
    if (category >= GeoDataFeature::PlaceCity && category <= GeoDataFeature::PlaceVillageCapital) {
        int const population = node.osmData().tagValue(QStringLiteral("population")).toInt();
        placemark->setPopulation(qMax(0, population));
        if (population > 0) {
            placemark->setZoomLevel(populationIndex(population));
            placemark->setPopularity(population);
        } else {
            switch (category) {
            case GeoDataFeature::PlaceCity:
            case GeoDataFeature::PlaceCityCapital:
                placemark->setZoomLevel(9);
                break;
            case GeoDataFeature::PlaceSuburb:
                placemark->setZoomLevel(13);
                break;
            case GeoDataFeature::PlaceHamlet:
                placemark->setZoomLevel(15);
                break;
            case GeoDataFeature::PlaceLocality:
                placemark->setZoomLevel(15);
                break;
            case GeoDataFeature::PlaceTown:
            case GeoDataFeature::PlaceTownCapital:
                placemark->setZoomLevel(11);
                break;
            case GeoDataFeature::PlaceVillage:
            case GeoDataFeature::PlaceVillageCapital:
                placemark->setZoomLevel(13);
                break;
            default:
                placemark->setZoomLevel(10); break;
            }
        }
    }

    OsmObjectManager::registerId(node.osmData().id());
    document->append(placemark);
}

void OsmParser::createWay(GeoDataDocument *document, const OsmWay &way, const OsmNodes &nodes, QSet<qint64> &usedNodes)
{
    OsmPlacemarkData osmData = way.osmData();
    GeoDataGeometry *geometry = 0;

    if (way.isArea()) {
        GeoDataLinearRing linearRing;

        foreach(qint64 nodeId, m_references) {
            auto const nodeIter = nodes.constFind(nodeId);
            if (nodeIter == nodes.constEnd()) {
                return;
            }

            OsmNode const & node = nodeIter.value();
            osmData.addNodeReference(node.coordinates(), node.osmData());
            linearRing.append(node.coordinates());
            usedNodes << nodeId;
        }

        geometry = new GeoDataLinearRing(linearRing.optimized());
    } else {
        GeoDataLineString lineString;

        foreach(qint64 nodeId, m_references) {
            auto const nodeIter = nodes.constFind(nodeId);
            if (nodeIter == nodes.constEnd()) {
                return;
            }

            OsmNode const & node = nodeIter.value();
            osmData.addNodeReference(node.coordinates(), node.osmData());
            lineString.append(node.coordinates());
            usedNodes << nodeId;
        }

        geometry = new GeoDataLineString(lineString.optimized());
    }

    Q_ASSERT(geometry != nullptr);

    OsmObjectManager::registerId(way.osmData().id());

    GeoDataPlacemark *placemark = new GeoDataPlacemark;
    placemark->setGeometry(geometry);
    placemark->setVisualCategory(StyleBuilder::determineVisualCategory(way.osmData()));
    placemark->setName(way.osmData().tagValue(QStringLiteral("name")));
    if (placemark->name().isEmpty()) {
        placemark->setName(way.osmData().tagValue(QStringLiteral("ref")));
    }
    placemark->setOsmData(osmData);
    placemark->setVisible(placemark->visualCategory() != GeoDataFeature::None);

    document->append(placemark);
}

void OsmParser::createRelation(GeoDataDocument *document, const OsmRelation &relation, OsmWays &ways, const OsmNodes &nodes, QSet<qint64> &usedNodes, QSet<qint64> &usedWays)
{
    if (!relation.osmData().containsTag(QStringLiteral("type"), QStringLiteral("multipolygon"))) {
        return;
    }

    QStringList const outerRoles = QStringList() << QStringLiteral("outer") << QString();
    QSet<qint64> outerWays;
    QSet<qint64> outerNodes;
    const QList<GeoDataLinearRing> outer = relation.rings(outerRoles, ways, nodes, outerNodes, outerWays);

    if (outer.isEmpty()) {
        return;
    }

    GeoDataFeature::GeoDataVisualCategory outerCategory = StyleBuilder::determineVisualCategory(relation.osmData());
    if (outerCategory == GeoDataFeature::None) {
        // Try to determine the visual category from the relation members
        GeoDataFeature::GeoDataVisualCategory const firstCategory =
                StyleBuilder::determineVisualCategory(ways[*outerWays.begin()].osmData());

        bool categoriesAreSame = true;
        foreach (auto wayId, outerWays) {
            GeoDataFeature::GeoDataVisualCategory const category =
                    StyleBuilder::determineVisualCategory(ways[wayId].osmData());
            if( category != firstCategory ) {
                categoriesAreSame = false;
                break;
            }
        }

        if( categoriesAreSame ) {
            outerCategory = firstCategory;
        }
    }

    foreach(qint64 wayId, outerWays) {
        Q_ASSERT(ways.contains(wayId));
        GeoDataFeature::GeoDataVisualCategory const category = StyleBuilder::determineVisualCategory(ways[wayId].osmData());
        if (category == GeoDataFeature::None || category == outerCategory) {
            // Schedule way for removal: It's a non-styled way only used to create the outer boundary in this polygon
            usedWays << wayId;
        } // else we keep it

        foreach(qint64 nodeId, ways[wayId].references()) {
            ways[wayId].osmData().addNodeReference(nodes[nodeId].coordinates(), nodes[nodeId].osmData());
        }
    }

    QStringList const innerRoles = QStringList() << QStringLiteral("inner");
    QSet<qint64> innerWays;
    const QList<GeoDataLinearRing> inner = relation.rings(innerRoles, ways, nodes, usedNodes, innerWays);

    OsmPlacemarkData osmData = relation.osmData();
    osmData.addMemberReference(-1, ways[*outerWays.begin()].osmData());
    int index = 0;
    foreach(qint64 wayId, innerWays) {
        Q_ASSERT(ways.contains(wayId));
        if (StyleBuilder::determineVisualCategory(ways[wayId].osmData()) == GeoDataFeature::None) {
            // Schedule way for removal: It's a non-styled way only used to create the inner boundary in this polygon
            usedWays << wayId;
        }
        foreach(qint64 nodeId, ways[wayId].references()) {
            ways[wayId].osmData().addNodeReference(nodes[nodeId].coordinates(), nodes[nodeId].osmData());
        }
        osmData.addMemberReference(index, ways[wayId].osmData());
        ++index;
    }

    if (outerCategory == GeoDataFeature::Bathymetry) {
        // In case of a bathymetry store elevation info since it is required during styling
        // The ele=* tag is present in the outermost way
        const QString ele = QStringLiteral("ele");
        const OsmPlacemarkData &outerWayData = ways[*outerWays.begin()].osmData();
        auto tagIter = outerWayData.findTag(ele);
        if (tagIter != outerWayData.tagsEnd()) {
            osmData.insertTag(ele, tagIter.value());
        }
    }

    bool const hasMultipleOuterRings = outer.size() > 1;
    foreach(auto const &outerRing, outer) {
        GeoDataPolygon *polygon = new GeoDataPolygon;
        polygon->setOuterBoundary(outerRing);
        foreach(const GeoDataLinearRing &innerRing, inner) {
            if (!innerRing.isEmpty() && outerRing.contains(innerRing.first())) {
                polygon->appendInnerBoundary(innerRing);
            }
        }

        GeoDataPlacemark *placemark = new GeoDataPlacemark;
        placemark->setName(relation.osmData().tagValue(QStringLiteral("name")));
        placemark->setVisualCategory(outerCategory);
        placemark->setStyle( GeoDataStyle::Ptr() );
        placemark->setVisible(outerCategory != GeoDataFeature::None);
        placemark->setGeometry(polygon);
        if (hasMultipleOuterRings) {
            /** @TODO Use a GeoDataMultiGeometry to keep the ID? */
            osmData.setId(0);
            OsmObjectManager::initializeOsmData(placemark);
        } else {
            OsmObjectManager::registerId(osmData.id());
        }
        placemark->setOsmData(osmData);
        usedNodes |= outerNodes;

        document->append(placemark);
    }
}

int OsmParser::populationIndex(qint64 population)
{
    int popidx = 3;

    if ( population < 2500 )        popidx=10;
    else if ( population < 5000)    popidx=9;
    else if ( population < 25000)   popidx=8;
    else if ( population < 75000)   popidx=7;
    else if ( population < 250000)  popidx=6;
    else if ( population < 750000)  popidx=5;
    else if ( population < 2500000) popidx=4;

    return popidx;
}

}
