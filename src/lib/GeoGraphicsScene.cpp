//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011      Konstantin Oblaukhov <oblaukhov.konstantin@gmail.com>
//

#include "GeoGraphicsScene.h"

#include "GeoDataDocument.h"
#include "GeoDataFeature.h"
#include "GeoDataFolder.h"
#include "GeoDataGroundOverlay.h"
#include "GeoDataLatLonBox.h"
#include "GeoDataLineStyle.h"
#include "GeoDataMultiTrack.h"
#include "GeoDataObject.h"
#include "GeoDataPhotoOverlay.h"
#include "GeoDataPlacemark.h"
#include "GeoDataPolygon.h"
#include "GeoDataPolyStyle.h"
#include "GeoDataScreenOverlay.h"
#include "GeoDataStyle.h"
#include "GeoDataStyleMap.h"
#include "GeoDataTrack.h"
#include "GeoDataTypes.h"
#include "GeoDataTreeModel.h"
#include "GeoGraphicsItem.h"
#include "GeoImageGraphicsItem.h"
#include "GeoLineStringGraphicsItem.h"
#include "GeoPhotoGraphicsItem.h"
#include "GeoPolygonGraphicsItem.h"
#include "GeoTrackGraphicsItem.h"
#include "MarblePlacemarkModel.h"
#include "ScreenOverlayGraphicsItem.h"
#include "TileId.h"
#include "TileCoordsPyramid.h"
#include "MarbleDebug.h"

#include <QtCore/QMap>

namespace Marble
{

bool zValueLessThan( GeoGraphicsItem* i1, GeoGraphicsItem* i2 )
{
    return i1->zValue() < i2->zValue();
}

class GeoGraphicsScenePrivate
{
public:
    GeoGraphicsScenePrivate( const QAbstractItemModel *model );

    static int maximumZoomLevel();

    void createGraphicsItems( const GeoDataObject *object );
    void createGraphicsItemFromGeometry( const GeoDataGeometry *object, const GeoDataPlacemark *placemark );
    void createGraphicsItemFromOverlay( const GeoDataOverlay *overlay );
    void removeGraphicsItems( const GeoDataFeature *feature );

    void removeItem( const GeoDataFeature* feature );
    void addItem( GeoGraphicsItem* item );
    void addItems(const TileId &tileId, QList<GeoGraphicsItem*> &result, int maxZoomLevel ) const;

    const QAbstractItemModel *const m_model;
    QMap<TileId, QList<GeoGraphicsItem*> > m_geoItems;
    QMultiHash<const GeoDataFeature*, TileId> m_features;
    QList<ScreenOverlayGraphicsItem*> m_screenItems;

private:
    static void initializeDefaultValues();

