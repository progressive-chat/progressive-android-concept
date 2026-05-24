#include "loading_spinner.hpp"

#include <QPainter>
#include <QConicalGradient>
#include <QPaintEvent>
#include <QtMath>

LoadingSpinner::LoadingSpinner(int size, QColor color, QWidget *parent)
    : QWidget(parent)
    , m_timer(new QTimer(this))
    , m_color(std::move(color))
    , m_size(size)
    , m_angle(0)
{
    m_timer->setInterval(16);
    m_timer->setTimerType(Qt::PreciseTimer);
    QObject::connect(m_timer, &QTimer::timeout, this, &LoadingSpinner::advanceAnimation);

    setFixedSize(m_size, m_size);
}

LoadingSpinner::~LoadingSpinner() = default;

void LoadingSpinner::start()
{
    m_timer->start();
    update();
}

void LoadingSpinner::stop()
{
    m_timer->stop();
    update();
}

bool LoadingSpinner::isSpinning() const
{
    return m_timer->isActive();
}

void LoadingSpinner::setColor(const QColor &color)
{
    if (m_color != color) {
        m_color = color;
        update();
    }
}

QColor LoadingSpinner::color() const
{
    return m_color;
}

void LoadingSpinner::setSize(int size)
{
    if (m_size != size) {
        m_size = size;
        setFixedSize(m_size, m_size);
        updateGeometry();
        update();
    }
}

int LoadingSpinner::spinnerSize() const
{
    return m_size;
}

QSize LoadingSpinner::sizeHint() const
{
    return QSize(m_size, m_size);
}

QSize LoadingSpinner::minimumSizeHint() const
{
    return QSize(m_size, m_size);
}

void LoadingSpinner::advanceAnimation()
{
    m_angle = (m_angle + 30) % 360;
    update();
}

void LoadingSpinner::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const qreal penWidth = m_size * 0.12;
    const qreal halfPen = penWidth / 2.0;
    const QRectF rect(halfPen, halfPen, m_size - penWidth, m_size - penWidth);

    if (!rect.isValid() || rect.isEmpty()) {
        return;
    }

    if (m_timer->isActive()) {
        QConicalGradient gradient(rect.center(), m_angle);
        gradient.setColorAt(0.0, m_color);
        gradient.setColorAt(0.45, m_color);
        gradient.setColorAt(0.75, QColor(m_color.red(), m_color.green(), m_color.blue(), 80));
        gradient.setColorAt(1.0, QColor(m_color.red(), m_color.green(), m_color.blue(), 0));

        QPen pen(QBrush(gradient), penWidth, Qt::SolidLine, Qt::RoundCap);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);

        const int startAngle = m_angle * 16;
        const int spanAngle = -300 * 16;
        painter.drawArc(rect, startAngle, spanAngle);
    } else {
        QColor faded(m_color.red(), m_color.green(), m_color.blue(), 50);
        QPen pen(faded, penWidth, Qt::SolidLine, Qt::RoundCap);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawArc(rect, 0, 360 * 16);
    }
}
