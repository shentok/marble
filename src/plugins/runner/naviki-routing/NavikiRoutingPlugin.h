//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2015      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//


#ifndef MARBLE_NAVIKI_ROUTINGPLUGIN_H
#define MARBLE_NAVIKI_ROUTINGPLUGIN_H

#include "RoutingRunnerPlugin.h"

namespace Marble
{

class NavikiRoutingPlugin : public RoutingRunnerPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA( IID "org.kde.edu.marble.NavikiRoutingPlugin" )
    Q_INTERFACES( Marble::RoutingRunnerPlugin )

public:
    explicit NavikiRoutingPlugin( QObject *parent = 0 );

    QString name() const;

    QString guiString() const;

    QString nameId() const;

    QString version() const;

    QString description() const;

    QString copyrightYears() const;

    QList<PluginAuthor> pluginAuthors() const;

    RoutingRunner *newRunner() const;

    ConfigWidget *configWidget();

    bool supportsTemplate( RoutingProfilesModel::ProfileTemplate profileTemplate ) const;
};

}

#endif
