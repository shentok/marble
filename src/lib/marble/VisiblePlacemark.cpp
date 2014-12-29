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

#include "VisiblePlacemark.h"

#include "MarbleDebug.h"
#include "RemoteIconLoader.h"

#include "AbstractProjection.h"
#include "GeoDataFeature.h"
#include "GeoDataStyle.h"
#include "GeoPainter.h"
#include "PlacemarkLayer.h"
#include "ViewportParams.h"

#include <QApplication>
#include <QPainter>
#include <QPalette>

using namespace Marble;

VisiblePlacemark::VisiblePlacemark( const GeoDataFeature *feature ) :
    GeoGraphicsItem( feature ),
    m_selected( false )
{
    const GeoDataStyle *style = feature->style();
    const RemoteIconLoader *remoteLoader = style->iconStyle().remoteIconLoader();
    QObject::connect( remoteLoader, SIGNAL(iconReady()),
                     this, SLOT(setSymbolPixmap()) );

    drawLabelPixmap();
}

void VisiblePlacemark::paint( GeoPainter *painter, const ViewportParams *viewport )
{
    const GeoDataStyle* style = feature()->style();
    if ( style ) {
        m_symbolPixmap = QPixmap::fromImage( style->iconStyle().icon() );
        m_symbolRect.setSize( m_symbolPixmap.size() );
    } else {
        mDebug() << "Style pointer null";
    }

    QRect labelRect = this->labelRect().toRect();
    QPoint symbolPos = m_symbolRect.topLeft();

    // when the map is such zoomed out that a given place
    // appears many times, we draw one placemark at each
    if ( viewport->currentProjection()->repeatableX() ) {
        const int symbolX = m_symbolRect.left();
        const int textX =   this->labelRect().x();

        for ( int i = symbolX % ( 4 * viewport->radius() );
             i <= viewport->width();
             i += 4 * viewport->radius() )
        {
            labelRect.moveLeft(i - symbolX + textX );
            symbolPos.setX( i );

            painter->drawPixmap( symbolPos, m_symbolPixmap );
            painter->drawPixmap( labelRect, m_labelPixmap );
        }
    }
    else { // simple case, one draw per placemark
        painter->drawPixmap( symbolPos, m_symbolPixmap );
        painter->drawPixmap( labelRect, m_labelPixmap );
    }
}

bool VisiblePlacemark::selected() const
{
    return m_selected;
}

void VisiblePlacemark::setSelected( bool selected )
{
    m_selected = selected;
    drawLabelPixmap();
}

const QRect& VisiblePlacemark::symbolRect() const
{
    return m_symbolRect;
}

const QPointF VisiblePlacemark::hotSpot() const
{
    const QSize iconSize = feature()->style()->iconStyle().icon().size();

    GeoDataHotSpot::Units xunits;
    GeoDataHotSpot::Units yunits;
    QPointF pixelHotSpot = feature()->style()->iconStyle().hotSpot( xunits, yunits );

    switch ( xunits ) {
    case GeoDataHotSpot::Fraction:
        pixelHotSpot.setX( iconSize.width() * pixelHotSpot.x() );
        break;
    case GeoDataHotSpot::Pixels:
        /* nothing to do */
        break;
    case GeoDataHotSpot::InsetPixels:
        pixelHotSpot.setX( iconSize.width() - pixelHotSpot.x() );
        break;
    }

    switch ( yunits ) {
    case GeoDataHotSpot::Fraction:
        pixelHotSpot.setY( iconSize.height() * ( 1.0 - pixelHotSpot.y() ) );
        break;
    case GeoDataHotSpot::Pixels:
        /* nothing to do */
        break;
    case GeoDataHotSpot::InsetPixels:
        pixelHotSpot.setY( iconSize.height() - pixelHotSpot.y() );
        break;
    }

    return pixelHotSpot;
}

void VisiblePlacemark::setSymbolPosition( const QPoint& position )
{
    m_symbolRect.setTopLeft( position );
}

