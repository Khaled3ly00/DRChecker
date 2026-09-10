#include "drcheck/gui/LayoutRenderItem.h"
#include "drcheck/geometry/Constants.h"

#include <algorithm>
#include <QSGNode>
#include <QSGFlatColorMaterial>
#include <QSGOpacityNode>
#include <utility>
#include <cmath>

namespace drcheck::gui {

LayoutRenderItem::LayoutRenderItem(QQuickItem* parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents, true);
}

QSGNode* LayoutRenderItem::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*)
{
    QSGNode* rootNode = oldNode;

    if (rootNode == nullptr)
    {
        rootNode = new QSGNode();

        for (int i = 0; i < 4; ++i)
        {
            auto* container = new QSGNode();

            container->setFlag(QSGNode::OwnedByParent);
            rootNode->appendChildNode(container);
        }

        geometryDirty = true;
        selectedLayerDirty = true;
        violationDirty = true;
        markerDirty = true;
        stylesDirty = true;
    }

    QSGNode* normalLayersNode = rootNode->firstChild();
    QSGNode* selectedLayerNode = normalLayersNode->nextSibling();
    QSGNode* violationShapesNode = selectedLayerNode->nextSibling();
    QSGNode* violationMarkerNode = violationShapesNode->nextSibling();

    if (layoutModel == nullptr || !layoutModel->hasLayout())
    {
        clearChildren(normalLayersNode);
        clearChildren(selectedLayerNode);
        clearChildren(violationShapesNode);
        clearChildren(violationMarkerNode);

        layerOrder.clear();

        geometryDirty = false;
        selectedLayerDirty = false;
        violationDirty = false;
        markerDirty = false;
        stylesDirty = false;

        return rootNode;
    }

    if (geometryDirty)
    {
        rebuildNormalLayers(normalLayersNode);

        geometryDirty = false;

        selectedLayerDirty = true;
        violationDirty = true;
        markerDirty = true;
        stylesDirty = true;
    }

    if (selectedLayerDirty)
    {
        rebuildSelectedLayer(selectedLayerNode);

        selectedLayerDirty = false;
        stylesDirty = true;
    }

    if (violationDirty)
    {
        rebuildViolationShapes(violationShapesNode);

        violationDirty = false;
    }

    if (markerDirty)
    {
        rebuildViolationMarker(violationMarkerNode);

        markerDirty = false;
    }

    if (stylesDirty)
    {
        updateLayerStyles(normalLayersNode, selectedLayerNode);

        stylesDirty = false;
    }

    return rootNode;
}

