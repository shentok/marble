//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2015      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include "CycleTravelPlugin.h"
#include "CycleTravelRunner.h"

namespace Marble
{

CycleTravelPlugin::CycleTravelPlugin( QObject *parent ) :
    RoutingRunnerPlugin( parent )
{
    setSupportedCelestialBodies( QStringList() << "earth" );
    setCanWorkOffline( false );
    setStatusMessage( tr ( "This service requires an Internet connection." ) );
}

QString CycleTravelPlugin::name() const
{
    return tr( "Cycle Travel Routing" );
}

QString CycleTravelPlugin::guiString() const
{
    return tr( "CycleTravel" );
}

QString CycleTravelPlugin::nameId() const
{
    return "cycletravel";
}

QString CycleTravelPlugin::version() const
{
    return "1.0";
}

QString CycleTravelPlugin::description() const
{
    return tr( "Worldwide routing using cycle.travel" );
}

QString CycleTravelPlugin::copyrightYears() const
{
    return "2015";
}

QList<PluginAuthor> CycleTravelPlugin::pluginAuthors() const
{
    return QList<PluginAuthor>()
            << PluginAuthor( QString::fromUtf8( "Bernhard Beschow" ), "bbeschow@cs.tu-berlin.de" );
}

RoutingRunner *CycleTravelPlugin::newRunner() const
{
    return new CycleTravelRunner;
}

bool CycleTravelPlugin::supportsTemplate( RoutingProfilesModel::ProfileTemplate profileTemplate ) const
{
    return profileTemplate == RoutingProfilesModel::BicycleTemplate;
}

}

Q_EXPORT_PLUGIN2( CycleTravelPlugin, Marble::CycleTravelPlugin )

#include "CycleTravelPlugin.moc"
