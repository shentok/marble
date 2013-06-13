//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009 Torsten Rahn <tackat@kde.org>
//

//
// This class is a graticule plugin.
//

#ifndef MARBLEGRATICULEPLUGIN_H
#define MARBLEGRATICULEPLUGIN_H

#include "RenderPlugin.h"
#include "DialogConfigurationInterface.h"

#include <QtCore/QMap>
#include <QtCore/QHash>
#include <QtGui/QIcon>

#include "GeoDataCoordinates.h"
#include "GeoDataLatLonAltBox.h"
#include "GeoDataStyle.h"

namespace Ui 
{
    class GraticuleConfigWidget;
}

namespace Marble
{

/**
 * @brief A plugin that creates a coordinate grid on top of the map.
 * Unlike in all other classes we are using degree by default in this class.
 * This choice was made due to the fact that all common coordinate grids focus fully 
 * on the degree system. 
 */

class GraticulePlugin : public RenderPlugin, public DialogConfigurationInterface
{
    Q_OBJECT
    Q_INTERFACES( Marble::RenderPluginInterface )
    Q_INTERFACES( Marble::DialogConfigurationInterface )
    MARBLE_PLUGIN( GraticulePlugin )

 public:
    GraticulePlugin();

    explicit GraticulePlugin( const MarbleModel *marbleModel );

    ~GraticulePlugin();

    QStringList backendTypes() const;

    QString renderPolicy() const;

    QStringList renderPosition() const;

    QString name() const;

    QString guiString() const;

    QString nameId() const;

    QString version() const;

    QString description() const;

    QString copyrightYears() const;

    QList<PluginAuthor> pluginAuthors() const;

    QIcon icon () const;

    QDialog *configDialog();

    void initialize ();

    bool isInitialized () const;

    virtual bool render( GeoPainter *painter, ViewportParams *viewport, const QString& renderPos, GeoSceneLayer * layer = 0 );

    virtual qreal zValue() const;

    virtual QHash<QString,QVariant> settings() const;

    virtual void setSettings( const QHash<QString,QVariant> &settings );



 public Q_SLOTS:
    void readSettings();
    void writeSettings();

    void gridGetColor();
    void tropicsGetColor();
    void equatorGetColor();


 private:
     /**
     * @brief Renders the coordinate grid within the defined view bounding box.
     * @param painter the painter used to draw the grid
     * @param viewport the viewport
     */
    void renderGrid( const ViewportParams *viewport );

     /**
     * @brief Renders a latitude line within the defined view bounding box.
     * @param painter the painter used to draw the latitude line
     * @param latitude the latitude of the coordinate line measured in degree .
     * @param viewLatLonAltBox the latitude longitude bounding box that is covered by the view.
     */
    void renderLatitudeLine(  const GeoDataStyle &style,
                              qreal latitude,
                              const GeoDataLatLonAltBox& viewLatLonAltBox = GeoDataLatLonAltBox(),
                              const QString& lineLabel = QString() );

    /**
     * @brief Renders a longitude line within the defined view bounding box.
     * @param painter the painter used to draw the latitude line
     * @param longitude the longitude of the coordinate line measured in degree .
     * @param viewLatLonAltBox the latitude longitude bounding box that is covered by the view.
     * @param polarGap the area around the poles in which most longitude lines are not drawn
     *        for reasons of aesthetics and clarity of the map. The polarGap avoids narrow
     *        concurring lines around the poles which obstruct the view onto the surface.
     *        The radius of the polarGap area is measured in degrees. 
     * @param lineLabel draws a label using the font and color properties set for the painter.
     */
    void renderLongitudeLine( const GeoDataStyle &style,
                              qreal longitude,                         
                              const GeoDataLatLonAltBox& viewLatLonAltBox = GeoDataLatLonAltBox(),
                              qreal northPolarGap = 0.0, qreal southPolarGap = 0.0,
                              const QString& lineLabel = QString() );

    /**
     * @brief Renders the latitude lines that are visible within the defined view bounding box.
     * @param painter the painter used to draw the latitude lines
     * @param viewLatLonAltBox the latitude longitude bounding box that is covered by the view.
     * @param step the angular distance between the lines measured in degrees .
     */
    void renderLatitudeLines( const GeoDataStyle &style,
                              const GeoDataLatLonAltBox& viewLatLonAltBox,
                              qreal step );

    /**
     * @brief Renders the longitude lines that are visible within the defined view bounding box.
     * @param painter the painter used to draw the latitude lines
     * @param viewLatLonAltBox the latitude longitude bounding box that is covered by the view.
     * @param step the angular distance between the lines measured in degrees .
     * @param northPolarGap the area around the north pole in which most longitude lines are not drawn
     *        for reasons of aesthetics and clarity of the map. The polarGap avoids narrow
     *        concurring lines around the poles which obstruct the view onto the surface.
     *        The radius of the polarGap area is measured in degrees.
     * @param southPolarGap the area around the south pole in which most longitude lines are not drawn
     *        for reasons of aesthetics and clarity of the map. The polarGap avoids narrow
     *        concurring lines around the poles which obstruct the view onto the surface.
     *        The radius of the polarGap area is measured in degrees. 
     */
    void renderLongitudeLines( const GeoDataStyle &style,
                               const GeoDataLatLonAltBox& viewLatLonAltBox, 
                               qreal step, 
                               qreal northPolarGap = 0.0, qreal southPolarGap = 0.0 );

    /**
     * @brief Renders UTM exceptions that are visible within the defined view bounding box.
     * @param painter the painter used to draw the latitude lines
     * @param viewLatLonAltBox the latitude longitude bounding box that is covered by the view.
     * @param step the angular distance between the lines measured in degrees .
     * @param northPolarGap the area around the north pole in which most longitude lines are not drawn
     *        for reasons of aesthetics and clarity of the map. The polarGap avoids narrow
     *        concurring lines around the poles which obstruct the view onto the surface.
     *        The radius of the polarGap area is measured in degrees.
     * @param southPolarGap the area around the south pole in which most longitude lines are not drawn
     *        for reasons of aesthetics and clarity of the map. The polarGap avoids narrow
     *        concurring lines around the poles which obstruct the view onto the surface.
     *        The radius of the polarGap area is measured in degrees.
     */
    void renderUtmExceptions( const GeoDataStyle &style,
                              const GeoDataLatLonAltBox& viewLatLonAltBox,
                              qreal step,
                              qreal northPolarGap, qreal southPolarGap,
                              const QString & label );

    /**
     * @brief Maps the number of coordinate lines per 360 deg against the globe radius on the screen.
     * @param notation Determines whether the graticule is according to the DMS or Decimal system.
     */
    void initLineMaps( GeoDataCoordinates::Notation notation );

    GeoDataCoordinates::Notation m_currentNotation;

    // Maps the zoom factor to the amount of lines per 360 deg
    QMap<qreal,qreal> m_boldLineMap;
    QMap<qreal,qreal> m_normalLineMap;

    // render styles
    GeoDataStyle m_gridStyle;
    GeoDataStyle m_boldGridStyle;
    GeoDataStyle m_equatorStyle;
    GeoDataStyle m_tropicsStyle;
    bool m_showPrimaryLabels;
    bool m_showSecondaryLabels;

    bool m_isInitialized;

    QIcon m_icon;

    Ui::GraticuleConfigWidget *ui_configWidget;
    QDialog *m_configDialog;

    class ItemHelper;
    QList<ItemHelper*> m_items;
};

}

#endif // MARBLEGRATICULEPLUGIN_H
