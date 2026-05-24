#include "features/lemmy/lemmy_notifications_widget.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPainter>
#include <QPixmap>
#include <QProgressBar>
#include <QPushButton>
#include <QTabWidget>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QVBoxLayout>
#include <QDateTime>
#include <QFont>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>

static const char *kLemmyNotifStyleSheet = R"(
    LemmyNotificationsWidget {
        background-color: #1e1e2e;
    }
    LemmyNotificationsWidget QTabWidget::pane {
        background-color: #1e1e2e;
        border: none;
    }
    LemmyNotificationsWidget QTabBar::tab {
        background-color: #313244;
        color: #6c7086;
        border: none;
        padding: 8px 16px;
        font-size: 12px;
        min-width: 60px;
        margin-right: 2px;
        border-top-left-radius: 6px;
        border-top-right-radius: 6px;
    }
    LemmyNotificationsWidget QTabBar::tab:selected {
        background-color: #45475a;
        color: #cdd6f4;
        font-weight: bold;
    }
    LemmyNotificationsWidget QTabBar::tab:hover:!selected {
        background-color: #3a3a4e;
        color: #bac2de;
    }
    LemmyNotificationsWidget QListWidget {
        background-color: #1e1e2e;
        color: #cdd6f4;
        border: none;
        outline: none;
    }
    LemmyNotificationsWidget QListWidget::item {
        background-color: #1e1e2e;
        border-bottom: 1px solid #313244;
        padding: 0px;
    }
    LemmyNotificationsWidget QListWidget::item:hover {
        background-color: #2a2a3c;
    }
    LemmyNotificationsWidget QListWidget::item:selected {
        background-color: #2a2a3c;
    }
    LemmyNotificationsWidget QPushButton#markAllReadBtn {
        background-color: #313244;
        color: #89b4fa;
        border: 1px solid #45475a;
        border-radius: 6px;
        padding: 6px 16px;
        font-size: 12px;
    }
    LemmyNotificationsWidget QPushButton#markAllReadBtn:hover {
        background-color: #45475a;
        border-color: #89b4fa;
    }
    LemmyNotificationsWidget QProgressBar {
        background-color: #313244;
        border: none;
        border-radius: 4px;
        height: 4px;
        text-align: center;
        font-size: 11px;
        color: #6c7086;
    }
    LemmyNotificationsWidget QProgressBar::chunk {
        background-color: #cba6f7;
        border-radius: 4px;
    }
)";

static QString relativeTime(qint64 unixSec)
{
    if (unixSec <= 0)
        return QStringLiteral("just now");

    qint64 now = QDateTime::currentSecsSinceEpoch();
    qint64 diff = now - unixSec;

    if (diff < 0) diff = 0;
    if (diff < 60)       return QStringLiteral("just now");
    if (diff < 3600)     return QStringLiteral("%1m").arg(diff / 60);
    if (diff < 86400)    return QStringLiteral("%1h").arg(diff / 3600);
    if (diff < 2592000)  return QStringLiteral("%1d").arg(diff / 86400);
    if (diff < 31536000) return QStringLiteral("%1mo").arg(diff / 2592000);

    return QStringLiteral("%1y").arg(diff / 31536000);
}

static qint64 parseTimestampSec(const QString &publishedStr)
{
    if (publishedStr.isEmpty())
        return 0;

    QDateTime dt = QDateTime::fromString(publishedStr, Qt::ISODate);
    if (!dt.isValid())
        dt = QDateTime::fromString(publishedStr, Qt::ISODateWithMs);

    return dt.isValid() ? dt.toSecsSinceEpoch() : 0;
}

static QPixmap makeAvatarPixmap(const QString &name, int size, const QColor &bg)
{
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);

    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect r(0, 0, size, size);
    painter.setPen(Qt::NoPen);
    painter.setBrush(bg);
    painter.drawRoundedRect(r, size / 4, size / 4);

    QString letter;
    if (!name.isEmpty())
        letter = name.at(0).toUpper();
    else
        letter = QStringLiteral("?");

    QFont font;
    font.setPixelSize(size / 2 + 2);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(r, Qt::AlignCenter, letter);

    painter.end();
    return pix;
}

static QColor avatarColorFromName(const QString &name)
{
    if (name.isEmpty())
        return QColor(0x6B, 0x72, 0x80);

    const uint hash = qHash(name);
    const int hue = static_cast<int>(hash % 360);
    return QColor::fromHsl(hue, 180, 160);
}

