#pragma once

#include <QWidget>
#include <QVector>
#include "protocol/protocol_room.hpp"
#include "protocol/protocol_message.hpp"
#include "protocol/protocol_type.hpp"

class QListWidget;
class QListWidgetItem;
class QPushButton;
class QLabel;
class QPropertyAnimation;

class ChatListBottomSheet : public QWidget
{
    Q_OBJECT

public:
    explicit ChatListBottomSheet(QWidget *parent = nullptr);
    ~ChatListBottomSheet() override;

    void setRooms(const QVector<ProtocolRoom> &rooms);

    void showPopup(QWidget *anchor);
    void dismiss();

    bool isVisiblePopup() const;

signals:
    void roomSelected(ProtocolRoom room);
    void closed();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onItemActivated(QListWidgetItem *item);

private:
    void setupUi();
    void animateIn();
    void animateOut();

    static QString formatTimestamp(qint64 timestampMs);
    static QColor protocolColor(ProtocolType type);
    static QPixmap makeAvatarPixmap(const QString &name, const QColor &bgColor);

    enum ItemDataRole {
        RoomIndexRole = Qt::UserRole + 1
    };

    QListWidget *m_roomList;
    QPushButton *m_closeButton;
    QLabel *m_titleLabel;
    QPropertyAnimation *m_animation;
    QVector<ProtocolRoom> m_rooms;

    static constexpr int kItemHeight = 40;
    static constexpr int kAvatarSize = 28;
    static constexpr int kMaxWidth = 360;
    static constexpr int kMaxHeight = 480;
};
