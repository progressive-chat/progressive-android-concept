#include "log_collector.hpp"

#include <QMutexLocker>
#include <QMutex>
#include <QTextStream>

static QMutex s_logMutex;
static QtMessageHandler s_originalHandler = nullptr;

LogCollector &LogCollector::instance()
{
    static LogCollector collector;
    return collector;
}

LogCollector::LogCollector(QObject *parent)
    : QObject(parent)
{
}

LogCollector::~LogCollector()
{
    restore();
}

void LogCollector::install()
{
    if (m_installed)
        return;

    QMutexLocker locker(&s_logMutex);
    s_originalHandler = qInstallMessageHandler(&LogCollector::messageHandler);
    m_installed = true;
}

void LogCollector::restore()
{
    QMutexLocker locker(&s_logMutex);
    if (s_originalHandler) {
        qInstallMessageHandler(s_originalHandler);
        s_originalHandler = nullptr;
    }
}

void LogCollector::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QMutexLocker locker(&s_logMutex);

    if (s_originalHandler)
        s_originalHandler(type, context, msg);

    LogCollector &self = instance();

    LogEntry entry;
    entry.level = type;
    entry.message = msg;
    entry.timestamp = QDateTime::currentDateTime();
    entry.category = context.category ? QString::fromUtf8(context.category) : QStringLiteral("default");

    self.m_entries.append(entry);

    while (self.m_entries.size() > self.m_maxEntries && !self.m_entries.isEmpty())
        self.m_entries.removeFirst();

    locker.unlock();

    emit self.newLogEntry(entry);
}

QVector<LogCollector::LogEntry> LogCollector::entries() const
{
    return m_entries;
}

void LogCollector::clear()
{
    QMutexLocker locker(&s_logMutex);
    m_entries.clear();
}

void LogCollector::setMaxEntries(int max)
{
    if (max < 1)
        return;
    m_maxEntries = max;
    QMutexLocker locker(&s_logMutex);
    while (m_entries.size() > m_maxEntries && !m_entries.isEmpty())
        m_entries.removeFirst();
}

int LogCollector::maxEntries() const
{
    return m_maxEntries;
}

QString LogCollector::toString() const
{
    QString result;
    QTextStream stream(&result);

    for (const auto &entry : m_entries) {
        const char *levelStr = "DEBUG";
        switch (entry.level) {
        case QtDebugMsg:    levelStr = "DEBUG"; break;
        case QtInfoMsg:     levelStr = "INFO"; break;
        case QtWarningMsg:  levelStr = "WARN"; break;
        case QtCriticalMsg: levelStr = "CRIT"; break;
        case QtFatalMsg:    levelStr = "FATAL"; break;
        }

        stream << entry.timestamp.toString(Qt::ISODateWithMs) << " "
               << "[" << levelStr << "] "
               << "[" << entry.category << "] "
               << entry.message << "\n";
    }

    return result;
}
