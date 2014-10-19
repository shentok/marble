//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010      Dennis Nienhüser <nienhueser@kde.org>
//

#ifndef MARBLE_ALTERNATIVEROUTESMODEL_H
#define MARBLE_ALTERNATIVEROUTESMODEL_H

#include "marble_export.h"

#include <QAbstractListModel>

#include "routing/Route.h"

/**
  * A QAbstractItemModel that contains a list of routing instructions.
  * Each item represents a routing step in the way from source to
  * destination. Steps near the source come first, steps near the target
  * last.
  */
namespace Marble
{

class Route;
class RouteRequest;

class MARBLE_EXPORT AlternativeRoutesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum WritePolicy {
        Instant,
        Lazy
    };

    /** Constructor */
    explicit AlternativeRoutesModel( QObject *parent = 0 );

    /** Destructor */
    ~AlternativeRoutesModel() override;

    // Model querying

    /** Overload of QAbstractListModel */
    int rowCount ( const QModelIndex &parent = QModelIndex() ) const override;

    /** Overload of QAbstractListModel */
    QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

    /** Overload of QAbstractListModel */
    QVariant data ( const QModelIndex &index, int role = Qt::DisplayRole ) const override;

    const Route *route(int index) const;

    // Model data filling

    /** Invalidate the current alternative routes and prepare for new ones to arrive */
    void newRequest( RouteRequest *request );

    /**
      * Old data in the model is discarded, the parsed content of the provided document
      * is used as the new model data and a model reset is done
      * @param route The route to add
      * @param policy In lazy mode (default), a short amount of time is waited for
      *   other addRoute() calls before adding the route to the model. Otherwise, the
      *   model is changed immediately.
      */
    void addRoute(const Route &route, WritePolicy policy = Lazy);

    /** Remove all alternative routes from the model */
    void clear();

    const Route *currentRoute() const;

public Q_SLOTS:
    void setCurrentRoute( int index );

Q_SIGNALS:
    void currentRouteChanged(const Route &newRoute);
    void currentRouteChanged( int index );

private Q_SLOTS:
    void addRestrainedRoutes();

private:
    class Private;
    Private *const d;
};

} // namespace Marble

#endif
