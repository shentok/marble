//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010,2011  Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include "MarbleGLView.h"

#include <QtGui/QPaintEvent>

#include "GeoPainter.h"
#include "MarbleMap.h"
#include "ViewportParams.h"

namespace Marble
{

class MarbleGLView::Private
{
 public:
    Private( MarbleMap *map, MarbleGLView *widget )
        : m_widget( widget ),
          m_map( map )
    {
        // Widget settings
        m_widget->setFocusPolicy( Qt::WheelFocus );
        m_widget->setFocus( Qt::OtherFocusReason );
#if QT_VERSION >= 0x40600
        m_widget->setAttribute( Qt::WA_AcceptTouchEvents );
#endif

        // Set background: black.
        m_widget->setPalette( QPalette ( Qt::black ) );

        // Set whether the black space gets displayed or the earth gets simply 
        // displayed on the widget background.
        m_widget->setAutoFillBackground( true );

        m_widget->setMouseTracking( m_widget );
    }

    ~Private()
    {
    }

    MarbleGLView *const m_widget;
    MarbleMap    *const m_map;
};


MarbleGLView::MarbleGLView( MarbleMap *map, QWidget *parent )
    : QGLWidget( parent ),
      d( new MarbleGLView::Private( map, this ) )
{
//    setAttribute( Qt::WA_PaintOnScreen, true );
    setAutoFillBackground( false );
    setAutoBufferSwap( true );

    connect( map, SIGNAL( repaintNeeded( const QRegion & ) ), SLOT( update() ) );
}

MarbleGLView::~MarbleGLView()
{
    delete d;
}

// ----------------------------------------------------------------


void MarbleGLView::resizeGL( int width, int height )
{
    d->m_map->setSize( width, height );

    QGLWidget::resizeGL( width, height );
}

void MarbleGLView::paintEvent( QPaintEvent *evt )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glDepthFunc( GL_LEQUAL );

    // Create a painter that will do the painting.
    GeoPainter painter( this, d->m_map->viewport(), d->m_map->mapQuality() );
    QRect  dirtyRect = evt->rect();

    d->m_map->paint( painter, dirtyRect );

    makeCurrent();
}

}

#include "MarbleGLView.moc"
