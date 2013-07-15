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
#include "AbstractDataPluginModel.h"

// Qt
#include <qmath.h>
#include <QUrl>
#include <QTimer>
#include <QPointF>
#include <QtAlgorithms>
#include <QVariant>
#include <QAbstractListModel>
#include <QMetaProperty>

// Marble
#include "MarbleDebug.h"
#include "AbstractDataPluginItem.h"
#include "CacheStoragePolicy.h"
#include "GeoDataCoordinates.h"
#include "GeoDataLatLonAltBox.h"
#include "HttpDownloadManager.h"
#include "MarbleModel.h"
#include "MarbleDirs.h"
#include "TileId.h"
#include "ViewportParams.h"
#include "GeoDataLinearRing.h"
#include "GeoPainter.h"

#include <cmath>

namespace Marble
{

const QString descriptionPrefix( "description_" );

// Time between two tried description file downloads (we decided not to download anything) in ms
const int timeBetweenTriedDownloads = 500;
// Time between two real description file downloads in ms
const int timeBetweenDownloads = 1500;

// The factor describing how much the box has to be changed to download a new description file.
// A higher factor means more downloads.
const qreal boxComparisonFactor = 16.0;

// Separator to separate the id of the item from the file type
const char fileIdSeparator = '_';

class FavoritesModel;

class AbstractDataPluginModelPrivate
{
public:
    AbstractDataPluginModelPrivate( const QString& name,
                                    const MarbleModel *marbleModel,
                                    AbstractDataPluginModel * parent );
    
    ~AbstractDataPluginModelPrivate();

    QString generateFilename( const QString& id, const QString& type ) const;
    QString generateFilepath( const QString& id, const QString& type ) const;

    void updateFavoriteItems();

    AbstractDataPluginModel *m_parent;
    const QString m_name;
    const MarbleModel *const m_marbleModel;
    QString m_currentPlanetId;
    QMap<TileId, QList<AbstractDataPluginItem *> > m_itemSet;
    QHash<QString, AbstractDataPluginItem*> m_downloadingItems;
    QList<AbstractDataPluginItem*> m_displayedItems;
    QHash<QString, QVariant> m_itemSettings;
    QStringList m_favoriteItems;
    bool m_favoriteItemsOnly;

    CacheStoragePolicy m_storagePolicy;
    HttpDownloadManager m_downloadManager;
    FavoritesModel* m_favoritesModel;
    QMetaObject m_metaObject;
    bool m_hasMetaObject;
    bool m_needsSorting;
};

class FavoritesModel : public QAbstractListModel
{
public:
    AbstractDataPluginModelPrivate* d;

    explicit FavoritesModel( AbstractDataPluginModelPrivate* d, QObject* parent = 0 );

    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;

    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;

