//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2015      Stanciu Marius-Valeriu <stanciumarius94@gmail.com>
//

//Self
#include "OsmNodeTagWriter.h"

//Qt
#include <QHash>

//Marble
#include "OsmElementDictionary.h"
#include "OsmObjectAttributeWriter.h"
#include "OsmTagTagWriter.h"
#include "GeoDataPoint.h"
#include "GeoDataLineString.h"
#include "GeoWriter.h"
#include "osm/OsmPlacemarkData.h"
#include "osm/OsmObjectManager.h"

namespace Marble
{


void OsmNodeTagWriter::writeNode( const OsmConverter::Node &node, GeoWriter& writer )
{
    writer.writeStartElement( osm::osmTag_node );

    OsmObjectAttributeWriter::writeAttributes(node.osmData(), writer);
    OsmTagTagWriter::writeTags(node.osmData(), writer);

    writer.writeEndElement();
}

void OsmNodeTagWriter::writeAllNodes( const OsmConverter::Nodes& nodes, GeoWriter& writer )
{
    // Writing all the component nodes
    qint64 lastId = 0;
    foreach(const auto &node, nodes) {
        if (node.osmData().id() != lastId) {
            writeNode(node, writer);
            lastId = node.osmData().id();
        } // else duplicate/shared node
    }
}

}
