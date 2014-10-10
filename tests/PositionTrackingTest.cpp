//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2012       Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//


#include "GeoDataTreeModel.h"
#include "PositionProviderPlugin.h"
#include "PositionTracking.h"
#include "TestUtils.h"

#include <QSignalSpy>

class FakeProvider : public Marble::PositionProviderPlugin
{
public:
    FakeProvider() :
        m_status( Marble::PositionProviderStatusAcquiring ),
        m_position(),
        m_accuracy(),
        m_speed( 0.0 ),
        m_direction( 0.0 ),
        m_timestamp(),
        m_lastInstance( 0 )
    {}

    QString name() const           { return "fake plugin"; }
    QString guiString() const      { return "fake"; }
    QString nameId() const         { return "fakeplugin"; }
    QString version() const        { return "1.0"; }
    QString description() const    { return "plugin for testing"; }
    QIcon icon() const             { return QIcon(); }
    QString copyrightYears() const { return "2012"; }
    QList<Marble::PluginAuthor> pluginAuthors() const { return QList<Marble::PluginAuthor>(); }
    void initialize() {}
    bool isInitialized() const     { return true; }

    Marble::PositionProviderStatus status() const { return m_status; }
    Marble::GeoDataCoordinates position() const { return m_position; }
    Marble::GeoDataAccuracy accuracy() const { return m_accuracy; }
    qreal speed() const { return m_speed; }
    qreal direction() const { return m_direction; }
    QDateTime timestamp() const { return m_timestamp; }

    Marble::PositionProviderPlugin *newInstance() const;

    void setStatus( Marble::PositionProviderStatus status );
    void setPosition( const Marble::GeoDataCoordinates &position,
                      const Marble::GeoDataAccuracy &accuracy,
                      qreal speed,
                      qreal direction,
                      const QDateTime &timestamp );

    FakeProvider *lastInstance() const { return m_lastInstance; }

private:
    Marble::PositionProviderStatus m_status;
    Marble::GeoDataCoordinates m_position;
    Marble::GeoDataAccuracy m_accuracy;
    qreal m_speed;
    qreal m_direction;
    QDateTime m_timestamp;
    mutable FakeProvider *m_lastInstance;
};

Marble::PositionProviderPlugin *FakeProvider::newInstance() const
{
    m_lastInstance = new FakeProvider;
    return m_lastInstance;
}

void FakeProvider::setStatus( Marble::PositionProviderStatus status )
{
    const Marble::PositionProviderStatus oldStatus = m_status;

    m_status = status;

    if ( oldStatus != m_status ) {
        emit statusChanged( m_status );
    }
}

void FakeProvider::setPosition( const Marble::GeoDataCoordinates &position,
                                const Marble::GeoDataAccuracy &accuracy,
                                qreal speed,
                                qreal direction,
                                const QDateTime &timestamp )
{
    setStatus( Marble::PositionProviderStatusAvailable );

    m_position = position;
    m_accuracy = accuracy;
    m_speed = speed;
    m_direction = direction;
    m_timestamp = timestamp;

    emit positionChanged( m_position, m_accuracy );
}

namespace Marble
{

class PositionTrackingTest : public QObject
{
    Q_OBJECT

 public:
    PositionTrackingTest();

 private Q_SLOTS:
    void construct();

    void setPositionProviderFactory();

    void statusChanged_data();
    void statusChanged();

