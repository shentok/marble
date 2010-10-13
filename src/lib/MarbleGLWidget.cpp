//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
// Copyright 2006-2007 Torsten Rahn <tackat@kde.org>
// Copyright 2007      Inge Wallin  <ingwa@kde.org>
//

#include "MarbleGLWidget.h"
#include "MarbleGLWidget.moc"

#include <cmath>

#include <QtCore/QTime>
#include <QtCore/QTimer>

#include "AbstractProjection.h"
#include "GeoPainter.h"
#include "GeoSceneDocument.h"
#include "GeoSceneMap.h"
#include "GeoSceneTexture.h"
#include "MarbleDebug.h"
#include "MarbleHeight.h"
#include "MarbleModel.h"
#include "PlacemarkLayout.h"
#include "SunLocator.h"
#include "TileLoaderHelper.h"
#include "TileId.h"
#include "VectorComposer.h"
#include "ViewParams.h"
#include "ViewportParams.h"

namespace Marble
{

#ifdef Q_CC_MSVC
# ifndef KDEWIN_MATH_H
   static long double sqrt( int a ) { return sqrt( (long double)a ); }
# endif
#endif


class MarbleGLWidget::Private
{
 public:
    Private( MarbleModel *model, MarbleGLWidget *widget )
        : m_widget( widget ),
          m_model( model ),
          m_height( model->downloadManager() ),
          m_showFrameRate( false ),
          m_showTileId( false ),
          m_previousLevel( -1 )
    {
        // Widget settings
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
        m_widget->connect( m_model, SIGNAL( repaintNeeded(QRegion) ),
                           m_widget, SLOT( update() ));
        m_widget->connect( m_model, SIGNAL( modelChanged() ),
                           m_widget, SLOT( update() ));

        // Set background: black.
        m_widget->setPalette( QPalette ( Qt::black ) );

        // Set whether the black space gets displayed or the earth gets simply 
        // displayed on the widget background.
        m_widget->setAutoFillBackground( true );

        m_widget->connect( m_model->sunLocator(), SIGNAL( updateStars() ),
                        m_widget, SLOT( update() ) );

        m_widget->connect( m_model->sunLocator(), SIGNAL( centerSun() ),
                        m_widget, SLOT( centerSun() ) );

        m_widget->setMouseTracking( m_widget );
    }

    ~Private()
    {
    }

    /**
      * @brief Update widget flags and cause a full repaint
      *
      * The background of the widget only needs to be redrawn in certain cases. This
      * method sets the widget flags accordingly and triggers a repaint.
      */
    void update();

    void paintFps( GeoPainter &painter, QRect &dirtyRect, qreal fps);

    void setPropertyValue( const QString &name, bool value );
    bool propertyValue( const QString &name );

    void geoCoordinates( const qreal x, const qreal y, qreal &lon, qreal &lat ) const;
    void projectionCoordinates( qreal lon, qreal lat, qreal &x, qreal &y ) const;

    MarbleGLWidget *const m_widget;
    MarbleModel    *const m_model;
    MarbleHeight          m_height;

    ViewParams m_viewParams;
    bool       m_showFrameRate;
    bool       m_showTileId;

    QList<Tile> m_tiles;
    QList<TileId> m_tileQueue;
    QTimer      m_tileQueueTimer;

    int m_previousLevel;
};


class MarbleGLWidget::Tile
{
public:
    Tile( const TileId &id, GLuint glName );

