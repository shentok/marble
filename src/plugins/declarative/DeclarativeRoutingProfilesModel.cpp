//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010      Niko Sams <niko.sams@gmail.com>
//

#include "DeclarativeRoutingProfilesModel.h"

#include "routing/RoutingProfilesModel.h"

#include "Routing.h"

DeclarativeRoutingProfilesModel::DeclarativeRoutingProfilesModel( QObject *parent ) :
    QSortFilterProxyModel( parent ),
    m_routing( 0 )
{
}

Routing *DeclarativeRoutingProfilesModel::routing()
{
    return m_routing;
}

QVariantMap DeclarativeRoutingProfilesModel::get( int row ) const
{
    QVariantMap result;

    QHash<int, QByteArray>::ConstIterator it = roleNames().constBegin();

    for ( ; it != roleNames().constEnd(); ++it ) {
        const int role = it.key();
        const QVariant data = this->data( index( row, 0 ), role );
        if ( !data.isValid() ) {
            continue;
        }
        result.insert( it.value(), data );
    }

    return result;
}

int DeclarativeRoutingProfilesModel::count() const
{
    return rowCount();
}

void DeclarativeRoutingProfilesModel::setRouting( Routing *routing )
{
    if ( m_routing.data() == routing )
        return;

    if ( m_routing ) {
        disconnect( m_routing.data(), 0, this, 0 );
    }

    m_routing = routing;

    updateSourceModel();

    if ( m_routing ) {
        connect( m_routing.data(), SIGNAL(mapChanged()), this, SLOT(updateSourceModel()) );
    }

    emit routingChanged();
}

void DeclarativeRoutingProfilesModel::updateSourceModel()
{
    setSourceModel( m_routing ? m_routing->routingProfilesModel() : 0 );
    QHash<int, QByteArray> roleNames = this->roleNames();
    roleNames.insert( Qt::DisplayRole, "name" );
    setRoleNames( roleNames );
}

#include "DeclarativeRoutingProfilesModel.moc"
