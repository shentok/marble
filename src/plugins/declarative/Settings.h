//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011      Dennis Nienhüser <earthwings@gentoo.org>
//

#ifndef MARBLE_DECLARATIVE_SETTINGS_H
#define MARBLE_DECLARATIVE_SETTINGS_H

#include <QDeclarativeItem>
#include <QSettings>

class Settings : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY( QString organizationName READ organizationName WRITE setOrganizationName )
    Q_PROPERTY( QString applicationName READ applicationName WRITE setApplicationName )
    Q_PROPERTY( QString groupName READ groupName WRITE setGroupName )

public:
    Settings();

    QString organizationName() const;

    void setOrganizationName( const QString &organization );

    QString applicationName() const;

    void setApplicationName( const QString &application );

    QString groupName() const;

    void setGroupName( const QString &groupName );

    Q_INVOKABLE void readSettings();
    Q_INVOKABLE void writeSettings();

private:
    QString m_organizationName;
    QString m_applicationName;
    QString m_groupName;
};

#endif // MARBLE_DECLARATIVE_SETTINGS_H
