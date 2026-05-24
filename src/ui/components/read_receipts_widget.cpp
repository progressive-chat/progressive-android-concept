#include "read_receipts_widget.hpp"

#include <QPainter>
#include <QPainterPath>
#include <QDateTime>
#include <QToolTip>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QApplication>
#include <QCursor>

ReadReceiptsWidget::ReadReceiptsWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    applyStyle();
    updateDisplay();
}

void ReadReceiptsWidget::setupUi()
{
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 2, 4, 2);
    mainLayout->setSpacing(4);

    m_statusIcon = new QLabel(this);
    m_statusIcon->setFixedSize(14, 14);
    m_statusIcon->setAlignment(Qt::AlignCenter);

    m_statusText = new QLabel(this);
    m_statusText->setCursor(Qt::PointingHandCursor);
    m_statusText->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    m_statusText->installEventFilter(this);
    connect(m_statusText, &QLabel::linkActivated, this, [this]() { onStatusTextClicked(); });

    m_avatarsRow = new QWidget(this);
    m_avatarsLayout = new QHBoxLayout(m_avatarsRow);
    m_avatarsLayout->setContentsMargins(0, 0, 0, 0);
    m_avatarsLayout->setSpacing(kAvatarSpacing);

    m_moreLabel = new QLabel(this);
    m_moreLabel->setCursor(Qt::PointingHandCursor);
    m_moreLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    m_moreLabel->installEventFilter(this);
    connect(m_moreLabel, &QLabel::linkActivated, this, [this]() { onMoreClicked(); });

    mainLayout->addWidget(m_statusIcon);
    mainLayout->addWidget(m_statusText);
    mainLayout->addWidget(m_avatarsRow);
    mainLayout->addWidget(m_moreLabel);
    mainLayout->addStretch();

    m_fullListFrame = new QFrame(this, Qt::Popup);
    m_fullListFrame->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    m_fullListFrame->setAttribute(Qt::WA_TranslucentBackground);
    m_fullListFrame->setVisible(false);

    auto* fullListLayout = new QVBoxLayout(m_fullListFrame);
    fullListLayout->setContentsMargins(8, 8, 8, 8);
    fullListLayout->setSpacing(4);
}

