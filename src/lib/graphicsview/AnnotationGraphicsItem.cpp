//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2012      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
// Copyright 2012      Mohammed Nafees <nafees.technocool@gmail.com>
//

// self
#include "AnnotationGraphicsItem.h"

// Marble
#include "ViewportParams.h"

#include <QtGui/QPainter>

namespace Marble
{

class AnnotationGraphicsItem::Private
{
 public:
    Private( const QString &text,
             const QPointF &bubbleOffset,
             qreal xRnd, qreal yRnd ) :
        m_text( text ),
        m_bubbleOffset( bubbleOffset ),
        m_xRnd( xRnd ),
        m_yRnd( yRnd )
    {
    }

    void createAnnotationLayout( const QSizeF &bubbleSize,
                                 const QPointF &bubbleOffset,
                                 qreal xRnd, qreal yRnd,
                                 QPainterPath &path, QRectF &rect );

    QString m_text;
    QPointF m_bubbleOffset;
    qreal m_xRnd;
    qreal m_yRnd;
};

void AnnotationGraphicsItem::Private::createAnnotationLayout( const QSizeF &bubbleSize,
                                                  const QPointF &bubbleOffset,
                                                  qreal xRnd, qreal yRnd,
                                                  QPainterPath &path, QRectF &rect )
{
    qreal arrowPosition = 0.3;
    qreal arrowWidth = 12.0;

    qreal width =  bubbleSize.width();
    qreal height = bubbleSize.height();

    qreal dx =  ( bubbleOffset.x() > 0 ) ? 1.0 : -1.0; // x-Mirror
    qreal dy =  ( bubbleOffset.y() < 0 ) ? 1.0 : -1.0; // y-Mirror

    qreal x0 =  bubbleOffset.x() - dx * ( 1.0 - arrowPosition ) * ( width - 2.0 * xRnd ) - xRnd * dx;
    qreal x1 =  bubbleOffset.x() - dx * ( 1.0 - arrowPosition ) * ( width - 2.0 * xRnd );
    qreal x2 =  bubbleOffset.x() - dx * ( 1.0 - arrowPosition ) * ( width - 2.0 * xRnd ) + xRnd * dx;
    qreal x3 =  bubbleOffset.x() - dx * arrowWidth / 2.0;
    qreal x4 =  bubbleOffset.x() + dx * arrowWidth / 2.0;
    qreal x5 =  bubbleOffset.x() + dx * arrowPosition * ( width - 2.0 * xRnd )- xRnd * dx;
    qreal x6 =  bubbleOffset.x() + dx * arrowPosition * ( width - 2.0 * xRnd );
    qreal x7 =  bubbleOffset.x() + dx * arrowPosition * ( width - 2.0 * xRnd ) + xRnd * dx;

    qreal y0 =  bubbleOffset.y();
    qreal y1 =  bubbleOffset.y() - dy * yRnd;
    qreal y2 =  bubbleOffset.y() - dy * 2 * yRnd;
    qreal y5 =  bubbleOffset.y() - dy * ( height - 2 * yRnd );
    qreal y6 =  bubbleOffset.y() - dy * ( height - yRnd );
    qreal y7 =  bubbleOffset.y() - dy * height;

    path.moveTo( 0, 0 );
    path.lineTo( x4, y0 );

    path.lineTo( x6, y0 );
    QRectF bottomRight( QPointF( x5, y2 ), QPointF( x7, y0 ) );
    path.arcTo( bottomRight, 270.0, 90.0 );

    path.lineTo( x7, y6 );
    QRectF topRight( QPointF( x5, y7 ), QPointF( x7, y5 ) );
    path.arcTo( topRight, 0.0, 90.0 );

    path.lineTo( x1, y7 );
    QRectF topLeft( QPointF( x0, y7 ), QPointF( x2, y5 ) );
    path.arcTo( topLeft, 90.0, 90.0 );

    path.lineTo( x0, y1 );
    QRectF bottomLeft( QPointF( x0, y2 ), QPointF( x2, y0 ) );
    path.arcTo( bottomLeft, 180.0, 90.0 );

    path.lineTo( x1, y0 );
    path.lineTo( x3, y0 );
    path.lineTo( 0, 0 );

    qreal left   = ( dx > 0 ) ? x1 : x6;
    qreal right  = ( dx > 0 ) ? x6 : x1;
    qreal top    = ( dy > 0 ) ? y6 : y1;
    qreal bottom = ( dy > 0 ) ? y1 : y6;

    rect.setTopLeft( QPointF( left, top ) );
    rect.setBottomRight( QPointF( right, bottom ) );
}

AnnotationGraphicsItem::AnnotationGraphicsItem( const GeoDataCoordinates &position,
                                                const QString &text,
                                                const QSizeF &bubbleSize,
                                                const QPointF &bubbleOffset,
                                                qreal xRnd, qreal yRnd ) :
    BillboardGraphicsItem(),
    d( new Private( text, bubbleOffset, xRnd, yRnd ) )
{
    setCoordinate( position );
    setAlignment( Qt::AlignBottom | Qt::AlignRight );
    if ( bubbleSize.height() <= 0 ) {
        const QRect rect = QRect( QPoint( 0, 0 ), ( bubbleSize - QSizeF( 2 * xRnd, 0.0 ) ).toSize() );
        const qreal idealTextHeight = QFontMetrics( QFont() ).boundingRect( rect, Qt::TextWordWrap, text ).height();
        setSize( QSize( bubbleSize.width(), 2 * yRnd + idealTextHeight ) );
    }
    else {
        setSize( bubbleSize );
    }
}

AnnotationGraphicsItem::~AnnotationGraphicsItem()
{
    delete d;
}

void AnnotationGraphicsItem::paint( QPainter *painter )
{
    QPainterPath path;
    QRectF rect;
    d->createAnnotationLayout( size(), d->m_bubbleOffset, d->m_xRnd, d->m_yRnd, path, rect );
    painter->drawPath( path );
    painter->drawText( rect, Qt::TextWordWrap, d->m_text, &rect );
}

} // Marble namespace
