#pragma once

#include <QDialog>

class QLineEdit;
class QPushButton;

class MatrixAuthDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MatrixAuthDialog(QWidget *parent = nullptr);
    ~MatrixAuthDialog() override = default;

private slots:
    void onLoginClicked();
    void onRegisterClicked();

private:
    void setupUi();

    QLineEdit *m_homeserverUrl;
    QLineEdit *m_username;
    QLineEdit *m_password;
    QPushButton *m_loginButton;
    QPushButton *m_registerButton;
};
