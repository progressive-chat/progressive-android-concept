#include "reaction_picker.hpp"

#include <QHBoxLayout>
#include <QPushButton>
#include <QPainter>
#include <QPaintEvent>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QShowEvent>
#include <QScreen>
#include <QGuiApplication>
#include <QFont>

ReactionPicker::ReactionPicker(QWidget* parent)
    : QWidget(parent)
{
    m_defaultEmojis = {
        QString::fromUtf8(u8"\U0001F44D"),
        QString::fromUtf8(u8"\u2764\uFE0F"),
        QString::fromUtf8(u8"\U0001F604"),
        QString::fromUtf8(u8"\U0001F62E"),
        QString::fromUtf8(u8"\U0001F622"),
        QString::fromUtf8(u8"\U0001F621")
    };

    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setFocusPolicy(Qt::StrongFocus);

    setupUi();
}

ReactionPicker::~ReactionPicker() = default;

void ReactionPicker::setupUi()
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(8, 6, 8, 6);
    m_layout->setSpacing(4);

    for (const auto& emoji : m_defaultEmojis) {
        auto* btn = createEmojiButton(emoji);
        m_emojiButtons.append(btn);
        m_layout->addWidget(btn);
    }

    m_moreButton = createMoreButton();
    m_layout->addWidget(m_moreButton);

    setFixedSize(m_layout->sizeHint());
}

QPushButton* ReactionPicker::createEmojiButton(const QString& emoji)
{
    auto* btn = new QPushButton(emoji, this);
    btn->setFixedSize(40, 40);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFocusPolicy(Qt::NoFocus);

    QFont font = btn->font();
    font.setPointSize(18);
    font.setStyleStrategy(QFont::NoFontMerging);
    btn->setFont(font);

    btn->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  background: transparent;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 0px;"
        "}"
        "QPushButton:hover {"
        "  background: rgba(255, 255, 255, 0.12);"
        "}"
    ));

    connect(btn, &QPushButton::clicked, this, [this, emoji]() {
        onEmojiClicked(emoji);
    });

    return btn;
}

QPushButton* ReactionPicker::createMoreButton()
{
    auto* btn = new QPushButton(QStringLiteral("···"), this);
    btn->setFixedSize(40, 40);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFocusPolicy(Qt::NoFocus);

    QFont font = btn->font();
    font.setPointSize(16);
    font.setBold(true);
    btn->setFont(font);

    btn->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  background: transparent;"
        "  border: none;"
        "  border-radius: 6px;"
        "  color: #aaa;"
        "  padding: 0px;"
        "}"
        "QPushButton:hover {"
        "  background: rgba(255, 255, 255, 0.12);"
        "  color: #ddd;"
        "}"
    ));

    connect(btn, &QPushButton::clicked, this, &ReactionPicker::moreRequested);

    return btn;
}

void ReactionPicker::onEmojiClicked(const QString& emoji)
{
    if (m_selectedEmoji == emoji) {
        m_selectedEmoji.clear();
    } else {
        m_selectedEmoji = emoji;
    }
    updateButtonStyles();
    emit reactionSelected(emoji);
}

void ReactionPicker::updateButtonStyles()
{
    for (int i = 0; i < m_emojiButtons.size() && i < m_defaultEmojis.size(); ++i) {
        auto* btn = m_emojiButtons[i];
        const auto& emoji = m_defaultEmojis[i];
        if (emoji == m_selectedEmoji) {
            btn->setStyleSheet(QStringLiteral(
                "QPushButton {"
                "  background: rgba(255, 255, 255, 0.18);"
                "  border: 1px solid rgba(255, 255, 255, 0.25);"
                "  border-radius: 6px;"
                "  padding: 0px;"
                "}"
                "QPushButton:hover {"
                "  background: rgba(255, 255, 255, 0.24);"
                "}"
            ));
        } else {
            btn->setStyleSheet(QStringLiteral(
                "QPushButton {"
                "  background: transparent;"
                "  border: none;"
                "  border-radius: 6px;"
                "  padding: 0px;"
                "}"
                "QPushButton:hover {"
                "  background: rgba(255, 255, 255, 0.12);"
                "}"
            ));
        }
    }
}

void ReactionPicker::showAt(const QPoint& globalPos)
{
    QPoint adjusted = globalPos;

    auto* screen = QGuiApplication::screenAt(globalPos);
    if (screen) {
        const QRect screenGeom = screen->availableGeometry();
        if (adjusted.x() + width() > screenGeom.right()) {
            adjusted.setX(screenGeom.right() - width());
        }
        if (adjusted.y() + height() > screenGeom.bottom()) {
            adjusted.setY(globalPos.y() - height() - 8);
        }
        if (adjusted.x() < screenGeom.left()) {
            adjusted.setX(screenGeom.left());
        }
        if (adjusted.y() < screenGeom.top()) {
            adjusted.setY(screenGeom.top());
        }
    }

    move(adjusted);
    show();
    setFocus();
}

void ReactionPicker::setSelectedEmoji(const QString& emoji)
{
    m_selectedEmoji = emoji;
    updateButtonStyles();
}

void ReactionPicker::clearSelection()
{
    m_selectedEmoji.clear();
    updateButtonStyles();
}

QString ReactionPicker::selectedEmoji() const
{
    return m_selectedEmoji;
}

void ReactionPicker::focusOutEvent(QFocusEvent* event)
{
    QWidget::focusOutEvent(event);
    close();
}

void ReactionPicker::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
        return;
    }
    QWidget::keyPressEvent(event);
}

void ReactionPicker::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setBrush(QColor(0x2d, 0x2d, 0x2d));
    painter.setPen(QPen(QColor(0x55, 0x55, 0x55), 1));
    painter.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 10, 10);
}

void ReactionPicker::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    setFocus();
}
