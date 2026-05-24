#pragma once

#include <QDialog>

class QLineEdit;
class QTextEdit;
class QComboBox;
class QCheckBox;
class QPushButton;
class QLabel;
class QNetworkAccessManager;

class CreateRoomDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateRoomDialog(QWidget *parent = nullptr);
    ~CreateRoomDialog() override = default;

private slots:
    void onCreateClicked();

private:
    void setupUi();

    QLineEdit *m_roomName;
    QTextEdit *m_topic;
    QLineEdit *m_roomAlias;
    QComboBox *m_visibility;
    QCheckBox *m_encryptionToggle;
    QPushButton *m_createButton;
    QLabel *m_statusLabel;
    QNetworkAccessManager *m_nam;
};
