//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2015      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#ifndef MARBLE_NAVIKI_ROUTING_CONFIGWIDGET_H
#define MARBLE_NAVIKI_ROUTING_CONFIGWIDGET_H

#include "RoutingRunnerPlugin.h"

namespace Ui {
    class NavikiRoutingConfigWidget;
}

namespace Marble
{

class NavikiRoutingConfigWidget : public RoutingRunnerPlugin::ConfigWidget
{
    Q_OBJECT

 public:
    NavikiRoutingConfigWidget();

    void loadSettings( const QHash<QString, QVariant> &settings );

    QHash<QString, QVariant> settings() const;

 private:
    Ui::NavikiRoutingConfigWidget *ui_configWidget;
};

}

#endif // MARBLE_NAVIKI_ROUTING_CONFIGWIDGET_H
