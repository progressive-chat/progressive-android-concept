#pragma once

#include <QDialog>
#include <QJsonObject>
#include <QString>

class QComboBox;
class QLineEdit;
class QCheckBox;
class QPushButton;
class QLabel;
class QNetworkAccessManager;

class RoomPermissionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RoomPermissionsDialog(const QString &roomId, QWidget *parent = nullptr);
    ~RoomPermissionsDialog() override = default;

private slots:
    void onSaveClicked();
    void onJoinRuleChanged(int index);
    void onValueChanged();

private:
    void setupUi();
    void fetchCurrentState();
    void populateFromState();
    bool hasChanges() const;
    void updateChangesIndicator();
    void setModified();

    QString m_roomId;
    QNetworkAccessManager *m_nam;

    QComboBox *m_joinRuleCombo;
    QLineEdit *m_roomAliasEdit;
    QComboBox *m_guestAccessCombo;
    QComboBox *m_historyVisibilityCombo;
    QCheckBox *m_encryptionCheck;

    QPushButton *m_saveBtn;
    QLabel *m_statusLabel;
    QLabel *m_changesIndicator;

    QJsonObject m_originalJoinRules;
    QJsonObject m_originalGuestAccess;
    QJsonObject m_originalHistoryVisibility;
    QJsonObject m_originalEncryption;

    bool m_modified = false;
};
