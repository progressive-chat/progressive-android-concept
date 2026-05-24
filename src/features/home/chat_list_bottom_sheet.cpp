#include "features/home/chat_list_bottom_sheet.hpp"

#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QDateTime>
#include <QFont>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>

// ---------------------------------------------------------------------------
// Compact dark stylesheet
// ---------------------------------------------------------------------------
static const char *kSheetStyle = R"(
    ChatListBottomSheet {
        background-color: #1e1e2e;
        border: 1px solid #45475a;
        border-radius: 12px;
    }
    ChatListBottomSheet QListWidget {
        background-color: transparent;
        color: #cdd6f4;
        border: none;
        outline: none;
        font-size: 12px;
    }
    ChatListBottomSheet QListWidget::item {
        background-color: transparent;
        border-bottom: 1px solid #313244;
        padding: 4px 8px;
    }
    ChatListBottomSheet QListWidget::item:hover {
        background-color: #2a2a3c;
    }
    ChatListBottomSheet QListWidget::item:selected {
        background-color: #45475a;
    }
    ChatListBottomSheet QPushButton {
        background-color: transparent;
        color: #6c7086;
        border: none;
        border-radius: 6px;
        font-size: 16px;
        font-weight: bold;
        padding: 2px 8px;
    }
    ChatListBottomSheet QPushButton:hover {
        background-color: #45475a;
        color: #cdd6f4;
    }
    ChatListBottomSheet QLabel {
        background-color: transparent;
        color: #cdd6f4;
        font-size: 13px;
        font-weight: bold;
    }
)";

// ---------------------------------------------------------------------------
ChatListBottomSheet::ChatListBottomSheet(QWidget *parent)
    : QWidget(parent)
    , m_roomList(nullptr)
    , m_closeButton(nullptr)
    , m_titleLabel(nullptr)
    , m_animation(nullptr)
{
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, false);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setVisible(false);

    setStyleSheet(QString::fromLatin1(kSheetStyle));
    setupUi();

    m_animation = new QPropertyAnimation(this, "geometry", this);
    m_animation->setDuration(200);
    m_animation->setEasingCurve(QEasingCurve::OutCubic);
}

ChatListBottomSheet::~ChatListBottomSheet() = default;

// ---------------------------------------------------------------------------
// UI construction
// ---------------------------------------------------------------------------
void ChatListBottomSheet::setupUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // --- shadow ---
    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(24);
    shadow->setColor(QColor(0, 0, 0, 160));
    shadow->setOffset(0, -4);
    setGraphicsEffect(shadow);

    // --- header bar ---
    auto *header = new QHBoxLayout();
    header->setContentsMargins(12, 6, 6, 4);

    m_titleLabel = new QLabel(tr("Chats"), this);
    header->addWidget(m_titleLabel);
    header->addStretch();

    m_closeButton = new QPushButton(QStringLiteral("\u2715"), this);
    m_closeButton->setFixedSize(24, 24);
    m_closeButton->setCursor(Qt::PointingHandCursor);
    connect(m_closeButton, &QPushButton::clicked, this, &ChatListBottomSheet::dismiss);
    header->addWidget(m_closeButton);

    root->addLayout(header);

    // --- list ---
    m_roomList = new QListWidget(this);
    m_roomList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_roomList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_roomList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_roomList->setIconSize(QSize(kAvatarSize, kAvatarSize));
    m_roomList->setUniformItemSizes(true);
    m_roomList->setFocusPolicy(Qt::StrongFocus);

    connect(m_roomList, &QListWidget::itemActivated,
            this, &ChatListBottomSheet::onItemActivated);
    connect(m_roomList, &QListWidget::itemClicked,
            this, &ChatListBottomSheet::onItemActivated);

    root->addWidget(m_roomList, 1);
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------
void ChatListBottomSheet::setRooms(const QVector<ProtocolRoom> &rooms)
{
    m_rooms = rooms;

    std::sort(m_rooms.begin(), m_rooms.end(),
              [](const ProtocolRoom &a, const ProtocolRoom &b) {
                  return a.lastMessageTimestamp > b.lastMessageTimestamp;
              });

    m_roomList->clear();
    for (int i = 0; i < m_rooms.size(); ++i) {
        const auto &room = m_rooms[i];

        auto *item = new QListWidgetItem();
        item->setData(static_cast<int>(RoomIndexRole), i);
        item->setSizeHint(QSize(0, kItemHeight));

        QColor bg = protocolColor(room.protocolType);
        item->setIcon(QIcon(makeAvatarPixmap(room.name, bg)));

        QString label = room.name;
        if (!room.lastMessagePreview.isEmpty()) {
            QString preview = room.lastMessagePreview;
            if (preview.length() > 40)
                preview = preview.left(37) + QStringLiteral("...");
            label += QStringLiteral("  \u2014  ") + preview;
        }

        // Append timestamp if available
        if (room.lastMessageTimestamp > 0)
            label += QStringLiteral("  ") + formatTimestamp(room.lastMessageTimestamp);

        item->setText(label);
        item->setToolTip(room.topic);

        m_roomList->addItem(item);
    }
}

