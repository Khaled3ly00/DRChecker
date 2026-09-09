import QtQuick

MouseArea {
    id: root

    required property int index
    required property string summaryText
    required property string message

    width: ListView.view.width
    height: 36

    hoverEnabled: true

    Rectangle {
        anchors.fill: parent

        border.width: 1
        border.color: "#4a4e55"

        color: root.ListView.isCurrentItem ? "#46566a" : root.containsMouse ? "#3a3e45" : "#30343a"

        Text {
            anchors.fill: parent
            anchors.margins: 8

            text: root.summaryText
            color: "white"

            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

    onClicked: {
        if (ListView.view.currentIndex === root.index) {
            ListView.view.currentIndex = -1
        }
        else {
            ListView.view.currentIndex = root.index
        }
    }
}