#pragma once

#include <QDialog>
#include <QList>
#include <QString>
#include <QDateTime>

class QTabWidget;
class QListWidget;
class QLabel;
class QPushButton;
class QLineEdit;
class QGroupBox;
class QSplitter;
class QFrame;
class QDialogButtonBox;
class QProgressBar;

struct DeviceInfo
{
    QString deviceId;
    QString displayName;
    QString deviceKey;      // ed25519 fingerprint
    QStringList algorithms;
    QDateTime lastSeen;
    enum VerificationStatus {
        Verified,
        NotVerified,
        Unverified
    };
    VerificationStatus verificationStatus = Unverified;
};

struct UserKeyInfo
{
    QString userId;
    QString displayName;
    QString deviceId;
    QString deviceKey;      // ed25519 fingerprint
    QStringList algorithms;
    enum KeyStatus {
        KeyVerified,        // green
        KeyNotVerified,     // black
        KeyChanged          // red
    };
    KeyStatus keyStatus = KeyNotVerified;
};

class EncryptionKeyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EncryptionKeyDialog(QWidget *parent = nullptr);
    ~EncryptionKeyDialog() override;

signals:
    void deviceVerificationRequested(const QString &deviceId);
    void signOutDeviceRequested(const QString &deviceId);
    void userVerificationRequested(const QString &userId);
    void userVerificationResetRequested(const QString &userId);
    void backupNowRequested();
    void restoreFromBackupRequested(const QString &recoveryKey);
    void exportKeysRequested(const QString &filePath);
    void importKeysRequested(const QString &filePath, const QString &passphrase);

private slots:
    void onDeviceSelectionChanged();
    void onVerifyDeviceClicked();
    void onSignOutDeviceClicked();
    void onDeviceDetailsCopied();
    void onUserSelectionChanged();
    void onVerifyUserClicked();
    void onResetVerificationClicked();
    void onBackupNowClicked();
    void onRestoreFromBackupClicked();
    void onExportKeysClicked();
    void onImportKeysClicked();
    void onRecoveryKeyTextChanged(const QString &text);

private:
    void setupUi();
    void populateDevices();
    void populateUsers();
    void loadBackupStatus();

    QWidget *createMyDevicesTab();
    QWidget *createUserKeysTab();
    QWidget *createKeyBackupTab();

    QWidget *createDeviceDetailPanel();
    QWidget *createUserDetailPanel();
    QWidget *createVerificationFlowWidget();
    void updateDeviceList();
    void updateUserList();
    void updateBackupStatus();

    QString verificationBadgeHtml(DeviceInfo::VerificationStatus status) const;
    QString keyStatusBadgeHtml(UserKeyInfo::KeyStatus status) const;
    QString truncatedDeviceId(const QString &deviceId) const;

    // ---- Data ----
    QList<DeviceInfo> m_devices;
    QList<UserKeyInfo> m_users;

    // ---- Tab widget ----
    QTabWidget *m_tabWidget;

    // ---- My Devices tab ----
    QSplitter *m_devicesSplitter;
    QListWidget *m_deviceList;
    QWidget *m_deviceDetailPanel;
    QLabel *m_deviceDetailIdLabel;
    QLabel *m_deviceDetailKeyLabel;
    QLabel *m_deviceDetailAlgoLabel;
    QLabel *m_deviceDetailSeenLabel;
    QLabel *m_deviceDetailStatusLabel;
    QPushButton *m_verifyDeviceButton;
    QPushButton *m_signOutDeviceButton;
    QPushButton *m_copyDeviceIdButton;

    // ---- User Keys tab ----
    QSplitter *m_usersSplitter;
    QListWidget *m_userList;
    QWidget *m_userDetailPanel;
    QLabel *m_userDetailNameLabel;
    QLabel *m_userDetailIdLabel;
    QLabel *m_userDetailKeyLabel;
    QLabel *m_userDetailStatusLabel;
    QLabel *m_userDetailDeviceCountLabel;
    QListWidget *m_userDeviceList;
    QPushButton *m_verifyUserButton;
    QPushButton *m_resetVerificationButton;

    // ---- Key Backup tab ----
    QLabel *m_backupStatusLabel;
    QLabel *m_backupLastTimeLabel;
    QLabel *m_backupLocationLabel;
    QProgressBar *m_backupProgressBar;
    QPushButton *m_backupNowButton;
    QPushButton *m_restoreBackupButton;
    QLineEdit *m_recoveryKeyEdit;
    QGroupBox *m_recoveryGroup;

    // ---- Export / Import ----
    QPushButton *m_exportButton;
    QPushButton *m_importButton;

    // ---- Common ----
    QDialogButtonBox *m_buttonBox;
    bool m_backupEnabled;
    QDateTime m_lastBackupTime;
    QString m_backupLocation;
};
