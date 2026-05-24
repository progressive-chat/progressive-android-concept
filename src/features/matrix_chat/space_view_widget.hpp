#pragma once

#include <QWidget>
#include <QString>
#include <QVector>
#include <QMap>

class QLabel;
class QPushButton;
class QListWidget;
class QListWidgetItem;
class QLineEdit;
class QStackedWidget;
class QProgressBar;
class QScrollArea;
class QFrame;
class QGridLayout;
class QVBoxLayout;

struct SpaceRoomInfo
{
    QString roomId;
    QString name;
    QString topic;
    QString avatarUrl;
    int memberCount = 0;
    bool suggested = false;
    QString parentSpaceId;
};

struct SpaceMemberInfo
{
    QString userId;
    QString displayName;
    QString avatarUrl;
    QString powerLevel;
};

class SpaceViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SpaceViewWidget(const QString &spaceId, QWidget *parent = nullptr);
    ~SpaceViewWidget() override;

    void setSpaceName(const QString &name);
    void setSpaceTopic(const QString &topic);
    void setSpaceAvatarUrl(const QString &url);
    void setJoined(bool joined);
    void setLoading(bool loading);
    void loadChildRooms();
    void loadMembers();

signals:
    void roomSelected(const QString &roomId);
    void backRequested();
    void joinSpaceRequested(const QString &spaceId);
    void leaveSpaceRequested(const QString &spaceId);
    void inviteRequested(const QString &spaceId);

private slots:
    void onRoomItemClicked(QListWidgetItem *item);
    void onBackClicked();
    void onJoinLeaveClicked();
    void onInviteClicked();
    void onMemberSearchChanged(const QString &text);

private:
    void setupUi();
    QWidget *createHeaderSection();
    QWidget *createRoomsSection();
    QWidget *createMembersSection();
    QWidget *createMemberCard(const SpaceMemberInfo &member);
    void refreshRoomList();
    void refreshMemberGrid(const QString &filter = {});
    void setSectionLoading(QStackedWidget *stack, bool loading);

    QString m_spaceId;
    QString m_spaceName;
    QString m_spaceTopic;
    QString m_spaceAvatarUrl;
    bool m_isJoined = false;
    bool m_isLoading = false;

    QVector<SpaceRoomInfo> m_suggestedRooms;
    QVector<SpaceRoomInfo> m_allRooms;
    QVector<SpaceMemberInfo> m_members;

    // Header
    QPushButton *m_backButton;
    QLabel *m_spaceAvatarLabel;
    QLabel *m_spaceNameLabel;
    QLabel *m_spaceTopicLabel;

    // Action buttons
    QPushButton *m_joinLeaveButton;
    QPushButton *m_inviteButton;

    // Loading
    QProgressBar *m_loadingBar;

    // Rooms section
    QStackedWidget *m_roomsStack;
    QListWidget *m_suggestedList;
    QListWidget *m_allRoomsList;
    QLabel *m_suggestedHeader;
    QLabel *m_allRoomsHeader;

    // Members section
    QStackedWidget *m_membersStack;
    QLineEdit *m_memberSearchEdit;
    QScrollArea *m_memberScrollArea;
    QWidget *m_memberGridContainer;
    QGridLayout *m_memberGridLayout;
    QLabel *m_memberCountLabel;
};
