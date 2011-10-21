//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010      Dennis Nienhüser <earthwings@gentoo.org>
//

#include "MarbleDeclarativeWidget.h"

#include <QtGui/QPainter>

#include "Coordinate.h"
#include "ZoomButtonInterceptor.h"

#include "GeoDataCoordinates.h"
#include "GeoPainter.h"
#include "MarbleWidgetInputHandler.h"
#include "MarbleMath.h"
#include "MapThemeManager.h"
#include "AbstractFloatItem.h"
#include "AbstractDataPlugin.h"
#include "RenderPlugin.h"
#include "MarbleMap.h"
#include "MarbleDirs.h"
#include "ViewParams.h"
#include "ViewportParams.h"
#include "DownloadRegion.h"
#include "BookmarkManager.h"
#include "routing/RoutingManager.h"
#include "routing/RoutingProfilesModel.h"
#include "DeclarativeDataPlugin.h"
#include "PluginManager.h"

MarbleWidget::MarbleWidget( QDeclarativeItem *parent ) :
    QDeclarativeItem( parent ),
    m_model(),
    m_map( &m_model ),
    m_inputEnabled( true ),
    m_interceptor( new ZoomButtonInterceptor( this, this ) )
{
    setFlag( QGraphicsItem::ItemHasNoContents, false );  // enable painting

    m_map.setMapThemeId( "earth/openstreetmap/openstreetmap.dgml" );

    QSettings settings;
    foreach( Marble::RenderPlugin *plugin, m_map.renderPlugins() ) {
        settings.beginGroup( QString( "plugin_" ) + plugin->nameId() );

        QHash<QString,QVariant> hash;

        foreach ( const QString& key, settings.childKeys() ) {
            hash.insert( key, settings.value( key ) );
        }

        plugin->setSettings( hash );

        settings.endGroup();
    }
    m_model.routingManager()->profilesModel()->loadDefaultProfiles();
    m_model.routingManager()->readSettings();
    m_model.bookmarkManager()->loadFile( "bookmarks/bookmarks.kml" );

    connect( &m_map, SIGNAL(visibleLatLonAltBoxChanged(GeoDataLatLonAltBox)),
             this, SIGNAL(visibleLatLonAltBoxChanged()) );
    connect( &m_model, SIGNAL(workOfflineChanged()),
             this, SIGNAL(workOfflineChanged()) );
    connect( &m_map, SIGNAL(radiusChanged(int)),
             this, SIGNAL(radiusChanged()) );
    connect( &m_map, SIGNAL(themeChanged(QString)),
             this, SIGNAL(mapThemeChanged()) );
    connect( &m_map, SIGNAL(projectionChanged(Projection)),
             this, SIGNAL(projectionChanged()) );
    connect( &m_map, SIGNAL(mouseClickGeoPosition(qreal,qreal,GeoDataCoordinates::Unit)),
             this, SLOT(forwardMouseClick(qreal,qreal,GeoDataCoordinates::Unit)) );
    connect( &m_center, SIGNAL(latitudeChanged()), this, SLOT(updateCenterPosition()));
    connect( &m_center, SIGNAL(longitudeChanged()), this, SLOT(updateCenterPosition()));
}

MarbleWidget::~MarbleWidget()
{
    QSettings settings;
    foreach( Marble::RenderPlugin *plugin, m_map.renderPlugins() ) {
        settings.beginGroup( QString( "plugin_" ) + plugin->nameId() );

        QHash<QString,QVariant> hash = plugin->settings();

        QHash<QString,QVariant>::iterator it = hash.begin();
        while( it != hash.end() ) {
            settings.setValue( it.key(), it.value() );
            ++it;
        }

        settings.endGroup();
    }
    m_model.routingManager()->writeSettings();
}

void MarbleWidget::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    if ( m_map.size() != QSize( width(), height() ) ) {
        m_map.setSize( QSize( width(), height() ) );
    }

    QPixmap px( width(), height() );
    if ( !m_map.viewport()->mapCoversViewport() ) {
        px.fill( Qt::black );
    }

    // Create a painter that will do the painting.
    Marble::GeoPainter geoPainter( &px, m_map.viewport(), m_map.mapQuality() );

    m_map.setViewContext( smooth() ? Marble::Still : Marble::Animation );
    m_map.paint( geoPainter, QRect() );

    painter->drawPixmap( 0, 0, width(), height(), px );
}

Marble::MarbleModel *MarbleWidget::model()
{
    return &m_model;
}

const Marble::ViewportParams *MarbleWidget::viewport() const
{
    return m_map.viewport();
}

QList<QObject*> MarbleWidget::renderPlugins() const
{
    QList<QObject*> result;
    foreach ( Marble::RenderPlugin* plugin, m_map.renderPlugins() ) {
        result << plugin;
    }

    return result;
}

QStringList MarbleWidget::activeFloatItems() const
{
    QStringList result;
    foreach( Marble::AbstractFloatItem * floatItem, m_map.floatItems() ) {
        if ( floatItem->enabled() && floatItem->visible() ) {
            result << floatItem->nameId();
        }
    }
    return result;
}

