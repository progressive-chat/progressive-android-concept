#pragma once

#include <QDialog>
#include <QVector>
#include <QString>

class QListWidget;
class QListWidgetItem;
class QComboBox;
class QLineEdit;
class QLabel;
class QPushButton;
class QMenu;
class QTcpSocket;
class QSslSocket;
class QTimer;
class IrcSession;

struct IrcUserInfo {
    QString nick;
    QString user;
    QString host;
    QString rank;
};

class IrcUserListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IrcUserListDialog(const QString &channel, IrcSession *session, QWidget *parent = nullptr);
    ~IrcUserListDialog() override;

private slots:
    void onRefreshClicked();
    void onSearchChanged(const QString &text);
    void onRankFilterChanged(int index);
    void onUserContextMenu(const QPoint &pos);
    void onWhoIs();
    void onStartPrivateChat();
    void onKickUser();
    void onBanUser();
    void onToggleOp();
    void onTempConnected();
    void onTempReadyRead();
    void onTempError();

private:
    void setupUi();
    void fetchUserList();
    void populateList();
    void updateUserCount();
    int rankValue(const QString &rank) const;
    void startTempConnection();
    void closeTempConnection();
    void sendLine(const QString &line);
    void executeMode(const QString &nick, const QString &mode, bool add);
    void executeKick(const QString &nick, const QString &reason);
    void executeBan(const QString &nick);
    void executeWhoIs(const QString &nick);
    QString extractNick(const QString &prefix) const;
    QString extractUser(const QString &prefix) const;
    QString extractHost(const QString &prefix) const;
    static QString cleanNick(const QString &nickWithRank);

    QString m_channel;
    IrcSession *m_session = nullptr;

    QLineEdit *m_searchBar;
    QComboBox *m_rankFilter;
    QListWidget *m_userList;
    QLabel *m_userCountLabel;
    QPushButton *m_refreshButton;
    QPushButton *m_closeButton;

    QTcpSocket *m_tempSocket = nullptr;
    QTimer *m_tempTimeout = nullptr;
    QByteArray m_tempBuffer;

    bool m_tempRegistered = false;
    bool m_namesInProgress = false;
    bool m_whoisInProgress = false;

    QVector<IrcUserInfo> m_allUsers;

    QString m_host;
    quint16 m_port = 6667;
    bool m_useSsl = false;
    QString m_nick;
    QString m_whoisTarget;
    QString m_whoisBuffer;
    QString m_contextNick;

    enum RankFilter {
        FilterAll = 0,
        FilterOps,
        FilterVoiced,
        FilterNormal,
        FilterCount
    };
};
