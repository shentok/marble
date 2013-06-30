//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2006-2007 Torsten Rahn     <tackat@kde.org>
// Copyright 2007-2008 Inge Wallin      <ingwa@kde.org>
// Copyright 2007-2008 Carlos Licea     <carlos.licea@kdemail.net>
// Copyright 2011      Michael Henning  <mikehenning@eclipse.net>
// Copyright 2011      Valery Kharitonov  <kharvd@gmail.com>
// Copyright 2012      Mohammed Nafees  <nafees.technocool@gmail.com>
//

#include "MeasureToolPlugin.h"
#include "ui_MeasureConfigWidget.h"

#include "GeoPainter.h"
#include "GeoDataPlacemark.h"
#include "GeoGraphicsItem.h"
#include "GeoLineStringGraphicsItem.h"
#include "GeoPointGraphicsItem.h"
#include "MarbleDebug.h"
#include "MarbleMath.h"
#include "MarbleModel.h"
#include "MarbleLocale.h"
#include "Planet.h"

#include <QtGui/QColor>
#include <QtGui/QPen>
#include <QtGui/QPixmap>
#include <QtGui/QPushButton>
#include <QtGui/QCheckBox>

namespace Marble
{

/* A helper class to ensure placemarks are destroyed along with their
   GeoGraphicsItems */
class MeasureToolPlugin::ItemHelper
{
    public:
        ItemHelper( const QString &name,
                    GeoDataLineString *line,
                    const GeoDataStyle *style )
            : placemark( name ),
              graphicsItem( new GeoLineStringGraphicsItem( &placemark, line ) )
        {
            graphicsItem->setStyle( style );
        }

        ItemHelper( const QString &name,
                    const GeoDataPoint &point,
                    const GeoDataStyle *style )
            : placemark( name ),
              graphicsItem( new GeoPointGraphicsItem( &placemark, point ) )
        {
            graphicsItem->setStyle( style );
        }

        ~ItemHelper()
        {
                delete graphicsItem;
        }

