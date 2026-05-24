#include "features/matrix_chat/space_view_widget.hpp"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <algorithm>

namespace {

QFrame *createSeparator(QWidget *parent)
{
    auto *line = new QFrame(parent);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setFixedHeight(1);
    return line;
}

QString initialsFromName(const QString &name)
{
    if (name.isEmpty())
        return QStringLiteral("?");

    const QStringList parts = name.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    if (parts.size() >= 2)
        return parts.at(0).left(1).toUpper() + parts.at(1).left(1).toUpper();
    return name.left(2).toUpper();
}

QString memberCountText(int count)
{
    if (count <= 0)
        return SpaceViewWidget::tr("0 members");
    if (count == 1)
        return SpaceViewWidget::tr("1 member");
    return SpaceViewWidget::tr("%1 members").arg(count);
}

constexpr int MEMBER_GRID_COLS = 5;
constexpr int AVATAR_SIZE = 64;
constexpr int ROOM_ITEM_HEIGHT = 52;

} // anonymous namespace

SpaceViewWidget::SpaceViewWidget(const QString &spaceId, QWidget *parent)
    : QWidget(parent)
    , m_spaceId(spaceId)
    , m_backButton(nullptr)
    , m_spaceAvatarLabel(nullptr)
    , m_spaceNameLabel(nullptr)
    , m_spaceTopicLabel(nullptr)
    , m_joinLeaveButton(nullptr)
    , m_inviteButton(nullptr)
    , m_loadingBar(nullptr)
    , m_roomsStack(nullptr)
    , m_suggestedList(nullptr)
    , m_allRoomsList(nullptr)
    , m_suggestedHeader(nullptr)
    , m_allRoomsHeader(nullptr)
    , m_membersStack(nullptr)
    , m_memberSearchEdit(nullptr)
    , m_memberScrollArea(nullptr)
    , m_memberGridContainer(nullptr)
    , m_memberGridLayout(nullptr)
    , m_memberCountLabel(nullptr)
{
    setupUi();
}

SpaceViewWidget::~SpaceViewWidget() = default;

void SpaceViewWidget::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Scroll area for the full content
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *contentWidget = new QWidget(scrollArea);
    auto *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    contentLayout->addWidget(createHeaderSection());
    contentLayout->addWidget(createSeparator(contentWidget));

    // Loading bar
    m_loadingBar = new QProgressBar(contentWidget);
    m_loadingBar->setRange(0, 0);
    m_loadingBar->setTextVisible(false);
    m_loadingBar->setFixedHeight(3);
    m_loadingBar->setVisible(false);
    m_loadingBar->setStyleSheet(QStringLiteral(
        "QProgressBar { border: none; background-color: transparent; } "
        "QProgressBar::chunk { background-color: #3498db; }"));
    contentLayout->addWidget(m_loadingBar);

    // Rooms section
    contentLayout->addWidget(createRoomsSection());
    contentLayout->addWidget(createSeparator(contentWidget));

    // Members section
    contentLayout->addWidget(createMembersSection());
    contentLayout->addStretch();

    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea, 1);

    // Bottom action bar
    auto *bottomBar = new QFrame(this);
    bottomBar->setStyleSheet(QStringLiteral(
        "QFrame { border-top: 1px solid #ddd; background-color: #fafafa; }"));
    auto *bottomLayout = new QHBoxLayout(bottomBar);
    bottomLayout->setContentsMargins(12, 8, 12, 8);
    bottomLayout->setSpacing(8);

    m_joinLeaveButton = new QPushButton(tr("Join Space"), bottomBar);
    m_joinLeaveButton->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: #27ae60; color: white; border: none; "
        "padding: 8px 20px; border-radius: 4px; font-weight: bold; } "
        "QPushButton:hover { background-color: #219a52; } "
        "QPushButton:pressed { background-color: #1e8449; }"));
    QObject::connect(m_joinLeaveButton, &QPushButton::clicked,
                     this, &SpaceViewWidget::onJoinLeaveClicked);
    bottomLayout->addWidget(m_joinLeaveButton);

    m_inviteButton = new QPushButton(tr("Invite People"), bottomBar);
    m_inviteButton->setEnabled(false);
    QObject::connect(m_inviteButton, &QPushButton::clicked,
                     this, &SpaceViewWidget::onInviteClicked);
    bottomLayout->addWidget(m_inviteButton);

    bottomLayout->addStretch();
    mainLayout->addWidget(bottomBar);
}

