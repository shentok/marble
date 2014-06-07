//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2014 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#ifndef SEARCHPLUGIN_H
#define SEARCHPLUGIN_H

#include "AbstractDataPlugin.h"
#include "DialogConfigurationInterface.h"

namespace Ui {
    class SearchConfigWidget;
}

namespace Marble {

class SearchPlugin : public AbstractDataPlugin, public DialogConfigurationInterface
{

    Q_OBJECT
    Q_PLUGIN_METADATA( IID "org.kde.edu.marble.SearchPlugin" )

    Q_INTERFACES(Marble::RenderPluginInterface)
    Q_INTERFACES(Marble::DialogConfigurationInterface)

    MARBLE_PLUGIN(SearchPlugin)

 public:
    SearchPlugin();

    explicit SearchPlugin( const MarbleModel *marbleModel );

    virtual void initialize();

    QString name() const;

    QString guiString() const;

    QString nameId() const;

    QString version() const;

    QString description() const;

    QString copyrightYears() const;

    QList<PluginAuthor> pluginAuthors() const;

    QIcon icon() const;

    QDialog *configDialog();

    QHash<QString,QVariant> settings() const;

    void setSettings( const QHash<QString,QVariant> &settings );

private Q_SLOTS:
    void readSettings();

    void writeSettings();

 private:
    QDialog * m_configDialog;
    Ui::SearchConfigWidget * m_uiConfigWidget;
    QString m_searchTerm;
};

}

#endif // SEARCHPLUGIN_H