    static int s_defaultZValues[GeoDataFeature::LastIndex];
    static int s_defaultMinZoomLevels[GeoDataFeature::LastIndex];
    static bool s_defaultValuesInitialized;
    static int s_maximumZoomLevel;
    static const int s_defaultZValue;
};

int GeoGraphicsScenePrivate::s_defaultZValues[GeoDataFeature::LastIndex];
int GeoGraphicsScenePrivate::s_defaultMinZoomLevels[GeoDataFeature::LastIndex];
bool GeoGraphicsScenePrivate::s_defaultValuesInitialized = false;
int GeoGraphicsScenePrivate::s_maximumZoomLevel = 0;
const int GeoGraphicsScenePrivate::s_defaultZValue = 50;

GeoGraphicsScenePrivate::GeoGraphicsScenePrivate( const QAbstractItemModel *model ) :
    m_model( model )
{
    initializeDefaultValues();
}

int GeoGraphicsScenePrivate::maximumZoomLevel()
{
    return s_maximumZoomLevel;
}

void GeoGraphicsScenePrivate::initializeDefaultValues()
{
    if ( s_defaultValuesInitialized )
        return;

    for ( int i = 0; i < GeoDataFeature::LastIndex; i++ )
        s_defaultZValues[i] = s_defaultZValue;

    for ( int i = 0; i < GeoDataFeature::LastIndex; i++ )
        s_defaultMinZoomLevels[i] = 15;

    s_defaultZValues[GeoDataFeature::None]                = 0;

    for ( int i = GeoDataFeature::LanduseAllotments; i <= GeoDataFeature::LanduseRetail; i++ )
        s_defaultZValues[(GeoDataFeature::GeoDataVisualCategory)i] = s_defaultZValue - 16;

    s_defaultZValues[GeoDataFeature::NaturalWater]        = s_defaultZValue - 16;
    s_defaultZValues[GeoDataFeature::NaturalWood]         = s_defaultZValue - 15;

    //Landuse

    s_defaultZValues[GeoDataFeature::LeisurePark]         = s_defaultZValue - 14;

    s_defaultZValues[GeoDataFeature::TransportParking]    = s_defaultZValue - 13;

    s_defaultZValues[GeoDataFeature::HighwayTertiaryLink] = s_defaultZValue - 12;
    s_defaultZValues[GeoDataFeature::HighwaySecondaryLink]= s_defaultZValue - 12;
    s_defaultZValues[GeoDataFeature::HighwayPrimaryLink]  = s_defaultZValue - 12;
    s_defaultZValues[GeoDataFeature::HighwayTrunkLink]    = s_defaultZValue - 12;
    s_defaultZValues[GeoDataFeature::HighwayMotorwayLink] = s_defaultZValue - 12;

    s_defaultZValues[GeoDataFeature::HighwayUnknown]      = s_defaultZValue - 11;
    s_defaultZValues[GeoDataFeature::HighwayPath]         = s_defaultZValue - 10;
    s_defaultZValues[GeoDataFeature::HighwayTrack]        = s_defaultZValue - 9;
    s_defaultZValues[GeoDataFeature::HighwaySteps]        = s_defaultZValue - 8;
    s_defaultZValues[GeoDataFeature::HighwayPedestrian]   = s_defaultZValue - 8;
    s_defaultZValues[GeoDataFeature::HighwayService]      = s_defaultZValue - 7;
    s_defaultZValues[GeoDataFeature::HighwayRoad]         = s_defaultZValue - 6;
    s_defaultZValues[GeoDataFeature::HighwayTertiary]     = s_defaultZValue - 5;
    s_defaultZValues[GeoDataFeature::HighwaySecondary]    = s_defaultZValue - 4;
    s_defaultZValues[GeoDataFeature::HighwayPrimary]      = s_defaultZValue - 3;
    s_defaultZValues[GeoDataFeature::HighwayTrunk]        = s_defaultZValue - 2;
    s_defaultZValues[GeoDataFeature::HighwayMotorway]     = s_defaultZValue - 1;
    s_defaultZValues[GeoDataFeature::RailwayRail]         = s_defaultZValue - 1;

    s_defaultMinZoomLevels[GeoDataFeature::Default]             = 1;
    s_defaultMinZoomLevels[GeoDataFeature::NaturalWater]        = 8;
    s_defaultMinZoomLevels[GeoDataFeature::NaturalWood]         = 8;
    s_defaultMinZoomLevels[GeoDataFeature::Building]            = 15;

        // OpenStreetMap highways
    s_defaultMinZoomLevels[GeoDataFeature::HighwaySteps]        = 15;
    s_defaultMinZoomLevels[GeoDataFeature::HighwayUnknown]      = 13;
    s_defaultMinZoomLevels[GeoDataFeature::HighwayPath]         = 13;
    s_defaultMinZoomLevels[GeoDataFeature::HighwayTrack]        = 13;
    s_defaultMinZoomLevels[GeoDataFeature::HighwayPedestrian]   = 14;
    s_defaultMinZoomLevels[GeoDataFeature::HighwayService]      = 14;
    s_defaultMinZoomLevels[GeoDataFeature::HighwayRoad]         = 13;
    s_defaultMinZoomLevels[GeoDataFeature::HighwayTertiaryLink] = 10;
    s_defaultMinZoomLevels[GeoDataFeature::HighwayTertiary]     = 10;
    s_defaultMinZoomLevels[GeoDataFeature::HighwaySecondaryLink]= 10;
    s_defaultMinZoomLevels[GeoDataFeature::HighwaySecondary]    = 9;
    s_defaultMinZoomLevels[GeoDataFeature::HighwayPrimaryLink]  = 10;
    s_defaultMinZoomLevels[GeoDataFeature::HighwayPrimary]      = 8;
    s_defaultMinZoomLevels[GeoDataFeature::HighwayTrunkLink]    = 10;
    s_defaultMinZoomLevels[GeoDataFeature::HighwayTrunk]        = 7;
    s_defaultMinZoomLevels[GeoDataFeature::HighwayMotorwayLink] = 10;
    s_defaultMinZoomLevels[GeoDataFeature::HighwayMotorway]     = 6;

    //FIXME: Bad, better to expand this
    for(int i = GeoDataFeature::AccomodationCamping; i <= GeoDataFeature::ReligionSikh; i++)
        s_defaultMinZoomLevels[i] = 15;

    s_defaultMinZoomLevels[GeoDataFeature::LeisurePark]         = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseAllotments]   = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseBasin]        = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseCemetery]     = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseCommercial]   = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseConstruction] = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseFarmland]     = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseFarmyard]     = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseGarages]      = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseGrass]        = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseIndustrial]   = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseLandfill]     = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseMeadow]       = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseMilitary]     = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseQuarry]       = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseRailway]      = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseReservoir]    = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseResidential]  = 11;
    s_defaultMinZoomLevels[GeoDataFeature::LanduseRetail]       = 11;

    s_defaultMinZoomLevels[GeoDataFeature::RailwayRail]         = 6;
    s_defaultMinZoomLevels[GeoDataFeature::RailwayTram]         = 13;
    s_defaultMinZoomLevels[GeoDataFeature::RailwayLightRail]    = 12;
    s_defaultMinZoomLevels[GeoDataFeature::RailwayAbandoned]    = 10;
    s_defaultMinZoomLevels[GeoDataFeature::RailwaySubway]       = 13;
    s_defaultMinZoomLevels[GeoDataFeature::RailwayPreserved]    = 13;
    s_defaultMinZoomLevels[GeoDataFeature::RailwayMiniature]    = 13;
    s_defaultMinZoomLevels[GeoDataFeature::RailwayConstruction] = 10;
    s_defaultMinZoomLevels[GeoDataFeature::RailwayMonorail]     = 12;
    s_defaultMinZoomLevels[GeoDataFeature::RailwayFunicular]    = 13;

    s_defaultMinZoomLevels[GeoDataFeature::Satellite]           = 0;

    for ( int i = 0; i < GeoDataFeature::LastIndex; ++i ) {
        s_maximumZoomLevel = qMax( s_maximumZoomLevel, s_defaultMinZoomLevels[i] );
    }

    s_defaultValuesInitialized = true;
}

