import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

ApplicationWindow {
    width: 1200
    height: 750

    minimumWidth: 900
    minimumHeight: 600

    visible: true
    title: "DRChecker"

    readonly property bool canRun:
        appController.layoutPath.length > 0 &&
        appController.rulePath.length > 0 &&
        appController.jsonReportDirectory.length > 0

    property int colorLayerRow: -1

    FileDialog {
        id: layoutFileDialog

        title: "Open Layout"

        nameFilters: [
            "Layout files (*.gds *.gdsii *.json)",
            "GDSII files (*.gds *.gdsii)",
            "JSON files (*.json)",
            "All files (*)"
        ]

        onAccepted: {
            appController.setLayoutFile(selectedFile)
        }
    }

    FileDialog {
        id: ruleFileDialog

        title: "Open Rule Deck"

        nameFilters: [
            "Rule decks (*.tcl *.json)",
            "Tcl rule decks (*.tcl)",
            "JSON rule decks (*.json)",
            "All files (*)"
        ]

        onAccepted: {
            appController.setRuleFile(selectedFile)
        }
    }

    FolderDialog {
        id: jsonReportFolderDialog

        title: "Please choose a folder to save JSON report"

        onAccepted: {
            appController.setJsonReportDirectory(selectedFolder)
        }
    }

    ColorDialog {
        id: layerColorDialog

        title: "Select Layer Color"

        onAccepted: {
            if (colorLayerRow >= 0) {
                layerModel.setLayerColor(colorLayerRow, selectedColor)
            }
        }
    }

    Shortcut {
        sequence: "F"
        enabled: layoutViewModel.hasLayout

        onActivated: {
            layoutView.fitView()
        }
    }

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.margins: 8

            Button {
                text: appController.layoutPath.length > 0 ? "Layout Selected" : "Open Layout"

                onClicked: {
                    layoutFileDialog.open()
                }
            }

            Button {
                text: appController.rulePath.length > 0 ? "Rules Selected" : "Open Rules"

                onClicked: {
                    ruleFileDialog.open()
                }
            }

            Button {
                text: appController.jsonReportDirectory.length > 0 ? "Report Folder Selected" : "Report Folder"

                onClicked: {
                    jsonReportFolderDialog.open()
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: "Run DRC"
                enabled: canRun

                onClicked: {
                    appController.runDRC()
                }
            }
        }
    }

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal

        Rectangle {
            SplitView.preferredWidth: 300
            SplitView.minimumWidth: 260
            SplitView.maximumWidth: 380

            color: "#25282d"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true

                    Text {
                        Layout.fillWidth: true

                        text: "Layers"
                        color: "white"
                        font.bold: true
                    }

                    Button {
                        text: layerModel.allVisible? "Hide All" : "Show All"

                        enabled: layerModel.count > 0

                        onClicked: {
                            layerModel.setAllLayersVisible(!layerModel.allVisible)
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 140

                    border.width: 1
                    border.color: "#4a4e55"

                    color: "#30343a"

                    ColumnLayout {
                        anchors.fill: parent

                        spacing: 2

                        RowLayout {
                            Layout.fillWidth: true

                            spacing: 6

                            Text {
                                Layout.fillWidth: true
                                Layout.alignment: Qt.AlignTop

                                text: " Layer Name"
                                color: "#cccccc"
                            }

                            Text {
                                Layout.preferredWidth: 60
                                Layout.alignment: Qt.AlignTop

                                text: "Color"
                                color: "#cccccc"

                                horizontalAlignment: Text.AlignHCenter
                            }

                            Text {
                                Layout.preferredWidth: 50
                                Layout.alignment: Qt.AlignTop

                                text: "Toggle"
                                color: "#cccccc"

                                horizontalAlignment: Text.AlignHCenter
                            }
                        }

                        ListView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            clip: true

                            model: layerModel

                            ScrollBar.vertical: ScrollBar {
                                policy: ScrollBar.AsNeeded
                            }

                            delegate: LayerItem {
                                layersModel: layerModel

                                onColorRequested: function(row, currentColor) {
                                    colorLayerRow = row

                                    layerColorDialog.selectedColor = currentColor

                                    layerColorDialog.open()
                                }
                            }
                        }
                    }
                }

                Text {
                    text: "Violations"
                    color: "white"
                    font.bold: true
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    border.width: 1
                    border.color: "#4a4e55"

                    color: "#1f2227"

                    ListView {
                        id: violationList

                        anchors.fill: parent
                        clip: true
                        model: violationModel

                        ScrollBar.vertical: ScrollBar {
                            policy: ScrollBar.AsNeeded
                        }

                        currentIndex: -1


                        onCurrentIndexChanged: {
                            appController.selectViolation(currentIndex)
                        }

                        delegate: ViolationItem {
                        }
                    }
                }

                Text {
                    text: "Violation Details"
                    color: "white"
                    font.bold: true
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 120

                    border.width: 1
                    border.color: "#4a4e55"

                    color: "#30343a"

                    ScrollView {
                        id: violationDetailsScrollView

                        anchors.fill: parent
                        anchors.margins: 8

                        contentWidth: availableWidth

                        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                        ScrollBar.vertical.policy: ScrollBar.AsNeeded

                        Text {
                            width: violationDetailsScrollView.availableWidth

                            text: violationList.currentItem ? violationList.currentItem.message : "Select a violation"

                            color: violationList.currentItem ? "white" : "#aaaaaa"

                            wrapMode: Text.WordWrap
                        }
                    }
                }
            }
        }
        LayoutView {
            id: layoutView

            SplitView.fillWidth: true

            layoutModel: layoutViewModel
            layersModel: layerModel
        }
    }

    footer: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.margins: 8

            Text {
                Layout.fillWidth: true

                text: "Status: " + appController.status
            }

            Button {
                text: "Fit"

                enabled: layoutViewModel.hasLayout

                onClicked: {
                    layoutView.fitView()
                }
            }

            Button {
                text: "Zoom +"

                enabled: layoutViewModel.hasLayout

                onClicked: {
                    layoutView.zoomIn()
                }
            }

            Button {
                text: "Zoom -"

                enabled: layoutViewModel.hasLayout

                onClicked: {
                    layoutView.zoomOut()
                }
            }

            Text {
                text: "Violations: " + violationModel.count
            }
        }
    }
}