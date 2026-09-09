#include "drcheck/gui/LayerListModel.h"

#include <QColor>
#include <set>

namespace drcheck::gui {

LayerListModel::LayerListModel(QObject* parent)
    : QAbstractListModel{ parent }
{
}

int LayerListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return static_cast<int>(layers.size());
}

int LayerListModel::getCount() const
{
    return static_cast<int>(layers.size());
}

QHash<int, QByteArray> LayerListModel::roleNames() const
{
    return {
        { LayerNameRole, "layerName" },
        { LayerColorRole, "layerColor" },
        { LayerVisibleRole, "layerVisible" }
    };
}

QVariant LayerListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(layers.size()))
    {
        return {};
    }

    const LayerDisplayData& layer = layers[index.row()];

    switch (role)
    {
    case LayerNameRole:
        return layer.name;

    case LayerColorRole:
        return layer.color;

    case LayerVisibleRole:
        return layer.visible;
    }

    return {};
}

QString LayerListModel::generateLayerColor(std::size_t index) const
{
    const int hue = static_cast<int>((index * 137) % 360);

    return QColor::fromHsv(hue, 190, 230).name();
}

void LayerListModel::setLayers(const std::vector<domain::Shape>& shapes)
{
    std::set<std::string> layerNames;

    for (const auto& shape : shapes)
    {
        layerNames.insert(shape.getLayer()->getName());
    }

    const int previousCount = static_cast<int>(layers.size());

    beginResetModel();

    layers.clear();

    std::size_t colorIndex = 0;

    for (const auto& layerName : layerNames)
    {
        layers.push_back({QString::fromStdString(layerName), generateLayerColor(colorIndex), true});

        ++colorIndex;
    }

    endResetModel();

    if (previousCount != static_cast<int>(layers.size())) {
        emit countChanged();
    }

    emit layerStylesChanged();
    emit allVisibleChanged();
}

QVariantMap LayerListModel::getLayerStyles() const
{
    QVariantMap styles;

    for (const auto& layer : layers)
    {
        QVariantMap style;

        style["color"] = layer.color;
        style["visible"] = layer.visible;

        styles[layer.name] = style;
    }

    return styles;
}

void LayerListModel::setLayerVisible(int row, bool visible)
{
    if (row < 0 || row >= static_cast<int>(layers.size()))
    {
        return;
    }

    LayerDisplayData& layer = layers[row];

    if (layer.visible == visible) {
        return;
    }

    const bool previouslyAllVisible = getAllVisible();

    layer.visible = visible;

    const QModelIndex modelIndex = index(row, 0);

    emit dataChanged(modelIndex, modelIndex, { LayerVisibleRole });

    emit layerStylesChanged();

    if (previouslyAllVisible != getAllVisible()) {
        emit allVisibleChanged();
    }
}

void LayerListModel::setAllLayersVisible(bool visible)
{
    if (layers.empty()) {
        return;
    }

    const bool previouslyAllVisible = getAllVisible();

    bool changed = false;

    for (auto& layer : layers)
    {
        if (layer.visible != visible)
        {
            layer.visible = visible;
            changed = true;
        }
    }

    if (!changed) {
        return;
    }

    emit dataChanged(index(0, 0), index(static_cast<int>(layers.size()) - 1, 0), { LayerVisibleRole });

    emit layerStylesChanged();

    if (previouslyAllVisible != getAllVisible()) {
        emit allVisibleChanged();
    }
}

void LayerListModel::setLayerColor(int row, const QColor& color)
{
    if (row < 0 || row >= static_cast<int>(layers.size()) || !color.isValid())
    {
        return;
    }

    LayerDisplayData& layer = layers[row];

    const QString newColor = color.name();

    if (layer.color == newColor) {
        return;
    }

    layer.color = newColor;

    const QModelIndex modelIndex = index(row, 0);

    emit dataChanged(modelIndex, modelIndex, { LayerColorRole });

    emit layerStylesChanged();
}

bool LayerListModel::getAllVisible() const
{
    if (layers.empty()) {
        return false;
    }

    for (const auto& layer : layers)
    {
        if (!layer.visible) {
            return false;
        }
    }

    return true;
}

void LayerListModel::clear()
{
    if (layers.empty()) {
        return;
    }

    beginResetModel();

    layers.clear();

    endResetModel();

    emit countChanged();
    emit layerStylesChanged();
    emit allVisibleChanged();
}

}
