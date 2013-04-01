import QtQuick 1.0
import org.hildon.components 1.0
import org.kde.edu.marble 0.11

Dialog {
    property MarbleWidget map

    titleText: qsTr( "Map View - Marble" )

    content: Flow {
        anchors.fill: parent

        SelectionItem {
            width: parent.width
            text: qsTr( "Projection" )
            value: map.projection
            valueLayout: ValueLayout.ValueBesideText
            model: ListModel {
                ListElement { name: "Globe"; value: "Spherical" }
                ListElement { name: "Flat Map"; value: "Equirectangular" }
                ListElement { name: "Mercator"; value: "Mercator" }
            }
            onValueChosen: map.setProjection( value )
        }

        SelectionButton {
            width: parent.width
            text: qsTr( "Celestial Body" )
            valueText: qsTr( "Earth" )
            enabled: false
            valueLayout: ValueLayout.ValueBesideText
        }

        SelectionButton {
            width: parent.width
            text: qsTr( "Theme" )
            valueText: qsTr( "OpenStreetMap" )
            enabled: false
            valueLayout: ValueLayout.ValueBesideText
        }
    }
}
