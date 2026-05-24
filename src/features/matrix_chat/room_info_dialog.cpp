#include "features/matrix_chat/room_info_dialog.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QTabWidget>
#include <QToolTip>
#include <QVBoxLayout>

#include "protocol/protocol_type.hpp"

namespace {

QFrame *createSeparator(QWidget *parent)
{
    auto *line = new QFrame(parent);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    return line;
}

QString roomTypeLabel(const ProtocolRoom &room) {
    if (room.isDirect)
        return RoomInfoDialog::tr("Direct Message");
    if (room.isPublic)
        return RoomInfoDialog::tr("Public");
    return RoomInfoDialog::tr("Private");
}

QString roomTypeBadgeStyle(const ProtocolRoom &room) {
    if (room.isDirect)
        return QStringLiteral("background-color: #9b59b6; color: white;");
    if (room.isPublic)
        return QStringLiteral("background-color: #27ae60; color: white;");
    return QStringLiteral("background-color: #7f8c8d; color: white;");
}

} // anonymous namespace

RoomInfoDialog::RoomInfoDialog(const ProtocolRoom &room, QWidget *parent)
    : QDialog(parent)
    , m_room(room)
    , m_tabWidget(nullptr)
    , m_roomNameLabel(nullptr)
    , m_topicLabel(nullptr)
    , m_roomIdLabel(nullptr)
    , m_roomAliasLabel(nullptr)
    , m_typeBadgeLabel(nullptr)
    , m_encryptionLabel(nullptr)
    , m_memberCountLabel(nullptr)
    , m_viewMembersButton(nullptr)
    , m_notificationsToggle(nullptr)
    , m_leaveRoomButton(nullptr)
    , m_shareLinkButton(nullptr)
    , m_roomSettingsButton(nullptr)
    , m_matrixVersionLabel(nullptr)
    , m_matrixJoinRulesLabel(nullptr)
    , m_matrixGuestAccessLabel(nullptr)
    , m_matrixEncryptionAlgoLabel(nullptr)
    , m_matrixCanonicalAliasLabel(nullptr)
    , m_ircChannelModesLabel(nullptr)
    , m_ircUserList(nullptr)
    , m_lemmySubscriberCountLabel(nullptr)
    , m_lemmyModeratorList(nullptr)
    , m_lemmyRulesLabel(nullptr)
{
    setupUi();
    loadRoomData();
}

void RoomInfoDialog::setupUi()
{
    setWindowTitle(tr("Room Info \u2014 %1").arg(m_room.name));
    setMinimumSize(500, 560);
    setModal(true);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 8);
    mainLayout->setSpacing(0);

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->addTab(createOverviewTab(), tr("Overview"));

    if (m_room.protocolType == ProtocolType::MATRIX)
        m_tabWidget->addTab(createMatrixTab(), tr("Matrix"));
    else if (m_room.protocolType == ProtocolType::IRC)
        m_tabWidget->addTab(createIrcTab(), tr("IRC"));
    else if (m_room.protocolType == ProtocolType::LEMMY)
        m_tabWidget->addTab(createLemmyTab(), tr("Lemmy"));

    mainLayout->addWidget(m_tabWidget, 1);

    auto *bottomBar = new QHBoxLayout;
    bottomBar->setContentsMargins(12, 8, 12, 8);

    m_roomSettingsButton = new QPushButton(tr("Room Settings"), this);
    bottomBar->addWidget(m_roomSettingsButton);

    bottomBar->addStretch();

    auto *closeButton = new QPushButton(tr("Close"), this);
    bottomBar->addWidget(closeButton);

    mainLayout->addLayout(bottomBar);

    connect(m_roomSettingsButton, &QPushButton::clicked,
            this, &RoomInfoDialog::onRoomSettingsClicked);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
}

