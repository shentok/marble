//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2006-2009 Torsten Rahn <tackat@kde.org>
// Copyright 2007      Inge Wallin  <ingwa@kde.org>
// Copyright 2008      Carlos Licea <carlos.licea@kdemail.net>
// Copyright 2009      Jens-Michael Hoffmann <jensmh@gmx.de>
//


// Own
#include "MarbleMap.h"
#include "MarbleMap_p.h"

// Posix
#include <cmath>

// Qt
#include <QtCore/QAbstractItemModel>
#include <QtCore/QTime>
#include <QtGui/QItemSelectionModel>
#include <QtGui/QSizePolicy>
#include <QtGui/QRegion>

#ifdef MARBLE_DBUS
#include <QtDBus/QDBusConnection>
#endif

// Marble
#include "AbstractProjection.h"
#include "AbstractScanlineTextureMapper.h"
#include "GeoDataFeature.h"
#include "GeoDataLatLonAltBox.h"
#include "GeoPainter.h"
#include "GeoSceneDocument.h"
#include "GeoSceneHead.h"
#include "MarbleDebug.h"
#include "MarbleDirs.h"
#include "MarbleLocale.h"
#include "MarbleModel.h"
#include "MarblePlacemarkModel.h"
#include "MeasureTool.h"
#include "MergedLayerDecorator.h"
#include "PlacemarkLayout.h"
#include "Planet.h"
#include "RenderPlugin.h"
#include "SunLocator.h"
#include "TextureColorizer.h"
#include "ViewParams.h"
#include "ViewportParams.h"

using namespace Marble;

#ifdef Q_CC_MSVC
# ifndef KDEWIN_MATH_H
   static long double sqrt( int a ) { return sqrt( (long double)a ); }
# endif
#endif


MarbleMapPrivate::MarbleMapPrivate( MarbleMap *parent )
        : m_parent( parent ),
          m_viewAngle( 110.0 )
{
}

void MarbleMapPrivate::construct()
{
    m_parent->connect( m_model, SIGNAL( themeChanged( QString ) ),
                       m_parent, SIGNAL( themeChanged( QString ) ) );
    m_parent->connect( m_model, SIGNAL( modelChanged() ),
                       m_parent, SLOT( setNeedsUpdate() ) );

    m_justModified = false;

    m_logzoom  = 0;


    m_parent->connect( m_model->sunLocator(), SIGNAL( updateSun() ),
                       m_parent,              SLOT( updateSun() ) );
    m_parent->connect( m_model->sunLocator(), SIGNAL( centerSun() ),
                       m_parent,              SLOT( centerSun() ) );

    m_parent->connect( m_model,               SIGNAL( repaintNeeded( QRegion ) ),
                       m_parent,              SIGNAL( repaintNeeded( QRegion ) ) );
}

// Used to be resizeEvent()
void MarbleMapPrivate::doResize()
{
    QSize size( m_parent->width(), m_parent->height() );
    m_viewParams.viewport()->setSize( size );

    // If the globe covers fully the screen then we can use the faster
    // RGB32 as there are no translucent areas involved.
    QImage::Format imageFormat = ( m_parent->mapCoversViewport() )
                                 ? QImage::Format_RGB32
                                 : QImage::Format_ARGB32_Premultiplied;

    // Recreate the canvas image with the new size.
    m_viewParams.setCanvasImage( new QImage( m_parent->width(), m_parent->height(),
                                             imageFormat ));

    // Recreate the coastline detection offscreen image
    m_viewParams.setCoastImage( new QImage( m_parent->width(), m_parent->height(),
                                            QImage::Format_RGB32 ));

    m_justModified = true;
}

void  MarbleMapPrivate::paintMarbleSplash( GeoPainter &painter, QRect &dirtyRect )
{
    Q_UNUSED( dirtyRect )

    painter.save();

    QPixmap logoPixmap( MarbleDirs::path( "svg/marble-logo-inverted-72dpi.png" ) );

    if ( logoPixmap.width() > m_parent->width() * 0.7
         || logoPixmap.height() > m_parent->height() * 0.7 )
    {
        logoPixmap = logoPixmap.scaled( QSize( m_parent->width(), m_parent->height() ) * 0.7,
                                        Qt::KeepAspectRatio, Qt::SmoothTransformation );
    }

    QPoint logoPosition( ( m_parent->width()  - logoPixmap.width() ) / 2,
                            ( m_parent->height() - logoPixmap.height() ) / 2 );
    painter.drawPixmap( logoPosition, logoPixmap );

    QString message; // "Please assign a map theme!";

    painter.setPen( Qt::white );

    int yTop = logoPosition.y() + logoPixmap.height() + 10;
    QRect textRect( 0, yTop,
                    m_parent->width(), m_parent->height() - yTop );
    painter.drawText( textRect, Qt::AlignHCenter | Qt::AlignTop, message ); 

    painter.restore();
}