LemmyNotificationsWidget::LemmyNotificationsWidget(QWidget *parent)
    : QWidget(parent)
    , m_tabWidget(nullptr)
    , m_allList(nullptr)
    , m_repliesList(nullptr)
    , m_mentionsList(nullptr)
    , m_messagesList(nullptr)
    , m_markAllReadButton(nullptr)
    , m_loadingBar(nullptr)
    , m_emptyLabel(nullptr)
    , m_refreshTimer(nullptr)
    , m_nam(nullptr)
{
    m_nam = new QNetworkAccessManager(this);
    setStyleSheet(QString::fromLatin1(kLemmyNotifStyleSheet));
    setupUi();
}

LemmyNotificationsWidget::~LemmyNotificationsWidget() = default;

void LemmyNotificationsWidget::setInstanceUrl(const QString &url)
{
    m_instanceUrl = url;
    if (!m_instanceUrl.isEmpty() && !m_jwt.isEmpty()) {
        startAutoRefresh();
        refreshNotifications();
    }
}

void LemmyNotificationsWidget::setAuthToken(const QString &jwt)
{
    m_jwt = jwt;
    if (!m_instanceUrl.isEmpty() && !m_jwt.isEmpty()) {
        startAutoRefresh();
        refreshNotifications();
    }
}

void LemmyNotificationsWidget::startAutoRefresh()
{
    if (!m_refreshTimer->isActive())
        m_refreshTimer->start();
}

int LemmyNotificationsWidget::totalUnreadCount() const
{
    int count = 0;
    for (const auto &n : m_allNotifications) {
        if (!n.read) ++count;
    }
    return count;
}

void LemmyNotificationsWidget::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto *topBar = new QHBoxLayout();
    topBar->setContentsMargins(12, 8, 12, 4);
    topBar->setSpacing(8);

    auto *titleLabel = new QLabel(tr("Notifications"), this);
    titleLabel->setStyleSheet(QStringLiteral(
        "color: #cdd6f4; font-size: 15px; font-weight: bold; background: transparent;"));
    topBar->addWidget(titleLabel);

    topBar->addStretch();

    m_markAllReadButton = new QPushButton(tr("Mark All Read"), this);
    m_markAllReadButton->setObjectName(QStringLiteral("markAllReadBtn"));
    m_markAllReadButton->setCursor(Qt::PointingHandCursor);
    topBar->addWidget(m_markAllReadButton);

    mainLayout->addLayout(topBar);

    m_loadingBar = new QProgressBar(this);
    m_loadingBar->setRange(0, 0);
    m_loadingBar->setFixedHeight(4);
    m_loadingBar->setVisible(false);
    mainLayout->addWidget(m_loadingBar);

    m_emptyLabel = new QLabel(tr("No notifications yet"), this);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet(QStringLiteral(
        "color: #6c7086; font-size: 14px; padding: 40px;"));
    m_emptyLabel->setVisible(true);
    mainLayout->addWidget(m_emptyLabel);

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setVisible(false);

    m_allList = new QListWidget(this);
    m_allList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_allList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_allList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_allList->setSpacing(2);

    m_repliesList = new QListWidget(this);
    m_repliesList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_repliesList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_repliesList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_repliesList->setSpacing(2);

    m_mentionsList = new QListWidget(this);
    m_mentionsList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_mentionsList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_mentionsList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_mentionsList->setSpacing(2);

    m_messagesList = new QListWidget(this);
    m_messagesList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_messagesList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_messagesList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_messagesList->setSpacing(2);

    m_tabWidget->addTab(m_allList, tr("All"));
    m_tabWidget->addTab(m_repliesList, tr("Replies"));
    m_tabWidget->addTab(m_mentionsList, tr("Mentions"));
    m_tabWidget->addTab(m_messagesList, tr("Messages"));

    mainLayout->addWidget(m_tabWidget, 1);

    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setInterval(60000);

    connect(m_tabWidget, &QTabWidget::currentChanged,
            this, &LemmyNotificationsWidget::onTabChanged);
    connect(m_allList, &QListWidget::itemActivated,
            this, &LemmyNotificationsWidget::onNotificationActivated);
    connect(m_repliesList, &QListWidget::itemActivated,
            this, &LemmyNotificationsWidget::onNotificationActivated);
    connect(m_mentionsList, &QListWidget::itemActivated,
            this, &LemmyNotificationsWidget::onNotificationActivated);
    connect(m_messagesList, &QListWidget::itemActivated,
            this, &LemmyNotificationsWidget::onNotificationActivated);
    connect(m_markAllReadButton, &QPushButton::clicked,
            this, &LemmyNotificationsWidget::onMarkAllReadClicked);
    connect(m_refreshTimer, &QTimer::timeout,
            this, &LemmyNotificationsWidget::refreshNotifications);
}