void GeoGraphicsScenePrivate::createGraphicsItems( const GeoDataObject *object )
{
    if ( const GeoDataPlacemark *placemark = dynamic_cast<const GeoDataPlacemark*>( object ) ) {
        createGraphicsItemFromGeometry( placemark->geometry(), placemark );
    } else if ( const GeoDataOverlay* overlay = dynamic_cast<const GeoDataOverlay*>( object ) ) {
        createGraphicsItemFromOverlay( overlay );
    }

    // parse all child objects of the container
    if ( const GeoDataContainer *container = dynamic_cast<const GeoDataContainer*>( object ) ) {
        int rowCount = container->size();
        for ( int row = 0; row < rowCount; ++row ) {
            createGraphicsItems( container->child( row ) );
        }
    }
}

void GeoGraphicsScenePrivate::createGraphicsItemFromGeometry( const GeoDataGeometry* object, const GeoDataPlacemark *placemark )
{
    GeoGraphicsItem *item = 0;
    if ( object->nodeType() == GeoDataTypes::GeoDataLineStringType ) {
        const GeoDataLineString* line = static_cast<const GeoDataLineString*>( object );
        item = new GeoLineStringGraphicsItem( placemark, line );
    }
    else if ( object->nodeType() == GeoDataTypes::GeoDataLinearRingType ) {
        const GeoDataLinearRing *ring = static_cast<const GeoDataLinearRing*>( object );
        item = new GeoPolygonGraphicsItem( placemark, ring );
    }
    else if ( object->nodeType() == GeoDataTypes::GeoDataPolygonType ) {
        const GeoDataPolygon *poly = static_cast<const GeoDataPolygon*>( object );
        item = new GeoPolygonGraphicsItem( placemark, poly );
    }
    else if ( object->nodeType() == GeoDataTypes::GeoDataMultiGeometryType ) {
        const GeoDataMultiGeometry *multigeo = static_cast<const GeoDataMultiGeometry*>( object );
        int rowCount = multigeo->size();
        for ( int row = 0; row < rowCount; ++row ) {
            createGraphicsItemFromGeometry( multigeo->child( row ), placemark );
        }
    }
    else if ( object->nodeType() == GeoDataTypes::GeoDataMultiTrackType  ) {
        const GeoDataMultiTrack *multitrack = static_cast<const GeoDataMultiTrack*>( object );
        int rowCount = multitrack->size();
        for ( int row = 0; row < rowCount; ++row ) {
            createGraphicsItemFromGeometry( multitrack->child( row ), placemark );
        }
    }
    else if ( object->nodeType() == GeoDataTypes::GeoDataTrackType ) {
        const GeoDataTrack *track = static_cast<const GeoDataTrack*>( object );
        item = new GeoTrackGraphicsItem( placemark, track );
    }

    if ( !item )
        return;

    item->setStyle( placemark->style() );
    item->setVisible( placemark->isGloballyVisible() );
    item->setZValue( s_defaultZValues[placemark->visualCategory()] );
    item->setMinZoomLevel( s_defaultMinZoomLevels[placemark->visualCategory()] );
    addItem( item );
}

