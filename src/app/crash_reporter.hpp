#pragma once

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QStringList>

struct CrashInfo
{
    QDateTime timestamp;
    int signalNumber = 0;
    QString signalName;
    QString appVersion;
    QString osInfo;
    QStringList stackTrace;

    QString toString() const;
    static QString signalNameFromNumber(int sig);
};

class CrashReporter : public QObject
{
    Q_OBJECT

public:
    static CrashReporter *instance();

    void init(const QString &appVersion);
    void checkForPreviousCrash();
    void sendCrashReport(const QString &crashLog);
    void setReportEndpoint(const QString &url);

    CrashInfo lastCrash() const;
    QString crashDirectory() const;

    static QString osInformation();

signals:
    void previousCrashDetected(const QString &crashLog,
                               const QDateTime &timestamp);

private:
    explicit CrashReporter(QObject *parent = nullptr);
    ~CrashReporter() override;
    CrashReporter(const CrashReporter &) = delete;
    CrashReporter &operator=(const CrashReporter &) = delete;

    void installSignalHandlers();

    static void signalHandler(int signal);

    static CrashReporter *s_instance;
    static QByteArray s_crashDirBytes;
    static QByteArray s_appVersionBytes;
    static QByteArray s_osInfoBytes;

    QString m_appVersion;
    QString m_reportEndpoint;
    QString m_crashDir;
    QString m_osInfo;
    CrashInfo m_lastCrash;
    bool m_initialized = false;
};
