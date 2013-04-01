import QtQuick 1.0
import org.hildon.components 1.0
import org.kde.edu.marble 0.11

Page {
    id: root
    title: qsTr( "Routing - Marble" )

    property Routing routing

    tools: MenuLayout {
        MenuItem {
            text: qsTr( "Open Route..." )
            enabled: false
        }

        MenuItem {
            text: qsTr( "Save Route..." )
            enabled: routing.hasRoute
        }
    }

    Flickable {
        anchors.fill: parent
        contentHeight: column.height + platformStyle.paddingNormal
        clip: true

        Column {
            id: column
            width: parent.width

            Repeater {
                id: listView
                model: RouteRequestModel {
                    routing: root.routing
                }

                Rectangle {
                    width: parent.width
                    height: 60

                    TextField {
                        text: display
                        anchors.fill: parent
                    }
                }
            }

            Row {
                width: parent.width

                SelectionItem {
                    anchors.left: parent.left
                    anchors.right: profileSettingsButton.left
                    valueLayout: ValueLayout.ValueUnderTextCentered
                    text: qsTr( "Way of Travel" )
                    valueText: qsTr( "Pedestrian" )
                    model: RoutingProfilesModel {
                        routing: root.routing
                    }
                }

                ToolButton {
                    id: profileSettingsButton
                    anchors.right: parent.right
                    platformIconId: "general_settings"
                    enabled: false
                }
            }
        }
    }
}
