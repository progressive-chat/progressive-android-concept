#include "badge_label.hpp"

#include <algorithm>
#include <QPainter>
#include <QPaintEvent>

BadgeLabel::BadgeLabel(int initialCount, QWidget* parent)
    : QWidget(parent)
    , m_count(std::max(0, initialCount))
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setVisible(m_count > 0);
}

int BadgeLabel::count() const { return m_count; }

void BadgeLabel::setCount(int count)
{
    const int clamped = std::max(0, count);
    if (m_count == clamped) return;

    m_count = clamped;
    setVisible(m_count > 0);
    updateGeometry();
    update();
    emit countChanged(m_count);
}

void BadgeLabel::increment() { setCount(m_count + 1); }
void BadgeLabel::decrement() { if (m_count > 0) setCount(m_count - 1); }
void BadgeLabel::clear() { setCount(0); }

QColor BadgeLabel::badgeColor() const { return m_badgeColor; }

void BadgeLabel::setBadgeColor(const QColor& color)
{
    if (m_badgeColor == color) return;
    m_badgeColor = color;
    update();
}

QColor BadgeLabel::textColor() const { return m_textColor; }

void BadgeLabel::setTextColor(const QColor& color)
{
    if (m_textColor == color) return;
    m_textColor = color;
    update();
}

QString BadgeLabel::displayText() const
{
    if (m_count > 99) return QStringLiteral("99+");
    return QString::number(m_count);
}

QSize BadgeLabel::badgeSize() const
{
    const int digits = displayText().length();

    if (digits <= 1) return {20, 20};
    if (digits <= 2) return {26, 20};
    return {34, 20};
}

QSize BadgeLabel::sizeHint() const { return badgeSize(); }

QSize BadgeLabel::minimumSizeHint() const { return badgeSize(); }

void BadgeLabel::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    if (m_count <= 0) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QRect r = rect();
    const int diameter = std::min(r.width(), r.height());

    painter.setBrush(m_badgeColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(r, diameter / 2.0, diameter / 2.0, Qt::RelativeSize);

    QFont font;
    font.setPixelSize(11);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(m_textColor);
    painter.drawText(r, Qt::AlignCenter, displayText());
}
