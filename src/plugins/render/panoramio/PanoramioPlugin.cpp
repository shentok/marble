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
#include "PanoramioPlugin.h"
#include "PanoramioModel.h"
#include "ui_ConfigWidget.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>

#include "MarbleWidget.h"

using namespace Marble;

PanoramioPlugin::PanoramioPlugin( const MarbleModel *marbleModel ) :
    AbstractDataPlugin( marbleModel ),
    m_configDialog( 0 ),
    m_uiConfigWidget( 0 )
{
    setSettings( QHash<QString, QVariant>() );
}

QString Marble::PanoramioPlugin::nameId() const
{
    return "panoramio";
}

void PanoramioPlugin::initialize()
{
    setModel( new PanoramioModel( marbleModel(), this ) );
}

QString PanoramioPlugin::name() const
{
    return tr( "Panoramio Photos" );
}

QString PanoramioPlugin::guiString() const
{
    return tr( "&Panoramio" );
}

QString PanoramioPlugin::description() const
{
    return tr( "Automatically downloads images from around the world in preference to their popularity" );
}

QIcon PanoramioPlugin::icon() const
{
    return QIcon( ":/icons/panoramio.png" );
}

QString PanoramioPlugin::version() const
{
    return "0.1";
}

QString PanoramioPlugin::copyrightYears() const
{
    return "2009, 2014";
}

QList<PluginAuthor> PanoramioPlugin::pluginAuthors() const
{
    return QList<PluginAuthor>() << PluginAuthor( "Bastian Holst", "bastianholst@gmx.de" );
}

QHash<QString, QVariant> PanoramioPlugin::settings() const
{
    QHash<QString, QVariant> settings = AbstractDataPlugin::settings();

    settings.insert( "itemsOnScreen", numberOfItems() );

    return settings;
}

void PanoramioPlugin::setSettings( const QHash<QString, QVariant> &settings )
{
    AbstractDataPlugin::setSettings( settings );

    setNumberOfItems( settings.value( "itemsOnScreen", 15 ).toInt() );

    emit settingsChanged( nameId() );
}

QDialog *PanoramioPlugin::configDialog()
{
    if ( !m_configDialog ) {
        m_configDialog = new QDialog();
        m_uiConfigWidget = new Ui::ConfigWidget;
        m_uiConfigWidget->setupUi( m_configDialog );
        readSettings();

        connect( m_uiConfigWidget->m_buttonBox, SIGNAL(accepted()),
                SLOT(writeSettings()) );
        connect( m_uiConfigWidget->m_buttonBox, SIGNAL(rejected()),
                SLOT(readSettings()) );
        QPushButton *applyButton = m_uiConfigWidget->m_buttonBox->button( QDialogButtonBox::Apply );
        connect( applyButton, SIGNAL(clicked()),
                 this,        SLOT(writeSettings()) );
    }

    return m_configDialog;
}

bool PanoramioPlugin::eventFilter(QObject *object, QEvent *event)
{
    if ( isInitialized() ) {
        Q_ASSERT( dynamic_cast<PanoramioModel *>( model() ) != 0 );

        PanoramioModel *photoPluginModel = static_cast<PanoramioModel *>( model() );
        MarbleWidget *widget = dynamic_cast<MarbleWidget *>( object );
        if ( widget ) {
            photoPluginModel->setMarbleWidget( widget );
        }
    }

    return AbstractDataPlugin::eventFilter( object, event );
}

void PanoramioPlugin::readSettings()
{
    if ( m_uiConfigWidget ) {
        m_uiConfigWidget->m_itemsOnScreenSpin->setValue( numberOfItems() );
    }
}

void PanoramioPlugin::writeSettings()
{
    if ( m_uiConfigWidget ) {
        setNumberOfItems( m_uiConfigWidget->m_itemsOnScreenSpin->value() );
    }

    emit settingsChanged( nameId() );
}

Q_EXPORT_PLUGIN2(PanoramioPlugin, Marble::PanoramioPlugin)

#include "PanoramioPlugin.moc"