QWidget *SpaceViewWidget::createHeaderSection()
{
    auto *container = new QWidget(this);
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(8);

    // Back button row
    auto *topRow = new QHBoxLayout;
    topRow->setSpacing(8);

    m_backButton = new QPushButton(QStringLiteral("\xE2\x86\x90  ") + tr("Back"), container);
    m_backButton->setFlat(true);
    m_backButton->setStyleSheet(QStringLiteral(
        "QPushButton { color: #3498db; font-weight: bold; border: none; padding: 4px 8px; } "
        "QPushButton:hover { color: #2980b9; text-decoration: underline; }"));
    QObject::connect(m_backButton, &QPushButton::clicked,
                     this, &SpaceViewWidget::onBackClicked);
    topRow->addWidget(m_backButton);
    topRow->addStretch();
    layout->addLayout(topRow);

    // Avatar, name, topic
    auto *infoRow = new QHBoxLayout;
    infoRow->setSpacing(16);

    // Large avatar
    m_spaceAvatarLabel = new QLabel(container);
    m_spaceAvatarLabel->setFixedSize(96, 96);
    m_spaceAvatarLabel->setAlignment(Qt::AlignCenter);
    m_spaceAvatarLabel->setStyleSheet(QStringLiteral(
        "background-color: #8e44ad; border-radius: 48px; color: white; "
        "font-size: 36px; font-weight: bold;"));
    infoRow->addWidget(m_spaceAvatarLabel);

    auto *textCol = new QVBoxLayout;
    textCol->setSpacing(4);

    m_spaceNameLabel = new QLabel(container);
    QFont nameFont = m_spaceNameLabel->font();
    nameFont.setPointSize(20);
    nameFont.setBold(true);
    m_spaceNameLabel->setFont(nameFont);
    m_spaceNameLabel->setWordWrap(true);
    textCol->addWidget(m_spaceNameLabel);

    m_spaceTopicLabel = new QLabel(container);
    m_spaceTopicLabel->setWordWrap(true);
    m_spaceTopicLabel->setStyleSheet(QStringLiteral("color: #666; font-size: 13px;"));
    textCol->addWidget(m_spaceTopicLabel);

    textCol->addStretch();
    infoRow->addLayout(textCol, 1);
    layout->addLayout(infoRow);

    return container;
}