void LayoutRenderItem::rebuildNormalLayers(QSGNode* parentNode)
{
    clearChildren(parentNode);

    if (layoutModel == nullptr || !layoutModel->hasLayout())
    {
        layerOrder.clear();
        return;
    }

    const auto& layers = layoutModel->getLayerPolygons();

    QStringList sortedLayerNames = layers.keys();
    std::sort(sortedLayerNames.begin(), sortedLayerNames.end());

    layerOrder.clear();

    const double minX = layoutModel->getMinX();
    const double maxY = layoutModel->getMaxY();

    for (const QString& layerName : std::as_const(sortedLayerNames))
    {
        const auto layerIterator = layers.constFind(layerName);

        if (layerIterator == layers.cend()) {
            continue;
        }

        const auto& polygons = layerIterator.value();

        if (polygons.empty()) {
            continue;
        }

        std::size_t lineVertexCount = 0;
        std::vector<QPointF> fillVertices;

        for (const auto& [shapeId, polygon] : polygons)
        {
            if (polygon.size() >= 2)
            {
                lineVertexCount += polygon.size() * 2;
            }

            if (polygon.size() >= 3)
            {
                const auto triangles = triangulatePolygon(polygon);

                fillVertices.insert(fillVertices.end(), triangles.begin(), triangles.end());
            }
        }

        if (lineVertexCount == 0) {
            continue;
        }

        layerOrder.append(layerName);

        auto* opacityNode = new QSGOpacityNode();
        opacityNode->setFlag(QSGNode::OwnedByParent);

        // Normal shape fill
        auto* fillOpacityNode = new QSGOpacityNode();
        fillOpacityNode->setFlag(QSGNode::OwnedByParent);
        fillOpacityNode->setOpacity(0.1);

        auto* fillGeometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), static_cast<int>(fillVertices.size()));

        fillGeometry->setDrawingMode(QSGGeometry::DrawTriangles);

        auto* fillVertexData = fillGeometry->vertexDataAsPoint2D();

        for (std::size_t i = 0; i < fillVertices.size(); ++i)
        {
            const QPointF& point = fillVertices[i];

            const float x = static_cast<float>(point.x() - minX);
            const float y = static_cast<float>(maxY - point.y());

            fillVertexData[i].set(x, y);
        }

        auto* fillMaterial = new QSGFlatColorMaterial();
        fillMaterial->setColor(QColor(184, 189, 197));

        auto* fillGeometryNode = new QSGGeometryNode();
        fillGeometryNode->setGeometry(fillGeometry);
        fillGeometryNode->setMaterial(fillMaterial);
        fillGeometryNode->setFlag(QSGNode::OwnedByParent);
        fillGeometryNode->setFlag(QSGNode::OwnsGeometry);
        fillGeometryNode->setFlag(QSGNode::OwnsMaterial);

        fillOpacityNode->appendChildNode(fillGeometryNode);

        // Normal shape outlines
        auto* geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), static_cast<int>(lineVertexCount));
        geometry->setDrawingMode(QSGGeometry::DrawLines);
        auto* vertexData = geometry->vertexDataAsPoint2D();

        std::size_t vertexIndex = 0;

        for (const auto& [shapeId, polygon] : polygons)
        {
            if (polygon.size() < 2) {
                continue;
            }

            for (std::size_t i = 0; i < polygon.size(); ++i)
            {
                const QPointF& first = polygon[i];
                const QPointF& second = polygon[(i + 1) % polygon.size()];

                const float firstX = static_cast<float>(first.x() - minX);
                const float firstY = static_cast<float>(maxY - first.y());
                const float secondX = static_cast<float>(second.x() - minX);
                const float secondY = static_cast<float>(maxY - second.y());

                vertexData[vertexIndex++].set(firstX, firstY);
                vertexData[vertexIndex++].set(secondX, secondY);
            }
        }

        auto* material = new QSGFlatColorMaterial();
        material->setColor(QColor(184, 189, 197));

        auto* geometryNode = new QSGGeometryNode();
        geometryNode->setGeometry(geometry);
        geometryNode->setMaterial(material);
        geometryNode->setFlag(QSGNode::OwnedByParent);
        geometryNode->setFlag(QSGNode::OwnsGeometry);
        geometryNode->setFlag(QSGNode::OwnsMaterial);

        opacityNode->appendChildNode(fillOpacityNode);
        opacityNode->appendChildNode(geometryNode);

        parentNode->appendChildNode(opacityNode);
    }
}