void LemmyNotificationsWidget::refreshNotifications()
{
    if (m_instanceUrl.isEmpty() || m_jwt.isEmpty())
        return;

    m_loadingBar->setVisible(true);
    fetchMentions();
    fetchReplies();
    fetchPrivateMessages();
}

void LemmyNotificationsWidget::fetchMentions()
{
    QUrl url(m_instanceUrl + QStringLiteral("/api/v3/user/mention"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("sort"), QStringLiteral("New"));
    query.addQueryItem(QStringLiteral("limit"), QStringLiteral("50"));
    query.addQueryItem(QStringLiteral("unread_only"), QStringLiteral("false"));
    url.setQuery(query);

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    if (!m_jwt.isEmpty())
        req.setRawHeader("Authorization", ("Bearer " + m_jwt).toUtf8());

    QNetworkReply *reply = m_nam->get(req);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        m_mentionNotifications.clear();

        if (reply->error() != QNetworkReply::NoError) {
            m_loadingBar->setVisible(false);
            return;
        }

        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        const QJsonObject root = doc.object();
        const QJsonArray mentions = root.value(QStringLiteral("mentions")).toArray();

        for (const QJsonValue &val : mentions) {
            const QJsonObject entry = val.toObject();
            const QJsonObject personMention = entry.value(QStringLiteral("person_mention")).toObject();
            const QJsonObject comment = entry.value(QStringLiteral("comment")).toObject();
            const QJsonObject creator = entry.value(QStringLiteral("creator")).toObject();
            const QJsonObject post = entry.value(QStringLiteral("post")).toObject();
            const QJsonObject community = entry.value(QStringLiteral("community")).toObject();

            LemmyNotification notif;
            notif.type = LemmyNotification::Mention;
            notif.id = static_cast<qint64>(personMention.value(QStringLiteral("id")).toInt());
            notif.read = personMention.value(QStringLiteral("read")).toBool();
            notif.contentPreview = comment.value(QStringLiteral("content")).toString();
            notif.timestampSec = parseTimestampSec(personMention.value(QStringLiteral("published")).toString());
            notif.creatorName = creator.value(QStringLiteral("name")).toString();
            notif.creatorAvatarUrl = creator.value(QStringLiteral("avatar")).toString();
            notif.creatorId = static_cast<qint64>(creator.value(QStringLiteral("id")).toInt());
            notif.postId = static_cast<qint64>(post.value(QStringLiteral("id")).toInt());
            notif.commentId = static_cast<qint64>(comment.value(QStringLiteral("id")).toInt());
            notif.communityName = community.value(QStringLiteral("name")).toString();

            m_mentionNotifications.append(notif);
        }

        std::sort(m_mentionNotifications.begin(), m_mentionNotifications.end(),
                  [](const LemmyNotification &a, const LemmyNotification &b) {
                      return a.timestampSec > b.timestampSec;
                  });

        mergeAndPopulateAllTab();
        onTabChanged(m_tabWidget->currentIndex());
        m_loadingBar->setVisible(false);
    });
}

