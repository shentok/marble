//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2006-2007 Torsten Rahn <tackat@kde.org>
// Copyright 2007      Inge Wallin  <ingwa@kde.org>
//

#include "MarbleWidget.h"

#include <cmath>

#include <QtCore/QAbstractItemModel>
#include <QtCore/QHash>
#include <QtCore/QSettings>
#include <QtCore/QTime>
#include <QtCore/QTimer>
#include <QtGui/QHBoxLayout>
#include <QtGui/QItemSelectionModel>
#include <QtGui/QPaintEvent>
#include <QtGui/QRegion>
#include <QtGui/QSizePolicy>
#include <QtNetwork/QNetworkProxy>

#ifdef MARBLE_DBUS
#include <QtDBus/QDBusConnection>
#endif

#include "AbstractProjection.h"
#include "DataMigration.h"
#include "FileViewModel.h"
#include "GeoDataLatLonAltBox.h"
#include "GeoPainter.h"
#include "MarbleDebug.h"
#include "MarbleDirs.h"
#include "MarbleLocale.h"
#include "MarbleMap.h"
#include "MarbleMapWidget.h"
#include "MarbleModel.h"
#include "MarblePhysics.h"
#include "MarblePlacemarkModel.h"
#include "MarbleWidgetInputHandler.h"
#include "MergedLayerDecorator.h"
#include "PlacemarkLayout.h"
#include "RenderPlugin.h"
#include "SunLocator.h"
#include "TileCreatorDialog.h"
#include "ViewportParams.h"
#include "BookmarkManager.h"
#include "routing/RoutingLayer.h"
#include "routing/RoutingManager.h"

namespace Marble
{

#ifdef Q_CC_MSVC
# ifndef KDEWIN_MATH_H
   static long double sqrt( int a ) { return sqrt( (long double)a ); }
# endif
#endif

const int REPAINT_SCHEDULING_INTERVAL = 1000;

class MarbleWidgetPrivate
{
 public:
    MarbleWidgetPrivate( MarbleModel *model, MarbleWidget *parent )
        : m_widget( parent ),
          m_model( model ),
          m_viewContext( Still ),
          m_stillQuality( HighQuality ),
          m_animationQuality( LowQuality ),
          m_animationsEnabled( false ),
          m_zoomStep( 40 ),
          m_inputhandler( 0 ),
          m_physics( new MarblePhysics( parent ) ),
          m_repaintTimer(),
          m_routingLayer( 0 ),
          m_showFrameRate( false )
    {
    }

    virtual ~MarbleWidgetPrivate()
    {
    }

    void  construct();

    /**
      * @brief Rotate the globe in the given direction in discrete steps
      * @param stepsRight Number of steps to go right. Negative values go left.
      * @param stepsDown Number of steps to go down. Negative values go up.
      * @param mode Interpolation mode to use when traveling to the target
      */
    void moveByStep( int stepsRight, int stepsDown, FlyToMode mode );

    /**
      * @brief Move camera to the given position. This can change
      * both the zoom value and the position
      */
    void flyTo( const GeoDataLookAt &lookAt );

    void paintFps( GeoPainter &painter, QRect &dirtyRect, qreal fps);

    void setDistance( qreal distance );

    virtual ViewportParams *viewport() = 0;
    virtual const ViewportParams *viewport() const = 0;

    virtual int radius() const = 0;
    virtual void setRadius( int radius ) = 0;

    virtual void zoomView( int zoom ) = 0;

    virtual qreal centerLatitude() const = 0;
    virtual qreal centerLongitude() const = 0;
    virtual void centerOn( qreal lon, qreal lat ) = 0;

    virtual bool showOverviewMap() const = 0;
    virtual bool showScaleBar() const = 0;
    virtual bool showCompass() const = 0;
    virtual bool showClouds() const = 0;
    virtual bool showAtmosphere() const = 0;
    virtual bool showGrid() const = 0;
    virtual bool showPlaces() const = 0;
    virtual bool showCities() const = 0;
    virtual bool showTerrain() const = 0;
    virtual bool showOtherPlaces() const = 0;
    virtual bool showRelief() const = 0;
    virtual bool showElevationModel() const = 0;
    virtual bool showIceLayer() const = 0;
    virtual bool showBorders() const = 0;
    virtual bool showRivers() const = 0;
    virtual bool showLakes() const = 0;
    virtual bool showGps() const = 0;

    virtual void setShowOverviewMap( bool show ) = 0;
    virtual void setShowScaleBar( bool show ) = 0;
    virtual void setShowCompass( bool show ) = 0;
    virtual void setShowClouds( bool show ) = 0;
    virtual void setShowAtmosphere( bool show ) = 0;
    virtual void setShowGrid( bool show ) = 0;
    virtual void setShowPlaces( bool show ) = 0;
    virtual void setShowCities( bool show ) = 0;
    virtual void setShowTerrain( bool show ) = 0;
    virtual void setShowOtherPlaces( bool show ) = 0;
    virtual void setShowRelief( bool show ) = 0;
    virtual void setShowElevationModel( bool show ) = 0;
    virtual void setShowIceLayer( bool show ) = 0;
    virtual void setShowBorders( bool show ) = 0;
    virtual void setShowRivers( bool show ) = 0;
    virtual void setShowLakes( bool show ) = 0;
    virtual void setShowGps( bool show ) = 0;
    virtual void setShowTileId( bool show ) = 0;

    virtual void setPropertyValue( const QString& name, bool value ) = 0;

    virtual Projection projection() const = 0;
    virtual void setProjection( Projection projection ) = 0;

    virtual void setMapThemeId( const QString& mapThemeId ) = 0;