        GeoDataPlacemark placemark;
        GeoGraphicsItem *graphicsItem;
};

MeasureToolPlugin::MeasureToolPlugin()
    : RenderPlugin( 0 ),
      m_configDialog( 0 ),
      m_uiConfigWidget( 0 )
{
}

MeasureToolPlugin::MeasureToolPlugin( const MarbleModel *marbleModel )
    : RenderPlugin( marbleModel ),
      m_measureLineString( GeoDataLineString( Tessellate ) ),
      m_mark( ":/mark.png" ),
      m_marbleWidget( 0 ),
      m_configDialog( 0 ),
      m_uiConfigWidget( 0 ),
      m_showSegmentLabels( true )
{
    const QColor segmentColors[3] = { Oxygen::brickRed4,
                                      Oxygen::forestGreen4,
                                      Oxygen::skyBlue4 };
#ifdef Q_OS_MACX
    const QFont font( "Sans Serif", 10, 50, false );
#else
    const QFont font( "Sans Serif",  8, 50, false );
#endif /* Q_OS_MACX */
    m_fontascent = QFontMetrics( font ).ascent();

    for(int i = 0; i < 3; i++) {
        m_segmentStyles[ i ].lineStyle().setWidth( 2.0f );
        m_segmentStyles[ i ].lineStyle().setColor( segmentColors[i % 3] );
        m_segmentStyles[ i ].labelStyle().setFont( font );
        m_segmentStyles[ i ].labelStyle().setAlignment( GeoDataLabelStyle::Center );
    }

    m_shadowStyle.lineStyle().setWidth( 4.0f );
    m_shadowStyle.lineStyle().setColor( Oxygen::aluminumGray5 );

    m_pointStyle.iconStyle().setIcon( m_mark.toImage() );

    m_totalLabel.setFrame( FrameGraphicsItem::RectFrame );
    m_totalLabel.setMarginTop( 105 );
    m_totalLabel.setMarginLeft( 10 );
    m_totalLabel.setPadding( 5.0 );
    m_totalLabel.setFont( font );
}

MeasureToolPlugin::~MeasureToolPlugin()
{
    qDeleteAll( m_items );
    qDeleteAll( m_segments );
}

QStringList MeasureToolPlugin::backendTypes() const
{
    return QStringList( "measuretool" );
}

QString MeasureToolPlugin::renderPolicy() const
{
    return QString( "ALWAYS" );
}

QStringList MeasureToolPlugin::renderPosition() const
{
    return QStringList() << "USER_TOOLS";
}

QString MeasureToolPlugin::name() const
{
    return tr( "Measure Tool" );
}

QString MeasureToolPlugin::guiString() const
{
    return tr( "&Measure Tool" );
}

QString MeasureToolPlugin::nameId() const
{
    return QString( "measure-tool" );
}

QString MeasureToolPlugin::version() const
{
    return "1.0";
}

QString MeasureToolPlugin::description() const
{
    return tr( "Measure distances between two or more points." );
}

QString MeasureToolPlugin::copyrightYears() const
{
    return "2006-2008, 2011";
}

QList<PluginAuthor> MeasureToolPlugin::pluginAuthors() const
{
    return QList<PluginAuthor>()
            << PluginAuthor( QString::fromUtf8( "Dennis Nienhüser" ), "earthwings@gentoo.org" )
            << PluginAuthor( "Torsten Rahn", "tackat@kde.org" )
            << PluginAuthor( "Inge Wallin", "ingwa@kde.org" )
            << PluginAuthor( "Carlos Licea", "carlos.licea@kdemail.net" )
            << PluginAuthor( "Michael Henning", "mikehenning@eclipse.net" )
            << PluginAuthor( "Valery Kharitonov", "kharvd@gmail.com" )
            << PluginAuthor( "Mohammed Nafees", "nafees.technocool@gmail.com" );
}

QIcon MeasureToolPlugin::icon () const
{
    return QIcon(":/icons/measure.png");
}

void MeasureToolPlugin::initialize ()
{
}

bool MeasureToolPlugin::isInitialized () const
{
    return true;
}

QDialog *MeasureToolPlugin::configDialog()
{
    if ( !m_configDialog ) {
        m_configDialog = new QDialog();
        m_uiConfigWidget = new Ui::MeasureConfigWidget;
        m_uiConfigWidget->setupUi( m_configDialog );
        connect( m_uiConfigWidget->m_buttonBox, SIGNAL(accepted()),
                SLOT(writeSettings()) );
        QPushButton *applyButton = m_uiConfigWidget->m_buttonBox->button( QDialogButtonBox::Apply );
        connect( applyButton, SIGNAL(clicked()),
                 this,        SLOT(writeSettings()) );
    }

    m_uiConfigWidget->m_showSegLabelsCheckBox->setChecked( m_showSegmentLabels );

    return m_configDialog;
}

QHash<QString,QVariant> MeasureToolPlugin::settings() const
{
    QHash<QString, QVariant> settings = RenderPlugin::settings();

    settings.insert( "showSegmentLabels", m_showSegmentLabels );

    return settings;
}

void MeasureToolPlugin::setSettings( const QHash<QString,QVariant> &settings )
{
    RenderPlugin::setSettings( settings );

    m_showSegmentLabels = settings.value( "showSegmentLabels", true ).toBool();
}

void MeasureToolPlugin::writeSettings()
{
    bool oldValue = m_showSegmentLabels;
    m_showSegmentLabels = m_uiConfigWidget->m_showSegLabelsCheckBox->isChecked();

    if ( oldValue != m_showSegmentLabels )
        updateScene();

    emit settingsChanged( nameId() );
    emit repaintNeeded();
}

bool MeasureToolPlugin::setViewport( const ViewportParams *viewport )
{
    for ( int i = 0; i < m_items.size(); ++i ) {
        m_items.at( i )->graphicsItem->setViewport( viewport );
    }

    return true;
}

bool MeasureToolPlugin::render( GeoPainter *painter, const QSize &viewportSize ) const
{
    Q_UNUSED( viewportSize );

    for( int i = 0; i < m_items.size(); ++i ) {
        m_items.at( i )->graphicsItem->paint( painter );
    }

    if ( m_items.size() > 1 ) {

        // FIXME: we have to paint the label this way, since there is
        //        no GeoDataGraphicsItem for 2d stuff like that

        painter->save();

        painter->setPen( Qt::black );
        painter->setBrush( QColor( 192, 192, 192, 192 ) );

        m_totalLabel.paintEvent( painter );

        painter->restore();
    }

    return true;
}

void MeasureToolPlugin::updateScene()
{
    qDeleteAll( m_items );
    m_items.clear();

    if ( m_showSegmentLabels ) {
        generateSegments();
    } else {
        m_items.append( new ItemHelper( "", &m_measureLineString, &m_segmentStyles[0] ) );
    }

    // Paint the nodes of the paths.
    generateMeasurePoints();

    // Paint the total distance in the upper left corner.
    qreal totalDistance = m_measureLineString.length( marbleModel()->planet()->radius() );

    if ( m_measureLineString.size() > 1 )
        generateTotalDistanceLabel( totalDistance );
}

void MeasureToolPlugin::generateSegments()
{
    qDeleteAll( m_segments );
    m_segments.clear();

    for ( int segmentIndex = 0; segmentIndex < m_measureLineString.size() - 1; ++segmentIndex ) {

        GeoDataLineString *segment = new GeoDataLineString( Tessellate );
        *segment << m_measureLineString[segmentIndex] ;
        *segment << m_measureLineString[segmentIndex + 1];
        m_segments.append( segment );

        m_items.append( new ItemHelper( "", segment, &m_shadowStyle ) );

        const QLocale::MeasurementSystem measurementSystem = MarbleGlobal::getInstance()->locale()->measurementSystem();

        const qreal segmentLength = segment->length( marbleModel()->planet()->radius() );

        QString distanceString;

        if ( measurementSystem == QLocale::MetricSystem ) {
            if ( segmentLength >= 1000.0 ) {
                distanceString = tr("%1 km").arg( segmentLength / 1000.0, 0, 'f', 2 );
            }
            else {
                distanceString = tr("%1 m").arg( segmentLength, 0, 'f', 2 );
            }
        }
        else {
            distanceString = QString("%1 mi").arg( segmentLength / 1000.0 * KM2MI, 0, 'f', 2 );
        }

        m_items.append( new ItemHelper( distanceString, segment,
                                        &m_segmentStyles[ segmentIndex % 3 ] ) );
    }
}

void MeasureToolPlugin::generateMeasurePoints()
{
    // Paint the marks.
    GeoDataLineString::const_iterator itpoint = m_measureLineString.constBegin();
    GeoDataLineString::const_iterator const endpoint = m_measureLineString.constEnd();
    for (; itpoint != endpoint; ++itpoint )
    {
        m_items.append( new ItemHelper( "", GeoDataPoint( *itpoint ), &m_pointStyle ) );
    }
}

void MeasureToolPlugin::generateTotalDistanceLabel( qreal totalDistance )
{
    QString  distanceString;

    QLocale::MeasurementSystem measurementSystem;
    measurementSystem = MarbleGlobal::getInstance()->locale()->measurementSystem();

    if ( measurementSystem == QLocale::MetricSystem ) {
        if ( totalDistance >= 1000.0 ) {
            distanceString = tr("Total Distance: %1 km").arg( totalDistance/1000.0 );
        }
        else {
            distanceString = tr("Total Distance: %1 m").arg( totalDistance );
        }
    }
    else {
        distanceString = QString("Total Distance: %1 mi").arg( totalDistance/1000.0 * KM2MI );
    }

    if ( m_totalLabel.text() != distanceString ) {

        m_totalLabel.setText( distanceString );

        QFontMetrics metric( m_totalRectStyle.labelStyle().font() );
        QSize frameSize( metric.boundingRect( distanceString ).width(),
                         m_fontascent );
        m_totalLabel.setContentSize( frameSize );
    }
}


void MeasureToolPlugin::addMeasurePoint( qreal lon, qreal lat )
{
    m_measureLineString << GeoDataCoordinates( lon, lat );

    emit numberOfMeasurePointsChanged( m_measureLineString.size() );
}

void MeasureToolPlugin::removeLastMeasurePoint()
{
    if (!m_measureLineString.isEmpty())
	m_measureLineString.remove( m_measureLineString.size() - 1 );

    emit numberOfMeasurePointsChanged( m_measureLineString.size() );
}

void MeasureToolPlugin::removeMeasurePoints()
{
    m_measureLineString.clear();

    emit numberOfMeasurePointsChanged( m_measureLineString.size() );
}

void MeasureToolPlugin::addContextItems()
{
    MarbleWidgetPopupMenu *menu = m_marbleWidget->popupMenu();

    // Connect the inputHandler and the measure tool to the popup menu
    m_addMeasurePointAction = new QAction( QIcon(":/icons/measure.png"), tr( "Add &Measure Point" ), this );
    m_removeLastMeasurePointAction = new QAction( tr( "Remove &Last Measure Point" ), this );
    m_removeLastMeasurePointAction->setEnabled( false );
    m_removeMeasurePointsAction = new QAction( tr( "&Remove Measure Points" ), this );
    m_removeMeasurePointsAction->setEnabled( false );
    m_separator = new QAction( this );
    m_separator->setSeparator( true );

    if ( ! MarbleGlobal::getInstance()->profiles() & MarbleGlobal::SmallScreen ) {
        menu->addAction( Qt::RightButton, m_addMeasurePointAction );
        menu->addAction( Qt::RightButton, m_removeLastMeasurePointAction );
        menu->addAction( Qt::RightButton, m_removeMeasurePointsAction );
        menu->addAction( Qt::RightButton, m_separator );
    }

    connect( m_addMeasurePointAction, SIGNAL(triggered()), SLOT(addMeasurePointEvent()) );
    connect( m_removeLastMeasurePointAction, SIGNAL(triggered()), SLOT(removeLastMeasurePoint()) );
    connect( m_removeMeasurePointsAction, SIGNAL(triggered()), SLOT(removeMeasurePoints()) );

    connect( this, SIGNAL(numberOfMeasurePointsChanged(int)), SLOT(setNumberOfMeasurePoints(int)) );
}

void MeasureToolPlugin::removeContextItems()
{
    delete m_addMeasurePointAction;
    delete m_removeLastMeasurePointAction;
    delete m_removeMeasurePointsAction;
    delete m_separator;
}

void MeasureToolPlugin::addMeasurePointEvent()
{
    QPoint p = m_marbleWidget->popupMenu()->mousePosition();

    qreal  lat;
    qreal  lon;
    m_marbleWidget->geoCoordinates( p.x(), p.y(), lon, lat, GeoDataCoordinates::Radian );

    addMeasurePoint( lon, lat );
}

void MeasureToolPlugin::setNumberOfMeasurePoints( int newNumber )
{
    const bool enableMeasureActions = ( newNumber > 0 );
    m_removeMeasurePointsAction->setEnabled(enableMeasureActions);
    m_removeLastMeasurePointAction->setEnabled(enableMeasureActions);

    updateScene();
}

bool MeasureToolPlugin::eventFilter( QObject *object, QEvent *e )
{
    if ( m_marbleWidget && !enabled() ) {
        m_marbleWidget = 0;
        removeContextItems();
        m_measureLineString.clear();
    }

    if ( m_marbleWidget || !enabled() || !visible() ) {
        return RenderPlugin::eventFilter( object, e );
    }

    MarbleWidget *widget = qobject_cast<MarbleWidget*>( object );

    if ( widget ) {
        m_marbleWidget = widget;
        addContextItems();
    }

    return RenderPlugin::eventFilter( object, e );
}

}

Q_EXPORT_PLUGIN2( MeasureToolPlugin, Marble::MeasureToolPlugin )

#include "MeasureToolPlugin.moc"