void ReadReceiptsWidget::applyStyle()
{
    setStyleSheet(QStringLiteral(R"(
        ReadReceiptsWidget {
            background-color: transparent;
        }
    )"));

    m_statusText->setStyleSheet(QStringLiteral(R"(
        QLabel {
            color: #888888;
            font-size: 11px;
            background-color: transparent;
        }
        QLabel:hover {
            color: #bbbbbb;
        }
    )"));

    m_moreLabel->setStyleSheet(QStringLiteral(R"(
        QLabel {
            color: #888888;
            font-size: 11px;
            background-color: transparent;
            padding-left: 2px;
        }
        QLabel:hover {
            color: #bbbbbb;
        }
    )"));

    m_fullListFrame->setStyleSheet(QStringLiteral(R"(
        QFrame {
            background-color: #2d2d2d;
            border: 1px solid #555555;
            border-radius: 8px;
        }
    )"));
}

QPixmap ReadReceiptsWidget::generateAvatar(const QString& displayName, int size)
{
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor fillColor = colorForName(displayName);
    QPainterPath path;
    path.addEllipse(0, 0, size, size);
    painter.setClipPath(path);
    painter.fillPath(path, fillColor);

    QString initial;
    if (!displayName.isEmpty()) {
        initial = displayName.at(0).toUpper();
    }

    QFont font;
    font.setPixelSize(size / 2);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(QRect(0, 0, size, size), Qt::AlignCenter, initial);

    painter.end();
    return pixmap;
}

QColor ReadReceiptsWidget::colorForName(const QString& name) const
{
    static const QColor palette[] = {
        QColor(0xE5, 0x3E, 0x3E),
        QColor(0xDD, 0x6B, 0x20),
        QColor(0xD6, 0x9E, 0x2E),
        QColor(0x31, 0x8F, 0x4A),
        QColor(0x31, 0x81, 0xCE),
        QColor(0x80, 0x51, 0xAD),
        QColor(0xD5, 0x3F, 0x8C),
        QColor(0x00, 0xA3, 0xC4),
    };

    if (name.isEmpty())
        return palette[0];

    uint hash = 0;
    for (const QChar& c : name) {
        hash = hash * 31 + c.unicode();
    }
    return palette[hash % 8];
}

QString ReadReceiptsWidget::formatTimestamp(qint64 timestamp) const
{
    if (timestamp <= 0)
        return {};

    QDateTime dt = QDateTime::fromMSecsSinceEpoch(timestamp);
    QDate today = QDate::currentDate();

    if (dt.date() == today) {
        return dt.toString(QStringLiteral("HH:mm"));
    } else if (dt.date().daysTo(today) < 7) {
        return dt.toString(QStringLiteral("ddd HH:mm"));
    } else {
        return dt.toString(QStringLiteral("MMM d, HH:mm"));
    }
}

void ReadReceiptsWidget::setDirectMessage(bool isDm)
{
    m_isDm = isDm;
    updateDisplay();
}

void ReadReceiptsWidget::setMessageState(MessageState state)
{
    m_messageState = state;
    updateDisplay();
}

void ReadReceiptsWidget::addReadReceipt(const QString& userId, const QString& displayName, qint64 timestamp)
{
    for (auto& entry : m_receipts) {
        if (entry.userId == userId) {
            entry.displayName = displayName;
            entry.timestamp = timestamp;
            updateDisplay();
            return;
        }
    }

    m_receipts.append({userId, displayName, timestamp});
    updateDisplay();
}

void ReadReceiptsWidget::clearReceipts()
{
    m_receipts.clear();
    m_fullListFrame->setVisible(false);
    m_fullListVisible = false;
    updateDisplay();
}

void ReadReceiptsWidget::updateDisplay()
{
    rebuildAvatars();

    if (m_isDm) {
        m_avatarsRow->setVisible(false);
        m_moreLabel->setVisible(false);

        switch (m_messageState) {
        case MessageState::Sent:
            m_statusIcon->setText(QStringLiteral("\xE2\x9C\x93"));
            m_statusText->setText(QStringLiteral("Sent"));
            break;
        case MessageState::Delivered:
            m_statusIcon->setText(QStringLiteral("\xE2\x9C\x93\xE2\x9C\x93"));
            m_statusText->setText(QStringLiteral("Delivered"));
            break;
        case MessageState::Read: {
            m_statusIcon->clear();
            if (!m_receipts.isEmpty()) {
                const auto& last = m_receipts.last();
                m_statusText->setText(QStringLiteral("Read %1").arg(formatTimestamp(last.timestamp)));
                m_avatarsRow->setVisible(true);
            } else {
                m_statusText->setText(QStringLiteral("Read"));
            }
            break;
        }
        }
    } else {
        m_statusIcon->clear();

        int count = m_receipts.size();
        if (count == 0) {
            m_statusText->setText({});
            m_avatarsRow->setVisible(false);
            m_moreLabel->setVisible(false);
        } else {
            m_statusText->setText(QStringLiteral("Read by %1").arg(count));
            m_avatarsRow->setVisible(true);

            if (count > kMaxVisibleAvatars) {
                m_moreLabel->setVisible(true);
                m_moreLabel->setText(QStringLiteral("+%1 more").arg(count - kMaxVisibleAvatars));
            } else {
                m_moreLabel->setVisible(false);
            }
        }
    }
}

void ReadReceiptsWidget::rebuildAvatars()
{
    QLayoutItem* item;
    while ((item = m_avatarsLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    int visibleCount = qMin(m_receipts.size(), kMaxVisibleAvatars);
    for (int i = 0; i < visibleCount; ++i) {
        const auto& entry = m_receipts[i];

        auto* avatar = new QLabel(m_avatarsRow);
        avatar->setFixedSize(kAvatarSize, kAvatarSize);
        avatar->setPixmap(generateAvatar(entry.displayName, kAvatarSize));
        avatar->setToolTip(QStringLiteral("%1\nRead: %2")
            .arg(entry.displayName)
            .arg(formatTimestamp(entry.timestamp)));
        avatar->setStyleSheet(QStringLiteral("QLabel { background-color: transparent; }"));

        m_avatarsLayout->addWidget(avatar);
    }
}

void ReadReceiptsWidget::onMoreClicked()
{
    showFullListPopup();
}

void ReadReceiptsWidget::onStatusTextClicked()
{
    if (!m_isDm && !m_receipts.isEmpty()) {
        showFullListPopup();
    }
}

void ReadReceiptsWidget::showFullListPopup()
{
    if (m_receipts.isEmpty())
        return;

    while (auto* layout = m_fullListFrame->layout()) {
        QLayoutItem* item;
        while ((item = layout->takeAt(0)) != nullptr) {
            if (item->widget()) {
                item->widget()->deleteLater();
            }
            delete item;
        }
        break;
    }

    auto* listLayout = qobject_cast<QVBoxLayout*>(m_fullListFrame->layout());
    if (!listLayout)
        return;

    for (const auto& entry : m_receipts) {
        auto* row = new QWidget(m_fullListFrame);
        auto* rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(4, 3, 4, 3);
        rowLayout->setSpacing(8);

        auto* avatar = new QLabel(row);
        avatar->setFixedSize(24, 24);
        avatar->setPixmap(generateAvatar(entry.displayName, 24));

        auto* nameLabel = new QLabel(entry.displayName, row);
        nameLabel->setStyleSheet(QStringLiteral("color: #e0e0e0; font-size: 13px; font-weight: bold; background: transparent;"));

        auto* timeLabel = new QLabel(formatTimestamp(entry.timestamp), row);
        timeLabel->setStyleSheet(QStringLiteral("color: #999999; font-size: 11px; background: transparent;"));

        rowLayout->addWidget(avatar);
        rowLayout->addWidget(nameLabel, 1);
        rowLayout->addWidget(timeLabel);

        listLayout->addWidget(row);
    }

    m_fullListFrame->adjustSize();

    QPoint globalPos = mapToGlobal(QPoint(0, height() + 4));
    m_fullListFrame->move(globalPos);
    m_fullListFrame->setVisible(true);
    m_fullListFrame->raise();
}

bool ReadReceiptsWidget::event(QEvent* ev)
{
    if (ev->type() == QEvent::MouseButtonPress && m_fullListVisible) {
        if (m_fullListFrame && m_fullListFrame->isVisible()) {
            QPoint globalClick = QCursor::pos();
            QRect popupRect = m_fullListFrame->geometry();
            QRect widgetRect = QRect(mapToGlobal(QPoint(0, 0)), size());
            if (!popupRect.contains(globalClick) && !widgetRect.contains(globalClick)) {
                m_fullListFrame->setVisible(false);
                m_fullListVisible = false;
            }
        }
    }
    return QWidget::event(ev);
}
