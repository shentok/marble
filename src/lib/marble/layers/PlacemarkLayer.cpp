//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2006-2007 Torsten Rahn <tackat@kde.org>
// Copyright 2007-2008 Inge Wallin  <ingwa@kde.org>
// Copyright 2011-2012 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include "PlacemarkLayer.h"

#include <QGLContext>
#include <QMatrix4x4>
#include <QModelIndex>
#include <QPoint>
#include <QPainter>
#include <QVector2D>

#include "MarbleDebug.h"
#include "AbstractProjection.h"
#include "GeoDataStyle.h"
#include "GeoPainter.h"
#include "GeoDataPlacemark.h"
#include "ViewportParams.h"
#include "VisiblePlacemark.h"

using namespace Marble;

bool PlacemarkLayer::m_useXWorkaround = false;

PlacemarkLayer::PlacemarkLayer( QAbstractItemModel *placemarkModel,
                                QItemSelectionModel *selectionModel,
                                MarbleClock *clock,
                                QObject *parent ) :
    QObject( parent ),
    m_layout( placemarkModel, selectionModel, clock )
{
    m_useXWorkaround = testXBug();
    mDebug() << "Use workaround: " << ( m_useXWorkaround ? "1" : "0" );

    connect( &m_layout, SIGNAL(repaintNeeded()), SIGNAL(repaintNeeded()) );
}

PlacemarkLayer::~PlacemarkLayer()
{
}

QStringList PlacemarkLayer::renderPosition() const
{
    return QStringList() << "HOVERS_ABOVE_SURFACE";
}

qreal PlacemarkLayer::zValue() const
{
    return 2.0;
}

bool PlacemarkLayer::render( GeoPainter *geoPainter, ViewportParams *viewport,
                               const QString &renderPos, GeoSceneLayer *layer )
{
    Q_UNUSED( renderPos )
    Q_UNUSED( layer )

    QVector<VisiblePlacemark*> visiblePlacemarks = m_layout.generateLayout( viewport );
    // draw placemarks less important first
    QVector<VisiblePlacemark*>::const_iterator visit = visiblePlacemarks.constEnd();
    QVector<VisiblePlacemark*>::const_iterator itEnd = visiblePlacemarks.constBegin();

    QPainter *const painter = geoPainter;

    while ( visit != itEnd ) {
        --visit;

        VisiblePlacemark *const mark = *visit;

        QRect labelRect( mark->labelRect().toRect() );
        QPoint symbolPos( mark->symbolPosition() );

        // when the map is such zoomed out that a given place
        // appears many times, we draw one placemark at each
        if (viewport->currentProjection()->repeatableX() ) {
            const int symbolX = mark->symbolPosition().x();
            const int textX =   mark->labelRect().x();

            for ( int i = symbolX % (4 * viewport->radius());
                 i <= viewport->width();
                 i += 4 * viewport->radius() )
            {
                labelRect.moveLeft(i - symbolX + textX );
                symbolPos.setX( i );

                painter->drawPixmap( symbolPos, mark->symbolPixmap() );
                painter->drawPixmap( labelRect, mark->labelPixmap() );
            }
        } else { // simple case, one draw per placemark
            painter->drawPixmap( symbolPos, mark->symbolPixmap() );
            painter->drawPixmap( labelRect, mark->labelPixmap() );
        }
    }

    return true;
}