void LayoutRenderItem::rebuildSelectedLayer(QSGNode* parentNode)
{
    clearChildren(parentNode);

    if (layoutModel == nullptr || layersModel == nullptr || !layoutModel->hasLayout())
    {
        return;
    }

    const QString highlightedLayerName = layersModel->getHighlightedLayerName();

    if (highlightedLayerName.isEmpty()) {
        return;
    }

    const auto& layers = layoutModel->getLayerPolygons();

    const auto layerIterator = layers.constFind(highlightedLayerName);

    if (layerIterator == layers.cend()) {
        return;
    }

    const auto& polygons = layerIterator.value();

    std::vector<QPointF> fillVertices;

    for (const auto& [shapeId, polygon] :polygons)
    {

        const auto triangles = triangulatePolygon(polygon);

        fillVertices.insert(fillVertices.end(), triangles.begin(), triangles.end());
    }

    if (fillVertices.empty()) {
        return;
    }

    const double minX = layoutModel->getMinX();
    const double maxY = layoutModel->getMaxY();

    auto* opacityNode = new QSGOpacityNode();
    opacityNode->setFlag(QSGNode::OwnedByParent);
    opacityNode->setOpacity(0.5);
    auto* geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), static_cast<int>(fillVertices.size()));
    geometry->setDrawingMode(QSGGeometry::DrawTriangles);
    auto* vertexData = geometry->vertexDataAsPoint2D();
    for (std::size_t i = 0; i < fillVertices.size();++i)
    {
        const QPointF& point = fillVertices[i];
        const float localX = static_cast<float>(point.x() - minX);
        const float localY = static_cast<float>(maxY - point.y());
        vertexData[i].set(localX, localY);
    }
    auto* material = new QSGFlatColorMaterial();
    material->setColor(QColor(184, 189, 197));
    auto* geometryNode = new QSGGeometryNode();
    geometryNode->setGeometry(geometry);
    geometryNode->setMaterial(material);
    geometryNode->setFlag(QSGNode::OwnedByParent);
    geometryNode->setFlag(QSGNode::OwnsGeometry);
    geometryNode->setFlag(QSGNode::OwnsMaterial);
    opacityNode->appendChildNode(geometryNode);
    parentNode->appendChildNode(opacityNode);
}

void LayoutRenderItem::rebuildViolationShapes(QSGNode* parentNode)
{
    clearChildren(parentNode);

    if (layoutModel == nullptr || layersModel == nullptr)
    {
        return;
    }

    const auto& selectedShapeIds = layoutModel->getSelectedViolationShapeIds();

    if (selectedShapeIds.empty()) {
        return;
    }

    const auto& layers = layoutModel->getLayerPolygons();
    const QVariantMap styles = layersModel->getLayerStyles();

    const double minX = layoutModel->getMinX();
    const double maxY = layoutModel->getMaxY();

    for (const QString& layerName : std::as_const(layerOrder))
    {
        const auto layerIterator = layers.constFind(layerName);

        if (layerIterator == layers.cend())
        {
            continue;
        }

        std::vector<QPointF> fillVertices;

        for (const auto& [shapeId, polygon] : layerIterator.value())
        {
            const bool selected =
                std::find(selectedShapeIds.begin(), selectedShapeIds.end(), shapeId) != selectedShapeIds.end();

            if (!selected) {
                continue;
            }

            const auto triangles = triangulatePolygon(polygon);

            fillVertices.insert(fillVertices.end(), triangles.begin(), triangles.end());
        }

        if (fillVertices.empty()) {
            continue;
        }

        const QVariantMap style = styles.value(layerName).toMap();
        const QColor layerColor(style.value("color").toString());
        const bool visible = style.value("visible", true).toBool();

        if (!layerColor.isValid()) {
            continue;
        }

        auto* opacityNode = new QSGOpacityNode();
        opacityNode->setFlag(QSGNode::OwnedByParent);
        opacityNode->setOpacity(visible ? 0.5 : 0.0);
        auto* geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), static_cast<int>(fillVertices.size()));
        geometry->setDrawingMode(QSGGeometry::DrawTriangles);
        auto* vertexData = geometry->vertexDataAsPoint2D();
        for (std::size_t i = 0; i < fillVertices.size(); ++i)
        {
            const QPointF& point = fillVertices[i];
            vertexData[i].set(static_cast<float>(point.x() - minX), static_cast<float>(maxY - point.y()));
        }
        auto* material = new QSGFlatColorMaterial();
        material->setColor(layerColor);
        auto* geometryNode = new QSGGeometryNode();
        geometryNode->setGeometry(geometry);
        geometryNode->setMaterial(material);
        geometryNode->setFlag(QSGNode::OwnedByParent);
        geometryNode->setFlag(QSGNode::OwnsGeometry);
        geometryNode->setFlag(QSGNode::OwnsMaterial);
        opacityNode->appendChildNode(geometryNode);
        parentNode->appendChildNode(opacityNode);
    }
}

