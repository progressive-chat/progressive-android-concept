#pragma once

#include <QDialog>
#include <QVector>
#include <QString>
#include <QStringList>
#include <QMap>

class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QLabel;
class QNetworkAccessManager;
class QTimer;
class QCheckBox;
class QProgressBar;

struct DirectChatUserInfo
{
    QString userId;
    QString displayName;
    QString avatarUrl;
};

class DirectChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DirectChatDialog(const QString &preSelectedUserId = QString(),
                              QWidget *parent = nullptr);
    ~DirectChatDialog() override;

signals:
    void dmCreated(const QString &roomId);
    void dmOpened(const QString &roomId);

private slots:
    void onSearchTextChanged(const QString &text);
    void onSearchDebounceTimeout();
    void onCreateOrOpenClicked();
    void onResultItemToggled(QListWidgetItem *item);

private:
    void setupUi();
    void performSearch(const QString &query);
    void loadExistingDMs();
    void loadRecentUsers();
    void showRecentUsers();
    void addSearchResultItem(const DirectChatUserInfo &user);
    void clearResultsList();
    void updateCreateButton();
    bool isExistingDM(const QString &userId) const;
    QString existingDMRoomId(const QString &userId) const;

    static bool isValidMatrixId(const QString &id);
    static QColor avatarColorFromName(const QString &name);

    QString m_preSelectedUserId;
    QVector<DirectChatUserInfo> m_selectedUsers;
    QVector<DirectChatUserInfo> m_recentUsers;
    QMap<QString, QStringList> m_directMap;

    QLineEdit *m_searchEdit;
    QListWidget *m_resultsList;
    QLabel *m_recentHeaderLabel;
    QLabel *m_searchHeaderLabel;
    QLineEdit *m_roomNameEdit;
    QCheckBox *m_encryptionToggle;
    QPushButton *m_actionButton;
    QPushButton *m_cancelButton;
    QLabel *m_statusLabel;
    QProgressBar *m_progressBar;
    QTimer *m_searchTimer;
    QNetworkAccessManager *m_nam;

    QString m_homeserverUrl;
    QString m_userId;
    QString m_accessToken;
};
