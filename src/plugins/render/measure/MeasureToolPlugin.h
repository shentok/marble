//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2006-2007 Torsten Rahn     <tackat@kde.org>
// Copyright 2007      Inge Wallin      <ingwa@kde.org>
// Copyright 2011      Michael Henning  <mikehenning@eclipse.net>
//

//
// MeasureToolPlugin enables Marble to set and display measure points
//

#ifndef MARBLE_MEASURETOOLPLUGIN_H
#define MARBLE_MEASURETOOLPLUGIN_H

#include "DialogConfigurationInterface.h"
#include "GeoDataLineString.h"
#include "GeoDataStyle.h"
#include "LabelGraphicsItem.h"
#include "RenderPlugin.h"
#include "MarbleWidget.h"
#include "MarbleWidgetPopupMenu.h"

#include <QtCore/QObject>
#include <QtGui/QFont>
#include <QtGui/QPen>
#include <QtGui/QAction>

namespace Ui {
    class MeasureConfigWidget;
}

namespace Marble
{

class MeasureToolPlugin : public RenderPlugin, public DialogConfigurationInterface
{
    Q_OBJECT
    Q_INTERFACES( Marble::RenderPluginInterface )
    Q_INTERFACES( Marble::DialogConfigurationInterface )
    MARBLE_PLUGIN( MeasureToolPlugin )

 public:
    MeasureToolPlugin();
    explicit MeasureToolPlugin( const MarbleModel *marbleModel );
    ~MeasureToolPlugin();

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

    void initialize ();

    bool isInitialized () const;

    bool setViewport( const ViewportParams *viewport );

    bool render( GeoPainter *painter, const QSize &viewportSize ) const;

    QDialog *configDialog();
    QHash<QString,QVariant> settings() const;
    void setSettings( const QHash<QString,QVariant> &settings );

 Q_SIGNALS:
    void  numberOfMeasurePointsChanged( int newNumber );

 public Q_SLOTS:
    bool  eventFilter( QObject *object, QEvent *event );

 private:
    void  updateScene();
    void  generateMeasurePoints();
    void  generateTotalDistanceLabel( qreal totalDistance );
    void  generateSegments();
    void  addContextItems();
    void  removeContextItems();

 private Q_SLOTS:
    void  setNumberOfMeasurePoints( int number );
    void  addMeasurePointEvent();

    void  addMeasurePoint( qreal lon, qreal lat );
    void  removeLastMeasurePoint();
    void  removeMeasurePoints();

    void writeSettings();

 private:
    Q_DISABLE_COPY( MeasureToolPlugin )

    // The line strings in the distance path.
    GeoDataLineString m_measureLineString;
    QList<GeoDataLineString*> m_segments;

    LabelGraphicsItem m_totalLabel;

    GeoDataStyle m_segmentStyles[3];
    GeoDataStyle m_shadowStyle;
    GeoDataStyle m_totalRectStyle;
    GeoDataStyle m_pointStyle;

    int m_fontascent;

    const QPixmap m_mark;

    QAction *m_addMeasurePointAction;
    QAction *m_removeLastMeasurePointAction;
    QAction *m_removeMeasurePointsAction;
    QAction *m_separator;

    MarbleWidget* m_marbleWidget;

    QDialog * m_configDialog;
    Ui::MeasureConfigWidget * m_uiConfigWidget;
    bool m_showSegmentLabels;

    class ItemHelper;
    QList<ItemHelper*> m_items;
};

}

#endif // MARBLE_MEASURETOOLPLUGIN_H
