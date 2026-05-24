#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace progressive_chat {

class WelcomeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WelcomeWidget(QWidget *parent = nullptr);

signals:
    void loginRequested();
    void createAccountRequested();

private:
    void setupUi();

    QLabel *m_titleLabel;
    QLabel *m_subtitleLabel;
    QPushButton *m_loginBtn;
    QPushButton *m_createAccountBtn;
};

} // namespace progressive_chat
