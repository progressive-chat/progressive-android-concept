#pragma once

#include <QDialog>
#include <QVector>
#include <QString>

class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QLabel;
class QProgressBar;
class QTcpSocket;
class QSslSocket;
class QTimer;
class IrcSession;

struct IrcChannelInfo {
    QString name;
    int userCount = 0;
    QString topic;
};

class IrcChannelListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IrcChannelListDialog(QWidget *parent = nullptr);
    ~IrcChannelListDialog() override;

    void setSession(IrcSession *session);
    void fetchChannelList();

signals:
    void channelJoined(QString channel);

private slots:
    void onTempConnected();
    void onTempReadyRead();
    void onTempError();
    void onResultItemDoubleClicked(QListWidgetItem *item);
    void onJoinClicked();
    void onFilterTextChanged(const QString &text);
    void onRefreshClicked();
    void onJoinManualClicked();
    void onSortColumnClicked(int column);

private:
    void setupUi();
    void startTempConnection();
    void closeTempConnection();
    void applyFilter();
    void populateList();
    void showLoading(bool loading);
    static bool compareNameAsc(const IrcChannelInfo &a, const IrcChannelInfo &b);
    static bool compareNameDesc(const IrcChannelInfo &a, const IrcChannelInfo &b);
    static bool compareUsersAsc(const IrcChannelInfo &a, const IrcChannelInfo &b);
    static bool compareUsersDesc(const IrcChannelInfo &a, const IrcChannelInfo &b);

    enum Column {
        ColName = 0,
        ColUsers,
        ColTopic,
        ColCount
    };

    enum ItemDataRole {
        ChannelIndexRole = Qt::UserRole + 1
    };

    QLineEdit *m_filterBar;
    QListWidget *m_channelList;
    QPushButton *m_refreshButton;
    QPushButton *m_joinButton;
    QProgressBar *m_loadingBar;
    QLabel *m_emptyLabel;

    QLineEdit *m_manualChannelInput;
    QPushButton *m_manualJoinButton;

    QPushButton *m_nameHeader;
    QPushButton *m_usersHeader;
    QPushButton *m_topicHeader;

    IrcSession *m_session = nullptr;

    QTcpSocket *m_tempSocket = nullptr;
    QTimer *m_tempTimeout = nullptr;
    QByteArray m_tempBuffer;
    bool m_tempRegistered = false;
    bool m_listInProgress = false;

    QVector<IrcChannelInfo> m_allChannels;
    QVector<IrcChannelInfo> m_filteredChannels;

    int m_sortColumn = 0;
    bool m_sortAscending = true;

    QString m_host;
    quint16 m_port = 6667;
    bool m_useSsl = false;
    QString m_nick;
};