void MarbleMap::paintGround( GeoPainter &painter, QRect &dirtyRect )
{
    if ( !d->m_viewParams.mapTheme() ) {
        mDebug() << "No theme yet!";
        d->paintMarbleSplash( painter, dirtyRect );
        return;
    }

    d->m_model->paintGlobe( &painter, &d->m_viewParams,
                            d->m_justModified || d->m_viewParams.canvasImage()->isNull(),
                            dirtyRect );
    d->m_justModified = false;
}

void MarbleMap::paintOverlay( GeoPainter &painter, QRect &dirtyRect )
{
    Q_UNUSED( dirtyRect )

    if ( !d->m_viewParams.mapTheme() ) {
        return;
    }

    // FIXME: Add this stuff into the Layermanager as something to be 
    // called before the float items.

    bool antialiased = false;

    if (   d->m_viewParams.mapQuality() == HighQuality
        || d->m_viewParams.mapQuality() == PrintQuality ) {
            antialiased = true;
    }

    d->m_model->measureTool()->render( &painter, d->m_viewParams.viewport() );
}

// ----------------------------------------------------------------


MarbleMap::MarbleMap()
    : d( new MarbleMapPrivate( this ) )
{
#ifdef MARBLE_DBUS
    QDBusConnection::sessionBus().registerObject( "/MarbleMap", this, 
                                                  QDBusConnection::ExportAllSlots
                                                  | QDBusConnection::ExportAllSignals
                                                  | QDBusConnection::ExportAllProperties );
#endif
    QTime t;
    t.start();
    
    d->m_model        = new MarbleModel( this );
    d->m_modelIsOwned = true;

    d->construct();
    qDebug("Model: Time elapsed: %d ms", t.elapsed());
}

MarbleMap::MarbleMap(MarbleModel *model)
    : d( new MarbleMapPrivate( this ) )
{
//     QDBusConnection::sessionBus().registerObject( "/marble", this,
//                                                   QDBusConnection::ExportAllSlots );

    d->m_model        = model;
    d->m_modelIsOwned = false;

    d->construct();
}

MarbleMap::~MarbleMap()
{
    // Some basic initializations.
    d->m_width  = 0;
    d->m_height = 0;

    if ( d->m_modelIsOwned )
        delete d->m_model;
    delete d;
}

MarbleModel *MarbleMap::model() const
{
    return d->m_model;
}

ViewportParams *MarbleMap::viewport()
{
    return d->m_viewParams.viewport();
}


void MarbleMap::setMapQuality( MapQuality quality )
{
    d->m_viewParams.setMapQuality( quality );
    // Update texture map during the repaint that follows:
    setNeedsUpdate();
}

MapQuality MarbleMap::mapQuality() const
{
    return d->m_viewParams.mapQuality();
}


void MarbleMap::setSize( int width, int height )
{
    d->m_width  = width;
    d->m_height = height;

    d->doResize();
    emit visibleLatLonAltBoxChanged( d->m_viewParams.viewport()->viewLatLonAltBox() );
}

void MarbleMap::setSize( const QSize& size )
{
    d->m_width  = size.width();
    d->m_height = size.height();

    d->doResize();
    emit visibleLatLonAltBoxChanged( d->m_viewParams.viewport()->viewLatLonAltBox() );
}

QSize MarbleMap::size() const
{
    return QSize( d->m_width, d->m_height );
}

int  MarbleMap::width() const
{
    return d->m_width;
}

int  MarbleMap::height() const
{
    return d->m_height;
}

Quaternion MarbleMap::planetAxis() const
{
    return d->m_viewParams.planetAxis();
}


int MarbleMap::radius() const
{
    return d->m_viewParams.radius();
}

void MarbleMap::setRadius( int radius )
{
    d->m_viewParams.setRadius( radius );

    setNeedsUpdate();

    d->m_logzoom = qRound( MarbleModel::zoomFromRadius( radius ) );
    emit zoomChanged( d->m_logzoom );
    emit distanceChanged( distanceString() );
    emit visibleLatLonAltBoxChanged( d->m_viewParams.viewport()->viewLatLonAltBox() );
}

