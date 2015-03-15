//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2015      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include "NavikiRoutingConfigWidget.h"

#include "ui_NavikiRoutingConfigWidget.h"

namespace Marble
{

NavikiRoutingConfigWidget::NavikiRoutingConfigWidget() :
    RoutingRunnerPlugin::ConfigWidget()
{
    ui_configWidget = new Ui::NavikiRoutingConfigWidget;
    ui_configWidget->setupUi( this );

    ui_configWidget->preference->addItem( tr( "Everyday" ), "5030" );
    ui_configWidget->preference->addItem( tr( "Leisure" ), "5031" );
    ui_configWidget->preference->addItem( tr( "Racing bike" ), "5034" );
    ui_configWidget->preference->addItem( tr( "Mountain bike" ), "5033" );
    ui_configWidget->preference->addItem( tr( "Shorter Route" ), "5032" );
}

void NavikiRoutingConfigWidget::loadSettings( const QHash<QString, QVariant> &settings )
{
    ui_configWidget->preference->setCurrentIndex(
        ui_configWidget->preference->findData( settings.value( "preference", "5030" ).toString() ) );
}

QHash<QString, QVariant> NavikiRoutingConfigWidget::settings() const
{
    QHash<QString, QVariant> settings;

    settings.insert( "preference", ui_configWidget->preference->itemData( ui_configWidget->preference->currentIndex() ) );

    return settings;
}

}

#include "NavikiRoutingConfigWidget.moc"
