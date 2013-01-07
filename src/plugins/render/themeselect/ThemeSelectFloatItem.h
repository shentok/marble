//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008 Dennis Nienhüser <earthwings@gentoo.org>
// Copyright 2013 Mohammed Nafees  <nafees.technocool@gmail.com>
//

#ifndef THEMESELECT_FLOAT_ITEM_H
#define THEMESELECT_FLOAT_ITEM_H

#include "AbstractFloatItem.h"

#include "LabelGraphicsItem.h"

namespace Ui
{
    class ThemeSelect;
}

namespace Marble
{

class MarbleWidget;
class LabelGraphicsItem;

/**
 * @short Provides a float item with zoom and move controls
 *
 */
class ThemeSelectFloatItem: public AbstractFloatItem
{
    Q_OBJECT

    Q_INTERFACES( Marble::RenderPluginInterface )

MARBLE_PLUGIN( ThemeSelectFloatItem )

 public:
    ThemeSelectFloatItem();
    explicit ThemeSelectFloatItem( const MarbleModel *marbleModel );
    ~ThemeSelectFloatItem();

    QStringList backendTypes() const;

    QString name() const;

    QString guiString() const;

    QString nameId() const;

    QString version() const;

    QString description() const;

    QString copyrightYears() const;

    QList<PluginAuthor> pluginAuthors() const;

    QIcon icon() const;

    void initialize();

    bool isInitialized() const;

 protected:
    bool eventFilter( QObject *object, QEvent *e );

 private:
    /** MarbleWidget this float item is installed as event filter for */
    MarbleWidget *m_marbleWidget;

    /** The GraphicsItem presenting the widgets. ThemeSelectFloatItem doesn't take direct ownership
        of this */
    LabelGraphicsItem m_widgetItem;
};

}

#endif // THEMESELECT_FLOAT_ITEM_H