void MarbleWidget::setActiveFloatItems( const QStringList &items )
{
    foreach( Marble::AbstractFloatItem * floatItem, m_map.floatItems() ) {
        floatItem->setEnabled( items.contains( floatItem->nameId() ) );
        floatItem->setVisible( items.contains( floatItem->nameId() ) );
    }
}

QStringList MarbleWidget::activeRenderPlugins() const
{
    QStringList result;
    foreach( Marble::RenderPlugin * plugin, m_map.renderPlugins() ) {
        if ( plugin->enabled() && plugin->visible() ) {
            result << plugin->nameId();
        }
    }
    return result;
}

QDeclarativeListProperty<DeclarativeDataPlugin> MarbleWidget::dataLayers()
{
    return QDeclarativeListProperty<DeclarativeDataPlugin>( this, 0, &MarbleWidget::addLayer );
}

void MarbleWidget::setActiveRenderPlugins( const QStringList &items )
{
    foreach( Marble::RenderPlugin * plugin, m_map.renderPlugins() ) {
        plugin->setEnabled( items.contains( plugin->nameId() ) );
        plugin->setVisible( items.contains( plugin->nameId() ) );
    }
}

bool MarbleWidget::inputEnabled() const
{
    return m_inputEnabled;
}

void MarbleWidget::setInputEnabled( bool enabled )
{
    m_inputEnabled = enabled;
// FIXME    m_marbleWidget->setInputEnabled( enabled );
}

QString MarbleWidget::mapThemeId() const
{
    return m_map.mapThemeId();
}

void MarbleWidget::setMapThemeId( const QString &mapThemeId )
{
    m_map.setMapThemeId( mapThemeId );
}

QString MarbleWidget::projection( ) const
{
    switch ( m_map.projection() ) {
    case Marble::Equirectangular:
        return "Equirectangular";
    case Marble::Mercator:
        return "Mercator";
    case Marble::Spherical:
        return "Spherical";
    }

    Q_ASSERT( false && "Marble got a new projection which we do not know about yet" );
    return "Spherical";
}

void MarbleWidget::setProjection( const QString &projection )
{
    if ( projection.compare( "Equirectangular", Qt::CaseInsensitive ) == 0 ) {
        m_map.setProjection( Marble::Equirectangular );
    } else if ( projection.compare( "Mercator", Qt::CaseInsensitive ) == 0 ) {
        m_map.setProjection( Marble::Mercator );
    } else {
        m_map.setProjection( Marble::Spherical );
    }
}

void MarbleWidget::zoomIn()
{
    setRadius( radius() * 2 );
}

void MarbleWidget::zoomOut()
{
    setRadius( radius() / 2 );
}

QPoint MarbleWidget::pixel( qreal lon, qreal lat ) const
{
    Marble::GeoDataCoordinates position( lon, lat, 0, Marble::GeoDataCoordinates::Degree );
    qreal x( 0.0 );
    qreal y( 0.0 );
    const Marble::ViewportParams *viewport = m_map.viewport();
    viewport->screenCoordinates( position, x, y );
    return QPoint( x, y );
}

Coordinate *MarbleWidget::coordinate( int x, int y )
{
    qreal lat( 0.0 ), lon( 0.0 );
    m_map.geoCoordinates( x, y, lon, lat );
    return new Coordinate( lon, lat, 0.0, this );
}

Coordinate* MarbleWidget::center()
{
    m_center.blockSignals( true );
    m_center.setLongitude( m_map.centerLongitude() );
    m_center.setLatitude( m_map.centerLatitude() );
    m_center.blockSignals( false );
    return &m_center;
}

void MarbleWidget::setCenter( Coordinate* center )
{
    if ( center ) {
        m_center.blockSignals( true );
        m_center.setLongitude( center->longitude() );
        m_center.setLatitude( center->latitude() );
        m_center.setAltitude( center->altitude() );
        m_center.blockSignals( false );
        updateCenterPosition();
    }
}

void MarbleWidget::centerOn( const Marble::GeoDataLatLonBox &bbox )
{
    //prevent divide by zero
    if( bbox.height() && bbox.width() ) {
        //work out the needed zoom level
        const int horizontalRadius = ( 0.25 * M_PI ) * ( m_map.height() / bbox.height() );
        const int verticalRadius = ( 0.25 * M_PI ) * ( m_map.width() / bbox.width() );
        const int newRadius = qMin<int>( horizontalRadius, verticalRadius );
        m_map.setRadius( newRadius );
    }

    m_map.centerOn( bbox.center().longitude( GeoDataCoordinates::Degree ), bbox.center().latitude( GeoDataCoordinates::Degree ) );
}

void MarbleWidget::centerOn( const Marble::GeoDataCoordinates &coordinates )
{
    m_map.centerOn( coordinates.longitude( GeoDataCoordinates::Degree ), coordinates.latitude( GeoDataCoordinates::Degree ) );
}

void MarbleWidget::updateCenterPosition()
{
    m_map.centerOn( m_center.longitude(), m_center.latitude() );
    update();
}