void MarbleMap::setNeedsUpdate()
{
    d->m_justModified = true;
}


int MarbleMap::zoom() const
{
    return d->m_logzoom;
}


qreal MarbleMap::distance() const
{
    return d->m_model->distanceFromRadius(radius());
}

void MarbleMap::setDistance( qreal newDistance )
{
    qreal minDistance = 0.001;

    if ( newDistance <= minDistance ) {
        mDebug() << "Invalid distance: 0 m";
        newDistance = minDistance;
    }    

    int newRadius = d->m_model->radiusFromDistance( newDistance );
    setRadius( newRadius );
}

qreal MarbleMap::centerLatitude() const
{
    // Calculate translation of center point
    qreal  centerLon;
    qreal  centerLat;

    d->m_viewParams.centerCoordinates( centerLon, centerLat );
    return centerLat * RAD2DEG;
}

qreal MarbleMap::centerLongitude() const
{
    // Calculate translation of center point
    qreal  centerLon;
    qreal  centerLat;

    d->m_viewParams.centerCoordinates( centerLon, centerLat );
    return centerLon * RAD2DEG;
}

int  MarbleMap::minimumZoom() const
{
    return d->m_model->minimumZoom();
}

int  MarbleMap::maximumZoom() const
{
    return d->m_model->maximumZoom();
}

QPixmap MarbleMap::mapScreenShot()
{
    QPixmap screenshotPixmap( size() );
    screenshotPixmap.fill( Qt::transparent );

    GeoPainter painter( &screenshotPixmap, viewport(),
                        PrintQuality );
    painter.begin( &screenshotPixmap );
    QRect dirtyRect( QPoint(), size() );
    paint( painter, dirtyRect );
    painter.end();

    return screenshotPixmap;
}

bool MarbleMap::propertyValue( const QString& name ) const
{
    bool value;
    d->m_viewParams.propertyValue( name, value );
    return value;
}

bool MarbleMap::showOverviewMap() const
{
    return propertyValue( "overviewmap" );
}

bool MarbleMap::showScaleBar() const
{
    return propertyValue( "scalebar" );
}

bool MarbleMap::showCompass() const
{
    return propertyValue( "compass" );
}

bool MarbleMap::showGrid() const
{
    return propertyValue( "coordinate-grid" );
}

bool MarbleMap::showClouds() const
{
    return d->m_viewParams.showClouds();
}

bool MarbleMap::showAtmosphere() const
{
    return d->m_viewParams.showAtmosphere();
}

bool MarbleMap::showPlaces() const
{
    return propertyValue( "places" );
}

bool MarbleMap::showCities() const
{
    return propertyValue( "cities" );
}

bool MarbleMap::showTerrain() const
{
    return propertyValue( "terrain" );
}

bool MarbleMap::showOtherPlaces() const
{
    return propertyValue( "otherplaces" );
}

bool MarbleMap::showRelief() const
{
    return propertyValue( "relief" );
}

bool MarbleMap::showElevationModel() const
{
    return d->m_viewParams.showElevationModel();
}

bool MarbleMap::showIceLayer() const
{
    return propertyValue( "ice" );
}

bool MarbleMap::showBorders() const
{
    return propertyValue( "borders" );
}

bool MarbleMap::showRivers() const
{
    return propertyValue( "rivers" );
}

bool MarbleMap::showLakes() const
{
    return propertyValue( "lakes" );
}

bool MarbleMap::showGps() const
{
    return d->m_viewParams.showGps();
}

void MarbleMap::zoomView( int newZoom )
{
    // Check for under and overflow.
    if ( newZoom < minimumZoom() )
        newZoom = minimumZoom();
    else if ( newZoom > maximumZoom() )
        newZoom = maximumZoom();

    // Prevent infinite loops.
    if ( newZoom  == d->m_logzoom )
        return;

    d->m_viewParams.setRadius( MarbleModel::radiusFromZoom( newZoom ) );

    setNeedsUpdate();

    d->m_logzoom = newZoom;
    emit zoomChanged( d->m_logzoom );
    emit distanceChanged( distanceString() );
    emit visibleLatLonAltBoxChanged( d->m_viewParams.viewport()->viewLatLonAltBox() );
}


void MarbleMap::zoomViewBy( int zoomStep )
{
    zoomView( zoom() + zoomStep );
}


