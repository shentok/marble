//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011      Dennis Nienhüser <earthwings@gentoo.org>
//

#include "Settings.h"

#include <QApplication>

Settings::Settings() :
    m_organizationName( QApplication::organizationName() ),
    m_applicationName( QApplication::applicationName() )
{
    // nothing to do
}

QString Settings::organizationName() const
{
    return m_organizationName;
}

void Settings::setOrganizationName( const QString &organization )
{
    m_organizationName = organization;
}

QString Settings::applicationName() const
{
    return m_applicationName;
}

void Settings::setApplicationName( const QString &application )
{
    m_applicationName = application;
}

QString Settings::groupName() const
{
    return m_groupName;
}

void Settings::setGroupName( const QString &groupName )
{
    m_groupName = groupName;
}

void Settings::readSettings()
{
    qDebug() << Q_FUNC_INFO;

    QSettings settings( m_organizationName, m_applicationName );
    settings.beginGroup( m_groupName );

    for ( int i = staticMetaObject.propertyCount(); i < metaObject()->propertyCount(); ++i ) {
        const QMetaProperty property = metaObject()->property( i );
        const QString key = property.name();
        const QVariant defaultValue = property.read( this );
        const QVariant value = settings.value( key, defaultValue );
        qDebug() << key << value;
        property.write( this, value );
    }

    settings.endGroup();
}

void Settings::writeSettings()
{
    qDebug() << Q_FUNC_INFO;

    QSettings settings( m_organizationName, m_applicationName );
    settings.beginGroup( m_groupName );

    for ( int i = staticMetaObject.propertyCount(); i < metaObject()->propertyCount(); ++i ) {
        const QMetaProperty property = metaObject()->property( i );
        const QString key = property.name();
        const QVariant value = property.read( this );
        qDebug() << key << value;
        settings.setValue( key, value );
    }

    settings.endGroup();
}

#include "Settings.moc"
