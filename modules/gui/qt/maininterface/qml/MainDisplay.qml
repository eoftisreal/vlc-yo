/*****************************************************************************
 * Copyright (C) 2019 VLC authors and VideoLAN
 *****************************************************************************/
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import VLC.Style
import VLC.MainInterface
import VLC.Widgets as Widgets
import VLC.Playlist
import VLC.Player

import VLC.Util
import VLC.Dialogs

FocusScope {
    id: g_mainDisplay

    // Properties

    property bool hasMiniPlayer: miniPlayer.visible

    property bool showEmbeddedPlayer: Player.hasVideoOutput && MainCtx.hasEmbededVideo

    // NOTE: The main view must be above the indexing bar and the mini player.
    property real displayMargin: (height - miniPlayer.y) + (loaderProgress.active ? loaderProgress.height : 0)

    //MainDisplay behave as a PageLoader
    property alias pagePrefix: stackView.pagePrefix

    readonly property int positionSliderY: {
        var size = miniPlayer.y + miniPlayer.sliderY

        if (MainCtx.pinVideoControls)
            return size - VLCStyle.margin_xxxsmall
        else
            return size
    }

    property bool _showMiniPlayer: false

    // functions

    //MainDisplay behave as a PageLoader
    function loadView(path, properties, focusReason) {
        const found = stackView.loadView(path, properties, focusReason)
        if (!found)
            return

        const item = stackView.currentItem

        sourcesBanner.localMenuDelegate = Qt.binding(function () {
            return item.localMenuDelegate ?? null
        })

        // NOTE: sortMenu is declared with the SortMenu type, so when it's undefined we have to
        //       return null to avoid a QML warning.
        sourcesBanner.sortMenu = Qt.binding(function () {
            return item.sortMenu ?? null
        })

        MainCtx.hasGridListMode = Qt.binding(() => item.hasGridListMode !== undefined && item.hasGridListMode)
        MainCtx.search.available = Qt.binding(() => item.isSearchable !== undefined && item.isSearchable)
        MainCtx.sort.model = Qt.binding(function () { return item.sortModel })
        MainCtx.sort.available = Qt.binding(function () { return Helpers.isArray(item.sortModel) && item.sortModel.length > 0 })

        if (Player.hasVideoOutput && MainCtx.hasEmbededVideo)
            _showMiniPlayer = true
    }

    Component.onCompleted: {
        if (MainCtx.canShowVideoPIP)
            pipPlayerComponent.createObject(this)
    }

    Navigation.cancelAction: function() {
        History.previous(Qt.BacktabFocusReason)
    }

    Keys.onPressed: (event) => {
        if (KeyHelper.matchSearch(event)) {
            MainCtx.search.askShow()
            event.accepted = true
        }
        //unhandled keys are forwarded as hotkeys
        if (!event.accepted)
            MainCtx.sendHotkey(event.key, event.modifiers);
    }

    readonly property var pageModel: [
        {
            listed: true,
            displayText: qsTr("Home"),
            icon: VLCIcons.home,
            name: "home",
            url: MainCtx.mediaLibraryAvailable ?
                 "qrc:///qt/qml/VLC/MediaLibrary/HomeDisplay.qml" :
                 "qrc:///qt/qml/VLC/MainInterface/NoMedialibHome.qml"
        }, {
            listed: MainCtx.mediaLibraryAvailable,
            displayText: qsTr("Video"),
            icon: VLCIcons.topbar_video,
            name: "video",
            url: "qrc:///qt/qml/VLC/MediaLibrary/VideoDisplay.qml"
        }, {
            listed: MainCtx.mediaLibraryAvailable,
            displayText: qsTr("Music"),
            icon: VLCIcons.topbar_music,
            name: "music",
            url: "qrc:///qt/qml/VLC/MediaLibrary/MusicDisplay.qml"
        }, {
            listed: true,
            displayText: qsTr("Browse"),
            icon: VLCIcons.topbar_network,
            name: "network",
            url: "qrc:///qt/qml/VLC/Network/BrowseDisplay.qml"
        }, {
            listed: true,
            displayText: qsTr("Discover"),
            icon: VLCIcons.topbar_discover,
            name: "discover",
            url: "qrc:///qt/qml/VLC/Network/DiscoverDisplay.qml"
        }, {
            listed: false,
            name: "mlsettings",
            url: "qrc:///qt/qml/VLC/MediaLibrary/MLFoldersSettings.qml"
        }
    ]


    property ListModel tabModel: ListModel {
        id: tabModelid
        Component.onCompleted: {
            pageModel.forEach(function(e) {
                if (!e.listed)
                    return
                append({
                           displayText: e.displayText,
                           icon: e.icon,
                           name: e.name,
                       })
            })
        }
    }

    ColorContext {
        id: theme
        palette: VLCStyle.palette
        colorSet: ColorContext.View
    }

    Loader {
        id: voronoiSnowLoader

        z: 1.5
        source: "qrc:///qt/qml/VLC/Widgets/VoronoiSnow.qml"
        anchors.fill: parent
        active: false

        function toggleActive() {
            voronoiSnowLoader.active = !voronoiSnowLoader.active
        }

        Component.onCompleted: {
            if (MainCtx.useXmasCone()) {
                MainCtx.kc_pressed.connect(voronoiSnowLoader.toggleActive)
            }
        }
    }

    RowLayout {
        id: mainRowLayout
        anchors.fill: parent
        spacing: 0

        Sidebar {
            id: sidebar
            Layout.fillHeight: true
            Layout.preferredWidth: implicitWidth
            z: 3

            model: g_mainDisplay.tabModel
            selectedIndex: sourcesBanner.selectedIndex

            onItemClicked: (index) => {
                const name = g_mainDisplay.tabModel.get(index).name

                if (stackView.isDefaulLoadedForPath([name])) {
                    return
                }

                sourcesBanner.selectedIndex = index
                History.push(["mc", name])
            }
        }

        ColumnLayout {
            id: contentColumn
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Navigation.parentItem: g_mainDisplay

            /* Source selection - Hidden but logic preserved */
            BannerSources {
                id: sourcesBanner
                visible: false
                Layout.preferredHeight: 0
                Layout.minimumHeight: 0
                Layout.maximumHeight: 0
                Layout.fillWidth: true

                showGlobalToolbar: false
                model: g_mainDisplay.tabModel

                playlistPane: playlistLoader.active ? playlistLoader.item
                                                    : (playlistWindowLoader.status === Loader.Ready ? playlistWindowLoader.item.playlistView
                                                                                                : null)

                onItemClicked: (index) => {
                    const name = g_mainDisplay.tabModel.get(index).name
                    if (stackView.isDefaulLoadedForPath([name])) {
                        return
                    }
                    selectedIndex = index
                    History.push(["mc", name])
                }

                Navigation.parentItem: contentColumn
                Navigation.downItem: mainRow
            }

            FocusScope {
                id: mainRow

                Layout.fillWidth: true
                Layout.fillHeight: true
                z: 0
                focus: true
                visible: !showEmbeddedPlayer

                Rectangle {
                    id: stackViewParent
                    anchors.fill: parent
                    color: theme.bg.primary

                    Widgets.PageLoader {
                        id: stackView
                        focus: true
                        anchors.fill: parent
                        anchors.rightMargin: (playlistLoader.shown && !VLCStyle.isScreenSmall) ? playlistLoader.width : 0
                        anchors.bottomMargin: g_mainDisplay.displayMargin

                        pageModel: g_mainDisplay.pageModel

                        leftPadding: VLCStyle.applicationHorizontalMargin
                        rightPadding: playlistLoader.shown ? 0 : VLCStyle.applicationHorizontalMargin

                        onCurrentItemChanged: {
                            if (currentItem) {
                                if (currentItem.displayMarginEnd !== undefined)
                                    currentItem.displayMarginEnd = Qt.binding(() => { return g_mainDisplay.displayMargin })
                                if (currentItem.enableEndFade !== undefined)
                                    currentItem.enableEndFade = Qt.binding(() => { return (g_mainDisplay.hasMiniPlayer === false) })
                            }
                        }
                        Navigation.parentItem: contentColumn
                        Navigation.upItem: sourcesBanner
                        Navigation.rightItem: playlistLoader
                        Navigation.downItem:  miniPlayer.visible ? miniPlayer : null
                    }
                }

                // Playlist Loader (Sidebar playlist)
                Loader {
                    id: playlistLoader
                    anchors { top: parent.top; right: parent.right }
                    width: 0
                    height: parent.height - g_mainDisplay.displayMargin
                    visible: false
                    active: MainCtx.playlistDocked
                    state: ((status === Loader.Ready) && MainCtx.playlistVisible) ? "expanded" : ""
                    readonly property bool shown: (status === Loader.Ready) && item.visible
                    onVisibleChanged: { if (!visible) { stackView.focus = true } }
                    states: State {
                        name: "expanded"
                        PropertyChanges { target: playlistLoader; width: playlistLoader.implicitWidth; visible: true }
                    }
                    transitions: Transition {
                        id: playlistTransition
                        enabled: true
                        from: ""; to: "expanded"; reversible: true
                        SequentialAnimation {
                            PropertyAction { property: "visible" }
                            NumberAnimation { property: "width"; duration: VLCStyle.duration_short; easing.type: Easing.InOutSine }
                        }
                    }
                    sourceComponent: PlaylistPane {
                        id: playlist
                        implicitWidth: Math.round(VLCStyle.isScreenSmall ? g_mainDisplay.width * 0.8 : Helpers.clamp(g_mainDisplay.width / resizeHandle.widthFactor, minimumWidth, g_mainDisplay.width / 2))
                        focus: true
                        leftPadding: VLCStyle.border
                        rightPadding: VLCStyle.applicationHorizontalMargin
                        topPadding: VLCStyle.layoutTitle_top_padding
                        bottomPadding: VLCStyle.margin_normal + Math.max(VLCStyle.applicationVerticalMargin - g_mainDisplay.displayMargin, 0)
                        useAcrylic: !VLCStyle.isScreenSmall
                        Navigation.parentItem: contentColumn
                        Navigation.upItem: sourcesBanner
                        Navigation.downItem: miniPlayer.visible ? miniPlayer : null
                        Navigation.cancelAction: function() { MainCtx.playlistVisible = false; stackView.forceActiveFocus() }
                        Widgets.HorizontalResizeHandle {
                            id: resizeHandle
                            property bool _inhibitMainInterfaceUpdate: false
                            parent: playlist
                            anchors { top: parent.top; bottom: parent.bottom; left: parent.left }
                            atRight: false
                            targetWidth: parent.width
                            sourceWidth: g_mainDisplay.width
                            visible: !VLCStyle.isScreenSmall
                            onWidthFactorChanged: { if (!_inhibitMainInterfaceUpdate && visible) MainCtx.setPlaylistWidthFactor(widthFactor) }
                            Component.onCompleted:  _updateFromMainInterface()
                            function _updateFromMainInterface() {
                                if (widthFactor == MainCtx.playlistWidthFactor) return
                                _inhibitMainInterfaceUpdate = true
                                widthFactor = MainCtx.playlistWidthFactor
                                _inhibitMainInterfaceUpdate = false
                            }
                            Connections { target: MainCtx; function onPlaylistWidthFactorChanged() { resizeHandle._updateFromMainInterface() } }
                        }
                    }
                }
            } // FocusScope

            // Embedded Player View
            Loader {
                id: embeddedPlayerLoader
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: showEmbeddedPlayer
                active: showEmbeddedPlayer
                source: "qrc:///qt/qml/VLC/Player/Player.qml"
            }

        } // ColumnLayout
    } // RowLayout

    Loader {
        id: loaderProgress
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: miniPlayer.top
        active: (MainCtx.mediaLibraryAvailable && MainCtx.mediaLibrary.idle === false)
        height: active ? implicitHeight : 0
        source: "qrc:///qt/qml/VLC/Widgets/ScanProgressBar.qml"
        onLoaded: {
            item.background.visible = Qt.binding(function() { return true }) // Simplified
            item.leftPadding = Qt.binding(function() { return VLCStyle.margin_large + VLCStyle.applicationHorizontalMargin })
            item.rightPadding = Qt.binding(function() { return VLCStyle.margin_large + VLCStyle.applicationHorizontalMargin })
            item.bottomPadding = Qt.binding(function() { return VLCStyle.margin_small + (miniPlayer.visible ? 0 : VLCStyle.applicationVerticalMargin) })
        }
    }

    Component {
        id: pipPlayerComponent
        PIPPlayer {
            id: playerPip
            anchors {
                bottom: miniPlayer.top
                left: parent.left
                bottomMargin: VLCStyle.margin_normal
                leftMargin: VLCStyle.margin_normal + VLCStyle.applicationHorizontalMargin
            }
            width: VLCStyle.dp(320, VLCStyle.scale)
            height: VLCStyle.dp(180, VLCStyle.scale)
            z: 2
            visible: g_mainDisplay._showMiniPlayer && MainCtx.hasEmbededVideo
            enabled: g_mainDisplay._showMiniPlayer && MainCtx.hasEmbededVideo
            dragXMin: 0
            dragXMax: g_mainDisplay.width - playerPip.width
            dragYMin: 0
            dragYMax: miniPlayer.y - playerPip.height
            Connections {
                target: g_mainDisplay
                function onWidthChanged() { if (playerPip.x > playerPip.dragXMax) playerPip.x = playerPip.dragXMax }
                function onHeightChanged() { if (playerPip.y > playerPip.dragYMax) playerPip.y = playerPip.dragYMax }
            }
        }
    }

    Dialogs {
        z: 10
        bgContent: g_mainDisplay
        anchors {
            bottom: miniPlayer.visible ? miniPlayer.top : parent.bottom
            left: parent.left
            right: parent.right
        }
    }

    Widgets.FloatingNotification {
        id: notif
        anchors {
            bottom: miniPlayer.top
            left: parent.left
            right: parent.right
            margins: VLCStyle.margin_large
        }
    }

    MiniPlayer {
        id: miniPlayer
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        z: 3
        visible: !showEmbeddedPlayer
        horizontalPadding: VLCStyle.applicationHorizontalMargin
        bottomPadding: VLCStyle.applicationVerticalMargin + VLCStyle.margin_xsmall
        background.visible: true // Simplified
        Navigation.parentItem: contentColumn
        Navigation.upItem: mainRow
        Navigation.cancelItem:sourcesBanner
        onVisibleChanged: {
            if (!visible && miniPlayer.activeFocus)
                stackView.forceActiveFocus()
        }
    }

    Connections {
        target: Player
        function onHasVideoOutputChanged() {
            if (Player.hasVideoOutput && MainCtx.hasEmbededVideo) {
                // MainCtx.requestShowPlayerView() // Handled by showEmbeddedPlayer in MainDisplay
            } else {
                _showMiniPlayer = false;
            }
        }
    }
}