QWidget *RoomInfoDialog::createOverviewTab()
{
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *page = new QWidget(scrollArea);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    // --- Avatar ---
    auto *avatarFrame = new QFrame(page);
    avatarFrame->setFixedSize(96, 96);
    avatarFrame->setStyleSheet(
        QStringLiteral("background-color: #3498db; border-radius: 48px; color: white;"));
    auto *avatarLayout = new QVBoxLayout(avatarFrame);
    avatarLayout->setAlignment(Qt::AlignCenter);
    auto *avatarChar = new QLabel(m_room.name.isEmpty()
                                      ? QStringLiteral("?")
                                      : m_room.name.left(1).toUpper(),
                                  avatarFrame);
    avatarChar->setStyleSheet(
        QStringLiteral("font-size: 40px; font-weight: bold; color: white; background: transparent;"));
    avatarChar->setAlignment(Qt::AlignCenter);
    avatarLayout->addWidget(avatarChar);

    auto *avatarRow = new QHBoxLayout;
    avatarRow->addStretch();
    avatarRow->addWidget(avatarFrame);
    avatarRow->addStretch();
    layout->addLayout(avatarRow);

    // --- Room Name ---
    m_roomNameLabel = new QLabel(page);
    m_roomNameLabel->setAlignment(Qt::AlignCenter);
    QFont nameFont = m_roomNameLabel->font();
    nameFont.setPointSize(18);
    nameFont.setBold(true);
    m_roomNameLabel->setFont(nameFont);
    m_roomNameLabel->setWordWrap(true);
    layout->addWidget(m_roomNameLabel);

    // --- Type Badge ---
    auto *badgeRow = new QHBoxLayout;
    badgeRow->addStretch();
    m_typeBadgeLabel = new QLabel(page);
    m_typeBadgeLabel->setContentsMargins(12, 4, 12, 4);
    m_typeBadgeLabel->setStyleSheet(
        QStringLiteral("border-radius: 10px; padding: 2px 10px; font-size: 12px; font-weight: bold;"));
    badgeRow->addWidget(m_typeBadgeLabel);
    badgeRow->addStretch();
    layout->addLayout(badgeRow);

    layout->addWidget(createSeparator(page));

    // --- Topic ---
    auto *topicHeader = new QLabel(tr("<b>Topic</b>"), page);
    layout->addWidget(topicHeader);
    m_topicLabel = new QLabel(page);
    m_topicLabel->setWordWrap(true);
    m_topicLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(m_topicLabel);

    layout->addWidget(createSeparator(page));

    // --- Room ID ---
    auto *idHeader = new QLabel(tr("<b>Room ID</b>"), page);
    layout->addWidget(idHeader);
    m_roomIdLabel = new QLabel(page);
    m_roomIdLabel->setWordWrap(true);
    m_roomIdLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_roomIdLabel->setStyleSheet(QStringLiteral("color: #555; font-family: monospace;"));
    layout->addWidget(m_roomIdLabel);

    // --- Room Alias ---
    auto *aliasHeader = new QLabel(tr("<b>Alias</b>"), page);
    layout->addWidget(aliasHeader);
    m_roomAliasLabel = new QLabel(page);
    m_roomAliasLabel->setWordWrap(true);
    m_roomAliasLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_roomAliasLabel->setStyleSheet(QStringLiteral("color: #555; font-family: monospace;"));
    layout->addWidget(m_roomAliasLabel);

    layout->addWidget(createSeparator(page));

    // --- Encryption Status ---
    m_encryptionLabel = new QLabel(page);
    m_encryptionLabel->setWordWrap(true);
    layout->addWidget(m_encryptionLabel);

    layout->addWidget(createSeparator(page));

    // --- Member Count + View Members ---
    auto *memberRow = new QHBoxLayout;
    m_memberCountLabel = new QLabel(page);
    memberRow->addWidget(m_memberCountLabel);
    memberRow->addStretch();
    m_viewMembersButton = new QPushButton(tr("View Members"), page);
    memberRow->addWidget(m_viewMembersButton);
    layout->addLayout(memberRow);

    layout->addWidget(createSeparator(page));

    // --- Notifications Toggle ---
    m_notificationsToggle = new QCheckBox(tr("Enable Notifications"), page);
    m_notificationsToggle->setChecked(true);
    layout->addWidget(m_notificationsToggle);

    layout->addWidget(createSeparator(page));

    // --- Action Buttons ---
    auto *actionLayout = new QVBoxLayout;
    actionLayout->setSpacing(8);

    m_shareLinkButton = new QPushButton(tr("Share Room Link"), page);
    actionLayout->addWidget(m_shareLinkButton);

    m_leaveRoomButton = new QPushButton(tr("Leave Room"), page);
    m_leaveRoomButton->setStyleSheet(
        QStringLiteral("QPushButton { background-color: #e74c3c; color: white; border: none; "
                       "padding: 8px; border-radius: 4px; font-weight: bold; } "
                       "QPushButton:hover { background-color: #c0392b; } "
                       "QPushButton:pressed { background-color: #a93226; }"));
    actionLayout->addWidget(m_leaveRoomButton);

    layout->addLayout(actionLayout);
    layout->addStretch();

    // --- Connections ---
    connect(m_viewMembersButton, &QPushButton::clicked,
            this, &RoomInfoDialog::onViewMembersClicked);
    connect(m_notificationsToggle, &QCheckBox::toggled,
            this, [this](bool checked) {
                emit notificationsToggled(m_room.id, !checked);
            });
    connect(m_shareLinkButton, &QPushButton::clicked,
            this, &RoomInfoDialog::onShareRoomLinkClicked);
    connect(m_leaveRoomButton, &QPushButton::clicked,
            this, &RoomInfoDialog::onLeaveRoomClicked);

    scrollArea->setWidget(page);
    return scrollArea;
}

