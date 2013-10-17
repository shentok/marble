//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include "QtLocationPlugin.h"

#include "MarbleGeoMappingManagerEngine.h"

QString QtLocationPlugin::providerName() const
{
    return "marble";
}

int QtLocationPlugin::providerVersion() const
{
    return 1;
}

QGeoMappingManagerEngine *QtLocationPlugin::createMappingManagerEngine(const QMap<QString, QVariant> &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    return new MarbleGeoMappingManagerEngine(parameters, error, errorString);
}

#include "QtLocationPlugin.moc"

Q_EXPORT_PLUGIN2( QtLocationPlugin, QtLocationPlugin )
