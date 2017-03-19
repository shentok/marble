//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010      Dennis Nienhüser <nienhueser@kde.org>
//

#include "RouteRequest.h"

#include "GeoDataContainer.h"
#include "GeoDataData.h"
#include "GeoDataExtendedData.h"
#include "GeoDataFolder.h"
#include "GeoDataLineString.h"
#include "GeoDataPlacemark.h"
#include "GeoDataTreeModel.h"
#include "MarbleDirs.h"

#include <QMap>
#include <QPainter>
#include <QDebug>

namespace Marble
{

struct PixmapElement
{
    int index;

    int size;

    explicit PixmapElement( int index=-1, int size=0 );

    bool operator < ( const PixmapElement &other ) const;
};

class RouteRequestPrivate
{
public:
    RouteRequestPrivate(GeoDataTreeModel *treeModel, GeoDataContainer *parentContainer);

    ~RouteRequestPrivate();

    GeoDataTreeModel *const m_treeModel;

    GeoDataFolder m_route;

    QMap<PixmapElement, QPixmap> m_pixmapCache;

    RoutingProfile m_routingProfile;

    /** Determines a suitable index for inserting a via point */
    int viaIndex( const GeoDataCoordinates &position ) const;
};

PixmapElement::PixmapElement( int index_, int size_ ) :
    index( index_ ), size( size_ )
{
    // nothing to do
}

bool PixmapElement::operator <(const PixmapElement &other) const
{
    return index < other.index || size < other.size;
}

RouteRequestPrivate::RouteRequestPrivate(GeoDataTreeModel *treeModel, GeoDataContainer *parentContainer) :
    m_treeModel(treeModel),
    m_route()
{
    m_route.setName("Route Request");
    m_route.setParent(parentContainer);
    m_treeModel->addFeature(static_cast<GeoDataContainer *>(m_route.parent()), &m_route);
}

RouteRequestPrivate::~RouteRequestPrivate()
{
    m_treeModel->removeFeature(&m_route);
}

int RouteRequestPrivate::viaIndex( const GeoDataCoordinates &position ) const
{
    /** @todo: Works, but does not look elegant at all */

    // Iterates over all ordered trip point pairs (P,Q) and finds the triple
    // (P,position,Q) or (P,Q,position) with minimum length
    qreal minLength = -1.0;
    int result = 0;
    GeoDataLineString viaFirst;
    GeoDataLineString viaSecond;
    for ( int i = 0; i < m_route.size(); ++i ) {
        Q_ASSERT( viaFirst.size() < 4 && viaSecond.size() < 4 );
        if ( viaFirst.size() == 3 ) {
            viaFirst.remove( 0 );
            viaFirst.remove( 0 );
        }

        if ( viaSecond.size() == 3 ) {
            viaSecond.remove( 0 );
            viaSecond.remove( 0 );
        }

        if ( viaFirst.size() == 1 ) {
            viaFirst.append( position );
        }

        viaFirst.append(dynamic_cast<const GeoDataPlacemark *>(&m_route.at(i))->coordinate());
        viaSecond.append(dynamic_cast<const GeoDataPlacemark *>(&m_route.at(i))->coordinate());

        if ( viaSecond.size() == 2 ) {
            viaSecond.append( position );
        }

        if ( viaFirst.size() == 3 ) {
            qreal len = viaFirst.length( EARTH_RADIUS );
            if ( minLength < 0.0 || len < minLength ) {
                minLength = len;
                result = i;
            }
        }

        /** @todo: Assumes that destination is the last point */
        if ( viaSecond.size() == 3 && i + 1 < m_route.size() ) {
            qreal len = viaSecond.length( EARTH_RADIUS );
            if ( minLength < 0.0 || len < minLength ) {
                minLength = len;
                result = i + 1;
            }
        }
    }

    Q_ASSERT( 0 <= result && result <= m_route.size() );
    return result;
}

RouteRequest::RouteRequest(GeoDataTreeModel *treeModel, GeoDataContainer *parentContainer, QObject *parent) :
    QObject(parent),
    d(new RouteRequestPrivate(treeModel, parentContainer))
{
    // nothing to do
}

RouteRequest::~RouteRequest()
{
    delete d;
}

int RouteRequest::size() const
{
    return d->m_route.size();
}

GeoDataCoordinates RouteRequest::source() const
{
    GeoDataCoordinates result;
    if ( d->m_route.size() ) {
        result = dynamic_cast<const GeoDataPlacemark *>(&d->m_route.first())->coordinate();
    }
    return result;
}

GeoDataCoordinates RouteRequest::destination() const
{
    GeoDataCoordinates result;
    if (!d->m_route.isEmpty()) {
        result = dynamic_cast<const GeoDataPlacemark *>(&d->m_route.last())->coordinate();
    }
    return result;
}

GeoDataCoordinates RouteRequest::at( int position ) const
{
    return dynamic_cast<const GeoDataPlacemark *>(&d->m_route.at(position))->coordinate();
}

QPixmap RouteRequest::pixmap(int position, int size, int margin ) const
{
    PixmapElement const element( position, size );

    if ( !d->m_pixmapCache.contains( element ) ) {
        // Transparent background
        bool const smallScreen = MarbleGlobal::getInstance()->profiles() & MarbleGlobal::SmallScreen;
        int const imageSize = size > 0 ? size : ( smallScreen ? 32 : 16 );
        QImage result( imageSize, imageSize, QImage::Format_ARGB32_Premultiplied );
        result.fill( qRgba( 0, 0, 0, 0 ) );

        // Paint a colored circle
        QPainter painter( &result );
        painter.setRenderHint( QPainter::Antialiasing, true );
        painter.setPen( QColor( Qt::black ) );
        bool const isVisited = visited( position );
        QColor const backgroundColor = isVisited ? Oxygen::aluminumGray4 : Oxygen::forestGreen4;
        painter.setBrush( QBrush( backgroundColor ) );
        painter.setPen( Qt::black );
        int const iconSize = imageSize - 2 * margin;
        painter.drawEllipse( margin, margin, iconSize, iconSize );

        char const text = char( 'A' + position );

        // Choose a suitable font size
        QFont font = painter.font();
        int fontSize = 20;
        while ( fontSize-- > 0 ) {
            font.setPointSize( fontSize );
            QFontMetrics const fontMetric( font );
            if ( fontMetric.width( text ) <= iconSize && fontMetric.height( ) <= iconSize ) {
                break;
            }
        }

        Q_ASSERT( fontSize );
        font.setPointSize( fontSize );
        painter.setFont( font );

        // Paint a character denoting the position (0=A, 1=B, 2=C, ...)
        painter.drawText( 0, 0, imageSize, imageSize, Qt::AlignCenter, QString( text ) );

        d->m_pixmapCache.insert( element, QPixmap::fromImage( result ) );
    }

    return d->m_pixmapCache[element];
}

void RouteRequest::clear()
{
    for ( int i=d->m_route.size()-1; i>=0; --i ) {
        remove( i );
    }
    Q_ASSERT(dynamic_cast<GeoDataContainer *>(d->m_route.parent()) != 0);
}

void RouteRequest::insert( int index, const GeoDataCoordinates &coordinates, const QString &name )
{
    GeoDataPlacemark placemark;
    placemark.setCoordinate( coordinates );
    placemark.setName( name );
    insert(index, placemark);
}

void RouteRequest::insert(int index, const GeoDataPlacemark &placemark)
{
    GeoDataPlacemark *newPlacemark = new GeoDataPlacemark(placemark);
    d->m_treeModel->addFeature(&d->m_route, newPlacemark, index);
    emit positionAdded( index );
}

void RouteRequest::swap(int index1, int index2)
{
    if (index1 < 0 || index2 < 0 || index1 > d->m_route.size()-1 || index2 > d->m_route.size()-1) {
        return;
    }

    std::swap(d->m_route.child(index1), d->m_route.child(index2));

    emit positionChanged(index1, static_cast<const GeoDataPlacemark &>(d->m_route.at(index1)).coordinate());
    emit positionChanged(index2, static_cast<const GeoDataPlacemark &>(d->m_route.at(index2)).coordinate());
}

void RouteRequest::append( const GeoDataCoordinates &coordinates, const QString &name )
{
    GeoDataPlacemark placemark;
    placemark.setCoordinate( coordinates );
    placemark.setName( name );
    append( placemark );
}

void RouteRequest::append( const GeoDataPlacemark &placemark )
{
    GeoDataPlacemark *newPlacemark = new GeoDataPlacemark(placemark);
    d->m_treeModel->addFeature(&d->m_route, newPlacemark);
    emit positionAdded( d->m_route.size()-1 );
}

void RouteRequest::remove( int index )
{
    const bool removed = d->m_treeModel->removeFeature(&d->m_route, index);
    if ( removed ) {
        emit positionRemoved( index );
    }
}

void RouteRequest::addVia( const GeoDataCoordinates &position )
{
    GeoDataPlacemark placemark;
    placemark.setCoordinate( position );
    addVia(placemark);
}

void RouteRequest::addVia(const GeoDataPlacemark &placemark)
{
    int index = d->viaIndex(placemark.coordinate());
    GeoDataPlacemark *newPlacemark = new GeoDataPlacemark(placemark);
    d->m_treeModel->addFeature(&d->m_route, newPlacemark, index);
    emit positionAdded( index );
}

void RouteRequest::setPosition( int index, const GeoDataCoordinates &position, const QString &name )
{
    if ( index >= 0 && index < d->m_route.size() ) {
        GeoDataPlacemark *placemark = dynamic_cast<GeoDataPlacemark *>(d->m_route.child(index));
        placemark->setName( name );
        if ( placemark->coordinate() != position ) {
            placemark->setCoordinate( position );
            setVisited( index, false );
            emit positionChanged( index, position );
        }
        d->m_treeModel->updateFeature( placemark );
    }
}

void RouteRequest::setName( int index, const QString &name )
{
    if ( index >= 0 && index < d->m_route.size() ) {
        GeoDataFeature *const feature = d->m_route.child(index);
        feature->setName( name );
        d->m_treeModel->updateFeature( feature );
    }
}

QString RouteRequest::name( int index ) const
{
    QString result;
    if ( index >= 0 && index < d->m_route.size() ) {
        result = d->m_route.child(index)->name();
    }
    return result;
}

void RouteRequest::setVisited( int index, bool visited )
{
    if ( index >= 0 && index < d->m_route.size() ) {
        d->m_route.child(index)->extendedData().addValue(GeoDataData(QStringLiteral("routingVisited"), visited));
        QMap<PixmapElement, QPixmap>::iterator iter = d->m_pixmapCache.begin();
        while ( iter != d->m_pixmapCache.end() ) {
             if ( iter.key().index == index ) {
                 iter = d->m_pixmapCache.erase( iter );
             } else {
                 ++iter;
             }
         }
        d->m_treeModel->updateFeature(d->m_route.child(index));
        emit positionChanged(index, static_cast<const GeoDataPlacemark *>(d->m_route.child(index))->coordinate());
    }
}

bool RouteRequest::visited( int index ) const
{
    bool visited = false;
    if ( index >= 0 && index < d->m_route.size() ) {
        if (d->m_route.child(index)->extendedData().contains(QStringLiteral("routingVisited"))) {
            visited = d->m_route.child(index)->extendedData().value(QStringLiteral("routingVisited")).value().toBool();
        }
    }
    return visited;
}

void RouteRequest::reverse()
{
    d->m_treeModel->removeFeature(&d->m_route);
    std::reverse(d->m_route.begin(), d->m_route.end());
    int const total = d->m_route.size();
    for (int i = 0; i < total; ++i) {
        setVisited( i, false );
    }
    Q_ASSERT(dynamic_cast<GeoDataContainer *>(d->m_route.parent()) != 0);
    d->m_treeModel->addFeature(static_cast<GeoDataContainer *>(d->m_route.parent()), &d->m_route);
}

void RouteRequest::setRoutingProfile( const RoutingProfile &profile )
{
    d->m_routingProfile = profile;
    emit routingProfileChanged();
}

RoutingProfile RouteRequest::routingProfile() const
{
    return d->m_routingProfile;
}

const GeoDataPlacemark &RouteRequest::operator [](int index) const
{
    return *static_cast<const GeoDataPlacemark *>(d->m_route.child(index));
}

const GeoDataFolder &RouteRequest::document() const
{
    return d->m_route;
}

} // namespace Marble


#include "moc_RouteRequest.cpp"