    virtual void setMapQuality( MapQuality mapQuality ) = 0;
    virtual MapQuality mapQuality() const = 0;

    virtual void setNeedsUpdate() = 0;
    virtual void updateSun() = 0;

    MarbleWidget    *const m_widget;
    // The model we are showing.
    MarbleModel     *const m_model;   // Owned by m_map.  Don't delete.

    ViewContext     m_viewContext;

    MapQuality      m_stillQuality;
    MapQuality      m_animationQuality;

    bool m_animationsEnabled;

    int              m_zoomStep;

    MarbleWidgetInputHandler  *m_inputhandler;

    MarblePhysics    *m_physics;

    // For scheduling repaints
    QTimer           m_repaintTimer;

    RoutingLayer     *m_routingLayer;
    bool             m_showFrameRate;
};


void MarbleWidgetPrivate::setDistance( qreal newDistance )
{
    qreal minDistance = 0.001;

    if ( newDistance <= minDistance ) {
        mDebug() << "Invalid distance: 0 m";
        newDistance = minDistance;
    }

    int newRadius = m_model->radiusFromDistance( newDistance );
    setRadius( newRadius );
}


class MarbleWidget::MapPrivate : public MarbleWidgetPrivate
{
public:
    MapPrivate( MarbleMap * map, MarbleWidget *parent )
        : MarbleWidgetPrivate( map->model(), parent )
        , m_widget( new MarbleMapWidget( map, parent ) )
    {
        parent->setLayout( new QHBoxLayout( parent ) );
        parent->layout()->setMargin( 0 );
        parent->layout()->addWidget( m_widget );

        // Initialize the map and forward some signals.
        m_widget->setMapQuality( m_stillQuality );

        // When some fundamental things change in the map, we got to show
        // this in the view, i.e. here.
        parent->connect( m_widget, SIGNAL( radiusChanged(int)),
                           parent, SLOT( onRadiusChanged(int)) );
        parent->connect( m_widget, SIGNAL( visibleLatLonAltBoxChanged( GeoDataLatLonAltBox )),
                           parent, SIGNAL( visibleLatLonAltBoxChanged( GeoDataLatLonAltBox )));
    }

    virtual ViewportParams *viewport() { return m_widget->viewport(); }
    virtual const ViewportParams *viewport() const { return m_widget->viewport(); }

    virtual int radius() const { return m_widget->radius(); }
    virtual void setRadius( int radius ) { m_widget->setRadius( radius ); }

    virtual void zoomView( int zoom ) { m_widget->zoomView( zoom ); }

    virtual qreal centerLatitude() const { return m_widget->centerLatitude(); }
    virtual qreal centerLongitude() const { return m_widget->centerLongitude(); }
    virtual void centerOn( qreal lon, qreal lat ) { m_widget->centerOn( lon, lat ); }

    virtual bool showOverviewMap() const { return m_widget->showOverviewMap(); }
    virtual bool showScaleBar() const { return m_widget->showScaleBar(); }
    virtual bool showCompass() const { return m_widget->showCompass(); }
    virtual bool showClouds() const { return m_widget->showClouds(); }
    virtual bool showAtmosphere() const { return m_widget->showAtmosphere(); }
    virtual bool showGrid() const { return m_widget->showGrid(); }
    virtual bool showPlaces() const { return m_widget->showPlaces(); }
    virtual bool showCities() const { return m_widget->showCities(); }
    virtual bool showTerrain() const { return m_widget->showTerrain(); }
    virtual bool showOtherPlaces() const { return m_widget->showOtherPlaces(); }
    virtual bool showRelief() const { return m_widget->showRelief(); }
    virtual bool showElevationModel() const { return m_widget->showElevationModel(); }
    virtual bool showIceLayer() const { return m_widget->showIceLayer(); }
    virtual bool showBorders() const { return m_widget->showBorders(); }
    virtual bool showRivers() const { return m_widget->showRivers(); }
    virtual bool showLakes() const { return m_widget->showLakes(); }
    virtual bool showGps() const { return m_widget->showGps(); }

    virtual void setShowOverviewMap( bool show ) { m_widget->setShowOverviewMap( show ); }
    virtual void setShowScaleBar( bool show ) { m_widget->setShowScaleBar( show ); }
    virtual void setShowCompass( bool show ) { m_widget->setShowCompass( show ); }
    virtual void setShowClouds( bool show ) { m_widget->setShowClouds( show ); }
    virtual void setShowAtmosphere( bool show ) { m_widget->setShowAtmosphere( show ); }
    virtual void setShowGrid( bool show ) { m_widget->setShowGrid( show ); }
    virtual void setShowPlaces( bool show ) { m_widget->setShowPlaces( show ); }
    virtual void setShowCities( bool show ) { m_widget->setShowCities( show ); }
    virtual void setShowTerrain( bool show ) { m_widget->setShowTerrain( show ); }
    virtual void setShowOtherPlaces( bool show ) { m_widget->setShowOtherPlaces( show ); }
    virtual void setShowRelief( bool show ) { m_widget->setShowRelief( show ); }
    virtual void setShowElevationModel( bool show ) { m_widget->setShowElevationModel( show ); }
    virtual void setShowIceLayer( bool show ) { m_widget->setShowIceLayer( show ); }
    virtual void setShowBorders( bool show ) { m_widget->setShowBorders( show ); }
    virtual void setShowRivers( bool show ) { m_widget->setShowRivers( show ); }
    virtual void setShowLakes( bool show ) { m_widget->setShowLakes( show ); }
    virtual void setShowGps( bool show ) { m_widget->setShowGps( show ); }
    virtual void setShowTileId( bool show ) { m_widget->setShowTileId( show ); }

