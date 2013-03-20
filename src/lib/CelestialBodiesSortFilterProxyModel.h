//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2012      Illya Kovalevskyy  <illya.kovalevskyy@gmail.com>
// Coprright 2013      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#ifndef MARBLE_CELESTIALBODIESSORTFILTERPROXYMODEL_H
#define MARBLE_CELESTIALBODIESSORTFILTERPROXYMODEL_H

#include <QtGui/QSortFilterProxyModel>

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QVariant>

namespace Marble
{

class CelestialSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

 public:
    explicit CelestialSortFilterProxyModel( QObject *parent = 0 );

    ~CelestialSortFilterProxyModel();

    // A small trick to change names for dwarfs and moons
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

 protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

 private:
    void setupPriorities();
    void setupMoonsList();
    void setupDwarfsList();

    QMap<QString, int> m_priority;
    QList<QString> m_moons;
    QList<QString> m_dwarfs;
};

}

#endif // MARBLE_CELESTIALBODIESSORTFILTERPROXYMODEL_H
