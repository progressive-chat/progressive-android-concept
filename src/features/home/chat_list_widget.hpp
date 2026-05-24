#pragma once

#include <QWidget>
#include <QVector>
#include "protocol/protocol_room.hpp"
#include "protocol/protocol_message.hpp"
#include "protocol/protocol_type.hpp"

class QListWidget;
class QListWidgetItem;
class QLineEdit;
class QComboBox;
class QPushButton;
class QLabel;

class ChatListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatListWidget(QWidget *parent = nullptr);
    ~ChatListWidget() override;

signals:
    void roomSelected(ProtocolRoom room);

public slots:
    void refreshRoomList();
    void onMessageReceived(ProtocolMessage message);

private slots:
    void onFilterChanged();
    void onItemActivated(QListWidgetItem *item);

private:
    void setupUi();
    void applyFilter();
    void populateItem(const ProtocolRoom &room, QListWidgetItem *item);

    static QColor protocolColor(ProtocolType type);
    static QString protocolName(ProtocolType type);
    static QString formatTimestamp(qint64 timestampMs);
    static QPixmap makeAvatarPixmap(const QString &name, int size, const QColor &bgColor);
    static QPixmap makeProtocolDot(int size, const QColor &color);

    enum ItemDataRole {
        RoomIndexRole = Qt::UserRole + 1
    };

    QLineEdit *m_searchEdit;
    QComboBox *m_protocolFilter;
    QPushButton *m_refreshButton;
    QListWidget *m_roomList;
    QVector<ProtocolRoom> m_rooms;
};
