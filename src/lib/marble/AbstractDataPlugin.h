//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Bastian Holst <bastianholst@gmx.de>
//

#ifndef MARBLE_ABSTRACTDATAPLUGIN_H
#define MARBLE_ABSTRACTDATAPLUGIN_H

#include <QObject>

// Marble
#include "marble_export.h"
#include "PluginInterface.h"

#if QT_VERSION < 0x050000
class QDeclarativeComponent;
#else
class QQmlComponent;
#endif
class QGraphicsItem;

namespace Marble
{
    
class ViewportParams;
class GeoSceneLayer;
class AbstractDataPluginItem;
class AbstractDataPluginModel;
class AbstractDataPluginPrivate;
class MarbleModel;

/**
 * @short An abstract class for plugins that show data that has a geo coordinate
 *
 * This is the abstract class for plugins that show data on Marble map.
 * It takes care of painting all items it gets from the corresponding AbstractDataPluginModel
 * that has to be set on initialisation.
 *
 * The user has to set the nameId as well as the number of items to fetch.
 * Additionally it should be useful to set standard values via setEnabled (often true)
 * and setVisible (often false) in the constructor of a subclass.
 **/
class MARBLE_EXPORT AbstractDataPlugin : public QObject, public PluginInterface
{
    Q_OBJECT

    Q_PROPERTY( bool favoriteItemsOnly READ isFavoriteItemsOnly WRITE setFavoriteItemsOnly NOTIFY favoriteItemsOnlyChanged )
    /** @todo FIXME Qt Quick segfaults if using the real class here instead of QObject */
    Q_PROPERTY( QObject* favoritesModel READ favoritesModel NOTIFY favoritesModelChanged )
    Q_PROPERTY( int numberOfItems READ numberOfItems WRITE setNumberOfItems NOTIFY changedNumberOfItems )
    
 public:    
    explicit AbstractDataPlugin( const MarbleModel *marbleModel );

    virtual ~AbstractDataPlugin();

    virtual AbstractDataPlugin *newInstance( const MarbleModel *marbleModel ) const = 0;

    bool enabled() const;

    bool visible() const;

    /**
     * @brief Settings of the plugin
     *
     * Settings is the map (hash table) of plugin's settings
     * This method is called to determine the current settings of the plugin
     * for serialization, e.g. when closing the application.
     *
     * @return plugin's settings
     * @see setSettings
     */
    virtual QHash<QString,QVariant> settings() const;

    /**
     * @brief Set the settings of the plugin
     *
     * Usually this is called at startup to restore saved settings.
     *
     * @param new plugin's settings
     * @see settings
     */
    virtual void setSettings( const QHash<QString,QVariant> &settings );

    /**
     * @brief Returns the name(s) of the backend that the plugin can render
     */
    QStringList backendTypes() const;
    
    /**
     * @brief Return how the plugin settings should be used.
     */
    QString renderPolicy() const;
    
    /**
     * @brief Preferred level in the layer stack for the rendering
     */
    QStringList renderPosition() const;

    const MarbleModel *marbleModel() const;

    /**
     * @return The model associated with the plugin.
     */
    AbstractDataPluginModel *model();
    const AbstractDataPluginModel *model() const;

    /**
     * Set the model of the plugin.
     */
    void setModel( AbstractDataPluginModel* model );

    /**
     * Set the number of items to be shown at the same time.
     */
    void setNumberOfItems( quint32 number );
    
    /**
     * @return The number of items to be shown at the same time.
     */
    quint32 numberOfItems() const;
    
    /**
     * This function returns all items at the position @p curpos. Depending on where they have
     * been painted the last time.
     *
     * @return The items at the given position.
     */
    QList<AbstractDataPluginItem *> whichItemAt( const QPoint& curpos );

#if QT_VERSION < 0x050000
    void setDelegate( QDeclarativeComponent* delegate, QGraphicsItem* parent );
#else
    void setDelegate( QQmlComponent* delegate, QGraphicsItem* parent );
#endif

    /** Convenience method to set the favorite item state on the current model */
    void setFavoriteItemsOnly( bool favoriteOnly );

    bool isFavoriteItemsOnly() const;

    QObject* favoritesModel();
    
 public Q_SLOTS:
    void setEnabled( bool enabled );
    void setVisible( bool visible );
    void handleViewportChange( const ViewportParams *viewport );

 private Q_SLOTS:
    virtual void favoriteItemsChanged( const QStringList& favoriteItems );

    void delayedUpdate();

 Q_SIGNALS:
    void changedNumberOfItems( quint32 number );

    void favoriteItemsOnlyChanged();

    void favoritesModelChanged();

    void settingsChanged( const QString &nameId );

 private:
    AbstractDataPluginPrivate * const d;
};

#define MARBLE_PLUGIN(T) public:\
    virtual AbstractDataPlugin* newInstance( const MarbleModel *marbleModel ) const { return new T( marbleModel ); }

}

#endif
