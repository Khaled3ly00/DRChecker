import QtQuick

Rectangle {
    id: root

    required property var layoutModel
    required property var layersModel

    property var polygons: layoutModel.polygons
    property var layerStyles: layersModel.layerStyles
    property var violationMarker: layoutModel.violationMarker

    color: "#111317"

    property real zoomFactor: 1.0
    property real panX: 0.0
    property real panY: 0.0

    readonly property real minimumZoom: 0.1
    readonly property real maximumZoom: 100.0

    readonly property real drawingPadding: 24

    readonly property real layoutWidth: layoutModel.maxX - layoutModel.minX
    readonly property real layoutHeight: layoutModel.maxY - layoutModel.minY
    readonly property real availableWidth: Math.max(0, canvas.width - 2 * drawingPadding)
    readonly property real availableHeight: Math.max(0, canvas.height - 2 * drawingPadding)

    readonly property real fitScale:
        layoutModel.hasLayout && layoutWidth > 0 && layoutHeight > 0
            ? Math.min(availableWidth / layoutWidth, availableHeight / layoutHeight) : 1.0

    readonly property real fittedWidth: layoutWidth * fitScale
    readonly property real fittedHeight: layoutHeight * fitScale

    readonly property real fitOffsetX: (canvas.width - fittedWidth) / 2

    readonly property real fitOffsetY: (canvas.height - fittedHeight) / 2

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

        canvas.requestPaint()
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

        canvas.requestPaint()
    }

    function zoomIn() {
        zoomAt(canvas.width / 2, canvas.height / 2, 1.2)
    }

    function zoomOut() {
        zoomAt(canvas.width / 2, canvas.height / 2, 1.0 / 1.2)
    }

    function centerOnWorldPoint(x, y) {
        const screenX = worldToScreenX(x)
        const screenY = worldToScreenY(y)

        panX += canvas.width / 2 - screenX
        panY += canvas.height / 2 - screenY

        canvas.requestPaint()
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

    function drawViolationMarker(ctx) {
        if (!layoutModel.hasViolationMarker) {
            return
        }

        const marker = violationMarker

        ctx.globalAlpha = 1.0
        ctx.strokeStyle = "#ff4d4d"
        ctx.fillStyle = "#ff4d4d"
        ctx.lineWidth = 2

        if (marker.hasFirstPoint && marker.hasSecondPoint)
        {
            const firstX = worldToScreenX(marker.firstX)
            const firstY = worldToScreenY(marker.firstY)
            const secondX = worldToScreenX(marker.secondX)
            const secondY = worldToScreenY(marker.secondY)

            ctx.beginPath()
            ctx.moveTo(firstX, firstY)
            ctx.lineTo(secondX, secondY)
            ctx.stroke()
            ctx.beginPath()
            ctx.arc(firstX, firstY, 5, 0, 2 * Math.PI)
            ctx.fill()

            ctx.beginPath()
            ctx.arc(secondX, secondY, 5, 0, 2 * Math.PI)
            ctx.fill()
        }
        if (marker.hasRegion)
        {
            const left = worldToScreenX(marker.regionMinX)
            const right = worldToScreenX(marker.regionMaxX)
            const top = worldToScreenY(marker.regionMaxY)
            const bottom = worldToScreenY(marker.regionMinY)

            ctx.globalAlpha = 0.15
            ctx.fillRect(left, top, right - left, bottom - top)
            ctx.globalAlpha = 1.0
            ctx.strokeRect(left, top, right - left, bottom - top)
        }
    }
    onPolygonsChanged: {
        fitView()
    }

    onLayerStylesChanged: {
        canvas.requestPaint()
    }

    onViolationMarkerChanged: {
        if (layoutModel.hasViolationMarker) {
            centerOnViolation()
        }

        canvas.requestPaint()
    }

    Canvas {
        id: canvas

        anchors.fill: parent

        onWidthChanged: {
            requestPaint()
        }

        onHeightChanged: {
            requestPaint()
        }

        onPaint: {
            const ctx = getContext("2d")

            ctx.clearRect(0, 0, width, height)

            if (!root.layoutModel.hasLayout) {
                return
            }

            const minX = root.layoutModel.minX
            const minY = root.layoutModel.minY
            const maxX = root.layoutModel.maxX
            const maxY = root.layoutModel.maxY

            const layoutWidth = maxX - minX
            const layoutHeight = maxY - minY

            if (layoutWidth <= 0 || layoutHeight <= 0) {
                return
            }

            for (let i = 0; i < root.polygons.length; ++i)
            {
                const polygon = root.polygons[i]

                const vertices = polygon.vertices

                const style = root.layerStyles[polygon.layerName]

                if (vertices.length < 3) {
                    continue
                }

                if (!style || !style.visible) {
                    continue
                }

                ctx.beginPath()

                let screenX = root.worldToScreenX(vertices[0].x)
                let screenY = root.worldToScreenY(vertices[0].y)

                ctx.moveTo(screenX, screenY)

                for (let j = 1; j < vertices.length; ++j)
                {
                    screenX = root.worldToScreenX(vertices[j].x)
                    screenY = root.worldToScreenY(vertices[j].y)

                    ctx.lineTo(screenX, screenY)
                }
                ctx.closePath()

                ctx.fillStyle = style.color
                ctx.globalAlpha = 0.3
                ctx.fill()

                ctx.globalAlpha = 1.0

                ctx.strokeStyle = style.color
                ctx.lineWidth = 0.5
                ctx.stroke()
            }
            root.drawViolationMarker(ctx)
        }
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

            canvas.requestPaint()
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