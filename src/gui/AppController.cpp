#include "drcheck/gui/AppController.h"
#include "drcheck/engine/DRCRunner.h"

#include <QDir>

namespace drcheck::gui {

AppController::AppController(QObject* parent)
    : QObject{ parent }
{
}

void AppController::setLayoutFile(const QUrl& fileUrl)
{
    const QString path = fileUrl.toLocalFile();

    if (layoutPath == path) {
        return;
    }

    layoutPath = path;
    emit layoutPathChanged();
}

QString AppController::getLayoutPath() const
{
    return layoutPath;
}

void AppController::setRuleFile(const QUrl& fileUrl)
{
    const QString path = fileUrl.toLocalFile();

    if (rulePath == path) {
        return;
    }

    rulePath = path;
    emit rulePathChanged();
}

QString AppController::getRulePath() const
{
    return rulePath;
}

void AppController::setJsonReportDirectory(const QUrl& fileUrl)
{
    const QString path = fileUrl.toLocalFile();

    if (jsonReportDirectory == path) {
        return;
    }

    jsonReportDirectory = path;
    emit jsonReportDirectoryChanged();
}

QString AppController::getJsonReportDirectory() const
{
    return jsonReportDirectory;
}

int AppController::getViolationCount() const
{
    return violationCount;
}

QString AppController::getStatus() const
{
    return status;
}

void AppController::setViolationCount(int count)
{
    if (violationCount == count) {
        return;
    }

    violationCount = count;
    emit violationCountChanged();
}

void AppController::setStatus(const QString& newStatus)
{
    if (status == newStatus) {
        return;
    }

    status = newStatus;
    emit statusChanged();
}

void AppController::runDRC()
{
    if (layoutPath.isEmpty() ||
        rulePath.isEmpty() ||
        jsonReportDirectory.isEmpty())
    {
        setViolationCount(0);
        setStatus("Layout, rule deck, and JSON report directory are required");
        return;
    }

    // Clear results from the previous run before starting a new DRC run.
    setViolationCount(0);
    setStatus("Running DRC...");

    try
    {
        engine::DRCRunConfig config;

        config.layoutPath = layoutPath.toStdString();
        config.rulesPath = rulePath.toStdString();

        const QString reportPath =
            QDir(jsonReportDirectory).filePath("report.json");

        config.reportPath = reportPath.toStdString();
        config.svgPath = std::nullopt;
        config.topCellName = std::nullopt;

        const auto violations = engine::DRCRunner::run(config);

        setViolationCount(static_cast<int>(violations.size()));

        setStatus("DRC completed successfully");
    }
    catch (const std::exception& exception)
    {
        setViolationCount(0);

        setStatus("Error: " + QString::fromUtf8(exception.what()));
    }
}

}