void ChatListBottomSheet::showPopup(QWidget *anchor)
{
    if (!anchor)
        return;

    const int w = qMin(kMaxWidth, anchor->width() - 16);
    const int itemCount = m_roomList->count();
    const int contentH = itemCount * kItemHeight;
    const int h = qMin(contentH + 40, kMaxHeight);

    const QPoint anchorBottom = anchor->mapToGlobal(QPoint(0, anchor->height()));
    const int x = anchorBottom.x() + (anchor->width() - w) / 2;
    const int y = anchorBottom.y() - h - 8;

    // Starting geometry (offscreen below)
    const QRect startGeom(x, anchorBottom.y(), w, h);
    const QRect endGeom(x, y, w, h);

    m_animation->setStartValue(startGeom);
    m_animation->setEndValue(endGeom);
    m_animation->setDuration(250);

    setGeometry(startGeom);
    show();
    raise();
    setFocus();

    m_animation->start();

    if (anchor->isVisible()) {
        installEventFilter(this);
        QApplication::instance()->installEventFilter(this);
    }
}

void ChatListBottomSheet::dismiss()
{
    animateOut();
}

bool ChatListBottomSheet::isVisiblePopup() const
{
    return isVisible();
}

// ---------------------------------------------------------------------------
// Slots
// ---------------------------------------------------------------------------
void ChatListBottomSheet::onItemActivated(QListWidgetItem *item)
{
    if (!item)
        return;

    bool ok = false;
    int idx = item->data(static_cast<int>(RoomIndexRole)).toInt(&ok);
    if (ok && idx >= 0 && idx < m_rooms.size()) {
        emit roomSelected(m_rooms[idx]);
        dismiss();
    }
}

// ---------------------------------------------------------------------------
// Animation helpers
// ---------------------------------------------------------------------------
void ChatListBottomSheet::animateIn()
{
    m_animation->setDirection(QAbstractAnimation::Forward);
    if (m_animation->state() == QAbstractAnimation::Stopped)
        m_animation->start();
}

void ChatListBottomSheet::animateOut()
{
    m_animation->setDirection(QAbstractAnimation::Backward);
    if (m_animation->state() == QAbstractAnimation::Stopped) {
        connect(m_animation, &QPropertyAnimation::finished, this, [this]() {
            hide();
            emit closed();
        });
        m_animation->start();
    } else {
        // Reverse a running animation
        m_animation->pause();
        m_animation->setDirection(QAbstractAnimation::Backward);
        m_animation->resume();
        connect(m_animation, &QPropertyAnimation::finished, this, [this]() {
            hide();
            emit closed();
        });
    }
}

// ---------------------------------------------------------------------------
// Event filter – dismiss when clicking outside the sheet
// ---------------------------------------------------------------------------
bool ChatListBottomSheet::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        auto *me = static_cast<QMouseEvent *>(event);
        if (!geometry().contains(me->globalPosition().toPoint())) {
            dismiss();
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

// ---------------------------------------------------------------------------
// Static helpers
// ---------------------------------------------------------------------------
QColor ChatListBottomSheet::protocolColor(ProtocolType type)
{
    switch (type) {
    case ProtocolType::MATRIX:  return QColor(0x89, 0xb4, 0xfa);
    case ProtocolType::IRC:     return QColor(0xa6, 0xe3, 0xa1);
    case ProtocolType::LEMMY:   return QColor(0xfa, 0xb3, 0x87);
    default:                    return QColor(0x6c, 0x70, 0x86);
    }
}

QString ChatListBottomSheet::formatTimestamp(qint64 timestampMs)
{
    if (timestampMs <= 0)
        return {};

    QDateTime dt = QDateTime::fromMSecsSinceEpoch(timestampMs);
    QDateTime now = QDateTime::currentDateTime();

    if (dt.date() == now.date())
        return dt.toString(QStringLiteral("HH:mm"));

    if (dt.date().year() == now.date().year())
        return dt.toString(QStringLiteral("MMM d"));

    return dt.toString(QStringLiteral("yy/MM/dd"));
}

QPixmap ChatListBottomSheet::makeAvatarPixmap(const QString &name, const QColor &bgColor)
{
    const int size = kAvatarSize;
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);

    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect circle(0, 0, size, size);
    painter.setPen(Qt::NoPen);
    painter.setBrush(bgColor);
    painter.drawEllipse(circle);

    QString letter;
    if (!name.isEmpty()) {
        QChar first = name.at(0);
        if (name.length() > 1) {
            int spaceIdx = name.indexOf(QChar::Space);
            if (spaceIdx != -1 && spaceIdx + 1 < name.length())
                letter = name.at(0).toUpper() + QString(name.at(spaceIdx + 1).toUpper());
            else
                letter = QString(first.toUpper());
        } else {
            letter = QString(first.toUpper());
        }
    } else {
        letter = QStringLiteral("?");
    }

    if (letter.length() > 2)
        letter = letter.left(2);

    QFont font;
    font.setPixelSize(size / 2);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(circle, Qt::AlignCenter, letter);

    painter.end();
    return pix;
}
