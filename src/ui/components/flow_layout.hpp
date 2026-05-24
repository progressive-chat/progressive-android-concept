#pragma once

#include <QLayout>
#include <QList>
#include <QWidget>
#include <QRect>
#include <QMargins>

class FlowLayout : public QLayout
{
    Q_OBJECT
public:
    explicit FlowLayout(QWidget *parent = nullptr, int margin = -1, int hSpacing = -1, int vSpacing = -1)
        : QLayout(parent)
        , m_hSpace(hSpacing >= 0 ? hSpacing : 6)
        , m_vSpace(vSpacing >= 0 ? vSpacing : 6)
    {
        if (margin >= 0)
            setContentsMargins(margin, margin, margin, margin);
    }

    ~FlowLayout()
    {
        QLayoutItem *item;
        while ((item = takeAt(0)))
            delete item;
    }

    void addItem(QLayoutItem *item) override
    {
        m_items.append(item);
    }

    int horizontalSpacing() const
    {
        return m_hSpace;
    }

    int verticalSpacing() const
    {
        return m_vSpace;
    }

    int count() const override
    {
        return m_items.size();
    }

    QLayoutItem *itemAt(int index) const override
    {
        return m_items.value(index);
    }

    QLayoutItem *takeAt(int index) override
    {
        if (index >= 0 && index < m_items.size())
            return m_items.takeAt(index);
        return nullptr;
    }

    Qt::Orientations expandingDirections() const override
    {
        return {};
    }

    bool hasHeightForWidth() const override
    {
        return true;
    }

    int heightForWidth(int width) const override
    {
        int height = doLayout(QRect(0, 0, width, 0), true);
        return height;
    }

    QSize minimumSize() const override
    {
        int left, top, right, bottom;
        getContentsMargins(&left, &top, &right, &bottom);
        QSize minSize;
        for (const QLayoutItem *item : m_items) {
            minSize = minSize.expandedTo(item->minimumSize());
        }
        return minSize + QSize(left + right, top + bottom);
    }

    QSize sizeHint() const override
    {
        return minimumSize();
    }

    void setGeometry(const QRect &rect) override
    {
        QLayout::setGeometry(rect);
        doLayout(rect, false);
    }

private:
    int doLayout(const QRect &rect, bool testOnly) const
    {
        int left, top, right, bottom;
        getContentsMargins(&left, &top, &right, &bottom);
        QRect effectiveRect = rect.adjusted(left, top, -right, -bottom);
        int x = effectiveRect.x();
        int y = effectiveRect.y();
        int lineHeight = 0;
        int xEnd = effectiveRect.right() + 1;

        for (QLayoutItem *item : m_items) {
            QWidget *widget = item->widget();
            int spaceX = horizontalSpacing();
            if (spaceX == -1)
                spaceX = widget ? widget->style()->layoutSpacing(
                    QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal) : 0;
            int spaceY = verticalSpacing();
            if (spaceY == -1)
                spaceY = widget ? widget->style()->layoutSpacing(
                    QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical) : 0;

            int nextX = x + item->sizeHint().width() + spaceX;
            if (nextX - spaceX > xEnd && lineHeight > 0) {
                x = effectiveRect.x();
                y = y + lineHeight + spaceY;
                nextX = x + item->sizeHint().width() + spaceX;
                lineHeight = 0;
            }

            if (!testOnly)
                item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));

            x = nextX;
            lineHeight = qMax(lineHeight, item->sizeHint().height());
        }

        return y + lineHeight - rect.y() + bottom;
    }

    QList<QLayoutItem *> m_items;
    int m_hSpace;
    int m_vSpace;
};
