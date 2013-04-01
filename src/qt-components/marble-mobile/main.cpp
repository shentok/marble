// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013 Bernhard Beschow <bbeschow@cs.tu-berlin.de>

#include <QtGui/QApplication>
#include <QtDeclarative/QDeclarativeView>
#include <QtDeclarative/QDeclarativeEngine>

int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );

    // Create main window based on QML.
    QDeclarativeView view;
    view.setSource( QUrl( "qrc:/main.qml" ) );

    QObject::connect( view.engine(), SIGNAL(quit()), &view, SLOT(close()) );

    // Window takes up full screen on arm (mobile) devices.
    view.show();

    return app.exec();
}
