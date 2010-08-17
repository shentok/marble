//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
// Copyright 2006-2008 Torsten Rahn <tackat@kde.org>
// Copyright 2007      Inge Wallin  <ingwa@kde.org>
//

#ifndef MARBLE_MARBLEGLWIDGET_H
#define MARBLE_MARBLEGLWIDGET_H


#include <QtOpenGL/QGLWidget>

#include "GeoDataCoordinates.h"
#include "GeoDataLatLonAltBox.h"
#include "global.h"             // types needed in all of marble.


namespace Marble
{

class GeoDataLatLonBox;
class MarbleModel;
class TileId;
class ViewportParams;

/**
 * @short A widget class that displays a view of the earth.
 *
 * This widget displays a view of the earth or any other globe,
 * depending on which dataset is used.  The user can navigate the
 * globe using either a control widget, e.g. the MarbleControlBox, or
 * the mouse.  The mouse and keyboard control is done through a
 * MarbleWidgetInputHandler. Only some aspects of the widget can be
 * controlled by the mouse and/or keyboard.
 *
 * By clicking on the globe and moving the mouse, the position can be
 * changed.  The user can also zoom by using the scroll wheel of the
 * mouse in the widget. The zoom value is not tied to any units, but
 * is an abstract value without any physical meaning. A value around
 * 1000 shows the full globe in a normal-sized window. Higher zoom
 * values give a more zoomed-in view.
 *
 * The MarbleWidget needs to be provided with a data model to
 * work. This model is contained in the MarbleModel class, and it is
 * painted by using a MarbleMap. The widget can also construct its own
 * map and model if none is given to the constructor.  A MarbleModel
 * contains 3 separate datatypes: <b>tiles</b> which provide the
 * background, <b>vectors</b> which provide things like country
 * borders and coastlines and <b>placemarks</b> which can show points
 * of interest, such as cities, mountain tops or the poles.
 *
 * In addition to navigating with the mouse, you can also use it to
 * get information about items on the map. You can either click on a
 * placemark with the left mouse button or with the right mouse button
 * anywhere on the map.
 *
 * The left mouse button opens up a menu with all the placemarks
 * within a certain distance from the mouse pointer. When you choose
 * one item from the menu, Marble will open up a dialog window with
 * some information about the placemark and also try to connect to
 * Wikipedia to retrieve an article about it. If there is such an
 * article, you will get a mini-browser window with the article in a tab.
 *
 * The right mouse button controls a distance tool.  The distance tool
 * is implemented as a menu where you can choose to either create or
 * remove so called Measure Points. Marble will keep track of the
 * Measure Points and show the total distance in the upper left of the
 * widget.  Measure Points are shown on the map as a little white
 * cross.
 *
 * @see MarbleControlBox
 * @see MarbleMap
 * @see MarbleModel
 */

class MarbleGLWidget : public QGLWidget
{
    Q_OBJECT

    Q_PROPERTY(bool showOverviewMap READ showOverviewMap    WRITE setShowOverviewMap)
    Q_PROPERTY(bool showScaleBar READ showScaleBar    WRITE setShowScaleBar)
    Q_PROPERTY(bool showCompass  READ showCompass     WRITE setShowCompass)
    Q_PROPERTY(bool showGrid     READ showGrid        WRITE setShowGrid)

    Q_PROPERTY(bool showClouds   READ showClouds      WRITE setShowClouds)
    Q_PROPERTY(bool showAtmosphere READ showAtmosphere WRITE setShowAtmosphere)
    Q_PROPERTY(bool showCrosshairs READ showCrosshairs WRITE setShowCrosshairs)

    Q_PROPERTY(bool showPlaces   READ showPlaces      WRITE setShowPlaces)
    Q_PROPERTY(bool showCities   READ showCities      WRITE setShowCities)
    Q_PROPERTY(bool showTerrain  READ showTerrain     WRITE setShowTerrain)
    Q_PROPERTY(bool showOtherPlaces READ showOtherPlaces WRITE setShowOtherPlaces)

    Q_PROPERTY(bool showRelief   READ showRelief      WRITE setShowRelief)
    Q_PROPERTY(bool showElevationModel READ showElevationModel WRITE setShowElevationModel)

    Q_PROPERTY(bool showIceLayer READ showIceLayer    WRITE setShowIceLayer)
    Q_PROPERTY(bool showBorders  READ showBorders     WRITE setShowBorders)
    Q_PROPERTY(bool showRivers   READ showRivers      WRITE setShowRivers)
    Q_PROPERTY(bool showLakes    READ showLakes       WRITE setShowLakes)

 public:

    /**
     * @brief Construct a new MarbleWidget.
     * @param model  the data model for the widget.
     * @param parent the parent widget
     *
     * This constructor should be used when you plan to use more than
     * one MarbleWidget for the same MarbleModel (not yet supported,
     * but will be soon).
     */
    explicit MarbleGLWidget( MarbleModel *model, QWidget *parent );

