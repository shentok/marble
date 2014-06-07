//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2014 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//


#include "SearchModel.h"
#include "SearchItem.h"

#include "MarbleDebug.h"
#include "MarbleGlobal.h"
#include "MarbleModel.h"
#include "GeoDataCoordinates.h"
#include "GeoDataPlacemark.h"
#include "GeoDataStyle.h"
#include "TileId.h"

using namespace Marble;


SearchModel::SearchModel( const MarbleModel *marbleModel, QObject *parent ) :
    AbstractDataPluginModel( "search", marbleModel, parent ),
    m_runnerManager( marbleModel, this ),
    m_searchTerm()
{
    connect( &m_runnerManager, SIGNAL(placemarkSearchFinished()), this, SLOT(finished()) );
}

SearchModel::~SearchModel()
{
    // Nothing to do...
}

void SearchModel::setSearchTerm( const QString &searchTerm )
{
    if ( m_searchTerm == searchTerm )
        return;

    m_searchTerm = searchTerm;

    clear();
}

QString SearchModel::searchTerm() const
{
    return m_searchTerm;
}

void SearchModel::getAdditionalItems( const GeoDataLatLonBox& box, qint32 number, const TileId& tileId )
{
    Q_UNUSED( number )

    if( marbleModel()->planetId() != "earth" )
        return;

    Foo foo;
    foo.box = box;
    foo.zoomLevel = tileId.zoomLevel();

    if ( m_queue.contains( foo ) ) {
        return;
    }

    if ( m_runnerManager.isRunning() ) {
        mDebug() << "queueing" << box.toString( GeoDataCoordinates::Degree );
        m_queue.prepend( foo );
    }
    else {
        mDebug() << "searching" << box.toString( GeoDataCoordinates::Degree );
        m_zoomLevel = tileId.zoomLevel();
        m_runnerManager.findPlacemarks( m_searchTerm, box );
    }
}

void SearchModel::parseFile( const QByteArray &file, const TileId &tileId )
{
    Q_UNUSED( file );
    Q_UNUSED( tileId );
}

void SearchModel::finished()
{
    QVector<GeoDataPlacemark *> placemarks = m_runnerManager.searchResult();

    for ( int i = 0; i < placemarks.size(); ++i ) {
        const QString id = m_runnerManager.bbox().toString() + QString::number( i );

        GeoDataPlacemark *placemark = placemarks.at( i );
        if( !itemExists( id ) ) {
            // If it does not exist, create it
            SearchItem *item = new SearchItem( this );
            item->setId( id );
            GeoDataCoordinates coordinates = placemark->coordinate();
            coordinates.setDetail( m_zoomLevel );
            item->setCoordinate( coordinates );
            item->setName( placemark->name() );
            item->setPixmap( QPixmap::fromImage( placemark->style()->iconStyle().icon() ) );

            const TileId tileId = TileId::fromCoordinates( coordinates, m_zoomLevel );

            addItemToList( item, tileId );
        }
    }

    if ( !m_queue.isEmpty() ) {
        const Foo foo = m_queue.takeFirst();
        mDebug() << "searching" << foo.box.toString( GeoDataCoordinates::Degree );
        m_zoomLevel = foo.zoomLevel;
        m_runnerManager.findPlacemarks( m_searchTerm, foo.box );
    }
}
 
#include "SearchModel.moc"