void LemmyNotificationsWidget::fetchReplies()
{
    QUrl url(m_instanceUrl + QStringLiteral("/api/v3/user/replies"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("sort"), QStringLiteral("New"));
    query.addQueryItem(QStringLiteral("limit"), QStringLiteral("50"));
    query.addQueryItem(QStringLiteral("unread_only"), QStringLiteral("false"));
    url.setQuery(query);

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    if (!m_jwt.isEmpty())
        req.setRawHeader("Authorization", ("Bearer " + m_jwt).toUtf8());

    QNetworkReply *reply = m_nam->get(req);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        m_replyNotifications.clear();

        if (reply->error() != QNetworkReply::NoError)
            return;

        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        const QJsonObject root = doc.object();
        const QJsonArray replies = root.value(QStringLiteral("replies")).toArray();

        for (const QJsonValue &val : replies) {
            const QJsonObject entry = val.toObject();
            const QJsonObject commentReply = entry.value(QStringLiteral("comment_reply")).toObject();
            const QJsonObject comment = entry.value(QStringLiteral("comment")).toObject();
            const QJsonObject creator = entry.value(QStringLiteral("creator")).toObject();
            const QJsonObject post = entry.value(QStringLiteral("post")).toObject();
            const QJsonObject community = entry.value(QStringLiteral("community")).toObject();

            LemmyNotification notif;
            notif.type = LemmyNotification::Reply;
            notif.id = static_cast<qint64>(commentReply.value(QStringLiteral("id")).toInt());
            notif.read = commentReply.value(QStringLiteral("read")).toBool();
            notif.contentPreview = comment.value(QStringLiteral("content")).toString();
            notif.timestampSec = parseTimestampSec(commentReply.value(QStringLiteral("published")).toString());
            notif.creatorName = creator.value(QStringLiteral("name")).toString();
            notif.creatorAvatarUrl = creator.value(QStringLiteral("avatar")).toString();
            notif.creatorId = static_cast<qint64>(creator.value(QStringLiteral("id")).toInt());
            notif.postId = static_cast<qint64>(post.value(QStringLiteral("id")).toInt());
            notif.commentId = static_cast<qint64>(comment.value(QStringLiteral("id")).toInt());
            notif.communityName = community.value(QStringLiteral("name")).toString();

            m_replyNotifications.append(notif);
        }

        std::sort(m_replyNotifications.begin(), m_replyNotifications.end(),
                  [](const LemmyNotification &a, const LemmyNotification &b) {
                      return a.timestampSec > b.timestampSec;
                  });

        mergeAndPopulateAllTab();
        onTabChanged(m_tabWidget->currentIndex());
    });
}

void LemmyNotificationsWidget::fetchPrivateMessages()
{
    QUrl url(m_instanceUrl + QStringLiteral("/api/v3/private_message/list"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("limit"), QStringLiteral("50"));
    query.addQueryItem(QStringLiteral("unread_only"), QStringLiteral("false"));
    url.setQuery(query);

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    if (!m_jwt.isEmpty())
        req.setRawHeader("Authorization", ("Bearer " + m_jwt).toUtf8());

    QNetworkReply *reply = m_nam->get(req);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        m_messageNotifications.clear();

        if (reply->error() != QNetworkReply::NoError)
            return;

        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        const QJsonObject root = doc.object();
        const QJsonArray messages = root.value(QStringLiteral("private_messages")).toArray();

        for (const QJsonValue &val : messages) {
            const QJsonObject entry = val.toObject();
            const QJsonObject privateMessage = entry.value(QStringLiteral("private_message")).toObject();
            const QJsonObject creator = entry.value(QStringLiteral("creator")).toObject();

            LemmyNotification notif;
            notif.type = LemmyNotification::PrivateMessage;
            notif.id = static_cast<qint64>(privateMessage.value(QStringLiteral("id")).toInt());
            notif.read = privateMessage.value(QStringLiteral("read")).toBool();
            notif.contentPreview = privateMessage.value(QStringLiteral("content")).toString();
            notif.timestampSec = parseTimestampSec(privateMessage.value(QStringLiteral("published")).toString());
            notif.creatorName = creator.value(QStringLiteral("name")).toString();
            notif.creatorAvatarUrl = creator.value(QStringLiteral("avatar")).toString();
            notif.creatorId = static_cast<qint64>(creator.value(QStringLiteral("id")).toInt());
            notif.privateMessageId = notif.id;

            m_messageNotifications.append(notif);
        }

        std::sort(m_messageNotifications.begin(), m_messageNotifications.end(),
                  [](const LemmyNotification &a, const LemmyNotification &b) {
                      return a.timestampSec > b.timestampSec;
                  });

        mergeAndPopulateAllTab();
        onTabChanged(m_tabWidget->currentIndex());
    });
}

