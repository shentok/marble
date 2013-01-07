//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include "ThemeSelectFloatItem.h"

#include <QtCore/qmath.h>
#include <QtCore/QRect>
#include <QtGui/QPixmap>
#include <QtGui/QToolButton>
#include <QtGui/QSlider>
#include <QtGui/QWidget>
#include <QtGui/QPainter>
#include <QPixmapCache>

#include "ui_themeselect.h"
#include "ViewportParams.h"
#include "MarbleDebug.h"
#include "MarbleWidget.h"
#include "WidgetGraphicsItem.h"
#include "MarbleGraphicsGridLayout.h"

using namespace Marble;
/* TRANSLATOR Marble::ThemeSelectFloatItem */

ThemeSelectFloatItem::ThemeSelectFloatItem()
    : AbstractFloatItem( 0 )
{
}

ThemeSelectFloatItem::ThemeSelectFloatItem( const MarbleModel *marbleModel )
    : AbstractFloatItem( marbleModel, QPointF( -10.0, 10.0 ) ),
      m_marbleWidget( 0 ),
      m_widgetItem( this )
{
    // Plugin is visible by default
    setEnabled( true );
    setVisible( true );

    setCacheMode( NoCache );
    setBackground( QBrush( QColor( Qt::transparent ) ) );
    setFrame( NoFrame );

    m_widgetItem.setIcon( QIcon( ":/data/layers.png" ), QSize( 50, 50 ) );

    MarbleGraphicsGridLayout *layout = new MarbleGraphicsGridLayout( 1, 1 );
    layout->addItem( &m_widgetItem, 0, 0 );

    setLayout( layout );
}

ThemeSelectFloatItem::~ThemeSelectFloatItem()
{
}

QStringList ThemeSelectFloatItem::backendTypes() const
{
    return QStringList("themeselect");
}

QString ThemeSelectFloatItem::name() const
{
    return tr("ThemeSelect");
}

QString ThemeSelectFloatItem::guiString() const
{
    return tr("&ThemeSelect");
}

QString ThemeSelectFloatItem::nameId() const
{
    return QString("themeselect");
}

QString ThemeSelectFloatItem::version() const
{
    return "1.0";
}

QString ThemeSelectFloatItem::description() const
{
    return tr("A mouse control to zoom and move the map");
}

QString ThemeSelectFloatItem::copyrightYears() const
{
    return "2013";
}

QList<PluginAuthor> ThemeSelectFloatItem::pluginAuthors() const
{
    return QList<PluginAuthor>()
            << PluginAuthor( "Bernhard Beschow", "bbeschow@cs.tu-berlin.de" );
}

QIcon ThemeSelectFloatItem::icon() const
{
    return QIcon( ":/data/layers.png" );
}

void ThemeSelectFloatItem::initialize()
{
}

bool ThemeSelectFloatItem::isInitialized() const
{
    return true;
}

bool ThemeSelectFloatItem::eventFilter( QObject *object, QEvent *e )
{
    if ( !enabled() || !visible() ) {
        return false;
    }

    MarbleWidget *widget = dynamic_cast<MarbleWidget*> (object);
    if ( !widget ) {
        return AbstractFloatItem::eventFilter( object, e );
    }

    if ( m_marbleWidget != widget ) {
        // Delayed initialization
        m_marbleWidget = widget;
     }

    return AbstractFloatItem::eventFilter(object, e);
}

Q_EXPORT_PLUGIN2( ThemeSelectFloatItem, Marble::ThemeSelectFloatItem )

#include "ThemeSelectFloatItem.moc"