    void clearTrack();
};

PositionTrackingTest::PositionTrackingTest()
{
    qRegisterMetaType<GeoDataCoordinates>( "GeoDataCoordinates" );
    qRegisterMetaType<PositionProviderStatus>( "PositionProviderStatus" );
}

void PositionTrackingTest::construct()
{
    GeoDataTreeModel treeModel;
    const PositionTracking tracking( &treeModel );

    QCOMPARE( tracking.isEnabled(), true );
    QCOMPARE( tracking.isActive(), false );
    QCOMPARE( tracking.positionProviderFactory(), static_cast<const PositionProviderPlugin *>( 0 ) );
    QCOMPARE( tracking.speed(), qreal( 0 ) );
    QCOMPARE( tracking.direction(), qreal( 0 ) );
    QCOMPARE( tracking.timestamp(), QDateTime() );
    QCOMPARE( tracking.accuracy(), GeoDataAccuracy() );
    QCOMPARE( tracking.trackVisible(), true );
    QCOMPARE( tracking.currentLocation(), GeoDataCoordinates() );
    QCOMPARE( tracking.status(), PositionProviderStatusUnavailable );
    QCOMPARE( tracking.isTrackEmpty(), true );

    QCOMPARE( treeModel.rowCount(), 1 );
    const QModelIndex indexPositionTracking = treeModel.index( 0, 0 );
    QCOMPARE( treeModel.data( indexPositionTracking, Qt::DisplayRole ).toString(), QString( "Position Tracking" ) );
    QCOMPARE( treeModel.rowCount( indexPositionTracking ), 2 );
    const QModelIndex indexCurrentPosition = treeModel.index( 0, 0, indexPositionTracking );
    QCOMPARE( treeModel.data( indexCurrentPosition, Qt::DisplayRole ).toString(), QString( "Current Position" ) );
    const QModelIndex indexCurrentTrack = treeModel.index( 1, 0, indexPositionTracking );
    QCOMPARE( treeModel.data( indexCurrentTrack, Qt::DisplayRole ).toString(), QString( "Current Track" ) );
}

void PositionTrackingTest::setPositionProviderFactory()
{
    GeoDataTreeModel treeModel;
    PositionTracking tracking( &treeModel );

    QSignalSpy gpsLocationSpy( &tracking, SIGNAL(gpsLocation(GeoDataCoordinates,qreal)) );

    const FakeProvider factory;
    tracking.setPositionProviderFactory( &factory );

    QCOMPARE( tracking.status(), PositionProviderStatusAcquiring );
    QCOMPARE( tracking.isActive(), true );

    QPointer<FakeProvider> provider = factory.lastInstance();

    const GeoDataCoordinates coordinates( 1.2, 0.9 );
    const GeoDataAccuracy accuracy( GeoDataAccuracy::Detailed, 10.0, 22.0 );
    const qreal speed = 32.8;
    const qreal direction = 49.7;
    const QDateTime timestamp( QDate( 1, 3, 1994 ) );
    provider->setPosition( coordinates, accuracy, speed, direction, timestamp );

    QCOMPARE( tracking.isActive(), true );
    QCOMPARE( tracking.currentLocation(), coordinates );
    QCOMPARE( tracking.accuracy(), accuracy );
    QCOMPARE( tracking.speed(), speed );
    QCOMPARE( tracking.direction(), direction );
    QCOMPARE( tracking.timestamp(), timestamp );
    QCOMPARE( gpsLocationSpy.count(), 1 );

    tracking.setPositionProviderFactory( 0 );

    QVERIFY( provider.isNull() );
}

void PositionTrackingTest::statusChanged_data()
{
    QTest::addColumn<PositionProviderStatus>( "finalStatus" );
    QTest::addColumn<bool>( "isActive" );

    addRow() << PositionProviderStatusError << false;
    addRow() << PositionProviderStatusUnavailable << false;
    addRow() << PositionProviderStatusAcquiring << true;
    addRow() << PositionProviderStatusAvailable << true;
}

void PositionTrackingTest::statusChanged()
{
    QFETCH( PositionProviderStatus, finalStatus );
    QFETCH( bool, isActive );
    const int expectedStatusChangedCount = ( finalStatus == PositionProviderStatusAcquiring ) ? 1 : 2;

    GeoDataTreeModel treeModel;
    PositionTracking tracking( &treeModel );

    QSignalSpy statusChangedSpy( &tracking, SIGNAL(statusChanged(PositionProviderStatus)) );

    const FakeProvider factory;
    tracking.setPositionProviderFactory( &factory );
    factory.lastInstance()->setStatus( finalStatus );

    QCOMPARE( tracking.status(), finalStatus );
    QCOMPARE( tracking.isActive(), isActive );
    QCOMPARE( statusChangedSpy.count(), expectedStatusChangedCount );
}

void PositionTrackingTest::clearTrack()
{
    GeoDataTreeModel treeModel;
    PositionTracking tracking( &treeModel );

    const FakeProvider factory;
    tracking.setPositionProviderFactory( &factory );

    tracking.clearTrack();

    QVERIFY( tracking.isTrackEmpty() );

    const GeoDataCoordinates position( 2.1, 0.8 );
    const GeoDataAccuracy accuracy( GeoDataAccuracy::Detailed, 10.0, 22.0 );
    const qreal speed = 32.8;
    const qreal direction = 49.7;
    const QDateTime timestamp( QDate( 1, 3, 1994 ) );
    factory.lastInstance()->setPosition( position, accuracy, speed, direction, timestamp );

    QVERIFY( !tracking.isTrackEmpty() );

    tracking.clearTrack();

    QVERIFY( tracking.isTrackEmpty() );
}

}

QTEST_MAIN( Marble::PositionTrackingTest )

#include "PositionTrackingTest.moc"
