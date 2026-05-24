#include "quick_reactions_bar.hpp"

#include <QPainter>
#include <QPainterPath>
#include <QFont>
#include <QGraphicsOpacityEffect>
#include <QEnterEvent>
#include <QToolTip>

const QStringList QuickReactionsBar::s_defaultEmojis = {
    QString::fromUtf8("\xF0\x9F\x91\x8D"),  // 👍
    QString::fromUtf8("\xE2\x9D\xA4\xEF\xB8\x8F"),  // ❤️
    QString::fromUtf8("\xF0\x9F\x98\x84"),  // 😄
    QString::fromUtf8("\xF0\x9F\x98\xAE"),  // 😮
    QString::fromUtf8("\xF0\x9F\x98\xA2"),  // 😢
    QString::fromUtf8("\xF0\x9F\x98\xA1"),  // 😡
};

// ---------------------------------------------------------------------------
// ReactionButton
// ---------------------------------------------------------------------------

ReactionButton::ReactionButton(const QString& emoji, int count, bool userReacted,
                               QWidget* parent)
    : QPushButton(parent)
    , m_emoji(emoji)
    , m_count(count)
    , m_userReacted(userReacted)
{
    setFixedSize(36, 28);
    setCursor(Qt::PointingHandCursor);
    setToolTip(emoji);
    setFlat(true);
    setStyleSheet("background: transparent; border: none;");
}

void ReactionButton::setCount(int count)
{
    m_count = count;
    update();
}

void ReactionButton::setUserReacted(bool reacted)
{
    m_userReacted = reacted;
    update();
}

void ReactionButton::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF rect = this->rect().adjusted(1, 1, -1, -1);
    qreal radius = 6.0;

    QColor bgColor = m_userReacted ? QColor("#89B4FA") : QColor("#313244");
    QColor borderColor = m_userReacted ? QColor("#89B4FA") : QColor("#45475A");
    QColor textColor = m_userReacted ? QColor("#1E1E2E") : QColor("#CDD6F4");

    if (underMouse() && !m_userReacted) {
        bgColor = QColor("#45475A");
    }

    // Background
    QPainterPath path;
    path.addRoundedRect(rect, radius, radius);
    painter.fillPath(path, bgColor);

    // Border
    if (m_userReacted) {
        painter.setPen(QPen(borderColor, 1.5));
    } else {
        painter.setPen(QPen(borderColor, 1));
    }
    painter.drawPath(path);

    // Emoji + count
    painter.setPen(textColor);
    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);

    QString displayText = m_emoji + QString::number(m_count);
    painter.drawText(rect, Qt::AlignCenter, displayText);
}


// ---------------------------------------------------------------------------
// QuickReactionsBar
// ---------------------------------------------------------------------------