void GeoGraphicsScenePrivate::createGraphicsItemFromOverlay( const GeoDataOverlay *overlay )
{
    GeoGraphicsItem* item = 0;
    if ( overlay->nodeType() == GeoDataTypes::GeoDataGroundOverlayType ) {
        GeoDataGroundOverlay const * groundOverlay = static_cast<GeoDataGroundOverlay const *>( overlay );
        GeoImageGraphicsItem *imageItem = new GeoImageGraphicsItem( overlay );
        imageItem->setImageFile( groundOverlay->absoluteIconFile() );
        imageItem->setLatLonBox( groundOverlay->latLonBox() );
        item = imageItem;
    } else if ( overlay->nodeType() == GeoDataTypes::GeoDataPhotoOverlayType ) {
        GeoDataPhotoOverlay const * photoOverlay = static_cast<GeoDataPhotoOverlay const *>( overlay );
        GeoPhotoGraphicsItem *photoItem = new GeoPhotoGraphicsItem( overlay );
        photoItem->setPhotoFile( photoOverlay->absoluteIconFile() );
        photoItem->setPoint( photoOverlay->point() );
        item = photoItem;
    } else if ( overlay->nodeType() == GeoDataTypes::GeoDataScreenOverlayType ) {
        GeoDataScreenOverlay const * screenOverlay = static_cast<GeoDataScreenOverlay const *>( overlay );
        ScreenOverlayGraphicsItem *screenItem = new ScreenOverlayGraphicsItem ( screenOverlay );
        m_screenItems.push_back( screenItem );
    }

    if ( item ) {
        item->setStyle( overlay->style() );
        item->setVisible( overlay->isGloballyVisible() );
        addItem( item );
    }
}

