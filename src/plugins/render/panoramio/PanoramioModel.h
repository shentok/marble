//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Bastian Holst <bastianholst@gmx.de>
//

#ifndef PANORAMIOMODEL_H
#define PANORAMIOMODEL_H

#include "AbstractDataPluginModel.h"

#include <QThreadPool>

struct panoramioDataStructure;

namespace Marble
{

class MarbleWidget;

class PanoramioWorker : public QObject, public QRunnable
{
    Q_OBJECT

 public:
    PanoramioWorker( const QByteArray &data, const qint32 count );

    void run();

 Q_SIGNALS:
    void finished( const QList<panoramioDataStructure> &list );

 private:
    QByteArray m_data;
    qint32 m_count;
};

class PanoramioModel : public AbstractDataPluginModel
{
    Q_OBJECT

 public:
    explicit PanoramioModel( const MarbleModel *marbleModel, QObject *parent = 0 );

    void setMarbleWidget( MarbleWidget *widget );

 protected:
    /**
     * Generates the download url for the description file from the web service depending on
     * the @p box surrounding the view and the @p number of files to show.
     **/
    void getAdditionalItems( const GeoDataLatLonAltBox &box, qint32 number = 10 );

    /**
     * The reimplementation has to parse the @p file and should generate widgets. This widgets
     * have to be scheduled to downloadWidgetData or could be directly added to the list,
     * depending on if they have to download information to be shown.
     **/
    void parseFile( const QByteArray &file );

 private Q_SLOTS:
    void addItems( const QList<panoramioDataStructure> &list );

private:
    MarbleWidget *m_marbleWidget;
    qint32 m_lastNumber;
    QByteArray m_lastData;
    QThreadPool m_threadPool;
};

}

#endif // PANORAMIOMODEL_H