void LayoutRenderItem::rebuildViolationMarker(QSGNode* parentNode)
{
    clearChildren(parentNode);

    if (layoutModel == nullptr || !layoutModel->hasLayout() || viewScale <= 0.0)
    {
        return;
    }

    const auto& optionalMarker = layoutModel->getSelectedViolationMarker();

    if (!optionalMarker.has_value()) {
        return;
    }

    const auto& marker = optionalMarker.value();

    std::vector<QPointF> markerLineTriangles;
    const double markerHalfSize = 8.0 / viewScale;
    const double markerLineThickness = 3.0 / viewScale;

    std::vector<QPointF> fillVertices;


    auto appendCross = [&markerLineTriangles, markerHalfSize, markerLineThickness](const geometry::Point& point)
    {
        const QPointF center(point.getX(), point.getY());

        appendThickLine(markerLineTriangles, QPointF(center.x() - markerHalfSize, center.y()),
                        QPointF(center.x() + markerHalfSize, center.y()), markerLineThickness);

        appendThickLine(markerLineTriangles, QPointF(center.x(), center.y() - markerHalfSize),
                        QPointF(center.x(), center.y() + markerHalfSize), markerLineThickness);
    };
    if (marker.firstPoint.has_value())
    {
        appendCross(marker.firstPoint.value());
    }

    if (marker.secondPoint.has_value())
    {
        appendCross(marker.secondPoint.value());
    }

    // Connector between the two witness points.
    if (marker.firstPoint.has_value() && marker.secondPoint.has_value())
    {
        appendThickLine(markerLineTriangles, QPointF(marker.firstPoint->getX(), marker.firstPoint->getY()),
                        QPointF(marker.secondPoint->getX(), marker.secondPoint->getY()), markerLineThickness);
    }

    // Density-rule marker.
    if (marker.region.has_value())
    {
        const auto& region = marker.region.value();
        const QPointF bottomLeft(region.getMinX(), region.getMinY());
        const QPointF bottomRight(region.getMaxX(), region.getMinY());
        const QPointF topRight(region.getMaxX(), region.getMaxY());
        const QPointF topLeft(region.getMinX(), region.getMaxY());

        // Rectangle outline.
        appendThickLine(markerLineTriangles, bottomLeft, bottomRight, markerLineThickness);
        appendThickLine(markerLineTriangles, bottomRight, topRight, markerLineThickness);
        appendThickLine(markerLineTriangles, topRight, topLeft, markerLineThickness);
        appendThickLine(markerLineTriangles, topLeft, bottomLeft, markerLineThickness);

        // Rectangle fill as two triangles.
        fillVertices.insert(fillVertices.end(),
            {
                bottomLeft,bottomRight, topRight,
                bottomLeft, topRight, topLeft
            }
            );
    }

    const double minX = layoutModel->getMinX();
    const double maxY = layoutModel->getMaxY();

    if (!fillVertices.empty())
    {
        auto* geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(),static_cast<int>(fillVertices.size()));
        geometry->setDrawingMode(QSGGeometry::DrawTriangles);
        auto* vertexData = geometry->vertexDataAsPoint2D();
        for (std::size_t i = 0; i < fillVertices.size(); ++i)
        {
            const QPointF& point = fillVertices[i];

            vertexData[i].set(static_cast<float>(point.x() - minX), static_cast<float>(maxY - point.y()));
        }

        auto* material = new QSGFlatColorMaterial();
        material->setColor(QColor(255, 77, 77, 45));
        auto* geometryNode = new QSGGeometryNode();
        geometryNode->setGeometry(geometry);
        geometryNode->setMaterial(material);
        geometryNode->setFlag(QSGNode::OwnedByParent);
        geometryNode->setFlag(QSGNode::OwnsGeometry);
        geometryNode->setFlag(QSGNode::OwnsMaterial);
        parentNode->appendChildNode(geometryNode);
    }

    if (!markerLineTriangles.empty())
    {
        auto* geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), static_cast<int>(markerLineTriangles.size()));
        geometry->setDrawingMode(QSGGeometry::DrawTriangles);
        auto* vertexData = geometry->vertexDataAsPoint2D();
        for (std::size_t i = 0; i < markerLineTriangles.size(); ++i)
        {
            const QPointF& point = markerLineTriangles[i];
            vertexData[i].set(static_cast<float>(point.x() - minX), static_cast<float>(maxY - point.y()));
        }
        auto* material = new QSGFlatColorMaterial();
        material->setColor(QColor(255, 77, 77));
        auto* geometryNode = new QSGGeometryNode();
        geometryNode->setGeometry(geometry);
        geometryNode->setMaterial(material);
        geometryNode->setFlag(QSGNode::OwnedByParent);
        geometryNode->setFlag(QSGNode::OwnsGeometry);
        geometryNode->setFlag(QSGNode::OwnsMaterial);
        parentNode->appendChildNode(geometryNode);
    }
}

