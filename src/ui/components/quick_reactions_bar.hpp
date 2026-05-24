#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QMap>
#include <QString>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>

class QEnterEvent;
class QLeaveEvent;

struct ReactionInfo {
    QString emoji;
    int count = 0;
    bool userReacted = false;
};

class ReactionButton : public QPushButton {
    Q_OBJECT

public:
    explicit ReactionButton(const QString& emoji, int count = 0, bool userReacted = false,
                            QWidget* parent = nullptr);
    void setCount(int count);
    void setUserReacted(bool reacted);
    int count() const { return m_count; }
    QString emoji() const { return m_emoji; }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QString m_emoji;
    int m_count;
    bool m_userReacted;
};


class QuickReactionsBar : public QWidget {
    Q_OBJECT

public:
    explicit QuickReactionsBar(QWidget* parent = nullptr);

    void setMessageId(const QString& messageId, const QString& roomId);
    QString messageId() const { return m_messageId; }

    void addExistingReaction(const QString& emoji, int count, bool userReacted);
    void clearReactions();

    void showAnimated();
    void hideAnimated();

signals:
    void reactionToggled(const QString& messageId, const QString& emoji, bool add);
    void pickerRequested(const QString& messageId);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private slots:
    void onReactionClicked();
    void onPlusClicked();

private:
    void setupUi();
    void updateLayout();

    QString m_messageId;
    QString m_roomId;
    QMap<QString, ReactionButton*> m_buttons; // emoji -> button
    QPushButton* m_plusButton;
    QWidget* m_container;
    QHBoxLayout* m_buttonLayout;
    QGraphicsOpacityEffect* m_opacityEffect;
    QPropertyAnimation* m_fadeAnimation;

    static const QStringList s_defaultEmojis;
};
