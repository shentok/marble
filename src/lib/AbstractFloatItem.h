//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008 Torsten Rahn <tackat@kde.org>
//

#ifndef MARBLE_ABSTRACTFLOATITEM_H
#define MARBLE_ABSTRACTFLOATITEM_H

#include <QtCore/QPointF>
#include <QtCore/QSizeF>
#include <QtCore/QString>
#include <QtCore/Qt>

#include <QtGui/QPen>
#include <QtGui/QFont>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QHelpEvent>
#include <QtGui/QWidget>

#include "FrameGraphicsItem.h"
#include "PluginInterface.h"
#include "marble_export.h"

class QMenu;
class QStandardItem;

namespace Marble
{

class AbstractFloatItemPrivate;
class MarbleModel;

/**
 * @short The abstract class that creates an "empty" float item.
 *
 */

class MARBLE_EXPORT AbstractFloatItem : public QObject, public PluginInterface, public FrameGraphicsItem
{
    Q_OBJECT

 public:
    explicit AbstractFloatItem( const MarbleModel *marbleModel,
                                const QPointF &point = QPointF( 10.0, 10.0 ),
                                const QSizeF &size = QSizeF( 150.0, 50.0 ) );
    virtual ~AbstractFloatItem();

    virtual QString guiString() const = 0;

    virtual AbstractFloatItem *newInstance( const MarbleModel *model ) const = 0;

    virtual void initialize() = 0;

    virtual bool isInitialized() const = 0;

    /**
     * @brief Returns the name(s) of the backend that the plugin can render
     *.
     * This method should return the name of the backend that the plugin
     * can render. The string has to be the same one that is given
     * for the attribute in the layer element of the DGML file that
     * backend is able to process.
     * Examples to replace available default backends would be "vector" or
     * "texture". To provide a completely new kind of functionality please
     * choose your own unique string.
     */
    virtual QStringList backendTypes() const = 0;

    const MarbleModel *marbleModel() const;

    QAction *action() const;

    QStandardItem *item();

    bool enabled() const;

    QPen pen() const;
    void setPen( const QPen &pen );

    QFont font() const;
    void setFont( const QFont &font );

    virtual QString renderPolicy() const;

    virtual QStringList renderPosition() const;

    bool positionLocked();

    virtual QHash<QString,QVariant> settings() const;

    virtual void setSettings( const QHash<QString, QVariant> &settings );

    void applyItemState();
    void retrieveItemState();

    virtual bool eventFilter( QObject *object, QEvent *e );

 public Q_SLOTS:
    void setPositionLocked( bool lock );
    void show();
    void hide();

    void setVisible( bool visible );
    void setEnabled( bool enabled );
    void restoreDefaultSettings();

 Q_SIGNALS:
    void settingsChanged( const QString &nameId );
    void repaintNeeded( const QRegion &region = QRegion() );

 protected:
    virtual void contextMenuEvent ( QWidget *w, QContextMenuEvent *e );
    virtual void toolTipEvent( QHelpEvent *e );
    QMenu* contextMenu();

 private:
    Q_DISABLE_COPY( AbstractFloatItem )
    AbstractFloatItemPrivate * const d;
};

}

Q_DECLARE_INTERFACE( Marble::AbstractFloatItem, "org.kde.Marble.AbstractFloatItem/1.00" )

#define MARBLE_FLOATITEM_PLUGIN( T ) \
    Q_INTERFACES( Marble::AbstractFloatItem ) \
    AbstractFloatItem *newInstance( const MarbleModel *model ) const { return new T( model ); }

#endif
