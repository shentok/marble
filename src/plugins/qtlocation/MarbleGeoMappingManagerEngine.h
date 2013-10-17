//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#ifndef MARBLE_GEOMAPPINGMANAGERENGINE_H
#define MARBLE_GEOMAPPINGMANAGERENGINE_H

#include <QGeoMappingManagerEngine>
#include <QGeoServiceProvider>

/**
  * Registers MarbleWidget, MarbleRunnerManager and MarbleThemeManager
  * as QDeclarative extensions for use in QML.
  */
class MarbleGeoMappingManagerEngine : public QtMobility::QGeoMappingManagerEngine
{
    Q_OBJECT

public:
    MarbleGeoMappingManagerEngine(const QMap<QString, QVariant> &parameters, QtMobility::QGeoServiceProvider::Error *error, QString *errorString, QObject *parent = 0);

    QtMobility::QGeoMapData *createMapData();
};

#endif
