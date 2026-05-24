#include "ui/components/avatar_widget.hpp"

#include <QPainter>
#include <QPaintEvent>
#include <QFont>
#include <QHash>
#include <QStringList>

static const QStringList kAvatarPalette = {
    QStringLiteral("#E53935"), // red
    QStringLiteral("#D81B60"), // pink
    QStringLiteral("#8E24AA"), // purple
    QStringLiteral("#5E35B1"), // deep purple
    QStringLiteral("#3949AB"), // indigo
    QStringLiteral("#1E88E5"), // blue
    QStringLiteral("#039BE5"), // light blue
    QStringLiteral("#00ACC1"), // cyan
    QStringLiteral("#00897B"), // teal
    QStringLiteral("#43A047"), // green
    QStringLiteral("#7CB342"), // light green
    QStringLiteral("#C0CA33"), // lime
};

AvatarWidget::AvatarWidget(const QString &name, const QString &avatarUrl,
                           int size, QWidget *parent)
    : QWidget(parent)
    , m_name(name)
    , m_avatarUrl(avatarUrl)
    , m_size(size)
{
    setFixedSize(m_size, m_size);
}

void AvatarWidget::setName(const QString &name)
{
    m_name = name;
    update();
}

void AvatarWidget::setSize(int size)
{
    m_size = size;
    setFixedSize(m_size, m_size);
    updateGeometry();
    update();
}

void AvatarWidget::setImageUrl(const QString &avatarUrl)
{
    m_avatarUrl = avatarUrl;
    update();
}

QSize AvatarWidget::sizeHint() const
{
    return QSize(m_size, m_size);
}

void AvatarWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int diameter = qMin(width(), height());
    const QRect circleRect((width() - diameter) / 2,
                           (height() - diameter) / 2,
                           diameter,
                           diameter);

    painter.setPen(Qt::NoPen);
    painter.setBrush(colorFromName(m_name));
    painter.drawEllipse(circleRect);

    QString initials = initialsFromName(m_name);

    QFont font;
    font.setPixelSize(diameter * 2 / 5);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(circleRect, Qt::AlignCenter, initials);
}

QColor AvatarWidget::colorFromName(const QString &name)
{
    const uint hash = qHash(name);
    const int index = static_cast<int>(hash) % kAvatarPalette.size();
    return QColor(kAvatarPalette.at(index));
}

QString AvatarWidget::initialsFromName(const QString &name)
{
    if (name.isEmpty()) {
        return QStringLiteral("?");
    }

    const QStringList parts = name.split(QChar::Space, Qt::SkipEmptyParts);
    if (parts.isEmpty()) {
        return name.left(1).toUpper();
    }

    QString result;
    result += parts.first().at(0).toUpper();
    if (parts.size() >= 2) {
        result += parts.at(1).at(0).toUpper();
    }
    return result;
}