void VisiblePlacemark::setSymbolPixmap()
{
    const GeoDataStyle *style = feature()->style();
    if ( style ) {
        m_symbolPixmap = QPixmap::fromImage( style->iconStyle().icon() );
        emit updateNeeded();
    }
    else {
        mDebug() << "Style pointer is Null";
    }
}

const QRectF& VisiblePlacemark::labelRect() const
{
    return m_labelRect;
}

void VisiblePlacemark::setLabelRect( const QRectF& labelRect )
{
    m_labelRect = labelRect;
}

void VisiblePlacemark::drawLabelPixmap()
{
    const GeoDataStyle* style = feature()->style();

    QString labelName = feature()->name();
    if ( labelName.isEmpty() ) {
        m_labelPixmap = QPixmap();
        return;
    }

    QFont  labelFont  = style->labelStyle().font();
    QColor labelColor = style->labelStyle().color();

    LabelStyle labelStyle = Normal;
    if ( m_selected ) {
        labelStyle = Selected;
    } else if ( style->labelStyle().glow() ) {
        labelStyle = Glow;
    }

    int textHeight = QFontMetrics( labelFont ).height();

    int textWidth;
    if ( style->labelStyle().glow() ) {
        labelFont.setWeight( 75 ); // Needed to calculate the correct pixmap size;
        textWidth = ( QFontMetrics( labelFont ).width( labelName )
            + qRound( 2 * s_labelOutlineWidth ) );
    } else {
        textWidth = ( QFontMetrics( labelFont ).width( labelName ) );
    }


    // Due to some XOrg bug this requires a workaround via
    // QImage in some cases (at least with Qt 4.2).
    if ( !PlacemarkLayer::m_useXWorkaround ) {
        m_labelPixmap = QPixmap( QSize( textWidth, textHeight ) );
        m_labelPixmap.fill( Qt::transparent );

        QPainter labelPainter( &m_labelPixmap );

        drawLabelText( labelPainter, labelName, labelFont, labelStyle, labelColor );
    } else {
        QImage image( QSize( textWidth, textHeight ),
                      QImage::Format_ARGB32_Premultiplied );
        image.fill( 0 );

        QPainter labelPainter( &image );

        drawLabelText( labelPainter, labelName, labelFont, labelStyle, labelColor );

        labelPainter.end();

        m_labelPixmap = QPixmap::fromImage( image );
    }
}

void VisiblePlacemark::drawLabelText(QPainter &labelPainter, const QString &text,
                                            const QFont &labelFont, LabelStyle labelStyle, const QColor &color )
{
    QFont font = labelFont;
    QFontMetrics metrics = QFontMetrics( font );
    int fontAscent = metrics.ascent();

    switch ( labelStyle ) {
    case Selected: {
        labelPainter.setPen( color );
        labelPainter.setFont( font );
        QRect textRect( 0, 0, metrics.width( text ), metrics.height() );
        labelPainter.fillRect( textRect, QApplication::palette().highlight() );
        labelPainter.setPen( QPen( QApplication::palette().highlightedText(), 1 ) );
        labelPainter.drawText( 0, fontAscent, text );
        break;
    }
    case Glow: {
        font.setWeight( 75 );
        fontAscent = QFontMetrics( font ).ascent();

        QPen outlinepen( color == QColor( Qt::white ) ? Qt::black : Qt::white );
        outlinepen.setWidthF( s_labelOutlineWidth );
        QBrush  outlinebrush( color );

        QPainterPath outlinepath;

        const QPointF  baseline( s_labelOutlineWidth / 2.0, fontAscent );
        outlinepath.addText( baseline, font, text );
        labelPainter.setRenderHint( QPainter::Antialiasing, true );
        labelPainter.setPen( outlinepen );
        labelPainter.setBrush( outlinebrush );
        labelPainter.drawPath( outlinepath );
        labelPainter.setPen( Qt::NoPen );
        labelPainter.drawPath( outlinepath );
        labelPainter.setRenderHint( QPainter::Antialiasing, false );
        break;
    }
    default: {
        labelPainter.setPen( color );
        labelPainter.setFont( font );
        labelPainter.drawText( 0, fontAscent, text );
    }
    }
}

#include "VisiblePlacemark.moc"