void LemmyNotificationsWidget::mergeAndPopulateAllTab()
{
    m_allNotifications.clear();

    for (auto n : m_replyNotifications) {
        n.type = LemmyNotification::Reply;
        m_allNotifications.append(n);
    }
    for (auto n : m_mentionNotifications) {
        n.type = LemmyNotification::Mention;
        m_allNotifications.append(n);
    }
    for (auto n : m_messageNotifications) {
        n.type = LemmyNotification::PrivateMessage;
        m_allNotifications.append(n);
    }

    std::sort(m_allNotifications.begin(), m_allNotifications.end(),
              [](const LemmyNotification &a, const LemmyNotification &b) {
                  return a.timestampSec > b.timestampSec;
              });

    populateTab(0, m_allNotifications);
    updateTabBadges();
}

void LemmyNotificationsWidget::onTabChanged(int index)
{
    switch (index) {
    case 0:
        populateTab(0, m_allNotifications);
        break;
    case 1:
        populateTab(1, m_replyNotifications);
        break;
    case 2:
        populateTab(2, m_mentionNotifications);
        break;
    case 3:
        populateTab(3, m_messageNotifications);
        break;
    }

    updateTabBadges();
}

void LemmyNotificationsWidget::populateTab(int tabIndex, const QVector<LemmyNotification> &notifications)
{
    QListWidget *targetList = nullptr;
    switch (tabIndex) {
    case 0: targetList = m_allList; break;
    case 1: targetList = m_repliesList; break;
    case 2: targetList = m_mentionsList; break;
    case 3: targetList = m_messagesList; break;
    }

    if (!targetList)
        return;

    targetList->clear();

    bool hasAny = !m_allNotifications.isEmpty()
                  || !m_replyNotifications.isEmpty()
                  || !m_mentionNotifications.isEmpty()
                  || !m_messageNotifications.isEmpty();

    if (!hasAny) {
        m_emptyLabel->setVisible(true);
        m_tabWidget->setVisible(false);
        return;
    }

    if (notifications.isEmpty()) {
        if (tabIndex == 0) {
            m_emptyLabel->setText(tr("No notifications yet"));
            m_emptyLabel->setVisible(true);
            m_tabWidget->setVisible(false);
        }
        return;
    }

    m_emptyLabel->setVisible(false);
    m_tabWidget->setVisible(true);

    for (int i = 0; i < notifications.size(); ++i) {
        const auto &notif = notifications[i];
        auto *item = new QListWidgetItem();
        item->setData(static_cast<int>(NotifIndexRole), i);
        item->setData(static_cast<int>(NotifTypeRole), static_cast<int>(notif.type));
        item->setSizeHint(QSize(0, 80));

        QWidget *card = createNotificationItemWidget(notif, i);
        targetList->addItem(item);
        targetList->setItemWidget(item, card);
    }
}

void LemmyNotificationsWidget::updateTabBadges()
{
    auto countUnread = [](const QVector<LemmyNotification> &vec) -> int {
        int c = 0;
        for (const auto &n : vec) {
            if (!n.read) ++c;
        }
        return c;
    };

    int allUnread = countUnread(m_allNotifications);

    m_tabWidget->setTabText(0, allUnread > 0
                                ? tr("All (%1)").arg(allUnread)
                                : tr("All"));

    int replyUnread = countUnread(m_replyNotifications);
    m_tabWidget->setTabText(1, replyUnread > 0
                                ? tr("Replies (%1)").arg(replyUnread)
                                : tr("Replies"));

    int mentionUnread = countUnread(m_mentionNotifications);
    m_tabWidget->setTabText(2, mentionUnread > 0
                                ? tr("Mentions (%1)").arg(mentionUnread)
                                : tr("Mentions"));

    int msgUnread = countUnread(m_messageNotifications);
    m_tabWidget->setTabText(3, msgUnread > 0
                                ? tr("Messages (%1)").arg(msgUnread)
                                : tr("Messages"));
}