void PlacemarkLayer::paintGL( QGLContext *glContext, const ViewportParams *viewport )
{
    static const float texCoords[] = { 0.f, 0.f,  0.f, 1.f,  1.f, 0.f,  1.f, 1.f };

    glPushAttrib( GL_ALL_ATTRIB_BITS );
    glPushClientAttrib( GL_CLIENT_ALL_ATTRIB_BITS );
    glViewport( 0, 0, viewport->width(), viewport->height() );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( 0, viewport->width(), viewport->height(), 0, -256000000/M_PI*80, 256/M_PI*32 );

    glEnable( GL_DEPTH_TEST );
    glEnable( GL_CULL_FACE );
    glEnable( GL_TEXTURE_2D );
    glFrontFace( GL_CCW );

    // blend placemarks on top of surface
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    QMatrix4x4 matrix;
    matrix.translate( viewport->width() / 2, viewport->height() / 2, 0 );
    if ( viewport->projection() == Spherical ) {
        const Quaternion axis = viewport->planetAxis();

        const qreal angle = 2 * acos( axis.v[Q_W] ) * RAD2DEG;
        const qreal ax = axis.v[Q_X];
        const qreal ay = -axis.v[Q_Y];
        const qreal az = axis.v[Q_Z];

        matrix.rotate( angle, ax, ay, az );
    } else {
        // Calculate translation of center point
        const GeoDataCoordinates coordinates = viewport->viewLatLonAltBox().center();
        const QVector3D center = viewport->currentProjection()->vertexCoordinates( coordinates );
        glTranslated( -center.x() * viewport->radius(),
                      -center.y() * viewport->radius(),
                      0 );
    }
    matrix.scale( viewport->radius() );

    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );

    QHash<const GeoDataPlacemark *, GlData> oldGlMap = m_glMap;
    m_glMap.clear();

    const QVector<VisiblePlacemark*> visiblePlacemarks = m_layout.generateLayout( viewport );
    // draw placemarks less important first
    QVector<VisiblePlacemark*>::const_iterator visit = visiblePlacemarks.constEnd();
    QVector<VisiblePlacemark*>::const_iterator itEnd = visiblePlacemarks.constBegin();

    while ( visit != itEnd ) {
        --visit;

        VisiblePlacemark *const mark = *visit;

        if ( oldGlMap.contains( mark->placemark() ) ) {
            m_glMap.insert( mark->placemark(), oldGlMap.take( mark->placemark() ) );
        }
        else {
            const GLuint symbolId = glContext->bindTexture( mark->symbolPixmap(), GL_TEXTURE_2D, GL_RGBA, QGLContext::LinearFilteringBindOption | QGLContext::PremultipliedAlphaBindOption );
            const GLuint labelId = glContext->bindTexture( mark->labelPixmap(), GL_TEXTURE_2D, GL_RGBA, QGLContext::LinearFilteringBindOption | QGLContext::PremultipliedAlphaBindOption );
            m_glMap.insert( mark->placemark(), GlData( symbolId, labelId ) );
        }

        QRect labelRect( mark->labelRect().toRect() );
        QPoint symbolPos( mark->symbolPosition() );

        // when the map is such zoomed out that a given place
        // appears many times, we draw one placemark at each
        if (viewport->currentProjection()->repeatableX() ) {
            const int symbolX = mark->symbolPosition().x();
            const int textX =   mark->labelRect().x();

            for ( int i = symbolX % (4 * viewport->radius());
                 i <= viewport->width();
                 i += 4 * viewport->radius() )
            {
                labelRect.moveLeft(i - symbolX + textX );
                symbolPos.setX( i );

                glContext->drawTexture( symbolPos, m_glMap[mark->placemark()].symbolId );
                glContext->drawTexture( labelRect, m_glMap[mark->placemark()].labelId );
            }
        } else { // simple case, one draw per placemark
            const QVector3D vector = viewport->currentProjection()->vertexCoordinates( mark->placemark()->coordinate() );
            const QVector3D center = ( matrix * vector ) * QVector3D( 1, 1, 1.2 );
            if ( m_glMap[mark->placemark()].symbolId != 0 ) {
                QVector<QVector3D> vertices;
                vertices << center + QVector3D( - 5, - 5, 0 )
                         << center + QVector3D( - 5, + 5, 0 )
                         << center + QVector3D( + 5, - 5, 0 )
                         << center + QVector3D( + 5, + 5, 0 );

                glBindTexture( GL_TEXTURE_2D, m_glMap[mark->placemark()].symbolId );
                glVertexPointer( 3, GL_FLOAT, 0, vertices.data() );
                glTexCoordPointer( 2, GL_FLOAT, 0, texCoords );
                glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
            }
            {
                QVector<QVector3D> vertices;
                vertices << QVector3D( QVector2D( labelRect.topLeft() ), center.z() )
                         << QVector3D( QVector2D( labelRect.bottomLeft() ), center.z() )
                         << QVector3D( QVector2D( labelRect.topRight() ), center.z() )
                         << QVector3D( QVector2D( labelRect.bottomRight() ), center.z() );

                glBindTexture( GL_TEXTURE_2D, m_glMap[mark->placemark()].labelId );
                glVertexPointer( 3, GL_FLOAT, 0, vertices.data() );
                glTexCoordPointer( 2, GL_FLOAT, 0, texCoords );
                glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
            }

//            glContext->drawTexture( symbolPos, m_glMap[mark->placemark()].symbolId );
//            glContext->drawTexture( labelRect, m_glMap[mark->placemark()].labelId );
        }
    }

    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );

    foreach ( const GlData &glData, oldGlMap ) {
        glContext->deleteTexture( glData.symbolId );
        glContext->deleteTexture( glData.labelId );
    }

    glDisable( GL_CULL_FACE );

    glPopClientAttrib();
    glPopAttrib();
}

