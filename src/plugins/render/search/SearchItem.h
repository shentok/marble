//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2014 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#ifndef SEARCHITEM_H
#define SEARCHITEM_H

#include "AbstractDataPluginItem.h"

#include <QPixmap>

namespace Marble
{

class SearchItem : public AbstractDataPluginItem
{
    Q_OBJECT

public:
    explicit SearchItem( QObject *parent );

    ~SearchItem();

    QString itemType() const;

    bool initialized() const;

    void paint( QPainter *painter );

    bool operator<( const AbstractDataPluginItem *other ) const;

    QString name() const;

    void setName( const QString &name );

    void setPixmap( const QPixmap &pixmap );

private:
    QString m_name;
    QPixmap m_pixmap;
};
 
}
#endif // SEARCHITEM_H