void LemmyNotificationsWidget::onNotificationActivated(QListWidgetItem *item)
{
    if (!item)
        return;

    bool ok = false;
    int idx = item->data(static_cast<int>(NotifIndexRole)).toInt(&ok);
    if (!ok || idx < 0)
        return;

    int notifTypeInt = item->data(static_cast<int>(NotifTypeRole)).toInt();
    auto notifType = static_cast<LemmyNotification::NotifType>(notifTypeInt);

    const LemmyNotification *notif = nullptr;

    switch (notifType) {
    case LemmyNotification::Reply:
        if (idx < m_replyNotifications.size())
            notif = &m_replyNotifications[idx];
        break;
    case LemmyNotification::Mention:
        if (idx < m_mentionNotifications.size())
            notif = &m_mentionNotifications[idx];
        break;
    case LemmyNotification::PrivateMessage:
        if (idx < m_messageNotifications.size())
            notif = &m_messageNotifications[idx];
        break;
    default:
        break;
    }

    if (!notif)
        return;

    markAsRead(*notif);

    QString typeStr;
    qint64 navId = 0;
    switch (notif->type) {
    case LemmyNotification::Reply:
        typeStr = QStringLiteral("comment_reply");
        navId = notif->commentId > 0 ? notif->commentId : notif->postId;
        break;
    case LemmyNotification::Mention:
        typeStr = QStringLiteral("person_mention");
        navId = notif->commentId > 0 ? notif->commentId : notif->postId;
        break;
    case LemmyNotification::PrivateMessage:
        typeStr = QStringLiteral("private_message");
        navId = notif->id;
        break;
    default:
        navId = notif->id;
        break;
    }

    emit notificationClicked(typeStr, navId);
}

void LemmyNotificationsWidget::onMarkAllReadClicked()
{
    auto markAll = [this](QVector<LemmyNotification> &vec) {
        for (auto &n : vec) {
            if (!n.read) {
                n.read = true;

                QString endpoint;
                switch (n.type) {
                case LemmyNotification::Reply:
                    endpoint = QStringLiteral("/api/v3/comment_reply/mark_as_read");
                    break;
                case LemmyNotification::Mention:
                    endpoint = QStringLiteral("/api/v3/person_mention/mark_as_read");
                    break;
                case LemmyNotification::PrivateMessage:
                    endpoint = QStringLiteral("/api/v3/private_message/mark_as_read");
                    break;
                default:
                    continue;
                }

                QJsonObject body;
                body[QStringLiteral("comment_reply_id")] = static_cast<int>(n.id);
                body[QStringLiteral("read")] = true;

                QUrl url(m_instanceUrl + endpoint);
                QNetworkRequest req(url);
                req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
                if (!m_jwt.isEmpty())
                    req.setRawHeader("Authorization", ("Bearer " + m_jwt).toUtf8());

                QNetworkReply *reply = m_nam->post(req,
                    QJsonDocument(body).toJson(QJsonDocument::Compact));
                connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
            }
        }
    };

    markAll(m_replyNotifications);
    markAll(m_mentionNotifications);
    markAll(m_messageNotifications);

    mergeAndPopulateAllTab();
    onTabChanged(m_tabWidget->currentIndex());
    updateTabBadges();
}

void LemmyNotificationsWidget::markAsRead(const LemmyNotification &notif)
{
    auto setRead = [this](QVector<LemmyNotification> &vec, qint64 id) {
        for (auto &n : vec) {
            if (n.id == id && !n.read) {
                n.read = true;

                QString endpoint;
                switch (n.type) {
                case LemmyNotification::Reply:
                    endpoint = QStringLiteral("/api/v3/comment_reply/mark_as_read");
                    break;
                case LemmyNotification::Mention:
                    endpoint = QStringLiteral("/api/v3/person_mention/mark_as_read");
                    break;
                case LemmyNotification::PrivateMessage:
                    endpoint = QStringLiteral("/api/v3/private_message/mark_as_read");
                    break;
                default:
                    return;
                }

                QJsonObject body;
                body[QStringLiteral("comment_reply_id")] = static_cast<int>(id);
                body[QStringLiteral("person_mention_id")] = static_cast<int>(id);
                body[QStringLiteral("private_message_id")] = static_cast<int>(id);
                body[QStringLiteral("read")] = true;

                QUrl url(m_instanceUrl + endpoint);
                QNetworkRequest req(url);
                req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
                if (!m_jwt.isEmpty())
                    req.setRawHeader("Authorization", ("Bearer " + m_jwt).toUtf8());

                QNetworkReply *reply = m_nam->post(req,
                    QJsonDocument(body).toJson(QJsonDocument::Compact));
                connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
                break;
            }
        }
    };

    setRead(m_replyNotifications, notif.id);
    setRead(m_mentionNotifications, notif.id);
    setRead(m_messageNotifications, notif.id);

    mergeAndPopulateAllTab();
    onTabChanged(m_tabWidget->currentIndex());
    updateTabBadges();
}