RenderState PlacemarkLayer::renderState() const
{
    return RenderState( "Placemarks" );
}

QString PlacemarkLayer::runtimeTrace() const
{
    return m_layout.runtimeTrace();
}

QVector<const GeoDataFeature *> PlacemarkLayer::whichPlacemarkAt( const QPoint &pos )
{
    return m_layout.whichPlacemarkAt( pos );
}

void PlacemarkLayer::setShowPlaces( bool show )
{
    m_layout.setShowPlaces( show );
}

void PlacemarkLayer::setShowCities( bool show )
{
    m_layout.setShowCities( show );
}

void PlacemarkLayer::setShowTerrain( bool show )
{
    m_layout.setShowTerrain( show );
}

void PlacemarkLayer::setShowOtherPlaces( bool show )
{
    m_layout.setShowOtherPlaces( show );
}

void PlacemarkLayer::setShowLandingSites( bool show )
{
    m_layout.setShowLandingSites( show );
}

void PlacemarkLayer::setShowCraters( bool show )
{
    m_layout.setShowCraters( show );
}

void PlacemarkLayer::setShowMaria( bool show )
{
    m_layout.setShowMaria( show );
}

void PlacemarkLayer::requestStyleReset()
{
    m_layout.requestStyleReset();
}


// Test if there a bug in the X server which makes 
// text fully transparent if it gets written on 
// QPixmaps that were initialized by filling them 
// with Qt::transparent

bool PlacemarkLayer::testXBug()
{
    QString  testchar( "K" );
    QFont    font( "Sans Serif", 10 );

    int fontheight = QFontMetrics( font ).height();
    int fontwidth  = QFontMetrics( font ).width(testchar);
    int fontascent = QFontMetrics( font ).ascent();

    QPixmap  pixmap( fontwidth, fontheight );
    pixmap.fill( Qt::transparent );

    QPainter textpainter;
    textpainter.begin( &pixmap );
    textpainter.setPen( QColor( 0, 0, 0, 255 ) );
    textpainter.setFont( font );
    textpainter.drawText( 0, fontascent, testchar );
    textpainter.end();

    QImage image = pixmap.toImage();

    for ( int x = 0; x < fontwidth; ++x ) {
        for ( int y = 0; y < fontheight; ++y ) {
            if ( qAlpha( image.pixel( x, y ) ) > 0 )
                return false;
        }
    }

    return true;
}

#include "PlacemarkLayer.moc"