void LayoutRenderItem::updateLayerStyles(QSGNode* normalLayersNode, QSGNode* selectedLayerNode)
{
    if (layersModel == nullptr) {
        return;
    }

    const QVariantMap styles = layersModel->getLayerStyles();
    QSGNode* layerNode = normalLayersNode->firstChild();

    for (const QString& layerName : std::as_const(layerOrder))
    {
        if (layerNode == nullptr) {
            break;
        }

        auto* opacityNode = static_cast<QSGOpacityNode*>(layerNode);

        auto* fillOpacityNode = static_cast<QSGOpacityNode*>(opacityNode->firstChild());
        auto* fillGeometryNode = static_cast<QSGGeometryNode*>(fillOpacityNode->firstChild());
        auto* fillMaterial = static_cast<QSGFlatColorMaterial*>(fillGeometryNode->material());

        auto* outlineGeometryNode = static_cast<QSGGeometryNode*>(fillOpacityNode->nextSibling());
        auto* outlineMaterial = static_cast<QSGFlatColorMaterial*>(outlineGeometryNode->material());

        const QVariantMap style = styles.value(layerName).toMap();
        const QColor color(style.value("color").toString());

        const bool visible = style.value("visible", true).toBool();

        if (color.isValid())
        {
            if (fillMaterial->color() != color)
            {
                fillMaterial->setColor(color);
                fillGeometryNode->markDirty(QSGNode::DirtyMaterial);
            }

            if (outlineMaterial->color() != color)
            {
                outlineMaterial->setColor(color);
                outlineGeometryNode->markDirty(QSGNode::DirtyMaterial);
            }
        }

        opacityNode->setOpacity(visible ? 1.0 : 0.0);

        layerNode = layerNode->nextSibling();
    }

    // Update the currently highlighted layer fill.
    const QString highlightedLayerName = layersModel->getHighlightedLayerName();

    if (highlightedLayerName.isEmpty()) {
        return;
    }

    QSGNode* selectedNode = selectedLayerNode->firstChild();

    if (selectedNode == nullptr) {
        return;
    }

    auto* selectedOpacityNode = static_cast<QSGOpacityNode*>(selectedNode);
    auto* selectedGeometryNode = static_cast<QSGGeometryNode*>(selectedOpacityNode->firstChild());
    auto* selectedMaterial = static_cast<QSGFlatColorMaterial*>(selectedGeometryNode->material());

    const QVariantMap selectedStyle = styles.value(highlightedLayerName).toMap();
    const QColor selectedColor(selectedStyle.value("color").toString());
    const bool selectedVisible = selectedStyle.value("visible", true).toBool();

    if (selectedColor.isValid() && selectedMaterial->color() != selectedColor)
    {
        selectedMaterial->setColor(selectedColor);
        selectedGeometryNode->markDirty(QSGNode::DirtyMaterial);
    }

    selectedOpacityNode->setOpacity(selectedVisible ? 0.5 : 0.0);
}

double LayoutRenderItem::getViewScale() const
{
    return viewScale;
}