    virtual ~MarbleGLWidget();

    /**
     * @brief Return the model that this view shows.
     */
    MarbleModel *model() const;

    ViewportParams *viewport();
    const ViewportParams *viewport() const;

    /**
     * @brief  Return the radius of the globe in pixels.
     */
    int radius() const;

    /**
     * @brief  Set the radius of the globe in pixels.
     * @param  radius  The new globe radius value in pixels.
     */
    void setRadius( int radius );

    /**
     * @brief Return the longitude of the center point.
     * @return The longitude of the center point in degree.
     */
    qreal centerLongitude() const;

    /**
     * @brief Return the latitude of the center point.
     * @return The latitude of the center point in degree.
     */
    qreal centerLatitude() const;

    /**
     * @brief  Return whether the overview map is visible.
     * @return The overview map visibility.
     */
    bool showOverviewMap() const;

    /**
     * @brief  Return whether the scale bar is visible.
     * @return The scale bar visibility.
     */
    bool showScaleBar() const;

    /**
     * @brief  Return whether the compass bar is visible.
     * @return The compass visibility.
     */
    bool showCompass() const;

    /**
     * @brief  Return whether the cloud cover is visible.
     * @return The cloud cover visibility.
     */
    bool showClouds() const;

    /**
     * @brief  Return whether the atmospheric glow is visible.
     * @return The cloud cover visibility.
     */
    bool showAtmosphere() const;

    /**
     * @brief  Return whether the crosshairs are visible.
     * @return The crosshairs' visibility.
     */
    bool showCrosshairs() const;

    /**
     * @brief  Return whether the coordinate grid is visible.
     * @return The coordinate grid visibility.
     */
    bool showGrid() const;

    /**
     * @brief  Return whether the place marks are visible.
     * @return The place mark visibility.
     */
    bool showPlaces() const;

    /**
     * @brief  Return whether the city place marks are visible.
     * @return The city place mark visibility.
     */
    bool showCities() const;

    /**
     * @brief  Return whether the terrain place marks are visible.
     * @return The terrain place mark visibility.
     */
    bool showTerrain() const;

    /**
     * @brief  Return whether other places are visible.
     * @return The visibility of other places.
     */
    bool showOtherPlaces() const;

    /**
     * @brief  Return whether the relief is visible.
     * @return The relief visibility.
     */
    bool showRelief() const;

    /**
     * @brief  Return whether the elevation model is visible.
     * @return The elevation model visibility.
     */
    bool showElevationModel() const;

    /**
     * @brief  Return whether the ice layer is visible.
     * @return The ice layer visibility.
     */
    bool showIceLayer() const;

    /**
     * @brief  Return whether the borders are visible.
     * @return The border visibility.
     */
    bool showBorders() const;

    /**
     * @brief  Return whether the rivers are visible.
     * @return The rivers' visibility.
     */
    bool showRivers() const;

    /**
     * @brief  Return whether the lakes are visible.
     * @return The lakes' visibility.
     */
    bool showLakes() const;

    /**
     * @brief Return whether Gps Data is visible.
     * @return The Gps Data's visibility.
     */
    bool showGps() const;

    /**
     * @brief  Return whether the frame rate gets displayed.
     * @return the frame rates visibility
     */
    bool showFrameRate() const;

    /**
     * @brief  Get the Projection used for the map
     * @return @c Spherical         a Globe
     * @return @c Equirectangular   a flat map
     * @return @c Mercator          another flat map
     */
    Projection projection() const;

    /**
     * @brief Retrieve the map quality depending on the view context 
     */
    MapQuality mapQuality();

 public Q_SLOTS:

    void updateSun();
    void centerSun();

    /**
     * @brief  Center the view on a geographical point
     * @param  lat  an angle in degrees parallel to the latitude lines
     *              +90(N) - -90(S)
     * @param  lon  an angle in degrees parallel to the longitude lines
     *              +180(W) - -180(E)
     */
    void centerOn( const qreal lon, const qreal lat );

    /**
     * @brief  Set the Projection used for the map
     * @param  projection projection type (e.g. Spherical, Equirectangular, Mercator)
     */
    void setProjection( Projection projection );

    /**
     * @brief Set a new map theme
     * @param maptheme  The ID of the new maptheme. To ensure that a unique 
     * identifier is being used the theme does NOT get represented by its 
     * name but the by relative location of the file that specifies the theme:
     *
     * Example: 
     *    maptheme = "earth/bluemarble/bluemarble.dgml" 
     */
    void setMapThemeId( const QString& maptheme );