QWidget *SpaceViewWidget::createRoomsSection()
{
    auto *container = new QWidget(this);
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(8);

    auto *header = new QLabel(tr("<b>Rooms</b>"), container);
    QFont hdrFont = header->font();
    hdrFont.setPointSize(14);
    hdrFont.setBold(true);
    header->setFont(hdrFont);
    layout->addWidget(header);

    // Stack: content ↔ loading ↔ empty
    m_roomsStack = new QStackedWidget(container);

    // Content page (index 0)
    auto *contentPage = new QWidget(m_roomsStack);
    auto *contentLayout = new QVBoxLayout(contentPage);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(4);

    // Suggested rooms
    m_suggestedHeader = new QLabel(tr("Suggested"), contentPage);
    QFont subFont = m_suggestedHeader->font();
    subFont.setBold(true);
    subFont.setPointSize(11);
    m_suggestedHeader->setFont(subFont);
    contentLayout->addWidget(m_suggestedHeader);

    m_suggestedList = new QListWidget(contentPage);
    m_suggestedList->setMaximumHeight(200);
    m_suggestedList->setAlternatingRowColors(true);
    m_suggestedList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_suggestedList->setStyleSheet(QStringLiteral(
        "QListWidget { border: 1px solid #e0e0e0; border-radius: 4px; } "
        "QListWidget::item { padding: 6px; border-bottom: 1px solid #f0f0f0; } "
        "QListWidget::item:hover { background-color: #f0f7ff; }"));
    QObject::connect(m_suggestedList, &QListWidget::itemClicked,
                     this, &SpaceViewWidget::onRoomItemClicked);
    contentLayout->addWidget(m_suggestedList);

    contentLayout->addSpacing(8);

    // All rooms
    m_allRoomsHeader = new QLabel(tr("All Rooms"), contentPage);
    m_allRoomsHeader->setFont(subFont);
    contentLayout->addWidget(m_allRoomsHeader);

    m_allRoomsList = new QListWidget(contentPage);
    m_allRoomsList->setMinimumHeight(120);
    m_allRoomsList->setAlternatingRowColors(true);
    m_allRoomsList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_allRoomsList->setStyleSheet(QStringLiteral(
        "QListWidget { border: 1px solid #e0e0e0; border-radius: 4px; } "
        "QListWidget::item { padding: 6px; border-bottom: 1px solid #f0f0f0; } "
        "QListWidget::item:hover { background-color: #f0f7ff; }"));
    QObject::connect(m_allRoomsList, &QListWidget::itemClicked,
                     this, &SpaceViewWidget::onRoomItemClicked);
    contentLayout->addWidget(m_allRoomsList);

    m_roomsStack->addWidget(contentPage);

    // Loading page (index 1)
    auto *loadingPage = new QWidget(m_roomsStack);
    auto *loadingLayout = new QVBoxLayout(loadingPage);
    loadingLayout->setAlignment(Qt::AlignCenter);
    auto *loadingLabel = new QLabel(tr("Loading rooms..."), loadingPage);
    loadingLabel->setAlignment(Qt::AlignCenter);
    loadingLabel->setStyleSheet(QStringLiteral("color: #888; padding: 40px;"));
    loadingLayout->addWidget(loadingLabel);
    m_roomsStack->addWidget(loadingPage);

    // Empty page (index 2)
    auto *emptyPage = new QWidget(m_roomsStack);
    auto *emptyLayout = new QVBoxLayout(emptyPage);
    emptyLayout->setAlignment(Qt::AlignCenter);
    auto *emptyLabel = new QLabel(tr("No rooms in this space yet."), emptyPage);
    emptyLabel->setAlignment(Qt::AlignCenter);
    emptyLabel->setStyleSheet(QStringLiteral("color: #888; padding: 40px;"));
    emptyLayout->addWidget(emptyLabel);
    m_roomsStack->addWidget(emptyPage);

    m_roomsStack->setCurrentIndex(1); // start with loading
    layout->addWidget(m_roomsStack, 1);

    return container;
}