void LayoutRenderItem::setViewScale(double scale)
{
    if (qFuzzyCompare(viewScale, scale)) {
        return;
    }

    viewScale = scale;

    markerDirty = true;

    emit viewScaleChanged();

    update();
}

QObject* LayoutRenderItem::getLayoutModel() const
{
    return layoutModel;
}


void LayoutRenderItem::setLayoutModel(QObject* model)
{
    LayoutViewModel* newLayoutModel = qobject_cast<LayoutViewModel*>(model);

    if (layoutModel == newLayoutModel) {
        return;
    }

    if (layoutModel != nullptr)
    {
        disconnect(layoutModel, nullptr, this, nullptr);
    }

    layoutModel = newLayoutModel;

    if (layoutModel != nullptr)
    {
        connect(layoutModel, &LayoutViewModel::layoutChanged, this, &LayoutRenderItem::handleLayoutChanged);
        connect(layoutModel, &LayoutViewModel::violationMarkerChanged, this, &LayoutRenderItem::handleViolationSelectionChanged);
    }

    geometryDirty = true;
    selectedLayerDirty = true;
    violationDirty = true;
    markerDirty = true;
    stylesDirty = true;

    emit layoutModelChanged();

    update();
}

void LayoutRenderItem::handleLayoutChanged()
{
    geometryDirty = true;
    selectedLayerDirty = true;
    violationDirty = true;
    markerDirty = true;
    stylesDirty = true;

    update();
}

void LayoutRenderItem::handleViolationSelectionChanged()
{
    violationDirty = true;
    markerDirty = true;

    update();
}

void LayoutRenderItem::handleHighlightedLayerChanged()
{
    selectedLayerDirty = true;
    violationDirty = true;
    markerDirty = true;

    update();
}

void LayoutRenderItem::handleLayerStylesChanged()
{
    stylesDirty = true;
    // Violation shapes use their own layer colors,
    violationDirty = true;

    update();
}

void LayoutRenderItem::clearChildren(QSGNode* node)
{
    while (QSGNode* child = node->firstChild())
    {
        node->removeChildNode(child);
        delete child;
    }
}

QObject* LayoutRenderItem::getLayersModel() const
{
    return layersModel;
}

void LayoutRenderItem::setLayersModel(QObject* model)
{
    LayerListModel* newLayersModel = qobject_cast<LayerListModel*>(model);

    if (layersModel == newLayersModel) {
        return;
    }

    if (layersModel != nullptr)
    {
        disconnect(layersModel, nullptr, this, nullptr);
    }

    layersModel = newLayersModel;

    if (layersModel != nullptr)
    {
        connect(layersModel, &LayerListModel::layerStylesChanged, this, &LayoutRenderItem::handleLayerStylesChanged);
        connect(layersModel, &LayerListModel::highlightedLayerNameChanged, this, &LayoutRenderItem::handleHighlightedLayerChanged);
    }

    stylesDirty = true;
    selectedLayerDirty = true;
    violationDirty = true;

    emit layersModelChanged();

    update();
}

double LayoutRenderItem::cross(const QPointF& first, const QPointF& second, const QPointF& third)
{
    return (second.x() - first.x()) * (third.y() - first.y()) - (second.y() - first.y()) * (third.x() - first.x());
}

bool LayoutRenderItem::pointInTriangle(const QPointF& point, const QPointF& first, const QPointF& second, const QPointF& third)
{
    const double firstCross = cross(first, second, point);
    const double secondCross = cross(second, third, point);
    const double thirdCross = cross(third, first, point);
    const bool hasNegative = firstCross < -drcheck::geometry::EPSILON || secondCross < -drcheck::geometry::EPSILON || thirdCross < -drcheck::geometry::EPSILON;
    const bool hasPositive = firstCross > drcheck::geometry::EPSILON || secondCross > drcheck::geometry::EPSILON || thirdCross > drcheck::geometry::EPSILON;

    return !(hasNegative && hasPositive);
}

