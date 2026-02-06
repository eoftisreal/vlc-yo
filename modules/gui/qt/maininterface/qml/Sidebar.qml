/*****************************************************************************
 * Copyright (C) 2024 VLC authors and VideoLAN
 *****************************************************************************/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import VLC.Style
import VLC.Widgets as Widgets
import VLC.Dialogs

Rectangle {
    id: root

    property alias model: navRepeater.model
    property int selectedIndex: 0
    signal itemClicked(int index)

    implicitWidth: VLCStyle.dp(240, VLCStyle.scale)
    color: "#0F0F0F" // Deep dark background

    ColorContext {
        id: theme
        palette: VLCStyle.palette
        colorSet: ColorContext.View
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: VLCStyle.margin_normal
        spacing: VLCStyle.margin_small

        // Logo Area
        Item {
            Layout.preferredHeight: VLCStyle.dp(60, VLCStyle.scale)
            Layout.fillWidth: true

            Widgets.BannerCone {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                height: VLCStyle.dp(32, VLCStyle.scale)
                width: height
                color: VLCStyle.accentColor
            }

            Widgets.LabelExt {
                anchors.left: parent.left
                anchors.leftMargin: VLCStyle.dp(40, VLCStyle.scale)
                anchors.verticalCenter: parent.verticalCenter
                text: "APOI"
                font.pixelSize: VLCStyle.fontSize_xlarge
                font.weight: Font.Bold
                color: theme.fg.primary
            }
        }

        // Navigation Items
        Widgets.NavigableCol {
            id: navCol
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: VLCStyle.margin_xxsmall

            Repeater {
                id: navRepeater
                delegate: Rectangle {
                    id: delegateItem
                    width: parent.width
                    height: VLCStyle.dp(40, VLCStyle.scale)
                    color: (mouseArea.containsMouse || isSelected) ? (isSelected ? "#2A2A2A" : "#1A1A1A") : "transparent"
                    radius: VLCStyle.dp(6, VLCStyle.scale)

                    readonly property bool isSelected: root.selectedIndex === index

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: VLCStyle.margin_normal
                        anchors.rightMargin: VLCStyle.margin_normal
                        spacing: VLCStyle.margin_normal

                        Widgets.IconLabel {
                            text: model.icon
                            font.pixelSize: VLCStyle.icon_normal
                            color: delegateItem.isSelected ? theme.fg.primary : theme.fg.secondary
                        }

                        Widgets.LabelExt {
                            text: model.displayText
                            color: delegateItem.isSelected ? theme.fg.primary : theme.fg.secondary
                            font.pixelSize: VLCStyle.fontSize_normal
                            font.weight: delegateItem.isSelected ? Font.DemiBold : Font.Normal
                            Layout.fillWidth: true
                        }
                    }

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: root.itemClicked(index)
                    }
                }
            }

            Item { Layout.fillHeight: true } // Spacer
        }

        // Open Media Button
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: VLCStyle.dp(56, VLCStyle.scale)
            color: openMediaHover.containsMouse ? "#2A2A2A" : "#1E1E1E"
            radius: VLCStyle.dp(12, VLCStyle.scale)

            RowLayout {
                anchors.centerIn: parent
                spacing: VLCStyle.margin_small
                Widgets.IconLabel {
                    text: VLCIcons.eject
                    color: VLCStyle.accentColor
                    font.pixelSize: VLCStyle.icon_normal
                }
                Widgets.LabelExt {
                    text: qsTr("Open media")
                    color: VLCStyle.accentColor
                    font.pixelSize: VLCStyle.fontSize_large
                    font.weight: Font.Medium
                }
            }

            MouseArea {
                id: openMediaHover
                anchors.fill: parent
                hoverEnabled: true
                onClicked: DialogsProvider.openFileDialog()
            }
        }

        // Bottom Icons
        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: VLCStyle.margin_small
            spacing: VLCStyle.margin_large

            Widgets.IconToolButton {
                text: VLCIcons.back
                font.pixelSize: VLCStyle.icon_small
                color: theme.fg.secondary
                onClicked: History.previous()
            }
             Widgets.IconToolButton {
                text: VLCIcons.next
                font.pixelSize: VLCStyle.icon_small
                color: theme.fg.secondary
                // Forward action?
            }
            Item { Layout.fillWidth: true }
            Widgets.IconToolButton {
                text: VLCIcons.menu
                font.pixelSize: VLCStyle.icon_small
                color: theme.fg.secondary
                onClicked: MainCtx.intfMainWindow.toggleMenu() // Just an example
            }
        }
    }

    // Vertical Divider
    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 1
        color: "#333333"
    }
}
