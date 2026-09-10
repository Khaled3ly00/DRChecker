import QtQuick
import DRCheck

Rectangle {
    id: root
    color: "#111317"

    clip: true

    required property var layoutModel
    required property var layersModel

    property var violationMarker: layoutModel.violationMarker

    property real zoomFactor: 1.0
    property real panX: 0.0
    property real panY: 0.0

    readonly property real minimumZoom: 0.1
    readonly property real maximumZoom: 100.0

    readonly property real drawingPadding: 24

    readonly property real layoutWidth: layoutModel.maxX - layoutModel.minX
    readonly property real layoutHeight: layoutModel.maxY - layoutModel.minY
    readonly property real availableWidth: Math.max(0, root.width - 2 * drawingPadding)
    readonly property real availableHeight: Math.max(0, root.height - 2 * drawingPadding)

    readonly property real fitScale:
        layoutModel.hasLayout && layoutWidth > 0 && layoutHeight > 0
            ? Math.min(availableWidth / layoutWidth, availableHeight / layoutHeight) : 1.0

    readonly property real fittedWidth: layoutWidth * fitScale
    readonly property real fittedHeight: layoutHeight * fitScale

    readonly property real fitOffsetX: (root.width - fittedWidth) / 2

    readonly property real fitOffsetY: (root.height - fittedHeight) / 2

    function worldToScreenX(x) {
        return fitOffsetX + panX + (x - layoutModel.minX) * fitScale * zoomFactor
    }

    function worldToScreenY(y) {
        return fitOffsetY + panY + (layoutModel.maxY - y) * fitScale * zoomFactor
    }

    function fitView() {
        zoomFactor = 1.0
        panX = 0.0
        panY = 0.0
    }

    function zoomAt(screenX, screenY, multiplier) {
        if (!layoutModel.hasLayout) {
            return
        }

        const oldZoom = zoomFactor

        const newZoom = Math.max(minimumZoom, Math.min(maximumZoom, oldZoom * multiplier))

        if (newZoom === oldZoom) {
            return
        }

        const fittedX = (screenX - fitOffsetX - panX) / oldZoom
        const fittedY = (screenY - fitOffsetY - panY) / oldZoom

        panX = screenX - fitOffsetX - fittedX * newZoom

        panY = screenY - fitOffsetY - fittedY * newZoom

        zoomFactor = newZoom
    }

    function zoomIn() {
        zoomAt(root.width / 2, root.height / 2, 1.2)
    }

    function zoomOut() {
        zoomAt(root.width / 2, root.height / 2, 1.0 / 1.2)
    }

    function centerOnWorldPoint(x, y) {
        const screenX = worldToScreenX(x)
        const screenY = worldToScreenY(y)

        panX += root.width / 2 - screenX
        panY += root.height / 2 - screenY
    }

    function centerOnViolation() {
        if (!layoutModel.hasViolationMarker) {
            return
        }

        const marker = violationMarker

        if (marker.hasRegion) {
            centerOnWorldPoint((marker.regionMinX + marker.regionMaxX) / 2, (marker.regionMinY + marker.regionMaxY) / 2)
            return
        }

        if (marker.hasFirstPoint && marker.hasSecondPoint) {
            centerOnWorldPoint((marker.firstX + marker.secondX) / 2, (marker.firstY + marker.secondY) / 2)
            return
        }

        if (marker.hasFirstPoint) {
            centerOnWorldPoint(marker.firstX, marker.firstY)
            return
        }

        if (marker.hasSecondPoint) {
            centerOnWorldPoint(marker.secondX, marker.secondY)
        }
    }

    Connections {
        target: root.layoutModel

        function onLayoutChanged() {
            root.fitView()
        }
    }

    onViolationMarkerChanged: {
        if (layoutModel.hasViolationMarker) {
            centerOnViolation()
        }
    }

    LayoutRenderItem {
        id: layoutRenderer

        layoutModel: root.layoutModel
        layersModel: root.layersModel

        width: root.layoutWidth
        height: root.layoutHeight

        x: root.fitOffsetX + root.panX
        y: root.fitOffsetY + root.panY

        scale: root.fitScale * root.zoomFactor
        viewScale: root.fitScale *root.zoomFactor

        transformOrigin: Item.TopLeft
    }

    MouseArea {
        id: navigationArea

        anchors.fill: parent

        acceptedButtons: Qt.MiddleButton

        property real lastMouseX: 0.0
        property real lastMouseY: 0.0

        onPressed: function(mouse) {
            lastMouseX = mouse.x
            lastMouseY = mouse.y
        }

        onPositionChanged: function(mouse) {
            if (!pressed) {
                return
            }

            const deltaX = mouse.x - lastMouseX
            const deltaY = mouse.y - lastMouseY

            root.panX += deltaX
            root.panY += deltaY

            lastMouseX = mouse.x
            lastMouseY = mouse.y
        }

        onWheel: function(wheel) {
            if (wheel.angleDelta.y > 0) {
                root.zoomAt(wheel.x, wheel.y, 1.2)
            }
            else if (wheel.angleDelta.y < 0) {
                root.zoomAt(wheel.x, wheel.y, 1.0 / 1.2)
            }
        }
    }

    Text {
        anchors.centerIn: parent

        visible: !root.layoutModel.hasLayout

        text: "No layout loaded"
        color: "#777777"
    }
}