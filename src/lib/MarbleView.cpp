//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010,2011  Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include "MarbleView.h"
#include "MarbleView.moc"

#include <cmath>

#include <QtCore/QTime>
#include <QtGui/QPaintEvent>

#include "AbstractProjection.h"
#include "GeoPainter.h"
#include "GeoSceneDocument.h"
#include "GeoSceneMap.h"
#include "GeoSceneTexture.h"
#include "MarbleDebug.h"
#include "MarbleMap.h"
#include "MarbleModel.h"
#include "MeasureTool.h"
#include "PlacemarkLayout.h"
#include "SunLocator.h"
#include "TileLoaderHelper.h"
#include "TileId.h"
#include "VectorComposer.h"
#include "ViewParams.h"
#include "ViewportParams.h"

namespace Marble
{

class MarbleView::Private
{
 public:
    Private( MarbleMap *map, MarbleView *widget )
        : q( widget ),
          m_map( map )
    {
        // Widget settings
        q->setFocusPolicy( Qt::WheelFocus );
        q->setFocus( Qt::OtherFocusReason );
#if QT_VERSION >= 0x40600
        q->setAttribute( Qt::WA_AcceptTouchEvents );
#endif

        // Set background: black.
        q->setPalette( QPalette ( Qt::black ) );

        // Set whether the black space gets displayed or the earth gets simply 
        // displayed on the widget background.
        q->setAutoFillBackground( true );

        q->setMouseTracking( q );
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

    MarbleView *const q;
    MarbleMap    *const m_map;
};


MarbleView::MarbleView( MarbleMap *map, QWidget *parent )
    : QWidget( parent ),
      d( new MarbleView::Private( map, this ) )
{
//    setAttribute( Qt::WA_PaintOnScreen, true );

    setAutoFillBackground( false );
}

MarbleView::~MarbleView()
{
    delete d;
}

void MarbleView::Private::update()
{
    q->setAttribute( Qt::WA_NoSystemBackground, true );
    q->update();
}

// ----------------------------------------------------------------


void MarbleView::resizeEvent( QResizeEvent *event )
{
    QWidget::resizeEvent( event );

    setUpdatesEnabled( false );
    d->m_map->setSize( width(), height() );
    d->update();
    setUpdatesEnabled( true );
}

void MarbleView::paintEvent( QPaintEvent *evt )
{
    QTime t;
    t.start();

    // FIXME: Better way to get the GeoPainter
    bool  doClip = true;
    if ( d->m_map->projection() == Spherical )
        doClip = ( d->m_map->radius() > width() / 2
                   || d->m_map->radius() > height() / 2 );

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
    GeoPainter painter( paintDevice, d->m_map->viewport(),
                        d->m_map->mapQuality(), doClip );
    QRect  dirtyRect = evt->rect();

    // Draws the map like MarbleMap::paint does, but adds our customPaint in between
    d->m_map->paint( painter, dirtyRect );

    if ( !isEnabled() )
    {
        // Draw a grayscale version of the intermediate image
        QRgb* pixel = reinterpret_cast<QRgb*>( image.scanLine( 0 ));
        for (int i=0; i<image.width()*image.height(); ++i, ++pixel) {
            int gray = qGray( *pixel );
            *pixel = qRgb( gray, gray, gray );
        }

        GeoPainter widgetPainter( this, d->m_map->viewport(),
                            d->m_map->mapQuality(), doClip );
        widgetPainter.drawImage( rect(), image );
    }
}

}
