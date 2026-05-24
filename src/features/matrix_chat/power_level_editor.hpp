#pragma once

#include <QDialog>
#include <QString>

class QSpinBox;
class QTableWidget;
class QLineEdit;
class QPushButton;
class QCheckBox;
class QLabel;
class QNetworkAccessManager;

class PowerLevelEditor : public QDialog
{
    Q_OBJECT

public:
    explicit PowerLevelEditor(const QString &roomId, QWidget *parent = nullptr);
    ~PowerLevelEditor() override = default;

private slots:
    void onSaveClicked();
    void onResetClicked();
    void onAddUserClicked();
    void onRemoveUserClicked();
    void onAddEventClicked();
    void onRemoveEventClicked();

private:
    void setupUi();
    void fetchCurrentPowerLevels();
    void populateFromJson(const QJsonObject &content);
    void addUserRow(const QString &userId, const QString &displayName, int powerLevel);
    void addEventRow(const QString &eventType, int requiredLevel);

    QString m_roomId;
    QNetworkAccessManager *m_nam;

    QSpinBox *m_usersDefault;
    QSpinBox *m_eventsDefault;
    QSpinBox *m_stateDefault;
    QSpinBox *m_redact;
    QSpinBox *m_ban;
    QSpinBox *m_kick;
    QSpinBox *m_invite;

    QTableWidget *m_usersTable;
    QTableWidget *m_eventsTable;
    QPushButton *m_addUserBtn;
    QPushButton *m_removeUserBtn;
    QPushButton *m_addEventBtn;
    QPushButton *m_removeEventBtn;

    QCheckBox *m_notifyCheck;
    QPushButton *m_saveBtn;
    QPushButton *m_resetBtn;
    QLabel *m_statusLabel;
    QLabel *m_warningLabel;
};