QuickReactionsBar::QuickReactionsBar(QWidget* parent)
    : QWidget(parent)
    , m_plusButton(nullptr)
    , m_container(nullptr)
    , m_buttonLayout(nullptr)
    , m_opacityEffect(nullptr)
    , m_fadeAnimation(nullptr)
{
    setupUi();
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

void QuickReactionsBar::setupUi()
{
    // Main horizontal container
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_container = new QWidget(this);
    m_container->setStyleSheet(R"(
        QWidget {
            background-color: #1E1E2E;
            border: 1px solid #45475A;
            border-radius: 8px;
        }
    )");
    mainLayout->addWidget(m_container);
    mainLayout->addStretch();

    m_buttonLayout = new QHBoxLayout(m_container);
    m_buttonLayout->setContentsMargins(4, 2, 4, 2);
    m_buttonLayout->setSpacing(2);

    // Create 6 default emoji buttons with count 0
    for (const QString& emoji : s_defaultEmojis) {
        auto* btn = new ReactionButton(emoji, 0, false, m_container);
        connect(btn, &QPushButton::clicked, this, &QuickReactionsBar::onReactionClicked);
        m_buttons[emoji] = btn;
        m_buttonLayout->addWidget(btn);
    }

    // Plus button
    m_plusButton = new QPushButton("+", m_container);
    m_plusButton->setFixedSize(22, 22);
    m_plusButton->setCursor(Qt::PointingHandCursor);
    m_plusButton->setToolTip("More reactions");
    m_plusButton->setFlat(true);
    m_plusButton->setStyleSheet(R"(
        QPushButton {
            background: transparent;
            color: #6C7086;
            border: none;
            border-radius: 4px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #45475A;
            color: #CDD6F4;
        }
    )");
    connect(m_plusButton, &QPushButton::clicked, this, &QuickReactionsBar::onPlusClicked);
    m_buttonLayout->addWidget(m_plusButton);

    // Opacity effect for fade animation
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    m_opacityEffect->setOpacity(1.0);
    m_container->setGraphicsEffect(m_opacityEffect);

    m_fadeAnimation = new QPropertyAnimation(m_opacityEffect, "opacity", this);
    m_fadeAnimation->setDuration(150);

    // Start hidden
    m_container->setVisible(false);
}

void QuickReactionsBar::setMessageId(const QString& messageId, const QString& roomId)
{
    m_messageId = messageId;
    m_roomId = roomId;
    clearReactions();
}

void QuickReactionsBar::addExistingReaction(const QString& emoji, int count, bool userReacted)
{
    if (m_buttons.contains(emoji)) {
        m_buttons[emoji]->setCount(count);
        m_buttons[emoji]->setUserReacted(userReacted);
        return;
    }

    // If it's not a default emoji, add it before the plus button
    auto* btn = new ReactionButton(emoji, count, userReacted, m_container);
    connect(btn, &QPushButton::clicked, this, &QuickReactionsBar::onReactionClicked);
    m_buttons[emoji] = btn;
    m_buttonLayout->insertWidget(m_buttonLayout->count() - 1, btn);
}

void QuickReactionsBar::clearReactions()
{
    // Reset default buttons
    for (const QString& emoji : s_defaultEmojis) {
        if (m_buttons.contains(emoji)) {
            m_buttons[emoji]->setCount(0);
            m_buttons[emoji]->setUserReacted(false);
        }
    }

    // Remove non-default buttons
    QStringList keys = m_buttons.keys();
    for (const QString& key : keys) {
        if (!s_defaultEmojis.contains(key)) {
            ReactionButton* btn = m_buttons.take(key);
            m_buttonLayout->removeWidget(btn);
            btn->deleteLater();
        }
    }
}

void QuickReactionsBar::showAnimated()
{
    m_container->setVisible(true);

    m_fadeAnimation->stop();
    m_fadeAnimation->setStartValue(0.0);
    m_fadeAnimation->setEndValue(1.0);
    m_fadeAnimation->start();

    show();
}

void QuickReactionsBar::hideAnimated()
{
    m_fadeAnimation->stop();
    m_fadeAnimation->setStartValue(m_opacityEffect->opacity());
    m_fadeAnimation->setEndValue(0.0);
    connect(m_fadeAnimation, &QPropertyAnimation::finished, m_container, &QWidget::hide);
    m_fadeAnimation->start();
}

void QuickReactionsBar::onReactionClicked()
{
    auto* btn = qobject_cast<ReactionButton*>(sender());
    if (!btn) return;

    QString emoji = btn->emoji();
    bool currentlyReacted = btn->property("userReacted").toBool();
    bool add = !currentlyReacted;

    // Optimistic local update
    btn->setUserReacted(add);
    btn->setCount(btn->count() + (add ? 1 : -1));

    emit reactionToggled(m_messageId, emoji, add);
}

void QuickReactionsBar::onPlusClicked()
{
    emit pickerRequested(m_messageId);
}

void QuickReactionsBar::enterEvent(QEnterEvent* event)
{
    Q_UNUSED(event);
    // Bar stays visible while hovered — handled by parent (message widget)
}

void QuickReactionsBar::leaveEvent(QEvent* event)
{
    Q_UNUSED(event);
    // Parent can call hideAnimated() on leave
}
