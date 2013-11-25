//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010 Dennis Nienhüser <earthwings@gentoo.org>

#include "HostipRunner.h"

#include "MarbleDebug.h"
#include "GeoDataDocument.h"
#include "GeoDataFeature.h"
#include "GeoDataPlacemark.h"

#include <QString>
#include <QTimer>
#include <QVector>
#include <QUrl>
#include <QNetworkReply>

namespace Marble
{

HostipRunner::HostipRunner( QObject *parent ) :
        SearchRunner( parent ),
        m_reply( 0 )
{
}

HostipRunner::~HostipRunner()
{
    if ( m_reply ) {
        m_reply->deleteLater();
    }
}

void HostipRunner::slotNoResults()
{
    emit searchFinished( QVector<GeoDataPlacemark*>() );
}

void HostipRunner::search( const QString &searchTerm, const GeoDataLatLonAltBox & )
{
    if( !searchTerm.contains('.') ) {
        // Simple IP/hostname heuristic to avoid requests not needed:
        // String must contain at least one dot.
        slotNoResults();
    }
    else {
        QEventLoop eventLoop;

        QTimer timer;
        timer.setSingleShot( true );
        timer.setInterval( 15000 );

        connect( &timer, SIGNAL(timeout()),
                 &eventLoop, SLOT(quit()));
        connect( this, SIGNAL(searchFinished(QVector<GeoDataPlacemark*>)),
                 &eventLoop, SLOT(quit()) );

        // Lookup the IP address for a hostname, or the hostname if an IP address was given
        QHostInfo ::lookupHost( searchTerm, this, SLOT(slotLookupFinished(QHostInfo)));
        timer.start();

        eventLoop.exec();
    }
}

void HostipRunner::slotLookupFinished(const QHostInfo &info)
{
    if ( !info.addresses().isEmpty() ) {
        m_hostInfo = info;
        QString hostAddress = info.addresses().first().toString();
        QString query = QString( "http://api.hostip.info/get_html.php?ip=%1&position=true" ).arg( hostAddress );
        m_request.setUrl( QUrl( query ) );

        // @todo FIXME Must currently be done in the main thread, see bug 257376
        QTimer::singleShot( 0, this, SLOT(get()) );
    }
    else
      slotNoResults();
}

void HostipRunner::get()
{
    m_reply = networkAccessManager()->get( m_request );
    connect( m_reply, SIGNAL(finished()),
            this, SLOT(slotRequestFinished()), Qt::DirectConnection );
    connect( m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
             this, SLOT(slotNoResults()), Qt::DirectConnection );
}

void HostipRunner::slotRequestFinished()
{
    double lon(0.0), lat(0.0);
    for ( QString line = m_reply->readLine(); !line.isEmpty(); line = m_reply->readLine() ) {
        QString lonInd = "Longitude: ";
        if ( line.startsWith(lonInd) ) {
            lon = line.mid( lonInd.length() ).toDouble();
        }

        QString latInd = "Latitude: ";
        if (line.startsWith( latInd) ) {
            lat = line.mid( latInd.length() ).toDouble();
        }
    }

    QVector<GeoDataPlacemark*> placemarks;

    if (lon != 0.0 && lat != 0.0) {
        GeoDataPlacemark *placemark = new GeoDataPlacemark;

        placemark->setName( m_hostInfo.hostName() );

        QString description("%1 (%2)");
        placemark->setDescription( description.
                                 arg( m_hostInfo.hostName() ).
                                 arg( m_hostInfo.addresses().first().toString() ) );

        placemark->setCoordinate( lon * DEG2RAD, lat * DEG2RAD );
        placemark->setVisualCategory( GeoDataFeature::Coordinate );
        placemarks << placemark;
    }
    
    emit searchFinished( placemarks );
}

} // namespace Marble

#include "HostipRunner.moc"
