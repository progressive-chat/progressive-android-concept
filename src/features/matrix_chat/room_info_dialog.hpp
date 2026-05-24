#pragma once

#include <QDialog>

#include "protocol/protocol_room.hpp"
#include "protocol/protocol_type.hpp"

class QTabWidget;
class QLabel;
class QPushButton;
class QCheckBox;
class QListWidget;
class QScrollArea;
class QFrame;

class RoomInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RoomInfoDialog(const ProtocolRoom &room, QWidget *parent = nullptr);
    ~RoomInfoDialog() override = default;

signals:
    void notificationsToggled(const QString &roomId, bool muted);
    void leaveRoomRequested(const QString &roomId);
    void openMemberListRequested(const QString &roomId);
    void roomSettingsRequested(const QString &roomId);

private slots:
    void onLeaveRoomClicked();
    void onShareRoomLinkClicked();
    void onViewMembersClicked();
    void onRoomSettingsClicked();

private:
    void setupUi();
    QWidget *createOverviewTab();
    QWidget *createMatrixTab();
    QWidget *createIrcTab();
    QWidget *createLemmyTab();
    void loadRoomData();

    ProtocolRoom m_room;

    QTabWidget *m_tabWidget;

    QLabel *m_roomNameLabel;
    QLabel *m_topicLabel;
    QLabel *m_roomIdLabel;
    QLabel *m_roomAliasLabel;
    QLabel *m_typeBadgeLabel;
    QLabel *m_encryptionLabel;
    QLabel *m_memberCountLabel;
    QPushButton *m_viewMembersButton;
    QCheckBox *m_notificationsToggle;
    QPushButton *m_leaveRoomButton;
    QPushButton *m_shareLinkButton;
    QPushButton *m_roomSettingsButton;

    QLabel *m_matrixVersionLabel;
    QLabel *m_matrixJoinRulesLabel;
    QLabel *m_matrixGuestAccessLabel;
    QLabel *m_matrixEncryptionAlgoLabel;
    QLabel *m_matrixCanonicalAliasLabel;

    QLabel *m_ircChannelModesLabel;
    QListWidget *m_ircUserList;

    QLabel *m_lemmySubscriberCountLabel;
    QListWidget *m_lemmyModeratorList;
    QLabel *m_lemmyRulesLabel;
};