    TileId id() const { return m_id; }
    GLuint glName() const { return m_glName; }
    bool operator==( const Tile &other );

private:
    TileId m_id;
    GLuint m_glName;
};


MarbleGLWidget::Tile::Tile( const Marble::TileId &id, GLuint glName )
    : m_id( id )
    , m_glName( glName )
{
}

bool MarbleGLWidget::Tile::operator==(const Tile &other)
{
    return m_id == other.id() && m_glName == other.glName();
}


void MarbleGLWidget::renderTile( const Tile &tile )
{
    static const int NumLatitudes = 10;
    static const int NumLongitudes = 10;

    const GeoSceneTexture *const textureLayer = d->m_model->textureLayer();

    const int numXTiles = TileLoaderHelper::levelToColumn( textureLayer->levelZeroColumns(), tile.id().zoomLevel() );
    const int numYTiles = TileLoaderHelper::levelToRow( textureLayer->levelZeroRows(), tile.id().zoomLevel() );

    glBindTexture( GL_TEXTURE_2D, tile.glName() );

    for (int row = 0; row < NumLatitudes; row++) {
        glBegin( GL_TRIANGLE_STRIP );
        for (int col = 0; col <= NumLongitudes; col++){
            const qreal x  = (tile.id().x() * NumLongitudes + col    ) * 1.0 / (NumLongitudes*numXTiles);
            const qreal y1 = (tile.id().y() * NumLatitudes  + row    ) * 1.0 / (NumLatitudes *numYTiles);
            const qreal y2 = (tile.id().y() * NumLatitudes  + row + 1) * 1.0 / (NumLatitudes *numYTiles);

            qreal lon, lat;

            d->geoCoordinates( x, y1, lon, lat );

            double const w0 = radius( lon, lat ) * sin(lon) * cos(lat);    //x
            double const w1 = radius( lon, lat )            * sin(lat);    //y
            double const w2 = radius( lon, lat ) * cos(lon) * cos(lat);    //z

            glTexCoord2d(col*1.0/NumLatitudes, row*1.0/NumLongitudes);
            glVertex3d(w0, w1, w2);

            d->geoCoordinates( x, y2, lon, lat );

            double const x0 = radius( lon, lat ) * sin(lon) * cos(lat);    //x
            double const x1 = radius( lon, lat )            * sin(lat);    //y
            double const x2 = radius( lon, lat ) * cos(lon) * cos(lat);    //z

            glTexCoord2d(col*1.0/NumLatitudes, (row+1)*1.0/NumLongitudes);
            glVertex3d(x0, x1, x2);
        }
        glEnd();
    }
}


MarbleGLWidget::MarbleGLWidget( MarbleModel *model, QWidget *parent )
    : QGLWidget( parent ),
      d( new MarbleGLWidget::Private( model, this ) )
{
//    setAttribute( Qt::WA_PaintOnScreen, true );
    connect( this, SIGNAL( visibleLatLonAltBoxChanged( const GeoDataLatLonAltBox & ) ),
             this, SLOT( updateTiles() ) );
    connect( model, SIGNAL( tileUpdateAvailable( const TileId & ) ),
             this, SLOT( tileUpdated( const TileId & ) ) );

    d->m_tileQueueTimer.setSingleShot( true );
    connect( &d->m_tileQueueTimer, SIGNAL( timeout() ),
             this, SLOT( processNextTile() ) );

    setAutoFillBackground( false );
    setAutoBufferSwap( true );
}

MarbleGLWidget::~MarbleGLWidget()
{
    delete d;
}

void MarbleGLWidget::Private::update()
{
    m_widget->setAttribute( Qt::WA_NoSystemBackground, true );
    m_widget->update();
}

void MarbleGLWidget::Private::paintFps( GeoPainter &painter, QRect &dirtyRect, qreal fps )
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

bool MarbleGLWidget::Private::propertyValue( const QString &name )
{
    bool value;
    m_viewParams.propertyValue( name, value );

    return value;
}

void MarbleGLWidget::Private::setPropertyValue( const QString &name, bool value )
{
    mDebug() << "In MarbleGLWidget the property " << name << "was set to " << value;
    m_viewParams.setPropertyValue( name, value );

    update();
}

void MarbleGLWidget::Private::geoCoordinates( qreal normalizedX, qreal normalizedY,
                                              qreal& lon, qreal& lat ) const
{
    Q_ASSERT( 0 <= normalizedX && normalizedX <= 1 );
    Q_ASSERT( 0 <= normalizedY && normalizedY <= 1 );

    const GeoSceneTexture *const textureLayer = m_model->textureLayer();

    switch ( textureLayer->projection() ) {
    case GeoSceneTexture::Mercator:
        lat = atan( sinh( ( 0.5 - normalizedY ) * 2 * M_PI ) );
        lon = ( normalizedX - 0.5 ) * 2 * M_PI;
        return;
    case GeoSceneTexture::Equirectangular:
        lat = ( 0.5 - normalizedY ) * M_PI;
        lon = ( normalizedX - 0.5 ) * 2 * M_PI;
        return;
    }

    Q_ASSERT( false ); // not reached

    return;
}

void MarbleGLWidget::Private::projectionCoordinates( qreal lon, qreal lat, qreal &x, qreal &y ) const
{
    const GeoSceneTexture *const textureLayer = m_model->textureLayer();

    switch ( textureLayer->projection() ) {
    case GeoSceneTexture::Mercator:
        if ( lat < -85*DEG2RAD ) lat = -85*DEG2RAD;
        if ( lat >  85*DEG2RAD ) lat =  85*DEG2RAD;
        x = ( 0.5 + 0.5 * lon / M_PI );
        y = ( 0.5 - 0.5 * atanh( sin( lat ) ) / M_PI );
        return;
    case GeoSceneTexture::Equirectangular:
        x = ( 0.5 + 0.5 * lon / M_PI );
        y = ( 0.5 - lat / M_PI );
        return;
    }

    Q_ASSERT( false ); // not reached

    return;
}

// ----------------------------------------------------------------


MarbleModel *MarbleGLWidget::model() const
{
    return d->m_model;
}


ViewportParams* MarbleGLWidget::viewport()
{
    return d->m_viewParams.viewport();
}

const ViewportParams* MarbleGLWidget::viewport() const
{
    return d->m_viewParams.viewport();
}


int MarbleGLWidget::radius() const
{
    return viewport()->radius();
}

qreal MarbleGLWidget::radius( qreal lon, qreal lat ) const
{
    qreal centerLon, centerLat;
    viewport()->centerCoordinates( centerLon, centerLat );

    return radius() - d->m_height.altitude( centerLon, centerLat ) + d->m_height.altitude( lon, lat );
}

void MarbleGLWidget::setRadius( int newRadius )
{
    if ( newRadius == radius() ) {
        return;
    }

    viewport()->setRadius( newRadius );

    d->m_height.setRadius( radius() );

    emit radiusChanged( radius() );
    emit visibleLatLonAltBoxChanged( d->m_viewParams.viewport()->viewLatLonAltBox() );

    d->update();
}


bool MarbleGLWidget::showOverviewMap() const
{
    return d->propertyValue( "overviewmap" );
}

bool MarbleGLWidget::showScaleBar() const
{
    return d->propertyValue( "scalebar" );
}

bool MarbleGLWidget::showCompass() const
{
    return d->propertyValue( "compass" );
}

bool MarbleGLWidget::showClouds() const
{
    return false;
}

bool MarbleGLWidget::showAtmosphere() const
{
    return d->m_viewParams.showAtmosphere();
}

bool MarbleGLWidget::showCrosshairs() const
{
    return d->m_model->showCrosshairs();
}

bool MarbleGLWidget::showGrid() const
{
    return d->propertyValue( "coordinate-grid" );
}

bool MarbleGLWidget::showPlaces() const
{
    return d->propertyValue( "places" );
}

bool MarbleGLWidget::showCities() const
{
    return d->propertyValue( "cities" );
}

bool MarbleGLWidget::showTerrain() const
{
    return d->propertyValue( "terrain" );
}

bool MarbleGLWidget::showOtherPlaces() const
{
    return d->propertyValue( "otherplaces" );
}

bool MarbleGLWidget::showRelief() const
{
    return d->propertyValue( "relief" );
}

bool MarbleGLWidget::showElevationModel() const
{
    return d->m_viewParams.showElevationModel();
}

bool MarbleGLWidget::showIceLayer() const
{
    return d->propertyValue( "ice" );
}

bool MarbleGLWidget::showBorders() const
{
    return d->propertyValue( "borders" );
}

bool MarbleGLWidget::showRivers() const
{
    return d->propertyValue( "rivers" );
}

bool MarbleGLWidget::showLakes() const
{
    return d->propertyValue( "lakes" );
}

bool MarbleGLWidget::showGps() const
{
    return d->m_viewParams.showGps();
}

bool MarbleGLWidget::showFrameRate() const
{
    return d->m_showFrameRate;
}


void MarbleGLWidget::centerOn( const qreal lon, const qreal lat )
{
    Quaternion roll;
    roll.createFromEuler( 0, 0, heading() * DEG2RAD );

    Quaternion  quat;
    quat.createFromEuler( -lat * DEG2RAD, lon * DEG2RAD, 0 );
    d->m_viewParams.setPlanetAxis( quat * roll );

    emit visibleLatLonAltBoxChanged( d->m_viewParams.viewport()->viewLatLonAltBox() );

    d->update();
}

void MarbleGLWidget::setHeading( qreal heading )
{
    Quaternion roll;
    roll.createFromEuler( 0, 0, heading * DEG2RAD );

    Quaternion  quat;
    quat.createFromEuler( -centerLatitude() * DEG2RAD, centerLongitude() * DEG2RAD, 0 );

    d->m_viewParams.setPlanetAxis( quat * roll );

    emit visibleLatLonAltBoxChanged( d->m_viewParams.viewport()->viewLatLonAltBox() );

    d->update();
}

void MarbleGLWidget::setTilt( qreal tilt )
{
    d->m_viewParams.setTilt( tilt );
}

Projection MarbleGLWidget::projection() const
{
    return d->m_viewParams.projection();
}

void MarbleGLWidget::setProjection( Projection projection )
{
    d->m_viewParams.setProjection( projection );

#if 0
    if ( d->m_viewParams.showAtmosphere() ) {
        d->m_dirtyAtmosphere = true;
    }
#endif

    // Update texture map during the repaint that follows:
    emit visibleLatLonAltBoxChanged( d->m_viewParams.viewport()->viewLatLonAltBox() );

    d->update();
}


void MarbleGLWidget::setupViewport( int width, int height )
{
    d->m_viewParams.viewport()->setSize( QSize( width, height ) );

    glViewport( 0, 0, width, height );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    glOrtho( -0.5*width, 0.5*width, -0.5*height, 0.5*height, -256000000/M_PI*80, 256/M_PI*32 );
    glMatrixMode( GL_MODELVIEW );
}


void MarbleGLWidget::resizeGL( int width, int height )
{
    setupViewport( width, height );

    d->update();
}

qreal MarbleGLWidget::centerLatitude() const
{
    // Calculate translation of center point
    qreal  centerLon;
    qreal  centerLat;

    d->m_viewParams.centerCoordinates( centerLon, centerLat );
    return centerLat * RAD2DEG;
}

qreal MarbleGLWidget::centerLongitude() const
{
    // Calculate translation of center point
    qreal  centerLon;
    qreal  centerLat;

    d->m_viewParams.centerCoordinates( centerLon, centerLat );
    return centerLon * RAD2DEG;
}

qreal MarbleGLWidget::heading() const
{
    return d->m_viewParams.heading();
}

qreal MarbleGLWidget::tilt() const
{
    return d->m_viewParams.tilt();
}

void MarbleGLWidget::updateTiles()
{
    const GeoSceneTexture *const textureLayer = d->m_model->textureLayer();

    if ( !textureLayer )
        return;

    const GeoDataLatLonAltBox bbox = viewport()->viewLatLonAltBox();

    int level = 0;
    int numXTiles = TileLoaderHelper::levelToColumn( textureLayer->levelZeroColumns(), level );
    int numYTiles = TileLoaderHelper::levelToRow( textureLayer->levelZeroRows(), level );

    const qreal averageLon = ( bbox.north() + bbox.south() ) / 2;

    while ( numXTiles * textureLayer->tileSize().width() < radius() * cos( averageLon ) * M_PI && level < textureLayer->maximumTileLevel()) {
        ++level;
        numXTiles = TileLoaderHelper::levelToColumn( textureLayer->levelZeroColumns(), level );
        numYTiles = TileLoaderHelper::levelToRow( textureLayer->levelZeroRows(), level );
    };

    qreal topLeftX, topLeftY, botRightX, botRightY;
    d->projectionCoordinates( bbox.west(), bbox.north(), topLeftX, topLeftY );
    d->projectionCoordinates( bbox.east(), bbox.south(), botRightX, botRightY );
    const int startXTile =       numXTiles * topLeftX;
    const int startYTile =       numYTiles * topLeftY;
          int endXTile   = 1.5 + numXTiles * botRightX;
          int endYTile   = 1.5 + numYTiles * botRightY;

    if ( endXTile <= startXTile )
        endXTile += numXTiles;
    if ( endYTile <= startYTile )
        endYTile += numYTiles;

    d->m_tileQueue.clear();

    QList<Tile> unusedTiles = d->m_tiles;

    const int hash = qHash( textureLayer->sourceDir() );

    for (int i = startXTile; i < endXTile; ++i)
    {
        for (int j = startYTile; j < endYTile; ++j)
        {
            const TileId id( hash, level, i % numXTiles, j % numYTiles );

            bool found = false;
            foreach ( const Tile &tile, unusedTiles) {
                if ( tile.id() == id ) {
                    found = true;
                    unusedTiles.removeAll( tile );
                }
            }
            if ( !found ) {
                d->m_tileQueue.append( id );
            }
        }
    }

    foreach ( const Tile &tile, unusedTiles) {
        d->m_tiles.removeAll( tile );
        deleteTexture( tile.glName() );
    }

    if ( d->m_previousLevel != level ) {
        d->m_previousLevel = level;
        while ( !d->m_tileQueue.isEmpty() ) {
            processNextTile();
        }
    }
    else {
        processNextTile();
    }
}

void MarbleGLWidget::tileUpdated( const TileId &id )
{
    foreach ( const Tile &tile, d->m_tiles ) {
        if ( tile.id() == id ) {
            d->m_tiles.removeAll( tile );
            deleteTexture( tile.glName() );
        }
    }

    d->m_tileQueue.prepend( id );
    processNextTile();
}

void MarbleGLWidget::processNextTile()
{
    if ( d->m_tileQueue.isEmpty() )
        return;

    const TileId id = d->m_tileQueue.takeFirst();
    const QImage image = d->m_model->tileImage( id, DownloadBrowse );
    const GLuint texture = bindTexture( image, GL_TEXTURE_2D, GL_RGBA, QGLContext::LinearFilteringBindOption | QGLContext::PremultipliedAlphaBindOption );
    d->m_tiles.append( Tile( id, texture ) );

    update();

    if ( !d->m_tileQueue.isEmpty() )
        d->m_tileQueueTimer.start();
}

void MarbleGLWidget::initializeGL()
{
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_CULL_FACE );
    glEnable( GL_TEXTURE_2D );
}

