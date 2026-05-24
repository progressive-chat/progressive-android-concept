#pragma once

#include <QDialog>
#include <QStringList>

class QLabel;
class QComboBox;
class QPushButton;
class QNetworkAccessManager;

class RoomUpgradeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RoomUpgradeDialog(const QString &roomId, QWidget *parent = nullptr);
    ~RoomUpgradeDialog() override = default;

signals:
    void goToNewRoom(const QString &newRoomId);

private slots:
    void onUpgradeClicked();
    void onGoToNewRoomClicked();
    void onVersionChanged(int index);

private:
    void setupUi();
    void fetchCurrentVersion();
    QString versionDescription(const QString &version) const;

    QString m_roomId;
    QString m_newRoomId;
    QString m_currentVersion;

    QLabel *m_currentVersionLabel;
    QComboBox *m_targetVersionCombo;
    QLabel *m_warningLabel;
    QLabel *m_changesLabel;
    QLabel *m_statusLabel;
    QLabel *m_newRoomIdLabel;
    QPushButton *m_upgradeButton;
    QPushButton *m_goToNewRoomButton;
    QPushButton *m_cancelButton;
    QNetworkAccessManager *m_nam;
};
