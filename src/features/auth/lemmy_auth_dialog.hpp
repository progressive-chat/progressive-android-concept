#pragma once

#include <QDialog>

class QLineEdit;
class QPushButton;

class LemmyAuthDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LemmyAuthDialog(QWidget *parent = nullptr);
    ~LemmyAuthDialog() override = default;

private slots:
    void onLoginClicked();

private:
    void setupUi();

    QLineEdit *m_instanceUrl;
    QLineEdit *m_username;
    QLineEdit *m_password;
    QPushButton *m_loginButton;
};
