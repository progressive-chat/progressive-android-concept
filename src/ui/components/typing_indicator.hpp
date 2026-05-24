#pragma once

#include <QWidget>
#include <QString>
#include <QMap>

class QLabel;
class QHBoxLayout;
class QTimer;

class TypingIndicator : public QWidget
{
    Q_OBJECT

public:
    explicit TypingIndicator(QWidget* parent = nullptr);
    ~TypingIndicator() override;

    void addTypingUser(const QString& displayName);
    void removeTypingUser(const QString& displayName);
    void clearTypingUsers();
    QString typingText() const;

private slots:
    void onTimeout();
    void animateDots();

private:
    void setupUi();
    void updateDisplay();

    QLabel* m_textLabel = nullptr;
    QHBoxLayout* m_layout = nullptr;
    QMap<QString, qint64> m_typingUsers;
    QTimer* m_clearTimer = nullptr;
    QTimer* m_animationTimer = nullptr;
    int m_animationStep = 0;

    static constexpr int kTypingTimeout = 5000;
    static constexpr int kAnimationInterval = 400;
};