std::vector<QPointF> LayoutRenderItem::triangulatePolygon(const std::vector<QPointF>& polygon)
{
    std::vector<QPointF> vertices;

    vertices.reserve(polygon.size());

    // Remove redundant collinear vertices.
    for (std::size_t i = 0; i < polygon.size(); ++i)
    {
        const QPointF& previous = polygon[(i + polygon.size() - 1) % polygon.size()];
        const QPointF& current = polygon[i];
        const QPointF& next = polygon[(i + 1) % polygon.size()];

        if (std::abs(cross(previous, current, next)) > drcheck::geometry::EPSILON)
        {
            vertices.push_back(current);
        }
    }

    if (vertices.size() < 3) {
        return {};
    }

    double signedArea = 0.0;

    for (std::size_t i = 0; i < vertices.size(); ++i)
    {
        const QPointF& current = vertices[i];
        const QPointF& next = vertices[(i + 1) %vertices.size()];

        signedArea += current.x() * next.y() - next.x() * current.y();
    }

    const bool counterClockwise = signedArea > 0.0;

    std::vector<std::size_t> indices;

    indices.reserve(vertices.size());

    for (std::size_t i = 0; i < vertices.size(); ++i)
    {
        indices.push_back(i);
    }

    std::vector<QPointF> triangles;

    triangles.reserve((vertices.size() - 2) * 3);

    while (indices.size() > 3)
    {
        bool earFound = false;

        for (std::size_t i = 0; i < indices.size(); ++i)
        {
            const std::size_t previousIndex = indices[(i + indices.size() - 1) % indices.size()];
            const std::size_t currentIndex = indices[i];
            const std::size_t nextIndex = indices[ (i + 1) %indices.size()];

            const QPointF& previous = vertices[previousIndex];
            const QPointF& current = vertices[currentIndex];
            const QPointF& next = vertices[nextIndex];

            const double cornerCross = cross(previous, current, next);

            const bool convex = counterClockwise ? cornerCross > drcheck::geometry::EPSILON : cornerCross < -drcheck::geometry::EPSILON;

            if (!convex) {
                continue;
            }

            bool containsVertex = false;

            for (const std::size_t index :
                 indices)
            {
                if (index == previousIndex || index == currentIndex || index == nextIndex)
                {
                    continue;
                }

                if (pointInTriangle(vertices[index], previous, current, next))
                {
                    containsVertex = true;
                    break;
                }
            }

            if (containsVertex) {
                continue;
            }

            triangles.push_back(previous);
            triangles.push_back(current);
            triangles.push_back(next);

            indices.erase(indices.begin() + static_cast<std::ptrdiff_t>(i));

            earFound = true;
            break;
        }

        if (!earFound)
        {
            // Valid simple polygons should not normally
            // reach this point.
            return {};
        }
    }

    triangles.push_back(vertices[indices[0]]);
    triangles.push_back(vertices[indices[1]]);
    triangles.push_back(vertices[indices[2]]);

    return triangles;
}

void LayoutRenderItem::appendThickLine(std::vector<QPointF>& triangles, const QPointF& first, const QPointF& second, double thickness)
{
    const double deltaX = second.x() - first.x();
    const double deltaY = second.y() - first.y();
    const double length = std::sqrt(deltaX * deltaX + deltaY * deltaY);

    if (length <= 0.0) {
        return;
    }

    const double halfThickness = thickness / 2.0;
    const double perpendicularX = -deltaY / length * halfThickness;
    const double perpendicularY = deltaX / length * halfThickness;
    const QPointF firstTop(first.x() + perpendicularX, first.y() + perpendicularY);
    const QPointF firstBottom(first.x() - perpendicularX, first.y() - perpendicularY);
    const QPointF secondTop(second.x() + perpendicularX, second.y() + perpendicularY);
    const QPointF secondBottom(second.x() - perpendicularX, second.y() - perpendicularY);

    triangles.insert(triangles.end(),
        {
            firstTop, firstBottom, secondTop,
            secondTop, firstBottom, secondBottom
        });
}
}