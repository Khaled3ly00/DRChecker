import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    required property int index
    required property string layerName
    required property string layerColor
    required property bool layerVisible
    required property var layersModel
    required property bool highlighted

    signal colorRequested(int row, string currentColor)

    width: ListView.view.width
    height: 24

    color: root.highlighted ? "#3a414a" : "#30343a"

    border.width: 1
    border.color: root.highlighted ? root.layerColor : "#4a4e55"

    MouseArea {
        anchors.fill: parent

        onClicked: {
            root.layersModel.toggleLayerHighlight(root.index)
        }
    }


    RowLayout {
        anchors.fill: parent
        anchors.margins: 4

        spacing: 6

        Text {
            Layout.fillWidth: true

            text: root.layerName
            color: "white"
        }

        Item {
            Layout.preferredWidth: 60
            Layout.fillHeight: true

            Rectangle {
                anchors.centerIn: parent

                width: 14
                height: 14
                radius: 2

                color: root.layerColor
                opacity: 0.6

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        root.colorRequested(root.index, root.layerColor)
                    }
                }
            }
        }

        Item {
            Layout.preferredWidth: 50
            Layout.fillHeight: true

            CheckBox {
                anchors.centerIn: parent

                checked: root.layerVisible

                onToggled: {
                    root.layersModel.setLayerVisible(root.index, checked)
                }
            }
        }
    }
}