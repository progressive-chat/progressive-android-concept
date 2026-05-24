#pragma once

#include <QMap>
#include <QObject>
#include <QSystemTrayIcon>
#include <QIcon>

class QAction;
class QMenu;
class QWidget;

class SystemTrayManager : public QObject {
    Q_OBJECT

public:
    enum class ProtocolType {
        Matrix,
        IRC,
        Lemmy
    };
    Q_ENUM(ProtocolType)

    static SystemTrayManager* instance();

    void setWindow(QWidget* window);
    void setConnected(bool connected, ProtocolType protocol);
    void setUnreadCount(int count);
    void showBalloon(const QString& title, const QString& message);

signals:
    void showWindowRequested();
    void hideWindowRequested();
    void quitRequested();

private slots:
    void onActivated(QSystemTrayIcon::ActivationReason reason);
    void onShowHideWindow();

private:
    explicit SystemTrayManager(QObject* parent = nullptr);
    ~SystemTrayManager() override;

    SystemTrayManager(const SystemTrayManager&) = delete;
    SystemTrayManager& operator=(const SystemTrayManager&) = delete;

    static SystemTrayManager* s_instance;

    bool eventFilter(QObject* obj, QEvent* event) override;

    void createTrayIcon();
    void createContextMenu();
    QIcon generateTrayIcon() const;
    static QIcon generateDotIcon(const QColor& color);

    void updateShowHideAction();
    void updateStatusTexts();
    void updateUnreadAction();
    void updateOverallStatus();

    QSystemTrayIcon* m_trayIcon = nullptr;
    QMenu* m_menu = nullptr;
    QWidget* m_window = nullptr;

    QAction* m_showHideAction = nullptr;
    QAction* m_statusAction = nullptr;
    QMenu* m_protocolMenu = nullptr;
    QMap<ProtocolType, QAction*> m_protocolActions;
    QAction* m_unreadAction = nullptr;
    QAction* m_settingsAction = nullptr;
    QAction* m_quitAction = nullptr;

    QMap<ProtocolType, bool> m_protocolStatus;
    int m_unreadCount = 0;
    bool m_trayAvailable = false;
};