QWidget *SpaceViewWidget::createMembersSection()
{
    auto *container = new QWidget(this);
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(8);

    auto *headerRow = new QHBoxLayout;
    auto *header = new QLabel(tr("<b>Members</b>"), container);
    QFont hdrFont = header->font();
    hdrFont.setPointSize(14);
    hdrFont.setBold(true);
    header->setFont(hdrFont);
    headerRow->addWidget(header);

    headerRow->addStretch();

    m_memberCountLabel = new QLabel(container);
    m_memberCountLabel->setStyleSheet(QStringLiteral("color: #888; font-size: 12px;"));
    headerRow->addWidget(m_memberCountLabel);
    layout->addLayout(headerRow);

    // Search
    m_memberSearchEdit = new QLineEdit(container);
    m_memberSearchEdit->setPlaceholderText(tr("Search members..."));
    m_memberSearchEdit->setClearButtonEnabled(true);
    m_memberSearchEdit->setStyleSheet(QStringLiteral(
        "QLineEdit { padding: 8px 12px; border: 1px solid #ccc; border-radius: 20px; "
        "background-color: #f5f5f5; font-size: 13px; } "
        "QLineEdit:focus { border-color: #3498db; background-color: #fff; }"));
    QObject::connect(m_memberSearchEdit, &QLineEdit::textChanged,
                     this, &SpaceViewWidget::onMemberSearchChanged);
    layout->addWidget(m_memberSearchEdit);

    // Stack: content ↔ loading
    m_membersStack = new QStackedWidget(container);

    // Content page (index 0) — scrollable grid
    m_memberScrollArea = new QScrollArea(m_membersStack);
    m_memberScrollArea->setWidgetResizable(true);
    m_memberScrollArea->setFrameShape(QFrame::NoFrame);
    m_memberScrollArea->setMinimumHeight(150);

    m_memberGridContainer = new QWidget(m_memberScrollArea);
    m_memberGridLayout = new QGridLayout(m_memberGridContainer);
    m_memberGridLayout->setContentsMargins(4, 4, 4, 4);
    m_memberGridLayout->setSpacing(8);
    m_memberGridLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    m_memberScrollArea->setWidget(m_memberGridContainer);
    m_membersStack->addWidget(m_memberScrollArea);

    // Loading page (index 1)
    auto *loadingPage = new QWidget(m_membersStack);
    auto *loadingLayout = new QVBoxLayout(loadingPage);
    loadingLayout->setAlignment(Qt::AlignCenter);
    auto *loadingLabel = new QLabel(tr("Loading members..."), loadingPage);
    loadingLabel->setAlignment(Qt::AlignCenter);
    loadingLabel->setStyleSheet(QStringLiteral("color: #888; padding: 40px;"));
    loadingLayout->addWidget(loadingLabel);
    m_membersStack->addWidget(loadingPage);

    m_membersStack->setCurrentIndex(1); // start with loading
    layout->addWidget(m_membersStack, 1);

    return container;
}

