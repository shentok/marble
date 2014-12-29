//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2006-2007 Torsten Rahn <tackat@kde.org>
// Copyright 2007      Inge Wallin  <ingwa@kde.org>
//

//
// VisiblePlacemarks are those Placemarks which become visible on the map
//

#ifndef MARBLE_VISIBLEPLACEMARK_H
#define MARBLE_VISIBLEPLACEMARK_H

#include <QObject>
#include "GeoGraphicsItem.h"

#include <QPixmap>
#include <QPoint>
#include <QRectF>
#include <QString>

namespace Marble
{

class GeoDataFeature;

static const qreal s_labelOutlineWidth = 2.5;

/**
 * @short A class which represents the visible place marks on a map.
 *
 * This class is used by PlacemarkLayout to pass the visible place marks
 * to the PlacemarkPainter.
 */
class VisiblePlacemark : public QObject, public GeoGraphicsItem
{
 Q_OBJECT

 public:
    explicit VisiblePlacemark( const GeoDataFeature *feature );

    void paint( GeoPainter *painter, const ViewportParams *viewport );

    /**
     * Returns the state of the place mark.
     */
    bool selected() const;

    /**
     * Sets the state of the place mark.
     */
    void setSelected( bool selected );

    /**
     * Returns the area covered by the placemark symbol on the map.
     */
    const QRect &symbolRect() const;

    /**
     * Returns the top left corner of the place mark symbol's hot spot
     */
    const QPointF hotSpot() const;

    /**
     * Sets the @p position of the place mark symbol on the map.
     */
    void setSymbolPosition( const QPoint& position );

    /**
     * Returns the area covered by the place mark name label on the map.
     */
    const QRectF& labelRect() const;

    /**
     * Sets the @p area covered by the place mark name label on the map.
     */
    void setLabelRect( const QRectF& area );

    enum LabelStyle {
        Normal = 0,
        Glow,
        Selected
    };

Q_SIGNALS:
    void updateNeeded();

private Q_SLOTS:
    void setSymbolPixmap();

 private:
    static void drawLabelText( QPainter &labelPainter, const QString &text, const QFont &labelFont, LabelStyle labelStyle, const QColor &color );
    void drawLabelPixmap();

    // View stuff
    QRect       m_symbolRect;     // bounding box of the placemark's symbol
    bool        m_selected;       // state of the placemark
    QPixmap     m_labelPixmap;    // the text label (most often name)
    QRectF      m_labelRect;      // bounding box of label

    mutable QPixmap     m_symbolPixmap; // cached value
};

}

#endif
