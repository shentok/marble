// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011 Dennis Nienhüser <earthwings@gentoo.org>
// Copyright 2011 Daniel Marth <danielmarth@gmx.at>

import QtQuick 1.0
import org.kde.edu.marble 0.11

/*
 * Settings of the application.
 */
Settings {
    id: root
    groupName: "MarbleTouch"

    // Load settings from file.
    property string mapTheme: "earth/openstreetmap/openstreetmap.dgml"
    property string streetMapTheme: "earth/openstreetmap/openstreetmap.dgml"
    property bool workOffline: false
    property real lastKnownLongitude: 0.0
    property real lastKnownLatitude: 0.0
    property bool gpsTracking: false
    property bool showPositionIndicator: false
    property bool showTrack: false
    property string projection: "Spherical"
    property variant defaultRenderPlugins: ["compass", "coordinate-grid", "progress",
        "crosshairs", "stars", "scalebar"]
    property variant activeRenderPlugins: defaultRenderPlugins
    property string lastActivity: ""
    property bool inhibitScreensaver: true
    property bool voiceNavigationMuted: false
    property bool voiceNavigationSoundEnabled: false
    property string voiceNavigationSpeaker: "English - Bugsbane"
    property bool navigationStartupWarning: false
    property bool navigationStartupWarningEverShown: false
    property string changelogShown: ""
    property bool owncloudSync: false
    property string owncloudServer: ""
    property string owncloudUsername: ""
    property string owncloudPassword: ""

    Component.onCompleted: {
        root.readSettings()
    }

    // Save settings to file.
    Component.onDestruction: {
        root.writeSettings()
    }

    function removeElementsFromArray(array, elements) {
        for( var j=0; j<elements.length; j++) {
            for( var i=0; i<array.length; i++) {
                if (array[i] === elements[j]) {
                    array.splice(i,1)
                }
            }
        }
    }
}
