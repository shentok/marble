//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#ifndef DECLARATIVE_ROUTINGPROFILESMODEL_H
#define DECLARATIVE_ROUTINGPROFILESMODEL_H

#include <QtGui/QSortFilterProxyModel>

#include <QtCore/QPointer>

class Routing;

class DeclarativeRoutingProfilesModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY( Routing* routing READ routing WRITE setRouting NOTIFY routingChanged )

public:
    explicit DeclarativeRoutingProfilesModel( QObject *parent = 0  );

    Routing *routing();

    Q_INVOKABLE QVariantMap get( int index ) const;
    Q_INVOKABLE int count() const;

public Q_SLOTS:
    void setRouting( Routing *routing );

private Q_SLOTS:
    void updateSourceModel();

Q_SIGNALS:
    void routingChanged();

private:
    QPointer<Routing> m_routing;
};

#endif // DECLARATIVE_ROUTINGPROFILESMODEL_H