void GeoGraphicsScenePrivate::removeGraphicsItems( const GeoDataFeature *feature )
{

    if( feature->nodeType() == GeoDataTypes::GeoDataPlacemarkType ) {
        removeItem( feature );
    }
    else if( feature->nodeType() == GeoDataTypes::GeoDataFolderType
             || feature->nodeType() == GeoDataTypes::GeoDataDocumentType ) {
        const GeoDataContainer *container = static_cast<const GeoDataContainer*>( feature );
        foreach( const GeoDataFeature *child, container->featureList() ) {
            removeGraphicsItems( child );
        }
    }
    else if( feature->nodeType() == GeoDataTypes::GeoDataScreenOverlayType ) {
        foreach( ScreenOverlayGraphicsItem  *item, m_screenItems ) {
            if( item->screenOverlay() == feature ) {
                m_screenItems.removeAll( item );
            }
        }
    }
}

GeoGraphicsScene::GeoGraphicsScene( const QAbstractItemModel *model, QObject *parent ) :
    QObject( parent ),
    d( new GeoGraphicsScenePrivate( model ) )
{
    const GeoDataObject *object = static_cast<GeoDataObject*>( d->m_model->index( 0, 0, QModelIndex() ).internalPointer() );
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

GeoGraphicsScene::~GeoGraphicsScene()
{
    delete d;
}

int GeoGraphicsScene::maximumZoomLevel() const
{
    return GeoGraphicsScenePrivate::maximumZoomLevel();
}

QList< GeoGraphicsItem* > GeoGraphicsScene::items( const GeoDataLatLonBox &box, int zoomLevel ) const
{
    if ( box.west() > box.east() ) {
        // Handle boxes crossing the IDL by splitting it into two separate boxes
        GeoDataLatLonBox left;
        left.setWest( -M_PI );
        left.setEast( box.east() );
        left.setNorth( box.north() );
        left.setSouth( box.south() );

        GeoDataLatLonBox right;
        right.setWest( box.west() );
        right.setEast( M_PI );
        right.setNorth( box.north() );
        right.setSouth( box.south() );

        QList< GeoGraphicsItem* > allItems = items( left, zoomLevel );
        foreach( GeoGraphicsItem* item, items( right, zoomLevel ) ) {
            if ( !allItems.contains( item ) ) {
                allItems << item;
            }
        }
        return allItems;
    }

    QList< GeoGraphicsItem* > result;
    QRect rect;
    qreal north, south, east, west;
    box.boundaries( north, south, east, west );
    TileId key;

    key = TileId::fromCoordinates( GeoDataCoordinates(west, north, 0), zoomLevel );
    rect.setLeft( key.x() );
    rect.setTop( key.y() );

    key = TileId::fromCoordinates( GeoDataCoordinates(east, south, 0), zoomLevel );
    rect.setRight( key.x() );
    rect.setBottom( key.y() );
    
    TileCoordsPyramid pyramid( 0, zoomLevel );
    pyramid.setBottomLevelCoords( rect );

    for ( int level = pyramid.topLevel(); level <= pyramid.bottomLevel(); ++level ) {
        QRect const coords = pyramid.coords( level );
        int x1, y1, x2, y2;
        coords.getCoords( &x1, &y1, &x2, &y2 );
        for ( int x = x1; x <= x2; ++x ) {
            for ( int y = y1; y <= y2; ++y ) {
                d->addItems( TileId ( 0, level, x, y ), result, zoomLevel );
            }
        }
    }
    return result;
}

QList<ScreenOverlayGraphicsItem *> GeoGraphicsScene::screenItems() const
{
    return d->m_screenItems;
}

void GeoGraphicsScenePrivate::removeItem( const GeoDataFeature* feature )
{
    QList<TileId> keys = m_features.values( feature );
    foreach( TileId key, keys ) {
        QList< GeoGraphicsItem* >& tileList = m_geoItems[key];
        foreach( GeoGraphicsItem* item, tileList ) {
            if( item->feature() == feature ) {
                m_features.remove( feature );
                tileList.removeAll( item );
                break;
            }
        }
    }
}

void GeoGraphicsScene::clear()
{
    d->m_geoItems.clear();
}

void GeoGraphicsScenePrivate::addItem( GeoGraphicsItem* item )
{
    // Select zoom level so that the object fit in single tile
    int zoomLevel;
    qreal north, south, east, west;
    item->latLonAltBox().boundaries( north, south, east, west );
    for(zoomLevel = item->minZoomLevel(); zoomLevel >= 0; zoomLevel--)
    {
        if( TileId::fromCoordinates( GeoDataCoordinates(west, north, 0), zoomLevel ) ==
            TileId::fromCoordinates( GeoDataCoordinates(east, south, 0), zoomLevel ) )
            break;
    }

    const TileId key = TileId::fromCoordinates( GeoDataCoordinates(west, north, 0), zoomLevel ); // same as GeoDataCoordinates(east, south, 0), see above

    QList< GeoGraphicsItem* >& tileList = m_geoItems[key];
    QList< GeoGraphicsItem* >::iterator position = qLowerBound( tileList.begin(), tileList.end(), item, zValueLessThan );
    tileList.insert( position, item );
    m_features.insert( item->feature(), key );
}

void GeoGraphicsScenePrivate::addItems( const TileId &tileId, QList<GeoGraphicsItem *> &result, int maxZoomLevel ) const
{
    const QList< GeoGraphicsItem* > &objects = m_geoItems.value(tileId);
    QList< GeoGraphicsItem* >::iterator before = result.begin();
    QList< GeoGraphicsItem* >::const_iterator currentItem = objects.constBegin();
    while( currentItem != objects.end() ) {
        while( ( currentItem != objects.end() )
          && ( ( before == result.end() ) || ( (*currentItem)->zValue() < (*before)->zValue() ) ) ) {
            if( (*currentItem)->minZoomLevel() <= maxZoomLevel && (*currentItem)->visible() ) {
                before = result.insert( before, *currentItem );
            }
            ++currentItem;
        }
        if ( before != result.end() ) {
            ++before;
        }
    }
}

void GeoGraphicsScene::addPlacemarks( const QModelIndex &parent, int first, int last )
{
    Q_ASSERT( first < d->m_model->rowCount( parent ) );
    Q_ASSERT( last < d->m_model->rowCount( parent ) );

    for( int i=first; i<=last; ++i ) {
        QModelIndex index = d->m_model->index( i, 0, parent );
        Q_ASSERT( index.isValid() );
        const GeoDataObject *object = qvariant_cast<GeoDataObject*>(index.data( MarblePlacemarkModel::ObjectPointerRole ) );
        Q_ASSERT( object );
        d->createGraphicsItems( object );
    }

    emit repaintNeeded();
}

void GeoGraphicsScene::removePlacemarks( const QModelIndex &parent, int first, int last )
{
    Q_ASSERT( last < d->m_model->rowCount( parent ) );

    for( int i=first; i<=last; ++i ) {
        QModelIndex index = d->m_model->index( i, 0, parent );
        Q_ASSERT( index.isValid() );
        const GeoDataObject *object = qvariant_cast<GeoDataObject*>(index.data( MarblePlacemarkModel::ObjectPointerRole ) );
        const GeoDataFeature *feature = dynamic_cast<const GeoDataFeature*>( object );
        Q_ASSERT( feature );
        d->removeGraphicsItems( feature );
    }

    emit repaintNeeded();
}

void GeoGraphicsScene::resetCacheData()
{
    for( QMap< TileId, QList< GeoGraphicsItem* > >::const_iterator i = d->m_geoItems.constBegin();
         i != d->m_geoItems.constEnd(); ++i ) {
        qDeleteAll(*i);
    }
    d->m_geoItems.clear();
    d->m_features.clear();

    qDeleteAll( d->m_screenItems );
    d->m_screenItems.clear();

    const GeoDataObject *object = static_cast<GeoDataObject*>( d->m_model->index( 0, 0, QModelIndex() ).internalPointer() );
    if ( object && object->parent() )
        d->createGraphicsItems( object->parent() );

    emit repaintNeeded();
}

}

#include "GeoGraphicsScene.moc"
