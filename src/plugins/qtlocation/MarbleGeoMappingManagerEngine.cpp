//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include "MarbleGeoMappingManagerEngine.h"

MarbleGeoMappingManagerEngine::MarbleGeoMappingManagerEngine(const QMap<QString, QVariant> &parameters, QtMobility::QGeoServiceProvider::Error *error, QString *errorString, QObject *parent) :
    QGeoMappingManagerEngine(parameters, parent)
{
}

QGeoMapData *MarbleGeoMappingManagerEngine::createMapData()
{
    return new MarbleMapData();
}

#include "MarbleGeoMappingManagerEngine.moc"
