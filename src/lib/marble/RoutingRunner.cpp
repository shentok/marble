//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2012,2013 Bernhard Beschow <bbeschow@cs.tu-berlin.de>

#include "RoutingRunner.h"

#include "GeoDataPlacemark.h"
#include "GeoDataExtendedData.h"

#include <QString>

namespace Marble
{

RoutingRunner::RoutingRunner( QObject *parent ) :
    QObject( parent )
{
}

const QString RoutingRunner::lengthString(qreal length) const
{
    const QString result = "%1 %2";
    QString unit = QLatin1String( "m" );
    if (length >= 1000) {
        length /= 1000.0;
        unit = "km";
    }
    return result.arg( length, 0, 'f', 1 ).arg( unit );
}

const QString RoutingRunner::durationString(const QTime& duration) const
{
    const int seconds = duration.hour() * 3600 + duration.minute() * 60 + duration.second();
    return durationString( seconds );
}

const QString RoutingRunner::durationString( int seconds )
{
    const int hours = seconds / 3600;
    const int minutes = ( seconds / 60 ) % 60;
    const QString hoursString = QString::number( hours );
    const QString minutesString = ( minutes < 10 ? "0" : "" ) + QString::number( minutes );
    const QString timeString = tr("%1:%2 h","journey duration").arg( hoursString ).arg( minutesString );
    return timeString;
}

const QString RoutingRunner::nameString(const QString& name, qreal length, const QTime& duration) const
{
    const int seconds = duration.hour() * 3600 + duration.minute() * 60 + duration.second();
    return nameString( name, length, seconds );
}

const QString RoutingRunner::nameString( const QString &name, qreal length, int seconds ) const
{
    const QString result = "%1; %2 (%3)";
    return result.arg( lengthString( length ) ).arg( durationString( seconds ) ).arg( name );
}

const GeoDataExtendedData RoutingRunner::routeData(qreal length, const QTime& duration) const
{
    const int seconds = duration.hour() * 3600 + duration.minute() * 60 + duration.second();
    return routeData( length, seconds );
}

const GeoDataExtendedData RoutingRunner::routeData( qreal length, int seconds )
{
    GeoDataExtendedData result;
    GeoDataData lengthData;
    lengthData.setName( "length" );
    lengthData.setValue( length );
    result.addValue( lengthData );
    GeoDataData durationData;
    durationData.setName( "duration" );
    durationData.setValue( QString::number( seconds ) );
    result.addValue( durationData );
    return result;
}

}

#include "RoutingRunner.moc"