void MarbleGLWidget::paintEvent( QPaintEvent *event )
{
    makeCurrent();

    // Stop repaint timer if it is already running
    QTime t;
    t.start();

    setupViewport(width(), height());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const Quaternion axis = d->m_viewParams.viewport()->planetAxis();
    const qreal scale = sqrt( axis.v[Q_X]*axis.v[Q_X] + axis.v[Q_Y]*axis.v[Q_Y] + axis.v[Q_Z]*axis.v[Q_Z] );
    const qreal angle = - 2 * acos( axis.v[Q_W] ) * RAD2DEG;
    const qreal ax = axis.v[Q_X] / scale;
    const qreal ay = axis.v[Q_Y] / scale;
    const qreal az = axis.v[Q_Z] / scale;

    qreal lon, lat;
    d->m_viewParams.centerCoordinates( lon, lat );
    glLoadIdentity();
    glTranslated( 0, 0, radius( lon, lat ) );
    glRotated( -tilt(), 1, 0, 0 );
    glTranslated( 0, 0, -radius( lon, lat ) );
    glRotated( angle, ax, ay, az );

    foreach ( const Tile &tile, d->m_tiles ) {
        renderTile( tile );
    }
    d->m_model->cleanupTileHash();
    d->m_model->resetTileHash();

    // FIXME: Better way to get the GeoPainter
    bool  doClip = true;
    if ( d->m_viewParams.projection() == Spherical )
        doClip = ( radius() > width() / 2
                   || radius() > height() / 2 );

    // Create a painter that will do the painting.
    GeoPainter painter( this, viewport(),
                        d->m_viewParams.mapQuality(), doClip );

    painter.beginNativePainting();

    QStringList renderPositions;
    renderPositions << "SURFACE";

    // Paint the vector layer.
    if ( d->m_model->mapTheme()->map()->hasVectorLayers() ) {
        if ( !d->m_model->mapTheme()->map()->hasTextureLayers() ) {
            d->m_model->vectorComposer()->paintBaseVectorMap( &painter, &d->m_viewParams );
        }

        d->m_model->renderLayers( &painter, &d->m_viewParams, renderPositions );
        // Add further Vectors
        d->m_model->vectorComposer()->paintVectorMap( &painter, &d->m_viewParams );
    }
    else {
        d->m_model->renderLayers( &painter, &d->m_viewParams, renderPositions );
    }

    // Paint the GeoDataPlacemark layer
    bool showPlaces, showCities, showTerrain, showOtherPlaces;

    d->m_viewParams.propertyValue( "places", showPlaces );
    d->m_viewParams.propertyValue( "cities", showCities );
    d->m_viewParams.propertyValue( "terrain", showTerrain );
    d->m_viewParams.propertyValue( "otherplaces", showOtherPlaces );

    if ( showPlaces && ( showCities || showTerrain || showOtherPlaces ) )
    {
        d->m_model->placemarkLayout()->paintPlaceFolder( &painter, &d->m_viewParams );
    }

#if 0
    // Paint the Gps Layer
    painter.save();
    QSize canvasSize = m_viewParams.canvasImage()->size();
    painter->restore();
#endif

    renderPositions.clear();
    renderPositions << "HOVERS_ABOVE_SURFACE";
    d->m_model->renderLayers( &painter, &d->m_viewParams, renderPositions );

#if 0
    // FIXME: This is really slow. That's why we defer this to
    //        PrintQuality. Either cache on a pixmap - or maybe
    //        better: Add to GlobeScanlineTextureMapper.

    if ( m_viewParams.mapQuality() == PrintQuality )
        drawFog( painter );
#endif

    renderPositions.clear();
    renderPositions << "ATMOSPHERE"
                    << "ORBIT" << "ALWAYS_ON_TOP" << "FLOAT_ITEM" << "USER_TOOLS";

    d->m_model->renderLayers( &painter, &d->m_viewParams, renderPositions );

#if 0
    // Draws the map like MarbleMap::paint does, but adds our customPaint in between
    d->m_map->paintOverlay( painter, dirtyRect );
#endif

#if 0
    if ( showFrameRate() )
    {
        qreal fps = 1000.0 / (qreal)( t.elapsed() + 1 );
        d->paintFps( painter, dirtyRect, fps );
        emit d->m_widget->framesPerSecond( fps );
    }
#endif
    painter.endNativePainting();
}