QWidget *RoomInfoDialog::createMatrixTab()
{
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *page = new QWidget(scrollArea);
    auto *layout = new QFormLayout(page);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(10);

    m_matrixVersionLabel = new QLabel(page);
    m_matrixVersionLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addRow(tr("Room Version:"), m_matrixVersionLabel);

    m_matrixJoinRulesLabel = new QLabel(page);
    m_matrixJoinRulesLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addRow(tr("Join Rules:"), m_matrixJoinRulesLabel);

    m_matrixGuestAccessLabel = new QLabel(page);
    m_matrixGuestAccessLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addRow(tr("Guest Access:"), m_matrixGuestAccessLabel);

    auto *encGroup = new QGroupBox(tr("Encryption"), page);
    auto *encLayout = new QVBoxLayout(encGroup);
    m_matrixEncryptionAlgoLabel = new QLabel(encGroup);
    m_matrixEncryptionAlgoLabel->setWordWrap(true);
    m_matrixEncryptionAlgoLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    encLayout->addWidget(m_matrixEncryptionAlgoLabel);
    layout->addRow(encGroup);

    m_matrixCanonicalAliasLabel = new QLabel(page);
    m_matrixCanonicalAliasLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_matrixCanonicalAliasLabel->setStyleSheet(QStringLiteral("font-family: monospace;"));
    layout->addRow(tr("Canonical Alias:"), m_matrixCanonicalAliasLabel);

    layout->addItem(new QSpacerItem(0, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));

    scrollArea->setWidget(page);
    return scrollArea;
}

QWidget *RoomInfoDialog::createIrcTab()
{
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *page = new QWidget(scrollArea);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(10);

    m_ircChannelModesLabel = new QLabel(page);
    m_ircChannelModesLabel->setWordWrap(true);
    m_ircChannelModesLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_ircChannelModesLabel->setStyleSheet(QStringLiteral("font-family: monospace; font-size: 14px;"));
    layout->addWidget(m_ircChannelModesLabel);

    layout->addWidget(createSeparator(page));

    auto *userListHeader = new QLabel(tr("<b>User List</b>"), page);
    layout->addWidget(userListHeader);

    m_ircUserList = new QListWidget(page);
    m_ircUserList->setMaximumHeight(200);
    layout->addWidget(m_ircUserList);

    layout->addStretch();

    scrollArea->setWidget(page);
    return scrollArea;
}