void MarbleWidget::forwardMouseClick(qreal lon, qreal lat, Marble::GeoDataCoordinates::Unit unit )
{
    Marble::GeoDataCoordinates position( lon, lat, unit );
    Marble::GeoDataCoordinates::Unit degree = Marble::GeoDataCoordinates::Degree;
    QPoint const point = pixel( position.longitude( degree ), position.latitude( degree ) );
    QVector<const Marble::GeoDataPlacemark*> const placemarks = m_map.whichFeatureAt( point );
    if ( !placemarks.isEmpty() ) {
        if ( placemarks.size() == 1 ) {
            Placemark* placemark = new Placemark;
            placemark->setGeoDataPlacemark( *placemarks.first() );
            emit placemarkSelected( placemark );
        }
    } else {
        emit mouseClickGeoPosition( position.longitude( degree ),
                                    position.latitude( degree ) );
    }
}

void MarbleWidget::addLayer( QDeclarativeListProperty<DeclarativeDataPlugin> *list, DeclarativeDataPlugin *layer )
{
    MarbleWidget *object = qobject_cast<MarbleWidget *>( list->object );
    if ( object ) {
        object->m_model.pluginManager()->addRenderPlugin( layer );
        object->setDataPluginDelegate( layer->nameId(), layer->delegate() );
        object->m_dataLayers << layer;
    }
}

QObject *MarbleWidget::mapThemeModel()
{
    return m_mapThemeManager.mapThemeModel();
}

void MarbleWidget::setGeoSceneProperty(const QString &key, bool value)
{
    m_map.setPropertyValue( key, value );
}

void MarbleWidget::downloadRoute( qreal offset, int topTileLevel, int bottomTileLevel )
{
    Marble::DownloadRegion region;
    region.setMarbleModel( &m_model );
    region.setVisibleTileLevel( m_map.tileZoomLevel() );
    region.setTileLevelRange( topTileLevel, bottomTileLevel );
    QVector<Marble::TileCoordsPyramid> const pyramid = region.routeRegion( m_map.textureLayer(), offset );
    if ( !pyramid.isEmpty() ) {
        m_map.downloadRegion( pyramid );
    }
}

void MarbleWidget::downloadArea(int topTileLevel, int bottomTileLevel)
{
    Marble::DownloadRegion region;
    region.setMarbleModel( &m_model );
    region.setVisibleTileLevel( m_map.tileZoomLevel() );
    region.setTileLevelRange( topTileLevel, bottomTileLevel );
    QVector<Marble::TileCoordsPyramid> const pyramid = region.region( m_map.textureLayer(), m_map.viewport()->viewLatLonAltBox() );
    if ( !pyramid.isEmpty() ) {
        m_map.downloadRegion( pyramid );
    }
}

void MarbleWidget::setDataPluginDelegate( const QString &plugin, QDeclarativeComponent *delegate )
{
    QList<Marble::RenderPlugin*> renderPlugins = m_map.renderPlugins();
    foreach( Marble::RenderPlugin* renderPlugin, renderPlugins ) {
        Marble::AbstractDataPlugin* dataPlugin = qobject_cast<Marble::AbstractDataPlugin*>( renderPlugin );
        if ( dataPlugin && dataPlugin->nameId() == plugin ) {
            dataPlugin->setDelegate( delegate, this );
        }
    }
}

bool MarbleWidget::workOffline() const
{
    return m_model.workOffline();
}

void MarbleWidget::setWorkOffline( bool workOffline )
{
    m_model.setWorkOffline( workOffline );
}

int MarbleWidget::radius() const
{
    return m_map.radius();
}

void MarbleWidget::setRadius( int radius )
{
    m_map.setRadius( radius );
    update();
}

Marble::RenderPlugin *MarbleWidget::renderPlugin( const QString & name )
{
    foreach( Marble::RenderPlugin * plugin, m_map.renderPlugins() )
    {
        if( plugin->nameId() == name ) {
            return plugin;
        }
    }
    return 0;
}

bool MarbleWidget::containsRenderPlugin( const QString & name )
{
    foreach( Marble::RenderPlugin * plugin, m_map.renderPlugins() )
    {
        if( plugin->nameId() == name ) {
            return true;
        }
    }
    return false;
}

QList<QObject*> MarbleWidget::floatItems() const
{
    QList<QObject*> result;
    foreach ( Marble::AbstractFloatItem* plugin, m_map.floatItems() ) {
        result << plugin;
    }

    return result;
}

Marble::AbstractFloatItem* MarbleWidget::floatItem( const QString & name )
{
    foreach( Marble::AbstractFloatItem * plugin, m_map.floatItems() )
    {
        if( plugin->nameId() == name ) {
            return plugin ;
        }
    }
    return 0;
}

bool MarbleWidget::containsFloatItem( const QString & name )
{
    foreach( Marble::AbstractFloatItem * plugin, m_map.floatItems() )
    {
        if( plugin->nameId() == name ) {
            return true;
        }
    }
    return false;
}

#include "MarbleDeclarativeWidget.moc"
