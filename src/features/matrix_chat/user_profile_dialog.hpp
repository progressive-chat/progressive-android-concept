#pragma once

#include <QDialog>
#include <QDateTime>

class QLabel;
class QLineEdit;
class QPushButton;
class AvatarWidget;

struct UserProfileData
{
    QString userId;
    QString displayName;
    QString avatarUrl;
    QString statusMessage;
    QString presence;       // "online", "offline", "unavailable", "busy"
    qint64 lastActiveAgo = -1; // milliseconds since last activity, -1 if unknown
    int deviceCount = 0;
    QDateTime joinedDate;
};

class UserProfileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserProfileDialog(const QString &userId, QWidget *parent = nullptr);
    ~UserProfileDialog() override = default;

signals:
    void chatStarted(const QString &userId);
    void inviteRequested(const QString &userId);
    void userBlocked(const QString &userId);
    void encryptionKeysRequested(const QString &userId);
    void profileUpdated(const QString &displayName, const QString &avatarUrl,
                        const QString &statusMessage);

private slots:
    void onStartChat();
    void onInviteToRoom();
    void onBlockUser();
    void onViewEncryptionKeys();
    void onEditProfile();
    void onSaveProfile();

private:
    void setupUi();
    void populateFromProfile(const UserProfileData &profile);
    void setEditable(bool editable);
    static QColor presenceColor(const QString &presence);
    static QString presenceText(const QString &presence);

    QString m_userId;
    UserProfileData m_profile;
    bool m_isOwnProfile = false;
    bool m_isEditing = false;

    // Header
    AvatarWidget *m_avatar;
    QLabel *m_displayNameLabel;
    QLineEdit *m_displayNameEdit;
    QLabel *m_userIdLabel;
    QLabel *m_presenceDot;
    QLabel *m_presenceLabel;
    QLabel *m_statusMessageLabel;
    QLineEdit *m_statusMessageEdit;

    // Action buttons
    QPushButton *m_startChatButton;
    QPushButton *m_inviteButton;
    QPushButton *m_blockButton;
    QPushButton *m_encryptionKeysButton;
    QPushButton *m_editButton;
    QPushButton *m_saveButton;
    QPushButton *m_cancelEditButton;

    // Info section
    QLabel *m_joinedLabel;
    QLabel *m_lastSeenLabel;
    QLabel *m_deviceCountLabel;
};