void MarbleGLWidget::setMapThemeId( const QString& mapThemeId )
{
    if ( !mapThemeId.isEmpty() && mapThemeId == d->m_model->mapThemeId() )
        return;

    d->m_viewParams.setMapThemeId( mapThemeId );
    GeoSceneDocument *mapTheme = d->m_viewParams.mapTheme();

    if ( mapTheme ) {
        d->m_model->setMapTheme( mapTheme, d->m_viewParams.projection() );

#if 0
        // We don't do this on every paintEvent to improve performance.
        // Redrawing the atmosphere is only needed if the size of the
        // globe changes.
        d->m_dirtyAtmosphere=true;
#endif

        centerSun();
    }

    // Now we want a full repaint as the atmosphere might differ
    setAttribute( Qt::WA_NoSystemBackground, false );

    d->m_tileQueue.clear();
    d->m_tiles.clear();
    updateTiles();
    d->update();
}

void MarbleGLWidget::setPropertyValue( const QString& name, bool value )
{
    mDebug() << "In MarbleGLWidget the property " << name << "was set to " << value;
    d->m_viewParams.setPropertyValue( name, value );

    d->update();
}

void MarbleGLWidget::setShowOverviewMap( bool visible )
{
    d->setPropertyValue( "overviewmap", visible );

    d->update();
}