QWidget *LemmyNotificationsWidget::createNotificationItemWidget(
    const LemmyNotification &notif, int dataIndex)
{
    Q_UNUSED(dataIndex)

    auto *container = new QWidget();
    container->setStyleSheet(QStringLiteral("background: transparent;"));

    auto *row = new QHBoxLayout(container);
    row->setContentsMargins(12, 8, 12, 8);
    row->setSpacing(10);

    if (!notif.read) {
        auto *unreadDot = new QLabel(container);
        unreadDot->setFixedSize(8, 8);
        unreadDot->setStyleSheet(QStringLiteral(
            "background-color: #89b4fa; border-radius: 4px;"));
        row->addWidget(unreadDot);
    } else {
        auto *spacer = new QWidget(container);
        spacer->setFixedSize(8, 8);
        row->addWidget(spacer);
    }

    QString iconText;
    QColor iconBg;
    switch (notif.type) {
    case LemmyNotification::Reply:
        iconText = QStringLiteral("\xE2\x86\xA9");   // ↩
        iconBg = QColor(0xa6e3a1);
        break;
    case LemmyNotification::Mention:
        iconText = QStringLiteral("@");
        iconBg = QColor(0xfab387);
        break;
    case LemmyNotification::PrivateMessage:
        iconText = QStringLiteral("\xE2\x9C\x89");   // ✉
        iconBg = QColor(0x89b4fa);
        break;
    default:
        iconText = QStringLiteral("?");
        iconBg = QColor(0x6c7086);
        break;
    }

    auto *iconLabel = new QLabel(iconText, container);
    iconLabel->setFixedSize(32, 32);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet(QStringLiteral(
        "background-color: %1; color: #1e1e2e; border-radius: 16px; "
        "font-size: 14px; font-weight: bold;")
        .arg(iconBg.name()));
    row->addWidget(iconLabel);

    QPixmap avatar = makeAvatarPixmap(notif.creatorName, 36, avatarColorFromName(notif.creatorName));
    auto *avatarLabel = new QLabel(container);
    avatarLabel->setPixmap(avatar);
    avatarLabel->setFixedSize(36, 36);
    row->addWidget(avatarLabel);

    auto *textCol = new QVBoxLayout();
    textCol->setSpacing(2);

    auto *headerRow = new QHBoxLayout();
    headerRow->setSpacing(6);

    auto *nameLabel = new QLabel(notif.creatorName, container);
    nameLabel->setStyleSheet(QStringLiteral(
        "color: #cdd6f4; font-weight: bold; font-size: 13px; background: transparent;"));
    if (!notif.read) {
        QFont f = nameLabel->font();
        f.setBold(true);
        nameLabel->setFont(f);
    }
    headerRow->addWidget(nameLabel);

    QString actionText;
    switch (notif.type) {
    case LemmyNotification::Reply:
        actionText = tr("replied to your comment");
        break;
    case LemmyNotification::Mention:
        actionText = tr("mentioned you");
        break;
    case LemmyNotification::PrivateMessage:
        actionText = tr("sent you a message");
        break;
    default:
        actionText = QString();
        break;
    }

    auto *actionLabel = new QLabel(actionText, container);
    actionLabel->setStyleSheet(QStringLiteral(
        "color: #6c7086; font-size: 12px; background: transparent;"));
    headerRow->addWidget(actionLabel);

    headerRow->addStretch();

    QString timeStr = relativeTime(notif.timestampSec);
    auto *timeLabel = new QLabel(timeStr, container);
    timeLabel->setStyleSheet(QStringLiteral(
        "color: #585b70; font-size: 11px; background: transparent;"));
    headerRow->addWidget(timeLabel);

    textCol->addLayout(headerRow);

    QString preview = notif.contentPreview.left(120).replace(QChar('\n'), QChar(' '));
    auto *previewLabel = new QLabel(preview, container);
    previewLabel->setStyleSheet(QStringLiteral(
        "color: #9399b2; font-size: 12px; background: transparent;"));
    previewLabel->setWordWrap(true);
    previewLabel->setMaximumHeight(previewLabel->fontMetrics().lineSpacing() * 2 + 2);
    textCol->addWidget(previewLabel);

    row->addLayout(textCol, 1);

    return container;
}
