//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2012      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#ifndef MARBLE_ANNOTATIONGRAPHICSITEM_H
#define MARBLE_ANNOTATIONGRAPHICSITEM_H

#include "BillboardGraphicsItem.h"
#include "marble_export.h"

namespace Marble
{

/**
 * @brief A text annotation that points to a geodesic position.
 *
 * The annotation consists of a bubble with the specified a text inside.
 * By choosing an appropriate pen for the painter it's possible to change
 * the color and line style of the bubble outline and the text. The brush
 * chosen for the painter is used to paint the background of the bubble.
 */
class MARBLE_EXPORT AnnotationGraphicsItem : public BillboardGraphicsItem
{
 public:
    /**
     * @brief Constructs a text annotation that points to a geodesic position.
     *
     * The optional parameters which describe the layout of the bubble are
     * similar to those used by QPainter::drawRoundRect().
     * Unlike in QPainter the rounded corners are not specified in percentage
     * but in pixels to provide for optimal aesthetics.
     * By choosing a positive or negative bubbleOffset it's possible to
     * place the annotation on top, bottom, left or right of the annotated
     * position.
     *
     * @param position The geodesic position
     * @param text The text contained by the bubble
     * @param bubbleSize The size of the bubble that holds the annotation text.
     *                   A height of 0 can be used to have the height calculated
     *                   automatically to fit the needed text height.
     * @param bubbleOffsetX The x-axis offset between the annotated position and
     *                      the "root" of the speech bubble's "arrow".
     * @param bubbleOffsetY The y-axis offset between the annotated position and
     *                      the "root" of the speech bubble's "arrow".
     * @param xRnd Specifies the geometry of the rounded corners in pixels along
     *             the x-axis.
     * @param yRnd Specifies the geometry of the rounded corners in pixels along
     *             the y-axis.
     *
     * @see GeoDataCoordinates
     */
    AnnotationGraphicsItem( const GeoDataCoordinates &position,
                            const QString &text,
                            const QSizeF &bubbleSize = QSizeF( 130, 100 ),
                            const QPointF &bubbleOffset = QPointF( -10, -30 ),
                            qreal xRnd = 5, qreal yRnd = 5 );

    ~AnnotationGraphicsItem();

    void paint( QPainter *painter );

 private:
    class Private;
    Private *const d;
};

} // Marble namespace

#endif