QWidget *SpaceViewWidget::createMemberCard(const SpaceMemberInfo &member)
{
    auto *card = new QWidget(m_memberGridContainer);
    card->setFixedSize(80, 100);

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(2, 4, 2, 2);
    cardLayout->setSpacing(4);
    cardLayout->setAlignment(Qt::AlignCenter);

    // Avatar circle
    auto *avatar = new QLabel(card);
    avatar->setFixedSize(AVATAR_SIZE, AVATAR_SIZE);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet(QStringLiteral(
        "background-color: #3498db; border-radius: %1px; color: white; "
        "font-size: 20px; font-weight: bold;")
            .arg(AVATAR_SIZE / 2));
    avatar->setText(initialsFromName(member.displayName));
    avatar->setToolTip(member.userId);
    cardLayout->addWidget(avatar, 0, Qt::AlignHCenter);

    // Name
    QString displayText = member.displayName;
    if (displayText.length() > 10)
        displayText = displayText.left(9) + QStringLiteral("\xE2\x80\xA6"); // …
    auto *nameLabel = new QLabel(displayText, card);
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setWordWrap(true);
    QFont nameFont = nameLabel->font();
    nameFont.setPointSize(10);
    nameLabel->setFont(nameFont);
    nameLabel->setMaximumHeight(28);
    cardLayout->addWidget(nameLabel, 0, Qt::AlignHCenter);

    return card;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void SpaceViewWidget::setSpaceName(const QString &name)
{
    m_spaceName = name;
    m_spaceNameLabel->setText(name.isEmpty() ? tr("Untitled Space") : name);
    m_spaceAvatarLabel->setText(initialsFromName(name));
}

void SpaceViewWidget::setSpaceTopic(const QString &topic)
{
    m_spaceTopic = topic;
    if (topic.isEmpty()) {
        m_spaceTopicLabel->setText(QStringLiteral("<i>%1</i>").arg(tr("No topic set")));
    } else {
        m_spaceTopicLabel->setText(topic);
    }
}

void SpaceViewWidget::setSpaceAvatarUrl(const QString &url)
{
    m_spaceAvatarUrl = url;
    // In a real implementation, fetch and render the avatar image.
    // For now the initials-based avatar is sufficient.
    Q_UNUSED(url)
}

void SpaceViewWidget::setJoined(bool joined)
{
    m_isJoined = joined;
    if (joined) {
        m_joinLeaveButton->setText(tr("Leave Space"));
        m_joinLeaveButton->setStyleSheet(QStringLiteral(
            "QPushButton { background-color: #e74c3c; color: white; border: none; "
            "padding: 8px 20px; border-radius: 4px; font-weight: bold; } "
            "QPushButton:hover { background-color: #c0392b; } "
            "QPushButton:pressed { background-color: #a93226; }"));
        m_inviteButton->setEnabled(true);
    } else {
        m_joinLeaveButton->setText(tr("Join Space"));
        m_joinLeaveButton->setStyleSheet(QStringLiteral(
            "QPushButton { background-color: #27ae60; color: white; border: none; "
            "padding: 8px 20px; border-radius: 4px; font-weight: bold; } "
            "QPushButton:hover { background-color: #219a52; } "
            "QPushButton:pressed { background-color: #1e8449; }"));
        m_inviteButton->setEnabled(false);
    }
}

void SpaceViewWidget::setLoading(bool loading)
{
    m_isLoading = loading;
    m_loadingBar->setVisible(loading);
}

void SpaceViewWidget::loadChildRooms()
{
    setLoading(true);
    setSectionLoading(m_roomsStack, true);

    // In a real implementation, this would POST to
    // /_matrix/client/r0/rooms/{spaceId}/hierarchy
    // and parse the response. Here we populate demo data.

    m_suggestedRooms.clear();
    m_allRooms.clear();

    {
        SpaceRoomInfo r;
        r.roomId = QStringLiteral("!general:%1").arg(m_spaceId.section(QLatin1Char(':'), 1));
        r.name = tr("General");
        r.topic = tr("General discussion for this space");
        r.memberCount = 142;
        r.suggested = true;
        r.parentSpaceId = m_spaceId;
        m_suggestedRooms.append(r);
        m_allRooms.append(r);
    }
    {
        SpaceRoomInfo r;
        r.roomId = QStringLiteral("!announcements:%1").arg(m_spaceId.section(QLatin1Char(':'), 1));
        r.name = tr("Announcements");
        r.topic = tr("Important announcements and updates");
        r.memberCount = 142;
        r.suggested = true;
        r.parentSpaceId = m_spaceId;
        m_suggestedRooms.append(r);
    }
    {
        SpaceRoomInfo r;
        r.roomId = QStringLiteral("!random:%1").arg(m_spaceId.section(QLatin1Char(':'), 1));
        r.name = tr("Random");
        r.topic = tr("Off-topic chat and fun stuff");
        r.memberCount = 89;
        r.suggested = false;
        r.parentSpaceId = m_spaceId;
        m_allRooms.append(r);
    }
    {
        SpaceRoomInfo r;
        r.roomId = QStringLiteral("!dev:%1").arg(m_spaceId.section(QLatin1Char(':'), 1));
        r.name = tr("Development");
        r.topic = tr("Technical discussions and code reviews");
        r.memberCount = 56;
        r.suggested = false;
        r.parentSpaceId = m_spaceId;
        m_allRooms.append(r);
    }
    {
        SpaceRoomInfo r;
        r.roomId = QStringLiteral("!design:%1").arg(m_spaceId.section(QLatin1Char(':'), 1));
        r.name = tr("Design");
        r.topic = tr("UI/UX design discussions and feedback");
        r.memberCount = 34;
        r.suggested = false;
        r.parentSpaceId = m_spaceId;
        m_allRooms.append(r);
    }

    refreshRoomList();
    setLoading(false);
    setSectionLoading(m_roomsStack, false);

    // Show empty state if no rooms
    if (m_allRooms.isEmpty()) {
        m_roomsStack->setCurrentIndex(2);
    }
}

void SpaceViewWidget::loadMembers()
{
    setSectionLoading(m_membersStack, true);

    m_members.clear();

    {
        SpaceMemberInfo m;
        m.userId = QStringLiteral("@alice:%1").arg(m_spaceId.section(QLatin1Char(':'), 1));
        m.displayName = QStringLiteral("Alice");
        m.powerLevel = QStringLiteral("Admin");
        m_members.append(m);
    }
    {
        SpaceMemberInfo m;
        m.userId = QStringLiteral("@bob:%1").arg(m_spaceId.section(QLatin1Char(':'), 1));
        m.displayName = QStringLiteral("Bob");
        m.powerLevel = QStringLiteral("Moderator");
        m_members.append(m);
    }
    {
        SpaceMemberInfo m;
        m.userId = QStringLiteral("@carol:%1").arg(m_spaceId.section(QLatin1Char(':'), 1));
        m.displayName = QStringLiteral("Carol");
        m.powerLevel = QStringLiteral("Member");
        m_members.append(m);
    }
    {
        SpaceMemberInfo m;
        m.userId = QStringLiteral("@dave:%1").arg(m_spaceId.section(QLatin1Char(':'), 1));
        m.displayName = QStringLiteral("Dave");
        m.powerLevel = QStringLiteral("Member");
        m_members.append(m);
    }
    {
        SpaceMemberInfo m;
        m.userId = QStringLiteral("@eve:%1").arg(m_spaceId.section(QLatin1Char(':'), 1));
        m.displayName = QStringLiteral("Eve Johnson");
        m.powerLevel = QStringLiteral("Member");
        m_members.append(m);
    }
    {
        SpaceMemberInfo m;
        m.userId = QStringLiteral("@frank:%1").arg(m_spaceId.section(QLatin1Char(':'), 1));
        m.displayName = QStringLiteral("Frank");
        m.powerLevel = QStringLiteral("Member");
        m_members.append(m);
    }
    {
        SpaceMemberInfo m;
        m.userId = QStringLiteral("@grace:%1").arg(m_spaceId.section(QLatin1Char(':'), 1));
        m.displayName = QStringLiteral("Grace Hopper");
        m.powerLevel = QStringLiteral("Member");
        m_members.append(m);
    }
    {
        SpaceMemberInfo m;
        m.userId = QStringLiteral("@heidi:%1").arg(m_spaceId.section(QLatin1Char(':'), 1));
        m.displayName = QStringLiteral("Heidi");
        m.powerLevel = QStringLiteral("Member");
        m_members.append(m);
    }

    // Sort by display name
    std::sort(m_members.begin(), m_members.end(),
              [](const SpaceMemberInfo &a, const SpaceMemberInfo &b) {
                  return a.displayName.compare(b.displayName, Qt::CaseInsensitive) < 0;
              });

    refreshMemberGrid();
    setSectionLoading(m_membersStack, false);
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void SpaceViewWidget::refreshRoomList()
{
    m_suggestedList->clear();
    m_allRoomsList->clear();

    // Add suggested rooms to the suggested list
    for (const auto &room : m_suggestedRooms) {
        QString label;
        if (room.topic.isEmpty()) {
            label = QStringLiteral("%1  |  %2")
                .arg(room.name, memberCountText(room.memberCount));
        } else {
            const QString truncatedTopic = room.topic.length() > 60
                ? room.topic.left(57) + QStringLiteral("...")
                : room.topic;
            label = QStringLiteral("%1\n<span style='color: #888; font-size: 11px;'>%2 \xE2\x80\xA2 %3</span>")
                .arg(room.name.toHtmlEscaped(),
                     truncatedTopic.toHtmlEscaped(),
                     memberCountText(room.memberCount));
        }

        auto *item = new QListWidgetItem(label, m_suggestedList);
        item->setData(Qt::UserRole, room.roomId);
        item->setSizeHint(QSize(0, ROOM_ITEM_HEIGHT));
        item->setToolTip(room.topic.isEmpty() ? room.name : room.topic);
    }

    // Add non-suggested rooms to the all-rooms list
    for (const auto &room : m_allRooms) {
        if (room.suggested)
            continue;

        QString label;
        if (room.topic.isEmpty()) {
            label = QStringLiteral("%1  |  %2")
                .arg(room.name, memberCountText(room.memberCount));
        } else {
            const QString truncatedTopic = room.topic.length() > 60
                ? room.topic.left(57) + QStringLiteral("...")
                : room.topic;
            label = QStringLiteral("%1\n<span style='color: #888; font-size: 11px;'>%2 \xE2\x80\xA2 %3</span>")
                .arg(room.name.toHtmlEscaped(),
                     truncatedTopic.toHtmlEscaped(),
                     memberCountText(room.memberCount));
        }

        auto *item = new QListWidgetItem(label, m_allRoomsList);
        item->setData(Qt::UserRole, room.roomId);
        item->setSizeHint(QSize(0, ROOM_ITEM_HEIGHT));
        item->setToolTip(room.topic.isEmpty() ? room.name : room.topic);
    }

    // Hide suggested section if empty
    m_suggestedHeader->setVisible(!m_suggestedRooms.isEmpty());
    m_suggestedList->setVisible(!m_suggestedRooms.isEmpty());

    // Adjust all-rooms list height
    const int allRoomCount = static_cast<int>(
        std::count_if(m_allRooms.begin(), m_allRooms.end(),
                      [](const SpaceRoomInfo &r) { return !r.suggested; }));
    m_allRoomsList->setMinimumHeight(std::min(allRoomCount * ROOM_ITEM_HEIGHT, 240));
}

void SpaceViewWidget::refreshMemberGrid(const QString &filter)
{
    // Clear existing items
    QLayoutItem *item;
    while ((item = m_memberGridLayout->takeAt(0)) != nullptr) {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }

    int visibleCount = 0;
    for (int i = 0; i < m_members.size(); ++i) {
        const auto &member = m_members.at(i);

        if (!filter.isEmpty()) {
            if (!member.displayName.contains(filter, Qt::CaseInsensitive)
                && !member.userId.contains(filter, Qt::CaseInsensitive)) {
                continue;
            }
        }

        const int row = visibleCount / MEMBER_GRID_COLS;
        const int col = visibleCount % MEMBER_GRID_COLS;

        auto *card = createMemberCard(member);
        m_memberGridLayout->addWidget(card, row, col);
        ++visibleCount;
    }

    m_memberCountLabel->setText(memberCountText(visibleCount));
}

void SpaceViewWidget::setSectionLoading(QStackedWidget *stack, bool loading)
{
    if (stack == m_roomsStack) {
        if (loading) {
            m_roomsStack->setCurrentIndex(1);
        } else if (!m_allRooms.isEmpty()) {
            m_roomsStack->setCurrentIndex(0);
        } else {
            m_roomsStack->setCurrentIndex(2);
        }
    } else if (stack == m_membersStack) {
        if (loading) {
            m_membersStack->setCurrentIndex(1);
        } else {
            m_membersStack->setCurrentIndex(0);
        }
    }
}

// ---------------------------------------------------------------------------
// Slots
// ---------------------------------------------------------------------------

void SpaceViewWidget::onRoomItemClicked(QListWidgetItem *item)
{
    if (!item)
        return;

    const QString roomId = item->data(Qt::UserRole).toString();
    if (!roomId.isEmpty()) {
        emit roomSelected(roomId);
    }
}

void SpaceViewWidget::onBackClicked()
{
    emit backRequested();
}

void SpaceViewWidget::onJoinLeaveClicked()
{
    if (m_isJoined) {
        emit leaveSpaceRequested(m_spaceId);
    } else {
        emit joinSpaceRequested(m_spaceId);
    }
}

void SpaceViewWidget::onInviteClicked()
{
    emit inviteRequested(m_spaceId);
}

void SpaceViewWidget::onMemberSearchChanged(const QString &text)
{
    refreshMemberGrid(text.trimmed());
}
