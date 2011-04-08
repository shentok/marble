//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010-2012  Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include "MarbleView.h"

#include <QtGui/QPaintEvent>

#include "GeoPainter.h"
#include "MarbleMap.h"
#include "ViewportParams.h"

namespace Marble
{

class MarbleView::Private
{
 public:
    Private( MarbleMap *map, MarbleView *widget )
        : m_widget( widget ),
          m_map( map )
    {
        // Widget settings
        m_widget->setFocusPolicy( Qt::WheelFocus );
        m_widget->setFocus( Qt::OtherFocusReason );
        m_widget->setAttribute( Qt::WA_AcceptTouchEvents );

        // Set background: black.
        m_widget->setPalette( QPalette( Qt::black ) );

        // Set whether the black space gets displayed or the earth gets simply 
        // displayed on the widget background.
        m_widget->setAutoFillBackground( true );

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
    void updateSystemBackgroundAttribute();

    MarbleView *const m_widget;
    MarbleMap    *const m_map;
};


MarbleView::MarbleView( MarbleMap *map, QWidget *parent )
    : QWidget( parent ),
      d( new MarbleView::Private( map, this ) )
{
    setAutoFillBackground( false );

    connect( d->m_map, SIGNAL(visibleLatLonAltBoxChanged(GeoDataLatLonAltBox)),
             this,     SLOT(updateSystemBackgroundAttribute()) );
}

MarbleView::~MarbleView()
{
    delete d;
}

void MarbleView::Private::updateSystemBackgroundAttribute()
{
    // We only have to repaint the background every time if the earth
    // doesn't cover the whole image.
    const bool isOn = m_map->viewport()->mapCoversViewport() && !m_map->mapThemeId().isEmpty();
    m_widget->setAttribute( Qt::WA_NoSystemBackground, isOn );
}

// ----------------------------------------------------------------


void MarbleView::paintEvent( QPaintEvent *evt )
{
    QPaintDevice *paintDevice = this;
    QImage image;
    if ( !isEnabled() ) {
        // If the globe covers fully the screen then we can use the faster
        // RGB32 as there are no translucent areas involved.
        QImage::Format imageFormat = ( d->m_map->viewport()->mapCoversViewport() )
                                     ? QImage::Format_RGB32
                                     : QImage::Format_ARGB32_Premultiplied;
        // Paint to an intermediate image
        image = QImage( rect().size(), imageFormat );
        image.fill( Qt::transparent );
        paintDevice = &image;
    }

    {
        // FIXME: Better way to get the GeoPainter
        // Create a painter that will do the painting.
        GeoPainter geoPainter( paintDevice, d->m_map->viewport(), d->m_map->mapQuality() );

        d->m_map->paint( geoPainter, evt->rect() );
    }

    if ( !isEnabled() ) {
        // Draw a grayscale version of the intermediate image
        QRgb* pixel = reinterpret_cast<QRgb*>( image.scanLine( 0 ) );
        for ( int i = 0; i < image.width()*image.height(); ++i, ++pixel ) {
            int gray = qGray( *pixel );
            *pixel = qRgb( gray, gray, gray );
        }

        QPainter widgetPainter( this );
        widgetPainter.drawImage( rect(), image );
    }
}

}

#include "MarbleView.moc"
