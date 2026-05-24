#pragma once

#include <QWidget>
#include <QColor>
#include <QTimer>
#include <QScopedPointer>

class LoadingSpinner : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor)
    Q_PROPERTY(int spinnerSize READ spinnerSize WRITE setSize)

public:
    explicit LoadingSpinner(int size = 32,
                            QColor color = QColor(100, 100, 100),
                            QWidget *parent = nullptr);
    ~LoadingSpinner() override;

    void start();
    void stop();
    bool isSpinning() const;

    void setColor(const QColor &color);
    QColor color() const;

    void setSize(int size);
    int spinnerSize() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void advanceAnimation();

    QTimer *m_timer;
    QColor m_color;
    int m_size;
    int m_angle;
};
