#include "progressive_native.hpp"

#include <QApplication>
#include <QSysInfo>
#include <QDesktopServices>
#include <QUrl>
#include <QFileDialog>
#include <QProcess>
#include <QScreen>
#include <QGuiApplication>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QSslSocket>

#ifdef Q_OS_LINUX
#include <sys/sysinfo.h>
#include <unistd.h>
#endif

namespace progressive_chat {

ProgressiveNative::ProgressiveNative(QObject *parent) : QObject(parent)
{
    setupPlatformHooks();
}

ProgressiveNative::~ProgressiveNative() = default;

bool ProgressiveNative::isLinux()
{
#ifdef Q_OS_LINUX
    return true;
#else
    return false;
#endif
}

bool ProgressiveNative::isMacOS()
{
#ifdef Q_OS_MACOS
    return true;
#else
    return false;
#endif
}

bool ProgressiveNative::isWindows()
{
#ifdef Q_OS_WIN
    return true;
#else
    return false;
#endif
}

bool ProgressiveNative::isWayland()
{
    return qEnvironmentVariable("XDG_SESSION_TYPE") == "wayland";
}

bool ProgressiveNative::isX11()
{
    return qEnvironmentVariable("XDG_SESSION_TYPE") == "x11";
}

QString ProgressiveNative::osName()
{
    return QSysInfo::prettyProductName();
}

QString ProgressiveNative::osVersion()
{
    return QSysInfo::productVersion();
}

QString ProgressiveNative::desktopEnvironment()
{
    return qEnvironmentVariable("XDG_CURRENT_DESKTOP", "Unknown");
}

QString ProgressiveNative::hostname()
{
#ifdef Q_OS_LINUX
    char host[256];
    if (gethostname(host, sizeof(host)) == 0)
        return QString::fromLocal8Bit(host);
#endif
    return "unknown";
}

qint64 ProgressiveNative::totalMemory()
{
#ifdef Q_OS_LINUX
    struct sysinfo info;
    if (sysinfo(&info) == 0)
        return (qint64)info.totalram * info.mem_unit;
#endif
    return 0;
}

qint64 ProgressiveNative::availableMemory()
{
#ifdef Q_OS_LINUX
    struct sysinfo info;
    if (sysinfo(&info) == 0)
        return (qint64)info.freeram * info.mem_unit;
#endif
    return 0;
}

int ProgressiveNative::cpuCores()
{
    return QThread::idealThreadCount();
}

void ProgressiveNative::openUrl(const QString &url)
{
    QDesktopServices::openUrl(QUrl(url));
}

void ProgressiveNative::openFileManager(const QString &path)
{
    QUrl url = QUrl::fromLocalFile(QFileInfo(path).absolutePath());
    QDesktopServices::openUrl(url);
}

void ProgressiveNative::setAutoStart(bool enabled)
{
    QString appName = QApplication::applicationName();
    QString desktopFile = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation)
                          + "/" + appName + ".desktop";

    if (enabled) {
        QFile file(desktopFile);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << "[Desktop Entry]\n";
            stream << "Type=Application\n";
            stream << "Name=" << appName << "\n";
            stream << "Exec=" << QApplication::applicationFilePath() << "\n";
            stream << "X-GNOME-Autostart-enabled=true\n";
        }
    } else {
        QFile::remove(desktopFile);
    }
}

bool ProgressiveNative::isAutoStartEnabled()
{
    QString appName = QApplication::applicationName();
    QString desktopFile = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation)
                          + "/" + appName + ".desktop";
    return QFileInfo::exists(desktopFile);
}

QImage ProgressiveNative::takeScreenshot()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen)
        return screen->grabWindow(0).toImage();
    return QImage();
}

bool ProgressiveNative::nativeNotificationsSupported()
{
#ifdef Q_OS_LINUX
    return QDBusConnection::sessionBus().isConnected() ||
           QProcess::execute("which", {"notify-send"}) == 0;
#else
    return false;
#endif
}

void ProgressiveNative::showNativeNotification(const QString &title, const QString &body,
                                                 const QString &iconPath)
{
#ifdef Q_OS_LINUX
    QStringList args;
    args << "--app-name=ProgressiveChat"
         << title
         << body;
    if (!iconPath.isEmpty())
        args << "--icon=" + iconPath;

    QProcess::startDetached("notify-send", args);
#endif
}

QString ProgressiveNative::saveFileDialog(const QString &title,
                                            const QString &defaultName,
                                            const QString &filter)
{
    return QFileDialog::getSaveFileName(nullptr, title, defaultName, filter);
}

QString ProgressiveNative::openFileDialog(const QString &title, const QString &filter)
{
    return QFileDialog::getOpenFileName(nullptr, title, QString(), filter);
}

QString ProgressiveNative::openDirectoryDialog(const QString &title)
{
    return QFileDialog::getExistingDirectory(nullptr, title);
}

void ProgressiveNative::setupPlatformHooks()
{
    // Monitor screen lock/unlock, system sleep/wake
}

} // namespace progressive_chat
