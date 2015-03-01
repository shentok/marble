//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2015      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include "GraphhopperRoutingPlugin.h"
#include "GraphhopperRoutingRunner.h"

namespace Marble
{

GraphhopperRoutingPlugin::GraphhopperRoutingPlugin( QObject *parent ) :
    RoutingRunnerPlugin( parent )
{
    setSupportedCelestialBodies( QStringList() << "earth" );
    setCanWorkOffline( false );
    setStatusMessage( tr ( "This service requires an Internet connection." ) );
}

QString GraphhopperRoutingPlugin::name() const
{
    return tr( "Graphhopper Routing" );
}

QString GraphhopperRoutingPlugin::guiString() const
{
    return tr( "Graphhopper" );
}

QString GraphhopperRoutingPlugin::nameId() const
{
    return "graphhopper routing";
}

QString GraphhopperRoutingPlugin::version() const
{
    return "1.0";
}

QString GraphhopperRoutingPlugin::description() const
{
    return tr( "Worldwide routing using graphhopper.com" );
}

QString GraphhopperRoutingPlugin::copyrightYears() const
{
    return "2015";
}

QList<PluginAuthor> GraphhopperRoutingPlugin::pluginAuthors() const
{
    return QList<PluginAuthor>()
            << PluginAuthor( QString::fromUtf8( "Bernhard Beschow" ), "bbeschow@cs.tu-berlin.de" );
}

RoutingRunner *GraphhopperRoutingPlugin::newRunner() const
{
    return new GraphhopperRoutingRunner;
}

bool GraphhopperRoutingPlugin::supportsTemplate( RoutingProfilesModel::ProfileTemplate profileTemplate ) const
{
    return profileTemplate == RoutingProfilesModel::CarFastestTemplate ||
           profileTemplate == RoutingProfilesModel::BicycleTemplate ||
           profileTemplate == RoutingProfilesModel::PedestrianTemplate;
}

}

Q_EXPORT_PLUGIN2( GraphhopperRoutingPlugin, Marble::GraphhopperRoutingPlugin )

#include "GraphhopperRoutingPlugin.moc"
