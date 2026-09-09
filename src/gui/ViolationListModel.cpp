#include "drcheck/gui/ViolationListModel.h"


namespace drcheck::gui {
ViolationListModel::ViolationListModel(QObject* parent)
    : QAbstractListModel{ parent }
{
}

int ViolationListModel::rowCount(const QModelIndex& parent) const
{
    // no child for a violation
    if (parent.isValid()) {
        return 0;
    }

    return static_cast<int>(violations.size());
}

int ViolationListModel::getCount() const
{
    return static_cast<int>(violations.size());
}

QHash<int, QByteArray> ViolationListModel::roleNames() const
{
    return {
        { MessageRole, "message" },
        { SummaryTextRole, "summaryText" }
    };
}

QVariant ViolationListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(violations.size()))
    {
        return {};
    }

    const domain::Violation& violation = violations[index.row()];

    switch (role)
    {
    case MessageRole:
        return QString::fromStdString(violation.getMessage());

    case SummaryTextRole:
    {
        QString firstLayerName;
        QString secondLayerName;

        const auto& marker = violation.getMarker();

        if (marker.has_value())
        {
            if (marker->firstLayer != nullptr)
            {
                firstLayerName = QString::fromStdString(marker->firstLayer->getName());
            }

            if (marker->secondLayer != nullptr)
            {
                secondLayerName = QString::fromStdString(marker->secondLayer->getName());
            }

            QString layerText;

            if (!firstLayerName.isEmpty())
            {
                layerText = firstLayerName;

                if (!secondLayerName.isEmpty() && secondLayerName != firstLayerName)
                {
                    layerText += "-" + secondLayerName;
                }
            }

            QString summary = QString::fromStdString(violation.getTypeAsString());

            if (!layerText.isEmpty())
            {
                summary += " " + layerText;
            }

            summary += " " + QString::number(violation.getActualValue()) + "/" + QString::number(violation.getRequiredValue());

            return summary;
        }
    }
    }
    return {};
}

void ViolationListModel::setViolations(std::vector<domain::Violation> newViolations)
{
    const int previousCount = static_cast<int>(violations.size());

    beginResetModel();

    violations = std::move(newViolations);

    endResetModel();

    if (previousCount != static_cast<int>(violations.size())) {
        emit countChanged();
    }
}

void ViolationListModel::clear()
{
    if (violations.empty()) {
        return;
    }

    beginResetModel();

    violations.clear();

    endResetModel();

    emit countChanged();
}
}