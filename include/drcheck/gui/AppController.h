#pragma once

#include <QObject>
#include <QString>
#include <QUrl>

namespace drcheck::gui {

class AppController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString layoutPath READ getLayoutPath NOTIFY layoutPathChanged)
    Q_PROPERTY(QString rulePath READ getRulePath NOTIFY rulePathChanged)
    Q_PROPERTY(QString jsonReportDirectory READ getJsonReportDirectory NOTIFY jsonReportDirectoryChanged)
    Q_PROPERTY(int violationCount READ getViolationCount NOTIFY violationCountChanged)
    Q_PROPERTY(QString status READ getStatus NOTIFY statusChanged)

public:
    explicit AppController(QObject* parent = nullptr);

    Q_INVOKABLE void setLayoutFile(const QUrl& fileUrl);
    Q_INVOKABLE void setRuleFile(const QUrl& fileUrl);
    Q_INVOKABLE void setJsonReportDirectory(const QUrl& fileUrl);
    Q_INVOKABLE void runDRC();

    QString getLayoutPath() const;
    QString getRulePath() const;
    QString getJsonReportDirectory() const;
    int getViolationCount() const;
    QString getStatus() const;

signals:
    void layoutPathChanged();
    void rulePathChanged();
    void jsonReportDirectoryChanged();
    void violationCountChanged();
    void statusChanged();

private:
    void setViolationCount(int count);
    void setStatus(const QString& newStatus);

    QString layoutPath;
    QString rulePath;
    QString jsonReportDirectory;
    QString status = "Ready";

    int violationCount = 0;
};

}