void MarbleMap::rotateBy( const Quaternion& incRot )
{
    d->m_viewParams.setPlanetAxis( incRot * d->m_viewParams.planetAxis() );
    setNeedsUpdate();
    emit visibleLatLonAltBoxChanged( d->m_viewParams.viewport()->viewLatLonAltBox() );
}

void MarbleMap::rotateBy( const qreal& deltaLon, const qreal& deltaLat )
{
    Quaternion  rotPhi( 1.0, deltaLat / 180.0, 0.0, 0.0 );
    Quaternion  rotTheta( 1.0, 0.0, deltaLon / 180.0, 0.0 );

    Quaternion  axis = d->m_viewParams.planetAxis();
    axis = rotTheta * axis;
    axis *= rotPhi;
    axis.normalize();
    d->m_viewParams.setPlanetAxis( axis );
    setNeedsUpdate();
    emit visibleLatLonAltBoxChanged( d->m_viewParams.viewport()->viewLatLonAltBox() );
}


void MarbleMap::centerOn( const qreal lon, const qreal lat )
{
    Quaternion  quat;
    quat.createFromEuler( -lat * DEG2RAD, lon * DEG2RAD, 0.0 );
    d->m_viewParams.setPlanetAxis( quat );
    setNeedsUpdate();
    emit visibleLatLonAltBoxChanged( d->m_viewParams.viewport()->viewLatLonAltBox() );
}

void MarbleMap::centerOn( const QModelIndex& index )
{
    QItemSelectionModel *selectionModel = d->m_model->placemarkSelectionModel();
    Q_ASSERT( selectionModel );

    selectionModel->clear();

    if ( index.isValid() ) {
        const GeoDataCoordinates point =
            index.data( MarblePlacemarkModel::CoordinateRole ).value<GeoDataCoordinates>();
  
        qreal  lon;
        qreal  lat;
        point.geoCoordinates( lon, lat );

        centerOn( lon * RAD2DEG, lat * RAD2DEG );

        selectionModel->select( index, QItemSelectionModel::SelectCurrent );
    }
}


void MarbleMap::setCenterLatitude( qreal lat )
{
    centerOn( centerLongitude(), lat );
}

void MarbleMap::setCenterLongitude( qreal lon )
{
    centerOn( lon, centerLatitude() );
}

Projection MarbleMap::projection() const
{
    return d->m_viewParams.projection();
}

void MarbleMap::setProjection( Projection projection )
{
    d->m_viewParams.setProjection( projection );

    d->m_model->setupTextureMapper( projection );

    // Update texture map during the repaint that follows:
    setNeedsUpdate();
    emit visibleLatLonAltBoxChanged( d->m_viewParams.viewport()->viewLatLonAltBox() );
}


bool MarbleMap::screenCoordinates( qreal lon, qreal lat,
                                   qreal& x, qreal& y ) const
{
    return d->m_viewParams.currentProjection()
        ->screenCoordinates( lon * DEG2RAD, lat * DEG2RAD,
                             d->m_viewParams.viewport(),
                             x, y );
}

bool MarbleMap::geoCoordinates( int x, int y,
                                qreal& lon, qreal& lat,
                                GeoDataCoordinates::Unit unit ) const
{
    return d->m_viewParams.currentProjection()
        ->geoCoordinates( x, y, d->m_viewParams.viewport(),
                          lon, lat, unit );
}

// Used to be paintEvent()
void MarbleMap::paint( GeoPainter &painter, QRect &dirtyRect )
{
    QTime t;
    t.start();
    
    paintGround( painter, dirtyRect );
    customPaint( &painter );
    d->m_model->measureTool()->render( &painter, viewport() );

    qreal fps = 1000.0 / (qreal)( t.elapsed() );
    emit framesPerSecond( fps );
}

void MarbleMap::customPaint( GeoPainter *painter )
{
    Q_UNUSED( painter );

    if ( !d->m_viewParams.mapTheme() ) {
        return;
    }
}

void MarbleMap::setMapThemeId( const QString& mapThemeId )
{
    if ( !mapThemeId.isEmpty() && mapThemeId == d->m_model->mapThemeId() )
        return;

    d->m_viewParams.setMapThemeId( mapThemeId );
    GeoSceneDocument *mapTheme = d->m_viewParams.mapTheme();

    if ( mapTheme ) {
        d->m_model->setMapTheme( mapTheme, d->m_viewParams.projection() );

        d->doResize();

        centerSun();
    }

    // Update texture map during the repaint that follows:
    setNeedsUpdate();
}

