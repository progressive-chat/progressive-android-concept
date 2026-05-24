#pragma once

#include <QDialog>

class QLineEdit;
class QSpinBox;
class QCheckBox;
class QPushButton;
struct IrcServerInfo;

class IrcAuthDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IrcAuthDialog(QWidget *parent = nullptr);
    ~IrcAuthDialog() override = default;

    IrcServerInfo serverInfo() const;

private slots:
    void onLoginClicked();

private:
    void setupUi();

    QLineEdit *m_host;
    QSpinBox *m_port;
    QCheckBox *m_ssl;
    QLineEdit *m_nickname;
    QLineEdit *m_username;
    QLineEdit *m_realName;
    QLineEdit *m_password;
    QPushButton *m_loginButton;
};
