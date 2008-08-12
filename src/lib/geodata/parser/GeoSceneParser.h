/*
    Copyright (C) 2007, 2008 Nikolas Zimmermann <zimmermann@kde.org>

    This file is part of the KDE project

    This library is free software you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    aint with this library see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef GeoSceneParser_h
#define GeoSceneParser_h

#include <geodata_export.h>
#include "GeoParser.h"

class GeoDocument;

namespace Marble
{

class GeoSceneDocument;

enum GeoSceneSourceType {
    GeoScene_DGML   = 0
};

class GEODATA_EXPORT GeoSceneParser : public GeoParser {
public:
    explicit GeoSceneParser(GeoSceneSourceType source);
    virtual ~GeoSceneParser();

private:
    virtual bool isValidElement(const QString& tagName) const;
    virtual bool isValidDocumentElement() const;
    virtual void raiseDocumentElementError();

    virtual GeoDocument* createDocument() const;
};

// Global helper function for the tag handlers
GeoSceneDocument* geoSceneDoc(GeoParser& parser);

}

#endif // GeoSceneParser_h
