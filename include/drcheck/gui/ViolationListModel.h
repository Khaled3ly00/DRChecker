#pragma once

#include <QObject>
#include <QAbstractListModel>

#include "drcheck/domain/Violation.h"

namespace drcheck::gui {
class ViolationListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ getCount NOTIFY countChanged)

public:
    enum ViolationRole
    {
        // Qt reserves lower role numbers for standard purposes so start at UserRole + 1
        MessageRole = Qt::UserRole + 1,
        SummaryTextRole
    };

    Q_ENUM(ViolationRole)

    explicit ViolationListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;

    int getCount() const;

    void setViolations(std::vector<domain::Violation> violations);

    void clear();

signals:
    void countChanged();

private:
    std::vector<domain::Violation> violations;
};
}