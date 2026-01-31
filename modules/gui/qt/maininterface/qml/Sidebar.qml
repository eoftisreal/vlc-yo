/*****************************************************************************
 * Copyright (C) 2024 VLC authors and VideoLAN
 *****************************************************************************/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import VLC.Style
import VLC.Widgets as Widgets

Rectangle {
    id: root

    property alias model: navRepeater.model
    property int selectedIndex: 0
    signal itemClicked(int index)

    implicitWidth: VLCStyle.dp(240, VLCStyle.scale)
    color: theme.bg.primary

    ColorContext {
        id: theme
        palette: VLCStyle.palette
        colorSet: ColorContext.View
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: VLCStyle.margin_small

        // Logo Area
        Item {
            Layout.preferredHeight: VLCStyle.dp(80, VLCStyle.scale)
            Layout.fillWidth: true

            Widgets.BannerCone {
                anchors.centerIn: parent
                height: VLCStyle.dp(40, VLCStyle.scale)
                width: height
                color: theme.accent
            }
        }

        Widgets.NavigableCol {
            id: navCol
            Layout.fillWidth: true
            Layout.fillHeight: true

            Repeater {
                id: navRepeater
                delegate: Rectangle {
                    id: delegateItem
                    width: parent.width
                    height: VLCStyle.dp(48, VLCStyle.scale)
                    color: (mouseArea.containsMouse && !selected) ? theme.bg.secondary : "transparent"

                    property bool selected: (root.model.get(index).name === root.parent.selectedName) // Hacky match? No, MainDisplay uses index
                    // Actually MainDisplay uses: selectedIndex = index
                    // Let's rely on root.selectedIndex

                    readonly property bool isSelected: root.selectedIndex === index

                    Rectangle {
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        height: parent.height * 0.6
                        width: VLCStyle.dp(4, VLCStyle.scale)
                        radius: width/2
                        color: theme.accent
                        visible: delegateItem.isSelected
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: VLCStyle.margin_large
                        anchors.rightMargin: VLCStyle.margin_normal
                        spacing: VLCStyle.margin_normal

                        Widgets.IconLabel {
                            text: model.icon
                            font.pixelSize: VLCStyle.icon_large
                            color: delegateItem.isSelected ? theme.accent : theme.text.primary
                        }

                        Widgets.LabelExt {
                            text: model.displayText
                            color: delegateItem.isSelected ? theme.accent : theme.text.primary
                            font.pixelSize: VLCStyle.fontSize_large
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
        }
    }

    // Vertical Divider
    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 1
        color: theme.separator
        opacity: 0.5
    }
}