void MarbleGLWidget::setShowScaleBar( bool visible )
{
    d->setPropertyValue( "scalebar", visible );

    d->update();
}

void MarbleGLWidget::setShowCompass( bool visible )
{
    d->setPropertyValue( "compass", visible );

    d->update();
}

void MarbleGLWidget::setShowClouds( bool /*visible*/ )
{
    mDebug() << "clouds layer is not yet implemented in OpenGL mode";

#if 0
    d->m_map->setShowClouds( visible );

    d->update();
#endif
}

void MarbleGLWidget::setShowAtmosphere( bool /*visible*/ )
{
    mDebug() << "athmosphere layer is not yet implemented in OpenGL mode";

#if 0
    d->m_map->setShowAtmosphere( visible );

    d->update();
#endif
}

void MarbleGLWidget::setShowCrosshairs( bool visible )
{
    d->m_model->setShowCrosshairs( visible );

    d->update();
}

void MarbleGLWidget::setShowGrid( bool visible )
{
    d->setPropertyValue( "coordinate-grid", visible );

    d->update();
}

void MarbleGLWidget::setShowPlaces( bool visible )
{
    d->setPropertyValue( "places", visible );

    d->update();
}

void MarbleGLWidget::setShowCities( bool visible )
{
    d->setPropertyValue( "cities", visible );

    d->update();
}

