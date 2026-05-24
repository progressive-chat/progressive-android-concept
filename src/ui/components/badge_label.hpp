#pragma once

#include <QWidget>
#include <QColor>

class BadgeLabel : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int count READ count WRITE setCount NOTIFY countChanged)
    Q_PROPERTY(QColor badgeColor READ badgeColor WRITE setBadgeColor)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)

public:
    explicit BadgeLabel(int initialCount = 0, QWidget* parent = nullptr);
    ~BadgeLabel() override = default;

    int count() const;
    void setCount(int count);
    void increment();
    void decrement();
    void clear();

    QColor badgeColor() const;
    void setBadgeColor(const QColor& color);

    QColor textColor() const;
    void setTextColor(const QColor& color);

signals:
    void countChanged(int count);

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    [[nodiscard]] QString displayText() const;
    [[nodiscard]] QSize badgeSize() const;

    int m_count = 0;
    QColor m_badgeColor{0xFF, 0x3B, 0x30};
    QColor m_textColor{Qt::white};
};
