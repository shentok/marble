//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2012      Dennis Nienhüser <nienhueser@kde.org>
//

#include <QApplication>
#include <QDebug>

#include <marble/MarbleWidget.h>
#include <marble/MarbleModel.h>
#include <marble/ReverseGeocodingRunnerManager.h>

using namespace Marble;

int main(int argc, char** argv)
{
    QApplication app( argc, argv );

    MarbleModel model;
    ReverseGeocodingRunnerManager manager( &model );

    const GeoDataCoordinates position(-GeoDataLongitude::fromDegrees(0.15845), GeoDataLatitude::fromDegrees(51.52380));
    qDebug() << position.toString() << "is" << manager.searchReverseGeocoding( position );
}