    virtual void setPropertyValue( const QString& name, bool value ) { m_widget->setPropertyValue( name, value ); }

    virtual Projection projection() const { return m_widget->projection(); }
    virtual void setProjection( Projection projection ) { m_widget->setProjection( projection ); }

    virtual void setMapThemeId( const QString& mapThemeId ) { m_widget->setMapThemeId( mapThemeId ); }

    virtual void setMapQuality( MapQuality mapQuality ) { m_widget->setMapQuality( mapQuality ); }
    virtual MapQuality mapQuality() const { return m_widget->mapQuality(); }

    virtual void setNeedsUpdate() { m_widget->setNeedsUpdate(); }
    virtual void updateSun() { m_widget->updateSun(); }

private:
    MarbleMapWidget *const m_widget;
};


MarbleWidget::MarbleWidget(QWidget *parent)
    : QWidget( parent ),
      d( new MapPrivate( new MarbleMap(), this ) )
{
    setAttribute( Qt::WA_NoSystemBackground, true );

    d->construct();
}


MarbleWidget::MarbleWidget(MarbleMap *map, QWidget *parent)
    : QWidget( parent ),
      d( new MapPrivate( map, this ) )
{
    setAttribute( Qt::WA_NoSystemBackground, true );

    d->construct();
}

MarbleWidget::~MarbleWidget()
{
    // Remove and delete an existing InputHandler
    // initialized in d->construct()
    setInputHandler( 0 );

    delete d;
}

void MarbleWidgetPrivate::construct()
{
    QPointer<DataMigration> dataMigration = new DataMigration( m_widget );
    dataMigration->exec();
    delete dataMigration;

#ifdef MARBLE_DBUS
    QDBusConnection::sessionBus().registerObject( "/MarbleWidget", m_widget,
                                                  QDBusConnection::ExportAllSlots
                                                  | QDBusConnection::ExportAllSignals
                                                  | QDBusConnection::ExportAllProperties );
#endif

    // Widget settings
    m_widget->setMinimumSize( 200, 300 );
    m_widget->setFocusPolicy( Qt::WheelFocus );
    m_widget->setFocus( Qt::OtherFocusReason );
#if QT_VERSION >= 0x40600
    m_widget->setAttribute( Qt::WA_AcceptTouchEvents );
#endif

    // When some fundamental things change in the model, we got to
    // show this in the view, i.e. here.
    m_widget->connect( m_model,  SIGNAL( projectionChanged( Projection ) ),
                       m_widget, SIGNAL( projectionChanged( Projection ) ) );
    m_widget->connect( m_model,  SIGNAL( themeChanged( QString ) ),
		       m_widget, SIGNAL( themeChanged( QString ) ) );
    m_widget->connect( m_model, SIGNAL( modelChanged() ),
                       m_widget, SLOT( updateChangedMap() ) );

    // Repaint scheduling
    m_widget->connect( m_model,  SIGNAL( repaintNeeded( QRegion ) ),
                       m_widget, SLOT( scheduleRepaint( QRegion ) ) );
    m_repaintTimer.setSingleShot( true );
    m_repaintTimer.setInterval( REPAINT_SCHEDULING_INTERVAL );
    m_widget->connect( &m_repaintTimer, SIGNAL( timeout() ),
                       m_widget, SLOT( update() ) );

    // Set background: black.
    m_widget->setPalette( QPalette ( Qt::black ) );

    // Set whether the black space gets displayed or the earth gets simply 
    // displayed on the widget background.
    m_widget->setAutoFillBackground( true );

    // Show a progress dialog when the model calculates new map tiles.
    m_widget->connect( m_model, SIGNAL( creatingTilesStart( TileCreator*, const QString&,
                                                            const QString& ) ),
                       m_widget, SLOT( creatingTilesStart( TileCreator*, const QString&,
                                                           const QString& ) ) );

    m_widget->connect( m_model->sunLocator(), SIGNAL( enableWidgetInput( bool ) ),
                       m_widget, SLOT( setInputEnabled( bool ) ) );

    m_widget->connect( m_model->sunLocator(), SIGNAL( updateStars() ),
                       m_widget, SLOT( update() ) );

    m_widget->connect( m_physics, SIGNAL( positionReached( GeoDataLookAt ) ),
                       m_widget, SLOT( updateAnimation( GeoDataLookAt ) ) );

    m_widget->connect( m_physics, SIGNAL( finished() ),
                       m_widget, SLOT( startStillMode() ) );

    m_widget->connect( m_model->sunLocator(), SIGNAL( centerSun() ),
                       m_widget, SLOT( centerSun() ) );

    m_widget->setInputHandler( new MarbleWidgetDefaultInputHandler );
    m_widget->setMouseTracking( m_widget );

    m_widget->connect( m_model, SIGNAL( pluginSettingsChanged() ),
                       m_widget, SIGNAL( pluginSettingsChanged() ) );
                       
    m_widget->connect( m_model, SIGNAL( renderPluginInitialized( RenderPlugin * ) ),
                       m_widget, SIGNAL( renderPluginInitialized( RenderPlugin * ) ) );

    m_routingLayer = new RoutingLayer( m_widget, m_widget );
    m_routingLayer->setRouteRequest( m_model->routingManager()->routeRequest() );
    m_model->addLayer( m_routingLayer );

    m_widget->connect( m_routingLayer, SIGNAL( routeDirty() ),
                       m_model->routingManager(), SLOT( updateRoute() ) );
}

void MarbleWidgetPrivate::moveByStep( int stepsRight, int stepsDown, FlyToMode mode )
{
    int polarity = viewport()->polarity();
    qreal left = polarity * stepsRight * m_widget->moveStep();
    qreal down = stepsDown * m_widget->moveStep();
    m_widget->rotateBy( left, down, mode );
}

void MarbleWidgetPrivate::flyTo( const GeoDataLookAt &lookAt )
{
    int zoom = m_model->zoomFromDistance( lookAt.range() * METER2KM );
    if ( zoom < m_model->minimumZoom() || zoom > m_model->maximumZoom() )
        return; // avoid moving when zooming is impossible

    setDistance( lookAt.range() * METER2KM );
    GeoDataCoordinates::Unit deg = GeoDataCoordinates::Degree;
    centerOn( lookAt.longitude( deg ), lookAt.latitude( deg ) );
}

void MarbleWidgetPrivate::paintFps( GeoPainter &painter, QRect &dirtyRect, qreal fps )
{
    Q_UNUSED( dirtyRect );

    if ( m_showFrameRate ) {
        QString fpsString = QString( "Speed: %1 fps" ).arg( fps, 5, 'f', 1, QChar(' ') );

        QPoint fpsLabelPos( 10, 20 );

        painter.setFont( QFont( "Sans Serif", 10 ) );

        painter.setPen( Qt::black );
        painter.setBrush( Qt::black );
        painter.drawText( fpsLabelPos, fpsString );

        painter.setPen( Qt::white );
        painter.setBrush( Qt::white );
        painter.drawText( fpsLabelPos.x() - 1, fpsLabelPos.y() - 1, fpsString );
    }
}

// ----------------------------------------------------------------


MarbleModel *MarbleWidget::model() const
{
    return d->m_model;
}


ViewportParams* MarbleWidget::viewport()
{
    return d->viewport();
}

const ViewportParams* MarbleWidget::viewport() const
{
    return d->viewport();
}


void MarbleWidget::setInputHandler( MarbleWidgetInputHandler *handler )
{
    delete d->m_inputhandler;
    d->m_inputhandler = handler;

    if ( d->m_inputhandler ) {
        d->m_inputhandler->init( this );
        installEventFilter( d->m_inputhandler );

        connect( d->m_inputhandler, SIGNAL( mouseClickScreenPosition( int, int ) ),
                 this,              SLOT( notifyMouseClick( int, int ) ) );

        connect( d->m_inputhandler, SIGNAL( mouseMoveGeoPosition( QString ) ),
                 this,              SIGNAL( mouseMoveGeoPosition( QString ) ) );
    }
}

MarbleWidgetInputHandler *MarbleWidget::inputHandler() const
{
  return d->m_inputhandler;
}

Quaternion MarbleWidget::planetAxis() const
{
    return viewport()->planetAxis();
}


int MarbleWidget::radius() const
{
    return d->radius();
}

void MarbleWidget::setRadius( int newRadius )
{
    if ( newRadius == radius() ) {
        return;
    }
    d->setRadius( newRadius );
}


QAbstractItemModel *MarbleWidget::placemarkModel() const
{
    return d->m_model->placemarkModel();
}

QItemSelectionModel *MarbleWidget::placemarkSelectionModel() const
{
    return d->m_model->placemarkSelectionModel();
}

qreal MarbleWidget::moveStep()
{
    if ( radius() < sqrt( (qreal)(width() * width() + height() * height()) ) )
        return 180.0 * 0.1;
    else
        return 180.0 * atan( (qreal)width()
                     / (qreal)( 2 * radius() ) ) * 0.2;
}

int MarbleWidget::zoom() const
{
    return MarbleModel::zoomFromRadius( radius() );
}

int  MarbleWidget::minimumZoom() const
{
    return d->m_model->minimumZoom();
}

int  MarbleWidget::maximumZoom() const
{
    return d->m_model->maximumZoom();
}

void MarbleWidget::addPlacemarkFile( const QString &filename )
{
    addGeoDataFile( filename );
}

void MarbleWidget::addPlacemarkData( const QString &data, const QString &key )
{
    addGeoDataString( data, key );
}

void MarbleWidget::removePlacemarkKey( const QString &key )
{
    removeGeoData( key );
}

QPixmap MarbleWidget::mapScreenShot()
{
    return QPixmap::grabWidget( this );
}

bool MarbleWidget::showOverviewMap() const
{
    return d->showOverviewMap();
}

bool MarbleWidget::showScaleBar() const
{
    return d->showScaleBar();
}

bool MarbleWidget::showCompass() const
{
    return d->showCompass();
}

bool MarbleWidget::showClouds() const
{
    return d->showClouds();
}

bool MarbleWidget::showAtmosphere() const
{
    return d->showAtmosphere();
}

bool MarbleWidget::showCrosshairs() const
{
    return d->m_model->showCrosshairs();
}

bool MarbleWidget::showGrid() const
{
    return d->showGrid();
}

bool MarbleWidget::showPlaces() const
{
    return d->showPlaces();
}

bool MarbleWidget::showCities() const
{
    return d->showCities();
}

bool MarbleWidget::showTerrain() const
{
    return d->showTerrain();
}

bool MarbleWidget::showOtherPlaces() const
{
    return d->showOtherPlaces();
}

bool MarbleWidget::showRelief() const
{
    return d->showRelief();
}

bool MarbleWidget::showElevationModel() const
{
    return d->showElevationModel();
}

bool MarbleWidget::showIceLayer() const
{
    return d->showIceLayer();
}

bool MarbleWidget::showBorders() const
{
    return d->showBorders();
}

bool MarbleWidget::showRivers() const
{
    return d->showRivers();
}

bool MarbleWidget::showLakes() const
{
    return d->showLakes();
}

bool MarbleWidget::showGps() const
{
    return d->showGps();
}

bool MarbleWidget::showFrameRate() const
{
    return d->m_showFrameRate;
}

quint64 MarbleWidget::persistentTileCacheLimit() const
{
    return d->m_model->persistentTileCacheLimit();
}

quint64 MarbleWidget::volatileTileCacheLimit() const
{
    return d->m_model->volatileTileCacheLimit();
}


void MarbleWidget::zoomView( int newZoom, FlyToMode mode )
{
    // It won't fly anyway. So we should do everything to keep the zoom value.
    if ( !d->m_animationsEnabled || mode == Instant ) {
        d->zoomView( newZoom );
        repaint();
    }
    else {
        GeoDataLookAt target = lookAt();
        target.setRange( KM2METER * d->m_model->distanceFromZoom( newZoom ) );

        flyTo( target, mode );
    }
}


void MarbleWidget::zoomViewBy( int zoomStep, FlyToMode mode )
{
    zoomView( zoom() + zoomStep, mode );
}


void MarbleWidget::zoomIn( FlyToMode mode )
{
    zoomViewBy( d->m_zoomStep, mode );
}

void MarbleWidget::zoomOut( FlyToMode mode )
{
    zoomViewBy( -d->m_zoomStep, mode );
}

void MarbleWidget::rotateBy( const Quaternion& incRot )
{
    Quaternion quat = incRot * planetAxis();

    centerOn( quat.yaw()*RAD2DEG, -quat.pitch()*RAD2DEG );
}

void MarbleWidget::rotateBy( const qreal deltaLon, const qreal deltaLat, FlyToMode mode )
{
    Quaternion  rotPhi( 1.0, deltaLat / 180.0, 0.0, 0.0 );
    Quaternion  rotTheta( 1.0, 0.0, deltaLon / 180.0, 0.0 );

    Quaternion  axis = planetAxis();
    qreal lon( 0.0 ), lat( 0.0 );
    axis.getSpherical( lon, lat );
    axis = rotTheta * axis;
    axis *= rotPhi;
    axis.normalize();
    lat = -axis.pitch();
    lon = axis.yaw();
    
    GeoDataLookAt target = lookAt();
    target.setLongitude( lon );
    target.setLatitude( lat );
    flyTo( target, mode );
}


void MarbleWidget::centerOn( const qreal lon, const qreal lat, bool animated )
{
    GeoDataCoordinates target( lon, lat, 0.0, GeoDataCoordinates::Degree );
    centerOn( target, animated );
}

void MarbleWidget::centerOn( const QModelIndex& index, bool animated )
{
    QItemSelectionModel *selectionModel = d->m_model->placemarkSelectionModel();
    Q_ASSERT( selectionModel );

    selectionModel->clear();

    if ( index.isValid() ) {
        const GeoDataCoordinates targetPosition =
            index.data( MarblePlacemarkModel::CoordinateRole ).value<GeoDataCoordinates>();

        GeoDataLookAt target = lookAt();
        target.setLongitude( targetPosition.longitude() );
        target.setLatitude( targetPosition.latitude() );
        flyTo( target, animated ? Automatic : Instant );

        selectionModel->select( index, QItemSelectionModel::SelectCurrent );
    }
}

void MarbleWidget::centerOn( const GeoDataCoordinates &position, bool animated )
{
    GeoDataLookAt target = lookAt();
    target.setLongitude( position.longitude() );
    target.setLatitude( position.latitude() );
    flyTo( target, animated ? Automatic : Instant );
}

void MarbleWidget::centerOn( const GeoDataLatLonBox &box, bool animated )
{
    ViewportParams* viewparams = viewport();
    //prevent divide by zero
    if( box.height() && box.width() ) {
        //work out the needed zoom level
        int horizontalRadius = ( 0.25 * M_PI ) * ( viewparams->height() / box.height() );
        int verticalRadius = ( 0.25 * M_PI ) * ( viewparams->width() / box.width() );
        setRadius( qMin<int>( horizontalRadius, verticalRadius ) );
    }

    //move the map
    centerOn( box.center().longitude( GeoDataCoordinates::Degree ),
              box.center().latitude( GeoDataCoordinates::Degree ),
              animated );

    repaint();
}

void MarbleWidget::setCenterLatitude( qreal lat, FlyToMode mode )
{
    centerOn( centerLongitude(), lat, mode );
}

void MarbleWidget::setCenterLongitude( qreal lon, FlyToMode mode )
{
    centerOn( lon, centerLatitude(), mode );
}

Projection MarbleWidget::projection() const
{
    return d->projection();
}

void MarbleWidget::setProjection( Projection projection )
{
    d->setProjection( projection );
}

void MarbleWidget::setProjection( int projection )
{
    setProjection( (Projection)( projection ) );
}

void MarbleWidget::home( qreal &lon, qreal &lat, int& zoom )
{
    d->m_model->home( lon, lat, zoom );
}

void MarbleWidget::setHome( qreal lon, qreal lat, int zoom )
{
    d->m_model->setHome( lon, lat, zoom );
}

void MarbleWidget::setHome( const GeoDataCoordinates& homePoint, int zoom )
{
    d->m_model->setHome( homePoint, zoom );
}


void MarbleWidget::moveLeft( FlyToMode mode )
{
    d->moveByStep( -1, 0, mode );
}

void MarbleWidget::moveRight( FlyToMode mode )
{
    d->moveByStep( 1, 0, mode );
}


void MarbleWidget::moveUp( FlyToMode mode )
{
    d->moveByStep( 0, -1, mode );
}

void MarbleWidget::moveDown( FlyToMode mode )
{
    d->moveByStep( 0, 1, mode );
}

void MarbleWidget::leaveEvent( QEvent* )
{
    emit mouseMoveGeoPosition( tr( NOT_AVAILABLE ) );
}

void MarbleWidget::connectNotify( const char * signal )
{
    if ( QByteArray( signal ) == 
         QMetaObject::normalizedSignature ( SIGNAL( mouseMoveGeoPosition( QString ) ) ) )
        if ( d->m_inputhandler )
            d->m_inputhandler->setPositionSignalConnected( true );
}

void MarbleWidget::disconnectNotify( const char * signal )
{
    if ( QByteArray( signal ) == 
         QMetaObject::normalizedSignature ( SIGNAL( mouseMoveGeoPosition( QString ) ) ) )
        if ( d->m_inputhandler )
            d->m_inputhandler->setPositionSignalConnected( false );
}

bool MarbleWidget::screenCoordinates( qreal lon, qreal lat,
                                      qreal& x, qreal& y )
{
    return viewport()->currentProjection()->screenCoordinates( lon, lat, viewport(), x, y );
}

bool MarbleWidget::geoCoordinates( int x, int y,
                                   qreal& lon, qreal& lat,
                                   GeoDataCoordinates::Unit unit )
{
    return viewport()->currentProjection()->geoCoordinates( x, y, viewport(), lon, lat, unit );
}

qreal MarbleWidget::centerLatitude() const
{
    return d->centerLatitude();
}

qreal MarbleWidget::centerLongitude() const
{
    return d->centerLongitude();
}

QRegion MarbleWidget::activeRegion()
{
    return viewport()->activeRegion();
}

void MarbleWidget::paintEvent( QPaintEvent *event )
{
    // Stop repaint timer if it is already running
    d->m_repaintTimer.stop();

    QWidget::paintEvent( event );
}

QRegion MarbleWidget::mapRegion()
{
    return viewport()->currentProjection()->mapRegion( viewport() );
}


void MarbleWidget::goHome( FlyToMode mode )
{
    qreal  homeLon = 0;
    qreal  homeLat = 0;
    int homeZoom = 0;
    d->m_model->home( homeLon, homeLat, homeZoom );

    GeoDataLookAt target;
    target.setLongitude( homeLon, GeoDataCoordinates::Degree );
    target.setLatitude( homeLat, GeoDataCoordinates::Degree );
    target.setRange( 1000 * d->m_model->distanceFromZoom( homeZoom ) );

    flyTo( target, mode );
}

QString MarbleWidget::mapThemeId() const
{
    return d->m_model->mapThemeId();
}

void MarbleWidget::setMapThemeId( const QString& mapThemeId )
{
    if ( !mapThemeId.isEmpty() && mapThemeId == d->m_model->mapThemeId() )
        return;
    
    d->setMapThemeId( mapThemeId );

    // Now we want a full repaint as the atmosphere might differ
    setAttribute( Qt::WA_NoSystemBackground,
                  false );

    centerSun();

    repaint();
}

GeoSceneDocument *MarbleWidget::mapTheme() const
{
    return d->m_model->mapTheme();
}

void MarbleWidget::setPropertyValue( const QString& name, bool value )
{
    mDebug() << "In MarbleWidget the property " << name << "was set to " << value;
    d->setPropertyValue( name, value );

    repaint();
}

void MarbleWidget::setShowOverviewMap( bool visible )
{
    d->setShowOverviewMap( visible );

    repaint();
}

void MarbleWidget::setShowScaleBar( bool visible )
{
    d->setShowScaleBar( visible );

    repaint();
}

void MarbleWidget::setShowCompass( bool visible )
{
    d->setShowCompass( visible );

    repaint();
}

void MarbleWidget::setShowClouds( bool visible )
{
    d->setShowClouds( visible );

    repaint();
}

void MarbleWidget::setShowAtmosphere( bool visible )
{
    d->setShowAtmosphere( visible );

    repaint();
}

void MarbleWidget::setShowCrosshairs( bool visible )
{
    d->m_model->setShowCrosshairs( visible );

    repaint();
}

void MarbleWidget::setShowGrid( bool visible )
{
    d->setShowGrid( visible );

    repaint();
}

void MarbleWidget::setShowPlaces( bool visible )
{
    d->setShowPlaces( visible );

    repaint();
}

void MarbleWidget::setShowCities( bool visible )
{
    d->setShowCities( visible );

    repaint();
}

void MarbleWidget::setShowTerrain( bool visible )
{
    d->setShowTerrain( visible );

    repaint();
}

void MarbleWidget::setShowOtherPlaces( bool visible )
{
    d->setShowOtherPlaces( visible );

    repaint();
}

void MarbleWidget::setShowRelief( bool visible )
{
    d->setShowRelief( visible );

    repaint();
}

void MarbleWidget::setShowElevationModel( bool visible )
{
    d->setShowElevationModel( visible );

    repaint();
}

void MarbleWidget::setShowIceLayer( bool visible )
{
    d->setShowIceLayer( visible );

    repaint();
}

void MarbleWidget::setShowBorders( bool visible )
{
    d->setShowBorders( visible );

    repaint();
}

void MarbleWidget::setShowRivers( bool visible )
{
    d->setShowRivers( visible );

    repaint();
}

void MarbleWidget::setShowLakes( bool visible )
{
    d->setShowLakes( visible );

    repaint();
}

void MarbleWidget::setShowFrameRate( bool visible )
{
    d->m_showFrameRate = visible;

    repaint();
}

void MarbleWidget::setShowGps( bool visible )
{
    d->setShowGps( visible );

    repaint();
}

void MarbleWidget::setShowTileId( bool visible )
{
    d->setShowTileId( visible );
}

void MarbleWidget::notifyMouseClick( int x, int y)
{
    qreal  lon   = 0;
    qreal  lat   = 0;

    bool const valid = geoCoordinates( x, y, lon, lat, GeoDataCoordinates::Radian );

    if ( valid ) {
        emit mouseClickGeoPosition( lon, lat, GeoDataCoordinates::Radian );
    }
}

void MarbleWidget::openGpxFile( const QString &filename )
{
    addGeoDataFile( filename );
}

FileViewModel* MarbleWidget::fileViewModel() const
{
    return d->m_model->fileViewModel();
}

void MarbleWidget::clearPersistentTileCache()
{
    d->m_model->clearPersistentTileCache();
}

void MarbleWidget::setPersistentTileCacheLimit( quint64 kiloBytes )
{
    d->m_model->setPersistentTileCacheLimit( kiloBytes );
}

void MarbleWidget::clearVolatileTileCache()
{
    mDebug() << "About to clear VolatileTileCache";
    d->m_model->clearVolatileTileCache();
}

void MarbleWidget::setVolatileTileCacheLimit( quint64 kiloBytes )
{
    d->m_model->setVolatileTileCacheLimit( kiloBytes );
}

// This slot will called when the Globe starts to create the tiles.

void MarbleWidget::creatingTilesStart( TileCreator *creator,
                                       const QString &name, 
                                       const QString &description )
{
    TileCreatorDialog dlg( creator, this );
    dlg.setSummary( name, description );
    dlg.exec();
}

void MarbleWidget::updateChangedMap()
{
    d->setNeedsUpdate();

    update();
}

void MarbleWidget::scheduleRepaint( const QRegion& dirtyRegion )
{
    Q_UNUSED( dirtyRegion );
    if ( !d->m_repaintTimer.isActive() ) {
        d->m_repaintTimer.start();
    }
}

MapQuality MarbleWidget::mapQuality( ViewContext viewContext )
{
    if ( viewContext == Still )
        return d->m_stillQuality;

    Q_ASSERT( viewContext == Animation );
    return d->m_animationQuality; 
}

void MarbleWidget::setMapQuality( MapQuality quality, ViewContext changedViewContext )
{
    const MapQuality oldQuality = mapQuality( viewContext() );

    // FIXME: Rewrite as a switch
    if ( changedViewContext == Still ) {
        d->m_stillQuality = quality;
    }
    else if ( changedViewContext == Animation ) {
        d->m_animationQuality = quality;
    }

    if ( viewContext() == Still ) {
        d->setMapQuality( d->m_stillQuality );
    }
    else if ( viewContext() == Animation )
    {
        d->setMapQuality( d->m_animationQuality );
    }

    if ( mapQuality( viewContext() ) != oldQuality )
        repaint();
}

ViewContext MarbleWidget::viewContext() const
{
    return d->m_viewContext;
}

void MarbleWidget::setViewContext( ViewContext viewContext )
{
    d->m_viewContext = viewContext;

    if ( viewContext == Still )
        d->setMapQuality( d->m_stillQuality );
    if ( viewContext == Animation )
        d->setMapQuality( d->m_animationQuality );

    if ( mapQuality( viewContext ) != mapQuality( Animation ) )
        repaint();
}

bool MarbleWidget::animationsEnabled() const
{
    return d->m_animationsEnabled;
}

void MarbleWidget::setAnimationsEnabled( bool enabled )
{
    d->m_animationsEnabled = enabled;
}

AngleUnit MarbleWidget::defaultAngleUnit() const
{
    if ( GeoDataCoordinates::defaultNotation() == GeoDataCoordinates::Decimal ) {
        return DecimalDegree;
    }

    return DMSDegree;
}

void MarbleWidget::setDefaultAngleUnit( AngleUnit angleUnit )
{
    if ( angleUnit == DecimalDegree ) {
        GeoDataCoordinates::setDefaultNotation( GeoDataCoordinates::Decimal );
        return;
    }

    GeoDataCoordinates::setDefaultNotation( GeoDataCoordinates::DMS );
}

QFont MarbleWidget::defaultFont() const
{
    return GeoDataFeature::defaultFont();
}

void MarbleWidget::setDefaultFont( const QFont& font )
{
    GeoDataFeature::setDefaultFont( font );
    d->m_model->placemarkLayout()->requestStyleReset();
}

void MarbleWidget::setSelection( const QRect& region )
{
    QPoint tl = region.topLeft();
    QPoint br = region.bottomRight();
    mDebug() << "Selection region: (" << tl.x() << ", " <<  tl.y() << ") (" 
             << br.x() << ", " << br.y() << ")" << endl;

    AbstractProjection *proj = viewport()->currentProjection();
    GeoDataLatLonAltBox box  = proj->latLonAltBox( region, viewport() );

    // NOTE: coordinates as lon1, lat1, lon2, lat2 (or West, North, East, South)
    // as left/top, right/bottom rectangle.
    QList<double> coordinates;
    coordinates << box.west( GeoDataPoint::Degree ) << box.north( GeoDataPoint::Degree )
                << box.east( GeoDataPoint::Degree ) << box.south( GeoDataPoint::Degree );

    mDebug() << "West: " << coordinates[0] << " North: " <<  coordinates[1]
             << " East: " << coordinates[2] << " South: " << coordinates[3] << endl;

    emit regionSelected( coordinates );
}

qreal MarbleWidget::distance() const
{
    return d->m_model->distanceFromRadius( radius() );
}

void MarbleWidget::setDistance( qreal distance )
{
    d->setDistance( distance );
}

QString MarbleWidget::distanceString() const
{
    qreal dist = distance();
    QString distanceUnitString;

    const DistanceUnit distanceUnit = MarbleGlobal::getInstance()->locale()->distanceUnit();

    if ( distanceUnit == Meter ) {
        distanceUnitString = tr("km");
    }
    else {
        dist *= KM2MI;
        distanceUnitString = tr("mi");
    }

    return QString( "%L1 %2" ).arg( dist, 8, 'f', 1, QChar(' ') ).arg( distanceUnitString );
}

void MarbleWidget::updateSun()
{
    d->updateSun();
}

void MarbleWidget::centerSun()
{
    SunLocator  *sunLocator = d->m_model->sunLocator();

    if ( sunLocator && sunLocator->getCentered() ) {
        qreal  lon = sunLocator->getLon();
        qreal  lat = sunLocator->getLat();
        centerOn( lon, lat );

        setInputEnabled( false );
    }
}

SunLocator* MarbleWidget::sunLocator()
{
    return d->m_model->sunLocator();
}

void MarbleWidget::setInputEnabled( bool enabled )
{
    //if input is set as enabled
    if ( enabled )
    {
        if ( !d->m_inputhandler ) {
            setInputHandler( new MarbleWidgetDefaultInputHandler );
        }
        else {
            installEventFilter( d->m_inputhandler );
        }
    }

    else // input is disabled
    {
        mDebug() << "MarbleWidget::disableInput";
        removeEventFilter( d->m_inputhandler );
        setCursor( Qt::ArrowCursor );
    }
}

QList<RenderPlugin *> MarbleWidget::renderPlugins() const
{
    return d->m_model->renderPlugins();
}

void MarbleWidget::readPluginSettings( QSettings& settings )
{
    foreach( RenderPlugin *plugin, renderPlugins() ) {
        settings.beginGroup( QString( "plugin_" ) + plugin->nameId() );

        QHash<QString,QVariant> hash = plugin->settings();

        foreach ( const QString& key, settings.childKeys() ) {
            hash.insert( key, settings.value( key ) );
        }

        plugin->setSettings( hash );

        settings.endGroup();
    }
}

void MarbleWidget::writePluginSettings( QSettings& settings ) const
{
    foreach( RenderPlugin *plugin, renderPlugins() ) {
        settings.beginGroup( QString( "plugin_" ) + plugin->nameId() );

        QHash<QString,QVariant> hash = plugin->settings();

        QHash<QString,QVariant>::iterator it = hash.begin();
        while( it != hash.end() ) {
            settings.setValue( it.key(), it.value() );
            ++it;
        }

        settings.endGroup();
    }
}

QList<AbstractFloatItem *> MarbleWidget::floatItems() const
{
    return d->m_model->floatItems();
}

AbstractFloatItem * MarbleWidget::floatItem( const QString &nameId ) const
{
    return d->m_model->floatItem( nameId );
}

void MarbleWidget::changeEvent( QEvent * event )
{
    if ( event->type() == QEvent::EnabledChange )
    {
        setInputEnabled(isEnabled());
    }

    QWidget::changeEvent(event);
}

void MarbleWidget::flyTo( const GeoDataLookAt &newLookAt, FlyToMode mode )
{
    if ( !d->m_animationsEnabled || mode == Instant ) {
        d->flyTo( newLookAt );
    }
    else {
        setViewContext( Marble::Animation );
        d->m_physics->flyTo( lookAt(), newLookAt, viewport(), mode );
    }
}

void MarbleWidget::reloadMap()
{
    d->m_model->reloadMap();
}

void MarbleWidget::updateAnimation( const GeoDataLookAt &lookAt )
{
    setViewContext( Marble::Animation );
    d->flyTo( lookAt );

    repaint();
}

void MarbleWidget::onRadiusChanged( int /*radius*/ )
{
    emit zoomChanged( zoom() );
    emit distanceChanged( distanceString() );
}

void MarbleWidget::startStillMode()
{
    setViewContext( Marble::Still );
}

GeoDataLookAt MarbleWidget::lookAt() const
{
    GeoDataLookAt result;
    const qreal lon = centerLongitude() / RAD2DEG;
    const qreal lat = centerLatitude() / RAD2DEG;

    result.setLongitude( lon );
    result.setLatitude( lat );
    result.setAltitude( 0.0 );
    result.setRange( distance() * KM2METER );

    return result;
}

void MarbleWidget::addBookmark( const GeoDataPlacemark &bookmark, const QString &folderName ) const
{
    d->m_model->bookmarkManager()->addBookmark( bookmark, folderName );
}

QString MarbleWidget::bookmarkFile() const
{
    return d->m_model->bookmarkManager()->bookmarkFile();
}

bool MarbleWidget::loadBookmarkFile( const QString &relativeFileName )
{
    return d->m_model->bookmarkManager()->loadFile( relativeFileName );
}

QVector<GeoDataFolder*> MarbleWidget::bookmarkFolders()
{
    return d->m_model->bookmarkManager()->folders();
}

void MarbleWidget::removeAllBookmarks()
{
    d->m_model->bookmarkManager()->removeAllBookmarks();
}

void MarbleWidget::addNewBookmarkFolder( const QString &name ) const
{
    d->m_model->bookmarkManager()->addNewBookmarkFolder( name );
}

RoutingLayer* MarbleWidget::routingLayer()
{
    return d->m_routingLayer;
}

void MarbleWidget::addGeoDataFile( const QString &filename )
{
    //d->m_map->addGeoDataFile( filename );
    d->m_model->addGeoDataFile( filename );
}

void MarbleWidget::addGeoDataString( const QString &data, const QString &key )
{
    d->m_model->addGeoDataString( data, key );
}

void MarbleWidget::removeGeoData( const QString &key )
{
    d->m_model->removeGeoData( key );
}

}

#include "MarbleWidget.moc"