    void reset();

#if QT_VERSION >= 0x050000
    QHash<int, QByteArray> roleNames() const;

private:
    QHash<int, QByteArray> m_roleNames;
#endif
};

AbstractDataPluginModelPrivate::AbstractDataPluginModelPrivate( const QString& name,
                                                                const MarbleModel *marbleModel,
                                AbstractDataPluginModel * parent )
    : m_parent( parent ),
      m_name( name ),
      m_marbleModel( marbleModel ),
      m_currentPlanetId( marbleModel->planetId() ),
      m_itemSettings(),
      m_favoriteItemsOnly( false ),
      m_storagePolicy( MarbleDirs::localPath() + "/cache/" + m_name + '/' ),
      m_downloadManager( &m_storagePolicy ),
      m_favoritesModel( 0 ),
      m_hasMetaObject( false ),
      m_needsSorting( false )
{
}

AbstractDataPluginModelPrivate::~AbstractDataPluginModelPrivate() {
    foreach ( const TileId &tileId, m_itemSet.keys() ) {
        QList<AbstractDataPluginItem *>::iterator lIt = m_itemSet[tileId].begin();
        QList<AbstractDataPluginItem *>::iterator const lItEnd = m_itemSet[tileId].end();
        for (; lIt != lItEnd; ++lIt ) {
            (*lIt)->deleteLater();
        }
    }

    QHash<QString,AbstractDataPluginItem*>::iterator hIt = m_downloadingItems.begin();
    QHash<QString,AbstractDataPluginItem*>::iterator const hItEnd = m_downloadingItems.end();
    for (; hIt != hItEnd; ++hIt ) {
        (*hIt)->deleteLater();
    }

    m_storagePolicy.clearCache();
}

void AbstractDataPluginModelPrivate::updateFavoriteItems()
{
    if ( m_favoriteItemsOnly ) {
        foreach( const QString &id, m_favoriteItems ) {
            if ( !m_parent->findItem( id ) ) {
                m_parent->getItem( id );
            }
        }
    }
}

void AbstractDataPluginModel::themeChanged()
{
    if ( d->m_currentPlanetId != d->m_marbleModel->planetId() ) {
        clear();
        d->m_currentPlanetId = d->m_marbleModel->planetId();
    }
}

static bool lessThanByPointer( const AbstractDataPluginItem *item1,
                               const AbstractDataPluginItem *item2 )
{
    if( item1 && item2 ) {
        // Compare by sticky and favorite status (sticky first, then favorites), last by operator<
        bool const sticky1 = item1->isSticky();
        bool const favorite1 = item1->isFavorite();
        if ( sticky1 != item2->isSticky() ) {
            return sticky1;
        } else if ( favorite1 != item2->isFavorite() ) {
            return favorite1;
        } else {
            return item1->operator<( item2 );
        }
    }
    else {
        return false;
    }
}

FavoritesModel::FavoritesModel( AbstractDataPluginModelPrivate *_d, QObject* parent ) :
    QAbstractListModel( parent ), d(_d)
{
    QHash<int,QByteArray> roles;
    int const size = d->m_hasMetaObject ? d->m_metaObject.propertyCount() : 0;
    for ( int i=0; i<size; ++i ) {
        QMetaProperty property = d->m_metaObject.property( i );
        roles[Qt::UserRole+i] = property.name();
    }
    roles[Qt::DisplayRole] = "display";
    roles[Qt::DecorationRole] = "decoration";
#if QT_VERSION < 0x050000
    setRoleNames( roles );
#else
    m_roleNames = roles;
#endif
}

int FavoritesModel::rowCount ( const QModelIndex &parent ) const
{
    if ( parent.isValid() ) {
        return 0;
    }

    int count = 0;
    foreach ( const TileId &tileId, d->m_itemSet.keys() ) {
        foreach( AbstractDataPluginItem* item, d->m_itemSet[tileId] ) {
            if ( item->initialized() && item->isFavorite() ) {
                ++count;
            }
        }
    }

    return count;
}

QVariant FavoritesModel::data( const QModelIndex &index, int role ) const
{
    int const row = index.row();
    if ( row >= 0 && row < rowCount() ) {
        int count = 0;
        foreach ( const TileId &tileId, d->m_itemSet.keys() ) {
            foreach( AbstractDataPluginItem* item, d->m_itemSet[tileId] ) {
                if ( item->initialized() && item->isFavorite() ) {
                    if ( count == row ) {
                        QString const roleName = roleNames().value( role );
                        return item->property( roleName.toAscii() );
                    }
                    ++count;
                }
            }
        }
    }

    return QVariant();
}

void FavoritesModel::reset()
{
    beginResetModel();
    endResetModel();
}

#if QT_VERSION >= 0x050000
QHash<int, QByteArray> FavoritesModel::roleNames() const
{
    return m_roleNames;
}
#endif

AbstractDataPluginModel::AbstractDataPluginModel( const QString &name, const MarbleModel *marbleModel, QObject *parent )
    : QObject(  parent ),
      d( new AbstractDataPluginModelPrivate( name, marbleModel, this ) )
{
    Q_ASSERT( marbleModel != 0 );

    // Initializing file and download System
    connect( &d->m_downloadManager, SIGNAL(downloadComplete(QString,QString)),
             this ,                 SLOT(processFinishedJob(QString,QString)) );

    connect( marbleModel, SIGNAL(themeChanged(QString)),
             this, SLOT(themeChanged()) );
}

AbstractDataPluginModel::~AbstractDataPluginModel()
{
    delete d;
}

const MarbleModel *AbstractDataPluginModel::marbleModel() const
{
    return d->m_marbleModel;
}

QList<AbstractDataPluginItem*> AbstractDataPluginModel::items( const ViewportParams *viewport,
                                                               qint32 number, GeoPainter *painter )
{
    const int zoomLevel = qLn( viewport->radius() * 4.0 / 512 ) / qLn( 2.0 );

    GeoDataLatLonAltBox currentBox = viewport->viewLatLonAltBox();

    d->m_displayedItems.clear();

    if ( d->m_needsSorting ) {
        // Both the candidates list and the list of all items need to be sorted
        foreach ( const TileId &tileId, d->m_itemSet.keys() ) {
            Q_ASSERT( !d->m_itemSet[tileId].contains( 0 ) && "Null item in m_itemSet. Please report a bug to marble-devel@kde.org" );
            qSort( d->m_itemSet[tileId].begin(), d->m_itemSet[tileId].end(), lessThanByPointer );
        }
        d->m_needsSorting =  false;
    }

    const TileId topLeft = TileId::fromCoordinates( GeoDataCoordinates( currentBox.west(), currentBox.north() ), zoomLevel );
    const TileId bottomRight = TileId::fromCoordinates( GeoDataCoordinates( currentBox.east(), currentBox.south() ), zoomLevel );
    for ( int x = topLeft.x(); x <= bottomRight.x() + 1; ++x ) {
        for ( int y = topLeft.y(); y <= bottomRight.y() + 1; ++y ) {
            const TileId tileId = TileId( 0, zoomLevel, x, y );
            const int numTiles = 1 << zoomLevel;
            const qreal west = -180 + ( tileId.x()     ) * 360.0 / numTiles;
            const qreal east = -180 + ( tileId.x() + 1 ) * 360.0 / numTiles;
            const qreal north = 90 - ( tileId.y()     ) * 180.0 / numTiles;
            const qreal south = 90 - ( tileId.y() + 1 ) * 180.0 / numTiles;

            if ( !d->m_itemSet.contains( tileId ) ) {
                d->m_itemSet.insert( tileId, QList<AbstractDataPluginItem *>() );

                const QString name = QString( "%1_%2_%3_%4" ).arg( descriptionPrefix ).arg( tileId.zoomLevel() ).arg( tileId.y() ).arg( tileId.x() );
                if ( d->m_storagePolicy.fileExists( name ) ) {
                    parseFile( d->m_storagePolicy.data( name ), tileId );
                }
                else {
                    const int numTiles = 1 << zoomLevel;
                    const qreal west = -180 + ( tileId.x()     ) * 360.0 / numTiles;
                    const qreal east = -180 + ( tileId.x() + 1 ) * 360.0 / numTiles;
                    const qreal north = 90 - ( tileId.y()     ) * 180.0 / numTiles;
                    const qreal south = 90 - ( tileId.y() + 1 ) * 180.0 / numTiles;
                    GeoDataLatLonBox bbox( north, south, east, west, GeoDataCoordinates::Degree );
                    getAdditionalItems( bbox, number, tileId );

                    continue;
                }
            }

            GeoDataLinearRing ring( Tessellate | RespectLatitudeCircle );
            ring.append( GeoDataCoordinates( west, north, 0, GeoDataCoordinates::Degree ) );
            ring.append( GeoDataCoordinates( west, south, 0, GeoDataCoordinates::Degree ) );
            ring.append( GeoDataCoordinates( east, south, 0, GeoDataCoordinates::Degree ) );
            ring.append( GeoDataCoordinates( east, north, 0, GeoDataCoordinates::Degree ) );

            painter->drawPolygon( ring );

            painter->drawText( GeoDataCoordinates( west, south, 0, GeoDataCoordinates::Degree ), QString( "%1, %2, %3").arg( zoomLevel ).arg( x ).arg( y ) );

            QList<AbstractDataPluginItem*>::const_iterator i = d->m_itemSet[tileId].constBegin();
            QList<AbstractDataPluginItem*>::const_iterator end = d->m_itemSet[tileId].constEnd();

            // Items that are already shown have the highest priority
            for (; i != end; ++i ) {
                // Only show items that are initialized
                if( !(*i)->initialized() ) {
                    continue;
                }

                // Hide non-favorite items if necessary
                if( d->m_favoriteItemsOnly && !(*i)->isFavorite() ) {
                    continue;
                }

                (*i)->setProjection( viewport );
                if( (*i)->positions().isEmpty() ) {
                    continue;
                }

                if ( !d->m_displayedItems.contains( *i ) ) {
                    bool collides = false;
                    for ( int j = 0; !collides && j < d->m_displayedItems.length(); ++j ) {
                        foreach( const QRectF &rect, d->m_displayedItems[j]->boundingRects() ) {
                            foreach( const QRectF &itemRect, (*i)->boundingRects() ) {
                                if ( rect.intersects( itemRect ) )
                                    collides = true;
                            }
                        }
                    }

                    if ( !collides ) {
                        d->m_displayedItems.append( *i );
                        (*i)->setSettings( d->m_itemSettings );
                    }
                }
                // TODO: Do we have to cleanup at some point? The list of all items keeps growing
            }
        }
    }

    return d->m_displayedItems;
}

QList<AbstractDataPluginItem *> AbstractDataPluginModel::whichItemAt( const QPoint& curpos )
{
    QList<AbstractDataPluginItem *> itemsAt;
    
    foreach( AbstractDataPluginItem* item, d->m_displayedItems ) {
        if( item && item->contains( QPointF( curpos ) ) )
            itemsAt.append( item );
    }
    
    return itemsAt;
}

void AbstractDataPluginModel::downloadItem( const QUrl& url,
                                                const QString& type,
                                                AbstractDataPluginItem *item )
{
    if( !item ) {
        return;
    }

    QString id = d->generateFilename( item->id(), type );

    d->m_downloadManager.addJob( url, id, id, DownloadBrowse );
    d->m_downloadingItems.insert( id, item );
}

void AbstractDataPluginModel::downloadDescriptionFile( const QUrl& url, const TileId& tileId )
{
    if( !url.isEmpty() ) {
        const QString id = QString( "%1:%2:%3" ).arg( tileId.zoomLevel() ).arg( tileId.y() ).arg( tileId.x() );
        const QString name = QString( "%1_%2_%3_%4" ).arg( descriptionPrefix ).arg( tileId.zoomLevel() ).arg( tileId.y() ).arg( tileId.x() );
        
        d->m_downloadManager.addJob( url, name, id, DownloadBrowse );
    }
}

void AbstractDataPluginModel::addItemToList( AbstractDataPluginItem *item, const TileId &tileId )
{
    addItemsToList( QList<AbstractDataPluginItem*>() << item, tileId );
}

void AbstractDataPluginModel::addItemsToList( const QList<AbstractDataPluginItem *> &items, const TileId &tileId )
{
    bool needsUpdate = false;
    bool favoriteChanged = false;
    foreach( AbstractDataPluginItem *item, items ) {
        if( !item ) {
            continue;
        }

        // If the item is already in our list, don't add it.
        if ( d->m_itemSet[tileId].contains( item ) ) {
            continue;
        }

        if( itemExists( item->id() ) ) {
            item->deleteLater();
            continue;
        }

        mDebug() << "New item" << item->id() << tileId;

        // This find the right position in the sorted to insert the new item
        QList<AbstractDataPluginItem*>::iterator i = qLowerBound( d->m_itemSet[tileId].begin(),
                                                                  d->m_itemSet[tileId].end(),
                                                                  item,
                                                                  lessThanByPointer );
        // Insert the item on the right position in the list
        d->m_itemSet[tileId].insert( i, item );

        connect( item, SIGNAL(stickyChanged()), this, SLOT(scheduleItemSort()) );
        connect( item, SIGNAL(destroyed(QObject*)), this, SLOT(removeItem(QObject*)) );
        connect( item, SIGNAL(updated()), this, SIGNAL(itemsUpdated()) );
        connect( item, SIGNAL(favoriteChanged(QString,bool)), this,
                 SLOT(favoriteItemChanged(QString,bool)) );

        if ( !needsUpdate && item->initialized() ) {
            needsUpdate = true;
        }

        if ( !favoriteChanged && item->initialized() && item->isFavorite() ) {
            favoriteChanged = true;
        }
    }

    if ( favoriteChanged && d->m_favoritesModel ) {
        d->m_favoritesModel->reset();
    }

    if ( needsUpdate ) {
        emit itemsUpdated();
    }
}

void AbstractDataPluginModel::getItem( const QString & )
{
    qWarning() << "Retrieving items by identifier is not implemented by this plugin";
}

void AbstractDataPluginModel::setFavoriteItems( const QStringList& list )
{
    if ( d->m_favoriteItems != list) {
        d->m_favoriteItems = list;
        d->updateFavoriteItems();
        if ( d->m_favoritesModel ) {
            d->m_favoritesModel->reset();
        }
        emit favoriteItemsChanged( d->m_favoriteItems );
    }
}

QStringList AbstractDataPluginModel::favoriteItems() const
{
    return d->m_favoriteItems;
}

void AbstractDataPluginModel::setFavoriteItemsOnly( bool favoriteOnly )
{
    if ( isFavoriteItemsOnly() != favoriteOnly ) {
        d->m_favoriteItemsOnly = favoriteOnly;
        d->updateFavoriteItems();
        emit favoriteItemsOnlyChanged();
    }
}

bool AbstractDataPluginModel::isFavoriteItemsOnly() const
{
    return d->m_favoriteItemsOnly;
}

QObject *AbstractDataPluginModel::favoritesModel()
{
    if ( !d->m_favoritesModel ) {
        d->m_favoritesModel = new FavoritesModel( d, this );
        d->updateFavoriteItems();
    }

    return d->m_favoritesModel;
}

void AbstractDataPluginModel::favoriteItemChanged( const QString& id, bool isFavorite )
{
    QStringList favorites = d->m_favoriteItems;

    if ( isFavorite ) {
        if ( !favorites.contains(id) )
            favorites.append( id );
    } else {
        favorites.removeOne( id );
    }

    setFavoriteItems( favorites );
    scheduleItemSort();
}

void AbstractDataPluginModel::scheduleItemSort()
{
    d->m_needsSorting = true;
}

QString AbstractDataPluginModelPrivate::generateFilename( const QString& id, const QString& type ) const
{
    QString name;
    name += id;
    name += fileIdSeparator;
    name += type;
    
    return name;
}

QString AbstractDataPluginModelPrivate::generateFilepath( const QString& id, const QString& type ) const
{
    return MarbleDirs::localPath() + "/cache/" + m_name + '/' + generateFilename( id, type );
}

AbstractDataPluginItem *AbstractDataPluginModel::findItem( const QString& id ) const
{
    foreach ( const TileId &tileId, d->m_itemSet.keys() ) {
        foreach ( AbstractDataPluginItem *item, d->m_itemSet[tileId] ) {
            if( item->id() == id ) {
                return item;
            }
        }
    }
    
    return 0;
}

bool AbstractDataPluginModel::itemExists( const QString& id ) const
{
    return findItem( id );
}

void AbstractDataPluginModel::setItemSettings( QHash<QString,QVariant> itemSettings )
{
    d->m_itemSettings = itemSettings;
}

void AbstractDataPluginModel::processFinishedJob( const QString& relativeUrlString,
                                                  const QString& id )
{
    if( relativeUrlString.startsWith( descriptionPrefix ) ) {
        const QStringList tileIdString = id.split(':');
        Q_ASSERT( tileIdString.size() == 3 );
        const int zoomLevel = tileIdString[0].toInt();
        const int y = tileIdString[1].toInt();
        const int x = tileIdString[2].toInt();
        parseFile( d->m_storagePolicy.data( relativeUrlString ), TileId( 0, zoomLevel, x, y ) );
    }
    else {
        // The downloaded file contains item data.
        
        // Splitting the id in itemId and fileType
        QStringList fileInformation = relativeUrlString.split( fileIdSeparator );
        
        if( fileInformation.size() < 2) {
            mDebug() << "Strange file information " << relativeUrlString;
            return;
        }
        QString itemId = fileInformation.at( 0 );
        fileInformation.removeAt( 0 );
        QString fileType = fileInformation.join( QString( fileIdSeparator ) );
        
        // Searching for the right item in m_downloadingItems
        QHash<QString, AbstractDataPluginItem *>::iterator i = d->m_downloadingItems.find( relativeUrlString );
        if( i != d->m_downloadingItems.end() ) {
            if( itemId != (*i)->id() ) {
                return;
            }
            
            (*i)->addDownloadedFile( d->generateFilepath( itemId, fileType ), fileType );

            d->m_downloadingItems.erase( i );
        }
    }
}

void AbstractDataPluginModel::removeItem( QObject *item )
{
    foreach ( const TileId &tileId, d->m_itemSet.keys() ) {
        d->m_itemSet[tileId].removeAll( (AbstractDataPluginItem *) item );
    }

    QHash<QString, AbstractDataPluginItem *>::iterator i;
    for( i = d->m_downloadingItems.begin(); i != d->m_downloadingItems.end(); ++i ) {
        if( *i == item ) {
            i = d->m_downloadingItems.erase( i );
        }
    }
}

void AbstractDataPluginModel::clear()
{
    d->m_displayedItems.clear();
    foreach ( const TileId &tileId, d->m_itemSet.keys() ) {
        QList<AbstractDataPluginItem*>::iterator iter = d->m_itemSet[tileId].begin();
        QList<AbstractDataPluginItem*>::iterator const end = d->m_itemSet[tileId].end();
        for (; iter != end; ++iter ) {
            (*iter)->deleteLater();
        }
        d->m_itemSet[tileId].clear();
    }
    d->m_itemSet.clear();
    emit itemsUpdated();
}

void AbstractDataPluginModel::registerItemProperties( const QMetaObject &item )
{
    d->m_metaObject = item;
    d->m_hasMetaObject = true;
}

} // namespace Marble

#include "AbstractDataPluginModel.moc"