    /**
     * @brief  Sets the value of a map theme property
     * @param  value  value of the property (usually: visibility)
     * 
     * Later on we might add a "setPropertyType and a QVariant
     * if needed.
     */
    void setPropertyValue( const QString& name, bool value );

    /**
     * @brief  Set whether the overview map overlay is visible
     * @param  visible  visibility of the overview map
     */
    void setShowOverviewMap( bool visible );

    /**
     * @brief  Set whether the scale bar overlay is visible
     * @param  visible  visibility of the scale bar
     */
    void setShowScaleBar( bool visible );

    /**
     * @brief  Set whether the compass overlay is visible
     * @param  visible  visibility of the compass
     */
    void setShowCompass( bool visible );

    /**
     * @brief  Set whether the cloud cover is visible
     * @param  visible  visibility of the cloud cover
     */
    void setShowClouds( bool visible );

    /**
     * @brief  Set whether the atmospheric glow is visible
     * @param  visible  visibility of the atmospheric glow
     */
    void setShowAtmosphere( bool visible );

    /**
     * @brief  Set whether the crosshairs are visible
     * @param  visible  visibility of the crosshairs
     */
    void setShowCrosshairs( bool visible );

    /**
     * @brief  Set whether the coordinate grid overlay is visible
     * @param  visible  visibility of the coordinate grid
     */
    void setShowGrid( bool visible );

    /**
     * @brief  Set whether the place mark overlay is visible
     * @param  visible  visibility of the place marks
     */
    void setShowPlaces( bool visible );

    /**
     * @brief  Set whether the city place mark overlay is visible
     * @param  visible  visibility of the city place marks
     */
    void setShowCities( bool visible );

    /**
     * @brief  Set whether the terrain place mark overlay is visible
     * @param  visible  visibility of the terrain place marks
     */
    void setShowTerrain( bool visible );

    /**
     * @brief  Set whether the other places overlay is visible
     * @param  visible  visibility of other places
     */
    void setShowOtherPlaces( bool visible );

    /**
     * @brief  Set whether the relief is visible
     * @param  visible  visibility of the relief
     */
    void setShowRelief( bool visible );

    /**
     * @brief  Set whether the elevation model is visible
     * @param  visible  visibility of the elevation model
     */
    void setShowElevationModel( bool visible );

    /**
     * @brief  Set whether the ice layer is visible
     * @param  visible  visibility of the ice layer
     */
    void setShowIceLayer( bool visible );

    /**
     * @brief  Set whether the borders visible
     * @param  visible  visibility of the borders
     */
    void setShowBorders( bool visible );

    /**
     * @brief  Set whether the rivers are visible
     * @param  visible  visibility of the rivers
     */
    void setShowRivers( bool visible );

    /**
     * @brief  Set whether the lakes are visible
     * @param  visible  visibility of the lakes
     */
    void setShowLakes( bool visible );

    /**
     * @brief Set whether Gps Data is visible
     * @param visible  visibility of the Gps Data
     */
    void setShowGps( bool visible );

    /**
     * @brief Set whether the frame rate gets shown
     * @param visible  visibility of the frame rate
     */
    void setShowFrameRate( bool visible );

    /**
     * @brief Set whether the is tile is visible
     * NOTE: This is part of the transitional debug API
     *       and might be subject to changes until Marble 0.8
     * @param visible visibility of the tile
     */
    void setShowTileId( bool visible );

    /**
     * @brief Invalidate the map and update the widget.
     */
    void setNeedsUpdate();

    /**
     * @brief Set the map quality depending on the view context 
     */
    void setMapQuality( MapQuality );

 Q_SIGNALS:
    /**
     * @brief Signal that the radius has changed, and to what.
     * @param radius  The new value of the radius.
     * @see  setRadius()
     */
    void radiusChanged( int radius );

    /**
     * @brief Signal that the theme has changed
     * @param theme  Name of the new theme.
     */
    void themeChanged( const QString& theme );

    void projectionChanged( Projection );

    void mouseMoveGeoPosition( const QString& );

    void mouseClickGeoPosition( qreal lon, qreal lat, GeoDataCoordinates::Unit );

    void framesPerSecond( qreal fps );

    /**
     * This signal is emitted when the visible region of the map changes. This typically happens
     * when the user moves the map around or zooms.
     */
    void visibleLatLonAltBoxChanged( const GeoDataLatLonAltBox &visibleLatLonAltBox );

 protected Q_SLOTS:
     void updateTiles();
     void processNextTile();

 protected:
    virtual void initializeGL();

    /**
     * @brief Reimplementation of the paintEvent() function in QWidget.
     */
    virtual void paintGL();

    /**
     * @brief Reimplementation of the resizeEvent() function in QWidget.
     */
    virtual void resizeGL( int width, int height );

 private:
    Q_DISABLE_COPY( MarbleGLWidget )
    class Private;
    Private  * const d;
    class Tile;
};

}

#endif
