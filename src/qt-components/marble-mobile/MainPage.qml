import QtQuick 1.0
import org.hildon.components 1.0
import org.kde.edu.marble 0.11

Page {
    id: pinchmap

    title: qsTr( "Marble - Virtual Globe" )

    MarbleSettings {
        id: settings
    }

    tools: MenuLayout {
        MenuItem {
            text: qsTr( "Work Offline" )
            checkable: true
            checked: settings.workOffline
            onClicked: settings.workOffline = !settings.workOffline
        }

        MenuItem {
            text: qsTr( "Show Legend" )
            checkable: true
            enabled: false
        }

        MenuItem {
            text: qsTr( "Map View..." )
            onClicked: mapViewDialog.open()
        }

        MenuItem {
            text: qsTr( "Download Region..." )
            enabled: false
        }

        MenuItem {
            text: qsTr( "Routing..." )
            onClicked: pageStack.push( routingPage )
        }

        MenuItem {
            text: qsTr( "Tracking..." )
            enabled: false
        }

        MenuItem {
            text: qsTr( "Go To..." )
            enabled: false
        }

        MenuItem {
            text: qsTr( "Manage Bookmarks..." )
            enabled: false
        }

        MenuItem {
            text: qsTr( "About Marble Virtual Globe..." )
            enabled: false
        }
    }

    property double defaultLatitude: 37.69
    property double defaultLongitude: -97.33
    property int  defaultZoomLevel: 7

    MarbleWidget {
        id: map

        anchors.fill: parent
//        zoomLevel: pinchmap.defaultZoomLevel

        center: Coordinate {
            latitude: pinchmap.defaultLatitude
            longitude: pinchmap.defaultLongitude
        }

        projection: "Mercator"
        workOffline: settings.workOffline

        MouseArea {
            id: mousearea

            property bool __isPanning: false
            property int __lastX: -1
            property int __lastY: -1
            property real startCenterLon: 0.0
            property real startCenterLat: 0.0

            anchors.fill : parent

            onPressed: {
                __isPanning = true
                __lastX = mouse.x
                __lastY = mouse.y
                startCenterLon = map.center.longitude
                startCenterLat = map.center.latitude
            }

            onReleased: {
                __isPanning = false
            }

            onPositionChanged: {
                if (__isPanning) {
                    var dx = __lastX - mouse.x
                    var dy = __lastY - mouse.y
                    map.center.longitude = startCenterLon + 90 * dx / map.radius
                    map.center.latitude  = startCenterLat - 90 * dy / map.radius
                }
            }

            onCanceled: {
                __isPanning = false;
            }
        }
    }

    MapSelectionDialog {
        id: mapViewDialog
        map: map
    }

    RoutingPage {
        id: routingPage
        routing: Routing {
            map: map
        }
    }
}