void MarbleGLWidget::setShowTerrain( bool visible )
{
    d->setPropertyValue( "terrain", visible );

    d->update();
}

void MarbleGLWidget::setShowOtherPlaces( bool visible )
{
    d->setPropertyValue( "otherplaces", visible );

    d->update();
}

void MarbleGLWidget::setShowRelief( bool visible )
{
    d->setPropertyValue( "relief", visible );

    d->update();
}

void MarbleGLWidget::setShowElevationModel( bool visible )
{
    d->m_viewParams.setShowElevationModel( visible );

    d->update();
}

void MarbleGLWidget::setShowIceLayer( bool visible )
{
    d->setPropertyValue( "ice", visible );

    d->update();
}

void MarbleGLWidget::setShowBorders( bool visible )
{
    d->setPropertyValue( "borders", visible );

    d->update();
}

void MarbleGLWidget::setShowRivers( bool visible )
{
    d->setPropertyValue( "rivers", visible );

    d->update();
}

void MarbleGLWidget::setShowLakes( bool visible )
{
    d->setPropertyValue( "lakes", visible );

    d->update();
}

void MarbleGLWidget::setShowFrameRate( bool visible )
{
    d->m_showFrameRate = visible;

    d->update();
}

void MarbleGLWidget::setShowGps( bool visible )
{
    d->m_viewParams.setShowGps( visible );

    d->update();
}

void MarbleGLWidget::setShowTileId( bool visible )
{
    d->m_showTileId = visible;

    d->update();
}

MapQuality MarbleGLWidget::mapQuality()
{
    return d->m_viewParams.mapQuality();
}

void MarbleGLWidget::setNeedsUpdate()
{
    d->update();
}

void MarbleGLWidget::setMapQuality( MapQuality mapQuality )
{
    d->m_viewParams.setMapQuality( mapQuality );
}

void MarbleGLWidget::updateSun()
{
    d->m_model->update();

    d->update();
}

void MarbleGLWidget::centerSun()
{
    SunLocator  *sunLocator = d->m_model->sunLocator();

    if ( sunLocator && sunLocator->getCentered() ) {
        qreal  lon = sunLocator->getLon();
        qreal  lat = sunLocator->getLat();
        centerOn( lon, lat );

        mDebug() << "Centering on Sun at " << lat << lon;
    }
}

}