QWidget *RoomInfoDialog::createLemmyTab()
{
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *page = new QWidget(scrollArea);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(10);

    m_lemmySubscriberCountLabel = new QLabel(page);
    QFont subFont = m_lemmySubscriberCountLabel->font();
    subFont.setPointSize(14);
    subFont.setBold(true);
    m_lemmySubscriberCountLabel->setFont(subFont);
    layout->addWidget(m_lemmySubscriberCountLabel);

    layout->addWidget(createSeparator(page));

    auto *modHeader = new QLabel(tr("<b>Moderators</b>"), page);
    layout->addWidget(modHeader);

    m_lemmyModeratorList = new QListWidget(page);
    m_lemmyModeratorList->setMaximumHeight(150);
    layout->addWidget(m_lemmyModeratorList);

    layout->addWidget(createSeparator(page));

    auto *rulesHeader = new QLabel(tr("<b>Community Rules</b>"), page);
    layout->addWidget(rulesHeader);

    m_lemmyRulesLabel = new QLabel(page);
    m_lemmyRulesLabel->setWordWrap(true);
    m_lemmyRulesLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(m_lemmyRulesLabel);

    layout->addStretch();

    scrollArea->setWidget(page);
    return scrollArea;
}

void RoomInfoDialog::loadRoomData()
{
    m_roomNameLabel->setText(m_room.name);

    if (!m_room.topic.isEmpty()) {
        m_topicLabel->setText(m_room.topic);
    } else {
        m_topicLabel->setText(QStringLiteral("<i>%1</i>").arg(tr("No topic set")));
    }

    m_roomIdLabel->setText(m_room.id);

    const QString alias = m_room.metadata.value(QStringLiteral("canonical_alias")).toString();
    if (!alias.isEmpty()) {
        m_roomAliasLabel->setText(alias);
    } else {
        m_roomAliasLabel->setText(QStringLiteral("<i>%1</i>").arg(tr("No alias")));
    }

    // --- Type Badge ---
    m_typeBadgeLabel->setText(roomTypeLabel(m_room));
    m_typeBadgeLabel->setStyleSheet(
        QStringLiteral("border-radius: 10px; padding: 2px 10px; font-size: 12px; font-weight: bold; %1")
            .arg(roomTypeBadgeStyle(m_room)));

    // --- Encryption ---
    if (m_room.isEncrypted) {
        const QString algo = m_room.metadata.value(QStringLiteral("encryption_algorithm")).toString();
        if (algo.isEmpty()) {
            m_encryptionLabel->setText(
                QStringLiteral("\xF0\x9F\x94\x92  <b>%1</b>").arg(tr("Encrypted")));
        } else {
            m_encryptionLabel->setText(
                QStringLiteral("\xF0\x9F\x94\x92  <b>%1</b> \u2014 %2").arg(tr("Encrypted"), algo.toHtmlEscaped()));
        }
    } else {
        m_encryptionLabel->setText(
            QStringLiteral("\xF0\x9F\x94\x93  <b>%1</b>").arg(tr("Not Encrypted")));
    }

    // --- Member Count ---
    m_memberCountLabel->setText(tr("<b>Members:</b> %1").arg(m_room.memberCount));

    // --- Matrix Tab ---
    if (m_room.protocolType == ProtocolType::MATRIX) {
        m_matrixVersionLabel->setText(
            m_room.metadata.value(QStringLiteral("room_version")).toString(QStringLiteral("1")));
        m_matrixJoinRulesLabel->setText(
            m_room.metadata.value(QStringLiteral("join_rules")).toString(QStringLiteral("invite")));
        m_matrixGuestAccessLabel->setText(
            m_room.metadata.value(QStringLiteral("guest_access")).toString(
                QStringLiteral("forbidden")));

        const QString encAlgo =
            m_room.metadata.value(QStringLiteral("encryption_algorithm")).toString();
        m_matrixEncryptionAlgoLabel->setText(
            !encAlgo.isEmpty() ? encAlgo : tr("No encryption configured"));

        const QString canonical =
            m_room.metadata.value(QStringLiteral("canonical_alias")).toString();
        m_matrixCanonicalAliasLabel->setText(
            !canonical.isEmpty() ? canonical : tr("None"));
    }

    // --- IRC Tab ---
    if (m_room.protocolType == ProtocolType::IRC) {
        const QString modes =
            m_room.metadata.value(QStringLiteral("channel_modes")).toString();
        m_ircChannelModesLabel->setText(
            modes.isEmpty()
                ? tr("Channel Modes: <i>none</i>")
                : tr("Channel Modes: %1").arg(modes.toHtmlEscaped()));

        m_ircUserList->clear();
        const QJsonArray users = m_room.metadata.value(QStringLiteral("users")).toArray();
        for (const auto &user : users) {
            m_ircUserList->addItem(user.toString());
        }
        if (users.isEmpty()) {
            m_ircUserList->addItem(
                QStringLiteral("<i>%1</i>").arg(tr("Fetch user list from session...")));
        }
    }

    // --- Lemmy Tab ---
    if (m_room.protocolType == ProtocolType::LEMMY) {
        const int subscribers =
            m_room.metadata.value(QStringLiteral("subscriber_count")).toInt(m_room.memberCount);
        m_lemmySubscriberCountLabel->setText(
            tr("Subscribers: %1").arg(subscribers));

        m_lemmyModeratorList->clear();
        const QJsonArray moderators =
            m_room.metadata.value(QStringLiteral("moderators")).toArray();
        for (const auto &mod : moderators) {
            m_lemmyModeratorList->addItem(mod.toString());
        }
        if (moderators.isEmpty()) {
            m_lemmyModeratorList->addItem(
                QStringLiteral("<i>%1</i>").arg(tr("No moderators listed")));
        }

        const QString rules =
            m_room.metadata.value(QStringLiteral("community_rules")).toString();
        m_lemmyRulesLabel->setText(
            rules.isEmpty()
                ? QStringLiteral("<i>%1</i>").arg(tr("No community rules set"))
                : rules);

        const bool nsfw = m_room.metadata.value(QStringLiteral("nsfw")).toBool(false);
        if (nsfw) {
            m_lemmyRulesLabel->setText(
                m_lemmyRulesLabel->text()
                + QStringLiteral("\n\n<b>\xE2\x9A\xA0 %1</b>").arg(tr("NSFW Community")));
        }
    }
}

