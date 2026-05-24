#include "typing_indicator.hpp"

#include <QLabel>
#include <QHBoxLayout>
#include <QTimer>
#include <QDateTime>
#include <QFont>

TypingIndicator::TypingIndicator(QWidget* parent)
    : QWidget(parent)
{
    setupUi();

    m_clearTimer = new QTimer(this);
    m_clearTimer->setSingleShot(true);
    m_clearTimer->setInterval(kTypingTimeout);
    connect(m_clearTimer, &QTimer::timeout, this, &TypingIndicator::onTimeout);

    m_animationTimer = new QTimer(this);
    m_animationTimer->setInterval(kAnimationInterval);
    connect(m_animationTimer, &QTimer::timeout, this, &TypingIndicator::animateDots);
    m_animationTimer->start();

    hide();
}

TypingIndicator::~TypingIndicator() = default;

void TypingIndicator::setupUi()
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    m_textLabel = new QLabel(this);
    m_textLabel->setTextFormat(Qt::RichText);
    m_textLabel->setWordWrap(false);

    QFont font = m_textLabel->font();
    font.setPointSize(11);
    m_textLabel->setFont(font);

    m_layout->addWidget(m_textLabel);
    m_layout->addStretch();
}

void TypingIndicator::addTypingUser(const QString& displayName)
{
    m_typingUsers[displayName] = QDateTime::currentMSecsSinceEpoch();
    m_clearTimer->start();
    updateDisplay();
    show();
}

void TypingIndicator::removeTypingUser(const QString& displayName)
{
    m_typingUsers.remove(displayName);
    m_clearTimer->start();
    updateDisplay();

    if (m_typingUsers.isEmpty()) {
        hide();
    }
}

void TypingIndicator::clearTypingUsers()
{
    m_typingUsers.clear();
    m_clearTimer->stop();
    m_animationStep = 0;
    updateDisplay();
    hide();
}

QString TypingIndicator::typingText() const
{
    const int count = m_typingUsers.size();

    if (count == 0) {
        return {};
    }

    QStringList names = m_typingUsers.keys();

    if (count == 1) {
        return names.first() + QStringLiteral(" is typing");
    }

    if (count == 2) {
        return names[0] + QStringLiteral(" and ") + names[1] + QStringLiteral(" are typing");
    }

    const int others = count - 2;
    return names[0] + QStringLiteral(", ") + names[1] + QStringLiteral(", and ")
           + QString::number(others) + QStringLiteral(" other")
           + (others == 1 ? QStringLiteral(" is") : QStringLiteral("s are"))
           + QStringLiteral(" typing");
}

void TypingIndicator::updateDisplay()
{
    QString html;
    html += QStringLiteral("<span style='color:#999;'>");
    html += typingText();
    html += QStringLiteral("</span>");

    html += QStringLiteral(" <span style='font-weight:bold;'>");

    for (int i = 0; i < 3; ++i) {
        if (i == m_animationStep) {
            html += QStringLiteral("<span style='color:#777;'>\u25CF</span>");
        } else {
            html += QStringLiteral("<span style='color:#ccc;'>\u25CF</span>");
        }
    }

    html += QStringLiteral("</span>");

    m_textLabel->setText(html);
}

void TypingIndicator::onTimeout()
{
    clearTypingUsers();
}

void TypingIndicator::animateDots()
{
    m_animationStep = (m_animationStep + 1) % 3;
    updateDisplay();
}
