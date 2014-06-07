//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2014 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include "SearchItem.h"

#include <QPainter>

using namespace Marble;

SearchItem::SearchItem( QObject *parent ) :
    AbstractDataPluginItem( parent ),
    m_pixmap( QPixmap( 15, 15 ) )
{
    setSize( m_pixmap.size() );

    m_pixmap.fill( Qt::black );

    setCacheMode( ItemCoordinateCache );
}
 
SearchItem::~SearchItem()
{
}
 
QString SearchItem::itemType() const
{
    return "searchItem";
}
 
bool SearchItem::initialized() const
{
    return true;
}

bool SearchItem::operator<( const AbstractDataPluginItem *other ) const
{
    return id() < other->id();
}

void SearchItem::paint( QPainter *painter )
{
    painter->drawPixmap( 0, 0, m_pixmap );
}

QString SearchItem::name() const
{
    return m_name;
}

void SearchItem::setName( const QString &name )
{
    m_name = name;
}

void SearchItem::setPixmap(const QPixmap &pixmap)
{
    if ( pixmap.isNull() )
        return;

    m_pixmap = pixmap;
    setSize( pixmap.size() );

    update();
}

#include "SearchItem.moc"
