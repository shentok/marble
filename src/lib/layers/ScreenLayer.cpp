//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010           Thibaut Gridel <tgridel@free.fr>
// Copyright 2011-2013      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include "ScreenLayer.h"

// Marble
#include "GeoDataContainer.h"
#include "GeoDataObject.h"
#include "GeoDataTypes.h"
#include "MarbleDebug.h"
#include "GeoDataFeature.h"
#include "GeoPainter.h"
#include "ViewportParams.h"
#include "GeoDataScreenOverlay.h"
#include "ScreenOverlayGraphicsItem.h"
#include "MarblePlacemarkModel.h"

// Qt
#include <QtCore/qmath.h>
#include <QtCore/QAbstractItemModel>
#include <QtCore/QModelIndex>

namespace Marble
{
class ScreenLayer::Private
{
public:
    Private( const QAbstractItemModel *model, ScreenLayer *parent );

    void addPlacemarks( const QModelIndex &index, int first, int last );
    void removePlacemarks( const QModelIndex &index, int first, int last );
    void resetCacheData();

    void createGraphicsItems( const GeoDataObject *object );
    void createGraphicsItemFromOverlay( const GeoDataOverlay *overlay );
    void removeGraphicsItems( const GeoDataFeature *feature );

    ScreenLayer *const q;
    const QAbstractItemModel *const m_model;
    QString m_runtimeTrace;
    QList<ScreenOverlayGraphicsItem *> m_items;
};

ScreenLayer::Private::Private( const QAbstractItemModel *model, ScreenLayer *parent ) :
    q( parent ),
    m_model( model )
{
}

void ScreenLayer::Private::addPlacemarks( const QModelIndex &parent, int first, int last )
{
    Q_ASSERT( first < m_model->rowCount( parent ) );
    Q_ASSERT( last < m_model->rowCount( parent ) );

    for( int i = first; i <= last; ++i ) {
        const QModelIndex index = m_model->index( i, 0, parent );
        Q_ASSERT( index.isValid() );
        const GeoDataObject *object = qvariant_cast<GeoDataObject *>(index.data( MarblePlacemarkModel::ObjectPointerRole ) );
        Q_ASSERT( object );
        createGraphicsItems( object );
    }

    emit q->repaintNeeded();
}

void ScreenLayer::Private::removePlacemarks( const QModelIndex &parent, int first, int last )
{
    Q_ASSERT( last < m_model->rowCount( parent ) );

    for ( int i = first; i <= last; ++i ) {
        const QModelIndex index = m_model->index( i, 0, parent );
        Q_ASSERT( index.isValid() );
        const GeoDataObject *object = qvariant_cast<GeoDataObject *>( index.data( MarblePlacemarkModel::ObjectPointerRole ) );
        const GeoDataFeature *feature = dynamic_cast<const GeoDataFeature *>( object );
        Q_ASSERT( feature );
        removeGraphicsItems( feature );
    }

    emit q->repaintNeeded();
}

void ScreenLayer::Private::resetCacheData()
{
    qDeleteAll( m_items );
    m_items.clear();

    const GeoDataObject *object = static_cast<GeoDataObject *>( m_model->index( 0, 0, QModelIndex() ).internalPointer() );
    if ( object && object->parent() )
        createGraphicsItems( object->parent() );

    emit q->repaintNeeded();
}

void ScreenLayer::Private::createGraphicsItems( const GeoDataObject *object )
{
    if ( const GeoDataOverlay *overlay = dynamic_cast<const GeoDataOverlay *>( object ) ) {
        createGraphicsItemFromOverlay( overlay );
    }

    // parse all child objects of the container
    if ( const GeoDataContainer *container = dynamic_cast<const GeoDataContainer *>( object ) ) {
        for ( int row = 0; row < container->size(); ++row ) {
            createGraphicsItems( container->child( row ) );
        }
    }
}

void ScreenLayer::Private::createGraphicsItemFromOverlay( const GeoDataOverlay *overlay )
{
    if ( overlay->nodeType() == GeoDataTypes::GeoDataScreenOverlayType ) {
        const GeoDataScreenOverlay *screenOverlay = static_cast<const GeoDataScreenOverlay *>( overlay );
        ScreenOverlayGraphicsItem *screenItem = new ScreenOverlayGraphicsItem( screenOverlay );
        m_items.push_back( screenItem );
    }
}

void ScreenLayer::Private::removeGraphicsItems( const GeoDataFeature *feature )
{

    if( feature->nodeType() == GeoDataTypes::GeoDataFolderType
             || feature->nodeType() == GeoDataTypes::GeoDataDocumentType ) {
        const GeoDataContainer *container = static_cast<const GeoDataContainer *>( feature );
        foreach( const GeoDataFeature *child, container->featureList() ) {
            removeGraphicsItems( child );
        }
    }
    else if( feature->nodeType() == GeoDataTypes::GeoDataScreenOverlayType ) {
        foreach( ScreenOverlayGraphicsItem *item, m_items ) {
            if( item->screenOverlay() == feature ) {
                m_items.removeAll( item );
            }
        }
    }
}

ScreenLayer::ScreenLayer( const QAbstractItemModel *model ) :
    d( new Private( model, this ) )
{
    const GeoDataObject *object = static_cast<GeoDataObject *>( d->m_model->index( 0, 0, QModelIndex() ).internalPointer() );
    if ( object && object->parent() )
        d->createGraphicsItems( object->parent() );

    connect( model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
             this, SLOT(resetCacheData()) );
    connect( model, SIGNAL(rowsInserted(QModelIndex,int,int)),
             this, SLOT(addPlacemarks(QModelIndex,int,int)) );
    connect( model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
             this, SLOT(removePlacemarks(QModelIndex,int,int)) );
    connect( model, SIGNAL(modelReset()),
             this, SLOT(resetCacheData()) );
}

ScreenLayer::~ScreenLayer()
{
    delete d;
}

QStringList ScreenLayer::renderPosition() const
{
    return QStringList( "FLOAT_ITEM" );
}

bool ScreenLayer::setViewport( const ViewportParams *viewport )
{
    Q_UNUSED( viewport )

    foreach( ScreenOverlayGraphicsItem *item, d->m_items ) {
        item->setProjection( viewport );
    }

    d->m_runtimeTrace = QString( "Items: %1")
                .arg( d->m_items.size() );

    return true;
}

bool ScreenLayer::render( GeoPainter *painter, const QSize &viewportSize ) const
{
    Q_UNUSED( viewportSize )

    painter->save();

    foreach( ScreenOverlayGraphicsItem *item, d->m_items ) {
        item->paintEvent( painter );
    }

    painter->restore();

    return true;
}

QString ScreenLayer::runtimeTrace() const
{
    return d->m_runtimeTrace;
}

}

#include "ScreenLayer.moc"
