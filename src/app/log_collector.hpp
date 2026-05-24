#pragma once

#include <QObject>
#include <QDateTime>
#include <QString>
#include <QVector>

class LogCollector : public QObject
{
    Q_OBJECT

public:
    struct LogEntry
    {
        QtMsgType level;
        QString message;
        QDateTime timestamp;
        QString category;
    };

    static LogCollector &instance();

    void install();
    static void restore();

    QVector<LogEntry> entries() const;
    void clear();
    void setMaxEntries(int max);
    int maxEntries() const;

    QString toString() const;

signals:
    void newLogEntry(const LogEntry &entry);

private:
    explicit LogCollector(QObject *parent = nullptr);
    ~LogCollector() override;

    static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    QVector<LogEntry> m_entries;
    int m_maxEntries = 10000;
    bool m_installed = false;
};
