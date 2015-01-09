//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Bastian Holst <bastianholst@gmx.de>
//

// Self
#include "FakeWeatherService.h"

// Marble
#include "FakeWeatherItem.h"
#include "GeoDataCoordinates.h"
#include "GeoDataLatLonBox.h"
#include "WeatherData.h"
#include "WeatherModel.h"
#include "TileId.h"

using namespace Marble;

FakeWeatherService::FakeWeatherService( const MarbleModel *model, QObject *parent )
    : AbstractWeatherService( model, parent )
{
}

FakeWeatherService::~FakeWeatherService()
{
}
    
void FakeWeatherService::getAdditionalItems( const GeoDataLatLonBox& box,
                                             qint32 number,
                                             const TileId &tileId )
{
    Q_UNUSED( box );
    Q_UNUSED( number );

    FakeWeatherItem *item = new FakeWeatherItem( this );
    item->setStationName( "Fake" );
    item->setPriority( 0 );
    item->setCoordinate( GeoDataCoordinates( 1, 1, 0, GeoDataCoordinates::Radian ) );
    item->setId( "fake1" );
    
    WeatherData data;
    data.setCondition( WeatherData::ClearDay );
    data.setTemperature( 14.0, WeatherData::Celsius );
    item->setCurrentWeather( data );
        
    emit createdItems( QList<AbstractDataPluginItem*>() << item, tileId );
}

void FakeWeatherService::getItem( const QString & )
{
    getAdditionalItems( GeoDataLatLonBox(), 1, TileId( 0, 0, 0, 0 ) );
}

#include "FakeWeatherService.moc"
