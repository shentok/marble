//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2012   Mohammed Nafees   <nafees.technocool@gmail.com>
// Copyright 2012   Dennis Nienhüser  <earthwings@gentoo.org>
// Copyright 2012   Illya Kovalevskyy <illya.kovalevskyy@gmail.com>
//

#include "PopupLayer.h"

#include "GeoDataCoordinates.h"
#include "GeoPainter.h"
#include "MarbleWidget.h"
#include "ViewportParams.h"

#include <QPen>
#include <QSizeF>

namespace Marble
{

class PopupLayer::Private
{
public:
    Private( MarbleWidget *marbleWidget );

    /**
     * @brief Sets size of the popup item, based on the requested size and viewport size
     * @param viewport required to compute the maximum dimensions
     */
    static QString filterEmptyShortDescription(const QString &description);
    void setupDialogSatellite( const GeoDataPlacemark *index );
    void setupDialogCity( const GeoDataPlacemark *index );
    void setupDialogNation( const GeoDataPlacemark *index );
    void setupDialogGeoPlaces( const GeoDataPlacemark *index );
    void setupDialogSkyPlaces( const GeoDataPlacemark *index );

    MarbleWidget *const m_widget;
    QSizeF m_requestedSize;
    QString m_html;
    QUrl m_url;
    GeoDataCoordinates m_coordinates;
};

PopupLayer::Private::Private( MarbleWidget *marbleWidget ) :
    m_widget( marbleWidget )
{
}

PopupLayer::PopupLayer( MarbleWidget *marbleWidget, QObject *parent ) :
    QObject( parent ),
    d( new Private( marbleWidget ) )
{
}

PopupLayer::~PopupLayer()
{
    delete d;
}

QStringList PopupLayer::renderPosition() const
{
    return QStringList( "HOVERS_ABOVE_SURFACE" );
}

bool PopupLayer::render( GeoPainter *painter, ViewportParams *viewport,
                                const QString&, GeoSceneLayer* )
{
    painter->save();

    QPen pen;
    pen.setCapStyle( Qt::RoundCap );

    qreal x, y;
    viewport->screenCoordinates( d->m_coordinates, x, y );

    pen.setWidth( 2 );
    pen.setColor( Qt::gray );
    painter->setPen( pen );
    painter->drawLine( x, y + 1, viewport->width() + 10, viewport->height() / 2 + 1 );

    pen.setWidth( 2 );
    pen.setColor( Qt::white );
    painter->setPen( pen );
    painter->drawLine( x, y, viewport->width() + 10, viewport->height() / 2 );

    painter->restore();

    return true;
}

RenderState PopupLayer::renderState() const
{
    return RenderState( "Popup Window" );
}

void PopupLayer::popup()
{
    if ( d->m_html.isEmpty() ) {
        emit requestUrl( d->m_url );
    }
    else {
        emit requestHtml( d->m_html, d->m_url );
    }

    d->m_widget->centerOn( d->m_coordinates, true );
}

void PopupLayer::setCoordinates( const GeoDataCoordinates &coordinates , Qt::Alignment alignment )
{
    d->m_coordinates = coordinates;
}

void PopupLayer::setUrl( const QUrl &url )
{
    d->m_html.clear();
    d->m_url = url;
}

void PopupLayer::setContent( const QString &html, const QUrl &baseUrl )
{
    d->m_html = html;
    d->m_url = baseUrl;
}

void PopupLayer::setBackgroundColor(const QColor &color)
{
    if(color.isValid()) {
    }
}

void PopupLayer::setTextColor(const QColor &color)
{
    if(color.isValid()) {
    }
}

void PopupLayer::setSize( const QSizeF &size )
{
    d->m_requestedSize = size;
}

}

#include "PopupLayer.moc"