void RoomInfoDialog::onLeaveRoomClicked()
{
    const auto reply = QMessageBox::question(
        this,
        tr("Leave Room"),
        tr("Are you sure you want to leave \"%1\"?\n\nYou will no longer receive messages "
           "from this room until you rejoin.")
            .arg(m_room.name),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        emit leaveRoomRequested(m_room.id);
        accept();
    }
}

void RoomInfoDialog::onShareRoomLinkClicked()
{
    QString link = m_room.metadata.value(QStringLiteral("canonical_alias")).toString();
    if (link.isEmpty()) {
        link = m_room.id;
    }

    QClipboard *clipboard = QApplication::clipboard();
    if (clipboard) {
        clipboard->setText(link);
        QToolTip::showText(m_shareLinkButton->mapToGlobal(QPoint(0, 0)),
                           tr("Copied to clipboard!"),
                           m_shareLinkButton,
                           QRect(),
                           2000);
    }
}

void RoomInfoDialog::onViewMembersClicked()
{
    emit openMemberListRequested(m_room.id);

    auto *membersDialog = new QDialog(this);
    membersDialog->setWindowTitle(tr("Members \u2014 %1").arg(m_room.name));
    membersDialog->setMinimumSize(320, 360);
    membersDialog->setModal(true);

    auto *dlgLayout = new QVBoxLayout(membersDialog);

    auto *memberList = new QListWidget(membersDialog);
    dlgLayout->addWidget(memberList);

    // Populate from protocol-specific metadata if available
    if (m_room.protocolType == ProtocolType::IRC) {
        const QJsonArray users = m_room.metadata.value(QStringLiteral("users")).toArray();
        for (const auto &u : users)
            memberList->addItem(u.toString());
    } else if (m_room.protocolType == ProtocolType::LEMMY) {
        const QJsonArray mods = m_room.metadata.value(QStringLiteral("moderators")).toArray();
        for (const auto &m : mods)
            memberList->addItem(m.toString());
    }

    if (memberList->count() == 0) {
        memberList->addItem(QStringLiteral("<i>%1</i>").arg(tr("Member list not available offline.")));
    }

    auto *closeBtn = new QPushButton(tr("Close"), membersDialog);
    dlgLayout->addWidget(closeBtn);

    connect(closeBtn, &QPushButton::clicked, membersDialog, &QDialog::accept);

    membersDialog->show();
}

void RoomInfoDialog::onRoomSettingsClicked()
{
    emit roomSettingsRequested(m_room.id);
}
