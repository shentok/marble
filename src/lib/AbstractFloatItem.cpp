//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008      Torsten Rahn   <rahn@kde.org>
//

// Self
#include "AbstractFloatItem.h"

// Qt
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QtGui/QDialog>
#include <QtGui/QStandardItem>

// Marble
#include "DialogConfigurationInterface.h"
#include "MarbleDebug.h"
#include "PluginItemDelegate.h"

namespace Marble
{

class AbstractFloatItemPrivate
{
  public:
    AbstractFloatItemPrivate( const MarbleModel *marbleModel ) :
        m_marbleModel( marbleModel ),
        m_action( 0 ),
        m_item(),
        m_enabled( false ),
        m_contextMenu( 0 )
    {
    }

    ~AbstractFloatItemPrivate()
    {
        delete m_contextMenu;
    }


    static QPen         s_pen;
    static QFont        s_font;

    const MarbleModel *const m_marbleModel;

    QAction m_action;
    QStandardItem m_item;

    bool m_enabled;

    QMenu* m_contextMenu;
};

QPen         AbstractFloatItemPrivate::s_pen = QPen( Qt::black );
#ifdef Q_OS_MACX
    QFont AbstractFloatItemPrivate::s_font = QFont( "Sans Serif", 10 );
#else
    QFont AbstractFloatItemPrivate::s_font = QFont( "Sans Serif", 8 );
#endif

AbstractFloatItem::AbstractFloatItem( const MarbleModel *marbleModel, const QPointF &point, const QSizeF &size )
    : FrameGraphicsItem(),
      d( new AbstractFloatItemPrivate( marbleModel ) )
{
    setCacheMode( MarbleGraphicsItem::ItemCoordinateCache );
    setFrame( RectFrame );
    setPadding( 4.0 );
    setContentSize( size );
    setPosition( point );

    connect( &d->m_action, SIGNAL( toggled( bool ) ),
             this,         SLOT( setVisible( bool ) ) );
}

AbstractFloatItem::~AbstractFloatItem()
{
    delete d;
}

const MarbleModel *AbstractFloatItem::marbleModel() const
{
    return d->m_marbleModel;
}

QAction *AbstractFloatItem::action() const
{
    d->m_action.setCheckable( true );
    d->m_action.setChecked( visible() );
    d->m_action.setIcon( icon() );
    d->m_action.setText( guiString() );
    d->m_action.setToolTip( description() );

    return &d->m_action;
}

QStandardItem *AbstractFloatItem::item()
{
    d->m_item.setIcon( icon() );
    d->m_item.setText( name() );
    d->m_item.setEditable( false );
    d->m_item.setCheckable( true );
    d->m_item.setCheckState( enabled() ?  Qt::Checked : Qt::Unchecked  );
    d->m_item.setToolTip( description() );
    d->m_item.setFlags( d->m_item.flags() & ~Qt::ItemIsSelectable );

    // Custom data
    d->m_item.setData( nameId(), PluginItemDelegate::NameId );
    d->m_item.setData( (bool) qobject_cast<DialogConfigurationInterface *>( this ), PluginItemDelegate::ConfigurationDialogAvailable );
    d->m_item.setData( backendTypes(), PluginItemDelegate::BackendTypes );

    return &d->m_item;
}

bool AbstractFloatItem::enabled() const
{
    return d->m_enabled;
}

void AbstractFloatItem::setEnabled( bool enabled )
{
    d->m_enabled = enabled;
}

void AbstractFloatItem::restoreDefaultSettings()
{
    setSettings( QHash<QString, QVariant>() );
}

QPen AbstractFloatItem::pen() const
{
    return d->s_pen;
}

void AbstractFloatItem::setPen( const QPen &pen )
{
    d->s_pen = pen;
    update();
}

QFont AbstractFloatItem::font() const
{
    return d->s_font;
}

void AbstractFloatItem::setFont( const QFont &font )
{
    d->s_font = font;
    update();
}

QString AbstractFloatItem::renderPolicy() const
{
    return "ALWAYS";
}

QStringList AbstractFloatItem::renderPosition() const
{
    return QStringList( "FLOAT_ITEM" );
}

void AbstractFloatItem::setVisible( bool visible )
{
    FrameGraphicsItem::setVisible( visible );
}

void AbstractFloatItem::setPositionLocked( bool lock )
{
    ScreenGraphicsItem::GraphicsItemFlags flags = this->flags();

    if ( lock ) {
        flags &= ~ScreenGraphicsItem::ItemIsMovable;
    }
    else {
        flags |= ScreenGraphicsItem::ItemIsMovable;
    }

    setFlags( flags );
}

bool AbstractFloatItem::positionLocked()
{
    return ( flags() & ScreenGraphicsItem::ItemIsMovable ) ? false : true;
}

QHash<QString, QVariant> AbstractFloatItem::settings() const
{
    return QHash<QString, QVariant>();
}

void AbstractFloatItem::setSettings( const QHash<QString, QVariant> &settings )
{
    Q_UNUSED( settings )
}

void AbstractFloatItem::applyItemState()
{
    setEnabled( d->m_item.checkState() == Qt::Checked );
}

void AbstractFloatItem::retrieveItemState()
{
    d->m_item.setCheckState( enabled() ?  Qt::Checked : Qt::Unchecked  );
}

bool AbstractFloatItem::eventFilter( QObject *object, QEvent *e )
{
    if ( !visible() ) {
        return false;
    }

    if( e->type() == QEvent::ContextMenu )
    {
        QWidget *widget = dynamic_cast<QWidget *>( object );
        QContextMenuEvent *menuEvent = dynamic_cast<QContextMenuEvent *> ( e );
        if( widget != NULL && menuEvent != NULL && contains( menuEvent->pos() ) )
        {
            contextMenuEvent( widget, menuEvent );
            return true;
        }
        return false;
    }
    else if( e->type() == QEvent::ToolTip )
    {
        QHelpEvent *helpEvent = dynamic_cast<QHelpEvent *>( e );
        if( helpEvent != NULL && contains( helpEvent->pos() ) )
        {
            toolTipEvent( helpEvent );
            return true;
        }
        return false;
    }
    else
        return ScreenGraphicsItem::eventFilter( object, e );
}

void AbstractFloatItem::contextMenuEvent ( QWidget *w, QContextMenuEvent *e )
{
    contextMenu()->exec( w->mapToGlobal( e->pos() ) );
}

void AbstractFloatItem::toolTipEvent ( QHelpEvent *e )
{
    Q_UNUSED( e );
}

void AbstractFloatItem::show()
{
    setVisible( true );
}

void AbstractFloatItem::hide()
{
    setVisible( false );
}

QMenu* AbstractFloatItem::contextMenu()
{
    if ( !d->m_contextMenu )
    {
        d->m_contextMenu = new QMenu;

        QAction *lockAction = d->m_contextMenu->addAction( tr( "&Lock" ) );
        lockAction->setCheckable( true );
        lockAction->setChecked( positionLocked() );
        connect( lockAction, SIGNAL( triggered( bool ) ), this, SLOT( setPositionLocked( bool ) ) );
        QAction *hideAction = d->m_contextMenu->addAction( tr( "&Hide" ) );
        connect( hideAction, SIGNAL( triggered() ), this, SLOT( hide() ) );
        DialogConfigurationInterface *configInterface = qobject_cast<DialogConfigurationInterface *>( this );
        QDialog *dialog = configInterface ? configInterface->configDialog() : 0;
        if( dialog )
        {
            d->m_contextMenu->addSeparator();
            QAction *configAction = d->m_contextMenu->addAction( tr( "&Configure..." ) );
            connect( configAction, SIGNAL( triggered() ), dialog, SLOT( exec() ) );
        }
    }

    Q_ASSERT( d->m_contextMenu );
    return d->m_contextMenu;
}

}

#include "AbstractFloatItem.moc"
