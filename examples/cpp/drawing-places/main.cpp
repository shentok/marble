#include <QApplication>
#include <QTreeView>

#include <marble/MarbleWidget.h>
#include <marble/MarbleModel.h>

#include <marble/GeoDataDocument.h>
#include <marble/GeoDataCoordinates.h>
#include <marble/GeoDataPlacemark.h>
#include <marble/GeoDataLineString.h>
#include <marble/GeoDataLinearRing.h>
#include <marble/GeoDataTreeModel.h>
#include <marble/GeoDataStyle.h>
#include <marble/GeoDataIconStyle.h>
#include <marble/GeoDataLineStyle.h>
#include <marble/GeoDataPolyStyle.h>

#include <cstdio>

using namespace Marble;

void addPoints(GeoDataLinearRing &linearRing)
{
    linearRing << GeoDataCoordinates(GeoDataLongitude::fromDegrees(25.97226722704463), GeoDataLatitude::fromDegrees(44.43497647488007))
                    << GeoDataCoordinates(GeoDataLongitude::fromDegrees(26.04711276456992), GeoDataLatitude::fromDegrees(44.4420741223712))
                    << GeoDataCoordinates(GeoDataLongitude::fromDegrees(25.99712510557899), GeoDataLatitude::fromDegrees(44.48015825036597))
                    << GeoDataCoordinates(GeoDataLongitude::fromDegrees(26.11268978668501), GeoDataLatitude::fromDegrees(44.53902366720936))
                    << GeoDataCoordinates(GeoDataLongitude::fromDegrees(26.12777496065434), GeoDataLatitude::fromDegrees(44.48972441010599))
                    << GeoDataCoordinates(GeoDataLongitude::fromDegrees(26.17769825773425), GeoDataLatitude::fromDegrees(44.47685689461117))
                    << GeoDataCoordinates(GeoDataLongitude::fromDegrees(26.16489863910029), GeoDataLatitude::fromDegrees(44.45366647920105))
                    << GeoDataCoordinates(GeoDataLongitude::fromDegrees(26.23394105442375), GeoDataLatitude::fromDegrees(44.43247765101769))
                    << GeoDataCoordinates(GeoDataLongitude::fromDegrees(26.23388161223319), GeoDataLatitude::fromDegrees(44.40720014793351))
                    << GeoDataCoordinates(GeoDataLongitude::fromDegrees(26.18689640043445), GeoDataLatitude::fromDegrees(44.40683215952335))
                    << GeoDataCoordinates(GeoDataLongitude::fromDegrees(26.1462530009004), GeoDataLatitude::fromDegrees(44.36252655873379))
                    << GeoDataCoordinates(GeoDataLongitude::fromDegrees(25.97226722704463), GeoDataLatitude::fromDegrees(44.43497647488007));
}

void createStyleBucharest( GeoDataStyle &style ) {
    GeoDataLineStyle lineStyle( QColor( 255, 0, 0, 90 ) );
    lineStyle.setWidth ( 8 );

    GeoDataPolyStyle polyStyle( QColor( 255, 0, 0, 40 ) );
    polyStyle.setFill( true );

    style.setLineStyle( lineStyle );
    style.setPolyStyle( polyStyle );
}

void createStyleArch( GeoDataStyle &style ) {
    GeoDataIconStyle iconStyle;
    iconStyle.setIconPath( "bucharest_small.jpg" );
    style.setIconStyle( iconStyle );
}

int main(int argc, char** argv) {

    QApplication app(argc,argv);

    // Create a Marble QWidget without a parent
    MarbleWidget *mapWidget = new MarbleWidget();

    // Load the OpenStreetMap map
    mapWidget->setMapThemeId("earth/openstreetmap/openstreetmap.dgml");

    //Create the Linear Ring (polygon) representing Bucharest's boundaries and include it in a placemark
    GeoDataLinearRing *Bucharest = new GeoDataLinearRing;
    addPoints( *Bucharest );

    GeoDataPlacemark *placemarkBucharest = new GeoDataPlacemark;
    placemarkBucharest->setGeometry( Bucharest );

    //Create the placemark representing the Arch of Triumph
    GeoDataPlacemark *placemarkArch = new GeoDataPlacemark( "Arch of Triumph" );
    placemarkArch->setCoordinate(GeoDataLongitude::fromDegrees(26.0783), GeoDataLatitude::fromDegrees(44.4671));


    //Add styles (icons, colors, etc.) to the two placemarks
    GeoDataStyle::Ptr styleBucharest(new GeoDataStyle);
    GeoDataStyle::Ptr styleArch(new GeoDataStyle);

    createStyleBucharest( *styleBucharest );
    placemarkBucharest->setStyle( styleBucharest );

    createStyleArch ( *styleArch );
    placemarkArch->setStyle( styleArch );


    //Create the document and add the two placemarks (the point representing the Arch of Triumph and the polygon with Bucharest's boundaries)
    GeoDataDocument *document = new GeoDataDocument;
    document->append( placemarkBucharest );
    document->append( placemarkArch );

    // Add the document to MarbleWidget's tree model
    mapWidget->model()->treeModel()->addDocument( document );

        // Center the map on Bucharest and set the zoom
    mapWidget->centerOn(GeoDataCoordinates(GeoDataLongitude::fromDegrees(26.0783), GeoDataLatitude::fromDegrees(44.4671)));
    mapWidget->zoomView( 2400 );


    mapWidget->show();

    return app.exec();
}
