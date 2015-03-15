//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2015      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include "NavikiRoutingPlugin.h"

#include "NavikiRoutingConfigWidget.h"
#include "NavikiRoutingRunner.h"

namespace Marble
{

NavikiRoutingPlugin::NavikiRoutingPlugin( QObject *parent ) :
    RoutingRunnerPlugin( parent )
{
    setSupportedCelestialBodies( QStringList() << "earth" );
    setCanWorkOffline( false );
    setStatusMessage( tr ( "This service requires an Internet connection." ) );
}

QString NavikiRoutingPlugin::name() const
{
    return tr( "Naviki Routing" );
}

QString NavikiRoutingPlugin::guiString() const
{
    return tr( "Naviki" );
}

QString NavikiRoutingPlugin::nameId() const
{
    return "naviki routing";
}

QString NavikiRoutingPlugin::version() const
{
    return "1.0";
}

QString NavikiRoutingPlugin::description() const
{
    return tr( "Europe-based routing using naviki.org" );
}

QString NavikiRoutingPlugin::copyrightYears() const
{
    return "2015";
}

QList<PluginAuthor> NavikiRoutingPlugin::pluginAuthors() const
{
    return QList<PluginAuthor>()
            << PluginAuthor( QString::fromUtf8( "Bernhard Beschow" ), "bbeschow@cs.tu-berlin.de" );
}

RoutingRunner *NavikiRoutingPlugin::newRunner() const
{
    return new NavikiRoutingRunner;
}

RoutingRunnerPlugin::ConfigWidget *NavikiRoutingPlugin::configWidget()
{
    return new NavikiRoutingConfigWidget();
}

bool NavikiRoutingPlugin::supportsTemplate( RoutingProfilesModel::ProfileTemplate profileTemplate ) const
{
    return profileTemplate == RoutingProfilesModel::BicycleTemplate;
}

}

Q_EXPORT_PLUGIN2( NavikiRoutingPlugin, Marble::NavikiRoutingPlugin )

#include "NavikiRoutingPlugin.moc"
