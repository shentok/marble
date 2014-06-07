//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2014 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include "SearchPlugin.h"
#include "SearchModel.h"

#include "ui_SearchConfigWidget.h"

#include <QLineEdit>
#include <QPushButton>

using namespace Marble;

SearchPlugin::SearchPlugin()
    : AbstractDataPlugin( 0 ),
      m_configDialog( 0 ),
      m_uiConfigWidget( 0 )
{
}

SearchPlugin::SearchPlugin( const MarbleModel *marbleModel )
    : AbstractDataPlugin( marbleModel ),
      m_configDialog( 0 ),
      m_uiConfigWidget( 0 )
{
    setEnabled( true ); // Plugin is enabled by default
    setVisible( false ); // Plugin is invisible by default
}

void SearchPlugin::initialize()
{
    SearchModel *model = new SearchModel( marbleModel(), this );
    setModel( model );
    model->setSearchTerm( m_searchTerm );
}

QString SearchPlugin::name() const
{
    return tr( "Search" );
}

QString SearchPlugin::guiString() const
{
    return tr( "&Search" );
}

QString SearchPlugin::nameId() const
{
    return "search";
}

QString SearchPlugin::version() const
{
    return "0.1";
}

QString SearchPlugin::description() const
{
    return tr( "Shows Search users' avatars and some extra information about them on the map." );
}

QString SearchPlugin::copyrightYears() const
{
    return "2014";
}

QList<PluginAuthor> SearchPlugin::pluginAuthors() const
{
    return QList<PluginAuthor>()
            << PluginAuthor( QString::fromUtf8( "Bernhard Beschow" ), "bbeschow@cs.tu-berlin.de" );
}

QIcon SearchPlugin::icon() const
{
    return QIcon( ":/icons/social.png" );
}

QDialog *SearchPlugin::configDialog()
{
    if ( !m_configDialog ) {
        m_configDialog = new QDialog();
        m_uiConfigWidget = new Ui::SearchConfigWidget;
        m_uiConfigWidget->setupUi( m_configDialog );
        readSettings();

        connect( m_uiConfigWidget->m_buttonBox, SIGNAL(accepted()),
                SLOT(writeSettings()) );
        connect( m_uiConfigWidget->m_buttonBox, SIGNAL(rejected()),
                SLOT(readSettings()) );
        QPushButton *applyButton = m_uiConfigWidget->m_buttonBox->button( 
                                                         QDialogButtonBox::Apply );
        connect( applyButton, SIGNAL(clicked()),
                 this,        SLOT(writeSettings()) );
    }

    return m_configDialog;
}

QHash<QString,QVariant> SearchPlugin::settings() const
{
    QHash<QString, QVariant> settings = AbstractDataPlugin::settings();

    settings.insert( "searchTerm", m_searchTerm );

    return settings;
}

void SearchPlugin::setSettings( const QHash<QString,QVariant> &settings )
{
    AbstractDataPlugin::setSettings( settings );

    m_searchTerm = settings.value( "searchTerm", "" ).toString();
    if ( model() ) {
        static_cast<SearchModel *>( model() )->setSearchTerm( m_searchTerm );
    }

    emit settingsChanged( nameId() );
}

void SearchPlugin::readSettings()
{
    if ( m_uiConfigWidget ) {
        m_uiConfigWidget->m_searchTerm->setText( m_searchTerm );
    }
}

void SearchPlugin::writeSettings()
{
    if ( m_uiConfigWidget ) {
        m_searchTerm = m_uiConfigWidget->m_searchTerm->text();
        if ( model() ) {
            static_cast<SearchModel *>( model() )->setSearchTerm( m_searchTerm );
        }
    }

    emit settingsChanged( nameId() );
}

Q_EXPORT_PLUGIN2( SearchPlugin, Marble::SearchPlugin )

#include "SearchPlugin.moc"