void MarbleMap::setPropertyValue( const QString& name, bool value )
{
    mDebug() << "In MarbleMap the property " << name << "was set to " << value;
    d->m_viewParams.setPropertyValue( name, value );
    setNeedsUpdate();
}

void MarbleMap::setShowOverviewMap( bool visible )
{
    setPropertyValue( "overviewmap", visible );
}

void MarbleMap::setShowScaleBar( bool visible )
{
    setPropertyValue( "scalebar", visible );
}

void MarbleMap::setShowCompass( bool visible )
{
    setPropertyValue( "compass", visible );
}

void MarbleMap::setShowAtmosphere( bool visible )
{
    bool previousVisible = showAtmosphere();

    if ( visible != previousVisible ) {
        d->m_viewParams.setShowAtmosphere( visible );

        // Quick and dirty way to force a whole update of the view
        d->doResize();
    }
}

void MarbleMap::setShowClouds( bool visible )
{
    d->m_viewParams.setShowClouds( visible );
    setNeedsUpdate();
}

void MarbleMap::setShowTileId( bool visible )
{
    d->m_model->setShowTileId( visible );
}

void MarbleMap::setShowGrid( bool visible )
{
    setPropertyValue( "coordinate-grid", visible );
}

void MarbleMap::setShowPlaces( bool visible )
{
    setPropertyValue( "places", visible );
}

void MarbleMap::setShowCities( bool visible )
{
    setPropertyValue( "cities", visible );
}

void MarbleMap::setShowTerrain( bool visible )
{
    setPropertyValue( "terrain", visible );
}

void MarbleMap::setShowOtherPlaces( bool visible )
{
    setPropertyValue( "otherplaces", visible );
}

void MarbleMap::setShowRelief( bool visible )
{
    setPropertyValue( "relief", visible );
    // Update texture map during the repaint that follows:
    setNeedsUpdate();
}

void MarbleMap::setShowElevationModel( bool visible )
{
    d->m_viewParams.setShowElevationModel( visible );
    // Update texture map during the repaint that follows:
    setNeedsUpdate();
}

void MarbleMap::setShowIceLayer( bool visible )
{
    setPropertyValue( "ice", visible );
    // Update texture map during the repaint that follows:
    setNeedsUpdate();
}

void MarbleMap::setShowBorders( bool visible )
{
    setPropertyValue( "borders", visible );
}

void MarbleMap::setShowRivers( bool visible )
{
    setPropertyValue( "rivers", visible );
}

void MarbleMap::setShowLakes( bool visible )
{
    setPropertyValue( "lakes", visible );
    // Update texture map during the repaint that follows:
    setNeedsUpdate();
}

void MarbleMap::setShowGps( bool visible )
{
    d->m_viewParams.setShowGps( visible );
}

void MarbleMap::notifyMouseClick( int x, int y )
{
    qreal  lon   = 0;
    qreal  lat   = 0;

    const bool valid = geoCoordinates( x, y, lon, lat, GeoDataCoordinates::Radian );

    if ( valid ) {
        emit mouseClickGeoPosition( lon, lat, GeoDataCoordinates::Radian );
    }
}

QString MarbleMap::distanceString() const
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


bool MarbleMap::mapCoversViewport()
{
    return d->m_viewParams.viewport()->mapCoversViewport();
}

void MarbleMap::updateSun()
{
    // Update the sun shading.
    //SunLocator  *sunLocator = d->m_model->sunLocator();

    mDebug() << "MarbleMap: Updating the sun shading map...";
    d->m_model->update();
    setNeedsUpdate();
    //mDebug() << "Finished updating the sun shading map";
}

void MarbleMap::centerSun()
{
    SunLocator  *sunLocator = d->m_model->sunLocator();

    if ( sunLocator && sunLocator->getCentered() ) {
        qreal  lon = sunLocator->getLon();
        qreal  lat = sunLocator->getLat();
        centerOn( lon, lat );

        mDebug() << "Centering on Sun at " << lat << lon;
    }
}

GeoDataLookAt MarbleMap::lookAt() const
{
    GeoDataLookAt result;
    qreal lon( 0.0 ), lat( 0.0 );

    d->m_viewParams.centerCoordinates( lon, lat );
    result.setLongitude( lon );
    result.setLatitude( lat );
    result.setAltitude( 0.0 );
    result.setRange( distance() * KM2METER );

    return result;
}

#include "MarbleMap.moc"
