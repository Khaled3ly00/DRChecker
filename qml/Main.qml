import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Window {
    width: 800
    height: 500
    visible: true
    title: "DRChecker"

    readonly property bool canRun:
        appController.layoutPath.length > 0 &&
        appController.rulePath.length > 0 &&
        appController.jsonReportDirectory.length > 0

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

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 20

        Button {
            text: "Open Layout"

            onClicked: {
                layoutFileDialog.open()
            }
        }

        Text {
            text: appController.layoutPath.length > 0 ? appController.layoutPath : "No layout selected"
        }

        Button {
            text: "Open Rules"

            onClicked: {
                ruleFileDialog.open()
            }
        }

        Text {
            text: appController.rulePath.length > 0 ? appController.rulePath : "No rule deck selected"
        }

        Button {
            text: "Select Report Folder"

            onClicked: {
                jsonReportFolderDialog.open()
            }
        }

        Text {
            text: appController.jsonReportDirectory.length > 0 ? appController.jsonReportDirectory : "No JSON report directory selected"
        }

        Button {
            text: "Run DRC"
            enabled: canRun

            onClicked: {
                appController.runDRC()
            }
        }

        Text {
            text: "Status: " + appController.status
        }

        Text {
            text: "Violations: " + appController.violationCount
        }
    }
}