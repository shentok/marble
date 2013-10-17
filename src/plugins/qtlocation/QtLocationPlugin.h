//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#ifndef QTLOCATION_PLUGIN_H
#define QTLOCATION_PLUGIN_H

#include <QGeoServiceProviderFactory>

QTM_USE_NAMESPACE

/**
  * Registers MarbleWidget, MarbleRunnerManager and MarbleThemeManager
  * as QDeclarative extensions for use in QML.
  */
class QtLocationPlugin : public QObject, public QGeoServiceProviderFactory
{
    Q_OBJECT
    Q_INTERFACES(QtMobility::QGeoServiceProviderFactory)

public:
    QString providerName() const;

    int providerVersion() const;

    QGeoMappingManagerEngine *createMappingManagerEngine(const QMap<QString, QVariant> &parameters, QGeoServiceProvider::Error *error, QString *errorString) const;
};

#endif
