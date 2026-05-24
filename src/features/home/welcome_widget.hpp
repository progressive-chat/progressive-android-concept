#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QFrame>

class WelcomeWidget : public QWidget {
    Q_OBJECT

public:
    explicit WelcomeWidget(QWidget *parent = nullptr);
    ~WelcomeWidget() override = default;

signals:
    void addMatrixRequested();
    void addIrcRequested();
    void addLemmyRequested();

private:
    void setupUi();
    QPixmap generateAppIcon(int size) const;
    QFrame *createActionCard(const QString &accentColor,
                              const QString &iconText,
                              const QString &iconBgColor,
                              const QString &title,
                              const QString &description,
                              const QString &buttonText,
                              QObject *receiver,
                              const char *signal);

    QLabel *m_headingLabel = nullptr;
    QLabel *m_subtitleLabel = nullptr;
    QFrame *m_matrixCard = nullptr;
    QFrame *m_ircCard = nullptr;
    QFrame *m_lemmyCard = nullptr;
    QLabel *m_learnMoreLabel = nullptr;
};
