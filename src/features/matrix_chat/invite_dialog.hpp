#pragma once

#include <QDialog>
#include <QString>
#include <QVector>

#include "protocol/protocol_type.hpp"

class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QLabel;
class QScrollArea;
class QProgressBar;
class QNetworkAccessManager;
class QTimer;
class QStackedWidget;
class QCheckBox;

struct UserInfo
{
    QString userId;
    QString displayName;
    QString avatarUrl;
    bool isRecent = false;
};

class InviteDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InviteDialog(const QString &roomId,
                          ProtocolType protocolType = ProtocolType::MATRIX,
                          QWidget *parent = nullptr);
    ~InviteDialog() override = default;

    static bool isValidMatrixId(const QString &id);
    static QString initialsFromName(const QString &name);
    static QColor avatarColorFromName(const QString &name);

private slots:
    void onSearchTextChanged(const QString &text);
    void onSearchDebounceTimeout();
    void onInviteClicked();
    void onResultItemChanged(QListWidgetItem *item);

private:
    void setupUi();
    QWidget *createMatrixInvitePage();
    QWidget *createIrcInvitePage();
    QWidget *createLemmyInvitePage();

    void performSearch(const QString &query);
    void loadRecentContacts();
    void showRecentContacts();
    void addResultItem(const UserInfo &user);
    void addChip(const UserInfo &user);
    void removeChip(const QString &userId);
    void updateSelectedCount();
    void inviteNextMatrixUser();
    void saveRecentContact(const UserInfo &user);
    void clearResultsList();

    QString m_roomId;
    ProtocolType m_protocolType;

    QLineEdit *m_searchEdit = nullptr;
    QListWidget *m_resultsList = nullptr;
    QLabel *m_recentHeaderLabel = nullptr;
    QWidget *m_chipContainer = nullptr;
    QPushButton *m_inviteButton = nullptr;
    QLabel *m_statusLabel = nullptr;
    QProgressBar *m_progressBar = nullptr;
    QCheckBox *m_ircNotifyCheck = nullptr;
    QLineEdit *m_ircNickEdit = nullptr;
    QStackedWidget *m_contentStack = nullptr;
    QNetworkAccessManager *m_nam = nullptr;
    QTimer *m_searchTimer = nullptr;

    QVector<UserInfo> m_selectedUsers;
    QVector<UserInfo> m_recentContacts;
    int m_currentInviteIndex = 0;
    int m_successCount = 0;
    int m_failCount = 0;
};
