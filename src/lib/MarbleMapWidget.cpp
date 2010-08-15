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

#include "MarbleMapWidget.h"

#include <cmath>

#include <QtCore/QAbstractItemModel>
#include <QtCore/QHash>
#include <QtCore/QSettings>
#include <QtCore/QTime>
#include <QtCore/QTimer>
#include <QtGui/QItemSelectionModel>
#include <QtGui/QPaintEvent>
#include <QtGui/QRegion>
#include <QtGui/QSizePolicy>
#include <QtNetwork/QNetworkProxy>

#include "AbstractProjection.h"
#include "DataMigration.h"
#include "FileViewModel.h"
#include "GeoDataLatLonAltBox.h"
#include "GeoPainter.h"
#include "MarbleDebug.h"
#include "MarbleDirs.h"
#include "MarbleMap.h"
#include "MarbleModel.h"
#include "MarblePhysics.h"
#include "MarblePlacemarkModel.h"
#include "MarbleWidget.h"
#include "MeasureTool.h"
#include "MergedLayerDecorator.h"
#include "RenderPlugin.h"
#include "SunLocator.h"
#include "TileCreatorDialog.h"
#include "ViewParams.h"
#include "ViewportParams.h"
#include "PositionTracking.h"
#include "BookmarkManager.h"
namespace Marble
{

#ifdef Q_CC_MSVC
# ifndef KDEWIN_MATH_H
   static long double sqrt( int a ) { return sqrt( (long double)a ); }
# endif
#endif


class MarbleMapWidget::Private
{
 public:
    Private( MarbleMap *map, MarbleMapWidget *parent )
        : m_widget( parent ),
          m_map( map ),
          m_model( map->model() ),
          m_showFrameRate( false )
    {
        // Initialize the map and forward some signals.
        map->setSize( m_widget->width(), m_widget->height() );

        // When some fundamental things change in the map, we got to show
        // this in the view, i.e. here.
        m_widget->connect( map,      SIGNAL( radiusChanged( int ) ),
                           m_widget, SIGNAL( radiusChanged( int ) ) );
        m_widget->connect( map,      SIGNAL( visibleLatLonAltBoxChanged( GeoDataLatLonAltBox )),
                           m_widget, SIGNAL( visibleLatLonAltBoxChanged( GeoDataLatLonAltBox )));
    }

    ~Private()
    {
        delete m_map;
    }

    /**
      * @brief Update widget flags and cause a full update
      *
      * The background of the widget only needs to be redrawn in certain cases. This
      * method sets the widget flags accordingly and triggers a update.
      */
    void update();

    void paintFps( GeoPainter &painter, QRect &dirtyRect, qreal fps);

    MarbleMapWidget *m_widget;
    // The model we are showing.
    MarbleMap       *m_map;
    MarbleModel     *m_model;   // Owned by m_map.  Don't delete.

