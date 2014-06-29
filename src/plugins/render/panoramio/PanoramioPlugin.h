//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Bastian Holst <bastianholst@gmx.de>
//

#ifndef PANORAMIOPLUGIN_H
#define PANORAMIOPLUGIN_H

#include "AbstractDataPlugin.h"
#include "DialogConfigurationInterface.h"

namespace Ui {
class ConfigWidget;
}

namespace Marble
{

class PanoramioPlugin : public AbstractDataPlugin, public DialogConfigurationInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA( IID "org.kde.edu.marble.PanoramioPlugin" )
    Q_INTERFACES( Marble::RenderPluginInterface )
    Q_INTERFACES( Marble::DialogConfigurationInterface )
    MARBLE_PLUGIN( PanoramioPlugin )

 public:
    PanoramioPlugin( const MarbleModel *marbleModel = 0 );

    QString nameId() const;

    QString version() const;

    QString copyrightYears() const;

    QList<PluginAuthor> pluginAuthors() const;

    void initialize();

    QString name() const;

    QString guiString() const;

    QString description() const;

    QIcon icon() const;

    QHash<QString,QVariant> settings() const;

    void setSettings( const QHash<QString,QVariant> &settings );

    QDialog *configDialog();

 protected:
    bool eventFilter( QObject *object, QEvent *event );

 private Q_SLOTS:
    void readSettings();
    void writeSettings();

 private:
    QDialog *m_configDialog;
    Ui::ConfigWidget *m_uiConfigWidget;
};

}

#endif // PANORAMIOPLUGIN_H