    bool             m_showFrameRate;
};



MarbleMapWidget::MarbleMapWidget(MarbleMap *map, MarbleWidget *parent)
    : QWidget( parent ),
      d( new MarbleMapWidget::Private( map, this ) )
{
    // Set background: black.
    setPalette( QPalette ( Qt::black ) );

    // Set whether the black space gets displayed or the earth gets simply 
    // displayed on the widget background.
    setAutoFillBackground( true );
}

MarbleMapWidget::~MarbleMapWidget()
{
    delete d;
}

void MarbleMapWidget::Private::update()
{
    // We only have to repaint the background every time if the earth
    // doesn't cover the whole image.
    m_widget->setAttribute( Qt::WA_NoSystemBackground,
                  m_widget->viewport()->mapCoversViewport() && !m_model->mapThemeId().isEmpty() );

    m_widget->update();
}

void MarbleMapWidget::Private::paintFps( GeoPainter &painter, QRect &dirtyRect, qreal fps )
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


MarbleModel *MarbleMapWidget::model() const
{
    return d->m_model;
}


ViewportParams* MarbleMapWidget::viewport()
{
    return d->m_map->viewport();
}

const ViewportParams* MarbleMapWidget::viewport() const
{
    return d->m_map->viewport();
}


int MarbleMapWidget::radius() const
{
    return d->m_map->radius();
}

void MarbleMapWidget::setRadius( int newRadius )
{
    if ( newRadius == radius() ) {
        return;
    }
        
    d->m_map->setRadius( newRadius );
    d->update();
}


bool MarbleMapWidget::showOverviewMap() const
{
    return d->m_map->showOverviewMap();
}

bool MarbleMapWidget::showScaleBar() const
{
    return d->m_map->showScaleBar();
}

bool MarbleMapWidget::showCompass() const
{
    return d->m_map->showCompass();
}

bool MarbleMapWidget::showClouds() const
{
    return d->m_map->showClouds();
}

bool MarbleMapWidget::showAtmosphere() const
{
    return d->m_map->showAtmosphere();
}

bool MarbleMapWidget::showCrosshairs() const
{
    return d->m_model->showCrosshairs();
}

bool MarbleMapWidget::showGrid() const
{
    return d->m_map->showGrid();
}

bool MarbleMapWidget::showPlaces() const
{
    return d->m_map->showPlaces();
}

bool MarbleMapWidget::showCities() const
{
    return d->m_map->showCities();
}

bool MarbleMapWidget::showTerrain() const
{
    return d->m_map->showTerrain();
}

bool MarbleMapWidget::showOtherPlaces() const
{
    return d->m_map->showOtherPlaces();
}

bool MarbleMapWidget::showRelief() const
{
    return d->m_map->showRelief();
}

bool MarbleMapWidget::showElevationModel() const
{
    return d->m_map->showElevationModel();
}

bool MarbleMapWidget::showIceLayer() const
{
    return d->m_map->showIceLayer();
}

bool MarbleMapWidget::showBorders() const
{
    return d->m_map->showBorders();
}

bool MarbleMapWidget::showRivers() const
{
    return d->m_map->showRivers();
}

bool MarbleMapWidget::showLakes() const
{
    return d->m_map->showLakes();
}

bool MarbleMapWidget::showGps() const
{
    return d->m_map->showGps();
}

bool MarbleMapWidget::showFrameRate() const
{
    return d->m_showFrameRate;
}


void MarbleMapWidget::zoomView( int newZoom )
{
    d->m_map->zoomView( newZoom );
    d->update();
}


void MarbleMapWidget::rotateBy( const Quaternion& incRot )
{
    d->m_map->rotateBy( incRot );
    d->update();
}


void MarbleMapWidget::centerOn( const qreal lon, const qreal lat )
{
    d->m_map->centerOn( lon, lat );
    d->update();
}

Projection MarbleMapWidget::projection() const
{
    return d->m_map->projection();
}

void MarbleMapWidget::setProjection( Projection projection )
{
    d->m_map->setProjection( projection );
    d->update();
}


void MarbleMapWidget::resizeEvent( QResizeEvent* )
{
    setUpdatesEnabled( false );
    d->m_map->setSize( width(), height() );
    d->update();
    setUpdatesEnabled( true );
}

qreal MarbleMapWidget::centerLatitude() const
{
    return d->m_map->centerLatitude();
}

qreal MarbleMapWidget::centerLongitude() const
{
    return d->m_map->centerLongitude();
}

void MarbleMapWidget::paintEvent( QPaintEvent *evt )
{
    // Stop repaint timer if it is already running
    QTime t;
    t.start();

    // FIXME: Better way to get the GeoPainter
    bool  doClip = true;
    if ( d->m_map->projection() == Spherical )
        doClip = ( radius() > width() / 2
                   || radius() > height() / 2 );

    QPaintDevice *paintDevice = this;
    QImage image;
    if (!isEnabled())
    {
        // If the globe covers fully the screen then we can use the faster
        // RGB32 as there are no translucent areas involved.
        QImage::Format imageFormat = ( d->m_map->mapCoversViewport() )
                                     ? QImage::Format_RGB32
                                     : QImage::Format_ARGB32_Premultiplied;
        // Paint to an intermediate image
        image = QImage( rect().size(), imageFormat );
        image.fill( Qt::transparent );
        paintDevice = &image;
    }

    // Create a painter that will do the painting.
    GeoPainter painter( paintDevice, viewport(),
                        d->m_map->mapQuality(), doClip );

    QRect  dirtyRect = evt->rect();

    // Draws the map like MarbleMap::paint does, but adds our customPaint in between
    d->m_map->paintGround( painter, dirtyRect );
    d->m_map->customPaint( &painter );
    d->m_model->measureTool()->render( &painter, viewport() );

    if ( !isEnabled() )
    {
        // Draw a grayscale version of the intermediate image
        QRgb* pixel = reinterpret_cast<QRgb*>( image.scanLine( 0 ));
        for (int i=0; i<image.width()*image.height(); ++i, ++pixel) {
            int gray = qGray( *pixel );
            *pixel = qRgb( gray, gray, gray );
        }

        GeoPainter widgetPainter( this, viewport(),
                            d->m_map->mapQuality(), doClip );
        widgetPainter.drawImage( rect(), image );
    }

    if ( showFrameRate() )
    {
        qreal fps = 1000.0 / (qreal)( t.elapsed() + 1 );
        d->paintFps( painter, dirtyRect, fps );
        emit d->m_widget->framesPerSecond( fps );
    }
}


void MarbleMapWidget::setMapThemeId( const QString& mapThemeId )
{
    if ( !mapThemeId.isEmpty() && mapThemeId == d->m_model->mapThemeId() )
        return;
    
    d->m_map->setMapThemeId( mapThemeId );

    // Now we want a full repaint as the atmosphere might differ
    setAttribute( Qt::WA_NoSystemBackground,
                  false );

    centerSun();

    update();
}

void MarbleMapWidget::setPropertyValue( const QString& name, bool value )
{
    mDebug() << "In MarbleWidget the property " << name << "was set to " << value;
    d->m_map->setPropertyValue( name, value );

    update();
}

void MarbleMapWidget::setShowOverviewMap( bool visible )
{
    d->m_map->setShowOverviewMap( visible );

    update();
}

void MarbleMapWidget::setShowScaleBar( bool visible )
{
    d->m_map->setShowScaleBar( visible );

    update();
}

void MarbleMapWidget::setShowCompass( bool visible )
{
    d->m_map->setShowCompass( visible );

    update();
}

void MarbleMapWidget::setShowClouds( bool visible )
{
    d->m_map->setShowClouds( visible );

    update();
}

void MarbleMapWidget::setShowAtmosphere( bool visible )
{
    d->m_map->setShowAtmosphere( visible );

    update();
}

void MarbleMapWidget::setShowCrosshairs( bool visible )
{
    d->m_model->setShowCrosshairs( visible );

    update();
}

void MarbleMapWidget::setShowGrid( bool visible )
{
    d->m_map->setShowGrid( visible );

    update();
}

void MarbleMapWidget::setShowPlaces( bool visible )
{
    d->m_map->setShowPlaces( visible );

    update();
}

void MarbleMapWidget::setShowCities( bool visible )
{
    d->m_map->setShowCities( visible );

    update();
}

void MarbleMapWidget::setShowTerrain( bool visible )
{
    d->m_map->setShowTerrain( visible );

    update();
}

void MarbleMapWidget::setShowOtherPlaces( bool visible )
{
    d->m_map->setShowOtherPlaces( visible );

    update();
}

void MarbleMapWidget::setShowRelief( bool visible )
{
    d->m_map->setShowRelief( visible );

    update();
}

void MarbleMapWidget::setShowElevationModel( bool visible )
{
    d->m_map->setShowElevationModel( visible );

    update();
}

void MarbleMapWidget::setShowIceLayer( bool visible )
{
    d->m_map->setShowIceLayer( visible );

    update();
}

void MarbleMapWidget::setShowBorders( bool visible )
{
    d->m_map->setShowBorders( visible );

    update();
}

void MarbleMapWidget::setShowRivers( bool visible )
{
    d->m_map->setShowRivers( visible );

    update();
}

void MarbleMapWidget::setShowLakes( bool visible )
{
    d->m_map->setShowLakes( visible );

    update();
}

void MarbleMapWidget::setShowFrameRate( bool visible )
{
    d->m_showFrameRate = visible;

    update();
}

void MarbleMapWidget::setShowGps( bool visible )
{
    d->m_map->setShowGps( visible );

    update();
}

void MarbleMapWidget::setShowTileId( bool visible )
{
    d->m_map->setShowTileId( visible );
}

void MarbleMapWidget::setNeedsUpdate()
{
    d->m_map->setNeedsUpdate();

    d->update();
}

MapQuality MarbleMapWidget::mapQuality()
{
    return d->m_map->mapQuality();
}

void MarbleMapWidget::setMapQuality( MapQuality mapQuality )
{
    d->m_map->setMapQuality( mapQuality );

    d->update();
}

qreal MarbleMapWidget::distance() const
{
    return d->m_map->distance();
}

void MarbleMapWidget::updateSun()
{
    d->m_map->updateSun();

    d->update();
}

void MarbleMapWidget::centerSun()
{
    d->m_map->centerSun();

    d->update();
}

}

#include "MarbleMapWidget.moc"
