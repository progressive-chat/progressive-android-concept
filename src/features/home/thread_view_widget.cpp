#include "features/home/thread_view_widget.hpp"
#include "matrix/matrix_session.hpp"
#include "ui/components/message_bubble.hpp"
#include "ui/components/loading_spinner.hpp"
#include "util/json_util.hpp"

#include <QDateTime>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QScrollBar>
#include <QSet>
#include <QUrl>

ThreadViewWidget::ThreadViewWidget(const QString &rootMessageId,
                                   const QString &roomId,
                                   MatrixSession *session,
                                   QWidget *parent)
    : QWidget(parent)
    , m_rootMessageId(rootMessageId)
    , m_roomId(roomId)
    , m_session(session)
    , m_nam(new QNetworkAccessManager(this))
    , m_rootFrame(nullptr)
    , m_pollTimer(new QTimer(this))
{
    setupUi();

    connect(m_session, &MatrixSession::messageReceived,
            this, &ThreadViewWidget::onSessionMessage);

    m_pollTimer->setInterval(10000);
    m_pollTimer->setSingleShot(false);
    connect(m_pollTimer, &QTimer::timeout,
            this, &ThreadViewWidget::pollForNewReplies);

    setLoading(true);
    fetchRootMessage();
}

void ThreadViewWidget::setupUi()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    // -- Header bar --
    auto *headerBar = new QWidget(this);
    m_headerLayout = new QHBoxLayout(headerBar);
    m_headerLayout->setContentsMargins(8, 6, 8, 6);
    m_headerLayout->setSpacing(8);

    m_backButton = new QPushButton(QStringLiteral("\u2190 Back"), headerBar);
    m_backButton->setFlat(true);
    m_backButton->setStyleSheet(QStringLiteral(
        "QPushButton { color: #0084FF; font-size: 13px; border: none; padding: 4px 8px; }"
        "QPushButton:hover { background: #E8F0FE; border-radius: 4px; }"));
    connect(m_backButton, &QPushButton::clicked, this, &ThreadViewWidget::backRequested);

    m_replyCountLabel = new QLabel(headerBar);
    m_replyCountLabel->setStyleSheet(QStringLiteral(
        "QLabel { color: #555; font-size: 14px; font-weight: bold; }"));

    m_viewInRoomButton = new QPushButton(QStringLiteral("View in room"), headerBar);
    m_viewInRoomButton->setFlat(true);
    m_viewInRoomButton->setStyleSheet(QStringLiteral(
        "QPushButton { color: #0084FF; font-size: 12px; border: 1px solid #0084FF; "
        "border-radius: 4px; padding: 4px 10px; }"
        "QPushButton:hover { background: #0084FF; color: white; }"));
    connect(m_viewInRoomButton, &QPushButton::clicked, this, &ThreadViewWidget::backRequested);

    m_headerLayout->addWidget(m_backButton);
    m_headerLayout->addWidget(m_replyCountLabel, 1);
    m_headerLayout->addWidget(m_viewInRoomButton);
    m_mainLayout->addWidget(headerBar);

    // -- Separator --
    auto *separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet(QStringLiteral("QFrame { color: #ddd; }"));
    m_mainLayout->addWidget(separator);

    // -- Spinner (centered overlay) --
    m_spinner = new LoadingSpinner(36, QColor(0x00, 0x84, 0xFF), this);
    m_spinner->setVisible(false);

    // -- Root message container --
    m_rootFrame = new QFrame(this);
    m_rootFrame->setStyleSheet(QStringLiteral(
        "QFrame#rootFrame { background: #F0F4FF; border-bottom: 1px solid #D0D8EE; }"));
    m_rootFrame->setObjectName(QStringLiteral("rootFrame"));
    m_rootLayout = new QVBoxLayout(m_rootFrame);
    m_rootLayout->setContentsMargins(8, 8, 8, 8);
    m_rootLayout->setSpacing(0);
    m_mainLayout->addWidget(m_rootFrame);

    // -- Replies scroll area --
    m_repliesScroll = new QScrollArea(this);
    m_repliesScroll->setWidgetResizable(true);
    m_repliesScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_repliesScroll->setStyleSheet(QStringLiteral(
        "QScrollArea { border: none; background: white; }"));
    m_repliesContent = new QWidget(m_repliesScroll);
    m_repliesLayout = new QVBoxLayout(m_repliesContent);
    m_repliesLayout->setContentsMargins(4, 4, 4, 4);
    m_repliesLayout->setSpacing(2);
    m_repliesLayout->addStretch();
    m_repliesScroll->setWidget(m_repliesContent);
    m_mainLayout->addWidget(m_repliesScroll, 1);

    // -- Reply input bar --
    auto *inputBar = new QWidget(this);
    auto *inputLayout = new QHBoxLayout(inputBar);
    inputLayout->setContentsMargins(8, 6, 8, 8);
    inputLayout->setSpacing(8);

    m_replyInput = new QTextEdit(inputBar);
    m_replyInput->setPlaceholderText(QStringLiteral("Reply to thread..."));
    m_replyInput->setMaximumHeight(80);
    m_replyInput->setStyleSheet(QStringLiteral(
        "QTextEdit { border: 1px solid #ccc; border-radius: 6px; "
        "padding: 6px 8px; font-size: 13px; background: #FAFAFA; }"
        "QTextEdit:focus { border-color: #0084FF; background: white; }"));
    connect(m_replyInput, &QTextEdit::textChanged, this, [this]() {
        m_sendButton->setEnabled(!m_replyInput->toPlainText().trimmed().isEmpty());
    });

    m_sendButton = new QPushButton(QStringLiteral("Send"), inputBar);
    m_sendButton->setEnabled(false);
    m_sendButton->setStyleSheet(QStringLiteral(
        "QPushButton { background: #0084FF; color: white; border: none; "
        "border-radius: 6px; padding: 8px 16px; font-size: 13px; font-weight: bold; }"
        "QPushButton:hover { background: #0073E6; }"
        "QPushButton:disabled { background: #B0D0F0; }"));
    connect(m_sendButton, &QPushButton::clicked, this, &ThreadViewWidget::onSendReply);

    inputLayout->addWidget(m_replyInput, 1);
    inputLayout->addWidget(m_sendButton);
    m_mainLayout->addWidget(inputBar);
}

QNetworkRequest ThreadViewWidget::makeRequest(const QString &path) const
{
    QUrl url(m_session->homeserverUrl() + path);
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    req.setRawHeader("Authorization", ("Bearer " + m_session->accessToken()).toUtf8());
    return req;
}

QString ThreadViewWidget::generateTxnId()
{
    return QStringLiteral("threadtxn%1_%2")
        .arg(QDateTime::currentMSecsSinceEpoch())
        .arg(++m_txnCounter);
}

ProtocolMessage ThreadViewWidget::parseEvent(const QJsonObject &event) const
{
    ProtocolMessage msg;
    msg.roomId = m_roomId;
    msg.protocolType = ProtocolType::MATRIX;
    msg.timestamp = JsonUtil::optLong(event, QStringLiteral("origin_server_ts"));
    msg.senderId = JsonUtil::optString(event, QStringLiteral("sender"));
    msg.id = static_cast<qint64>(msg.senderId.toUtf8().size()) | (msg.timestamp << 16);

    QJsonObject content = JsonUtil::optObject(event, QStringLiteral("content"));
    msg.metadata = content;
    msg.text = JsonUtil::optString(content, QStringLiteral("body"));
    QString formatted = JsonUtil::optString(content, QStringLiteral("formatted_body"));
    if (!formatted.isEmpty())
        msg.text = formatted;

    QString eventType = JsonUtil::optString(event, QStringLiteral("type"));
    if (eventType == QStringLiteral("m.room.message"))
        msg.contentType = ProtocolContentType::TEXT;
    else if (eventType == QStringLiteral("m.sticker"))
        msg.contentType = ProtocolContentType::STICKER;
    else
        msg.contentType = ProtocolContentType::TEXT;

    return msg;
}

// ---------------------------------------------------------------------------
// Fetching
// ---------------------------------------------------------------------------

void ThreadViewWidget::fetchRootMessage()
{
    QString path = QStringLiteral("/_matrix/client/r0/rooms/%1/event/%2")
                       .arg(m_roomId, m_rootMessageId);

    QNetworkReply *reply = m_nam->get(makeRequest(path));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError)
            return;

        QJsonObject json = JsonUtil::parseObject(QString::fromUtf8(reply->readAll()));
        m_rootEvent = json;

        ProtocolMessage msg = parseEvent(m_rootEvent);
        renderRootMessage(msg);

        fetchThreadReplies();
    });
}

void ThreadViewWidget::fetchThreadReplies()
{
    QString path = QStringLiteral(
        "/_matrix/client/r0/rooms/%1/relations/%2/m.thread?dir=b")
        .arg(m_roomId, m_rootMessageId);

    if (!m_lastFetchedEventId.isEmpty())
        path += QStringLiteral("&from=") + m_lastFetchedEventId;

    QNetworkReply *reply = m_nam->get(makeRequest(path));
    connect(reply, &QNetworkReply::finished, this, &ThreadViewWidget::onRepliesFetched,
            Qt::UniqueConnection);
    reply->setProperty("isPoll", !m_pollTimer->isActive());
}

void ThreadViewWidget::onRootFetched()
{
    // Handled inline in lambda for simplicity
}

void ThreadViewWidget::onRepliesFetched()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply)
        return;

    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        setLoading(false);
        m_pollTimer->start();
        return;
    }

    QJsonObject json = JsonUtil::parseObject(QString::fromUtf8(reply->readAll()));
    QJsonArray chunk = JsonUtil::optArray(json, QStringLiteral("chunk"));

    bool isPoll = reply->property("isPoll").toBool();

    for (const auto &ev : chunk) {
        QJsonObject evObj = ev.toObject();

        QString dedupKey = JsonUtil::optString(evObj, QStringLiteral("event_id"));
        if (dedupKey.isEmpty())
            dedupKey = JsonUtil::optString(evObj, QStringLiteral("sender"))
                       + QStringLiteral("|")
                       + QString::number(JsonUtil::optLong(evObj, QStringLiteral("origin_server_ts")));

        if (m_seenReplies.contains(dedupKey))
            continue;

        m_seenReplies.insert(dedupKey);

        m_replyEvents.append(evObj);

        if (!isPoll) {
            ProtocolMessage msg = parseEvent(evObj);
            addReplyBubble(msg);
        }
    }

    if (!chunk.isEmpty()) {
        QJsonObject lastEv = chunk.last().toObject();
        m_lastFetchedEventId = JsonUtil::optString(lastEv, QStringLiteral("event_id"));
    }

    updateReplyCount();
    setLoading(false);
    m_pollTimer->start();

    // Scroll to bottom on first fetch
    if (!isPoll && !m_replyEvents.isEmpty()) {
        auto *sb = m_repliesScroll->verticalScrollBar();
        sb->setValue(sb->maximum());
    }
}

// ---------------------------------------------------------------------------
// Render
// ---------------------------------------------------------------------------

void ThreadViewWidget::renderRootMessage(const ProtocolMessage &msg)
{
    auto *bubble = new MessageBubble(msg, m_rootFrame);
    bubble->setSentByMe(msg.senderId == m_session->userId());
    m_rootLayout->addWidget(bubble);
}

void ThreadViewWidget::addReplyBubble(const ProtocolMessage &msg)
{
    // Remove stretch before adding
    if (m_repliesLayout->count() > 0) {
        auto *lastItem = m_repliesLayout->itemAt(m_repliesLayout->count() - 1);
        if (lastItem && lastItem->spacerItem())
            m_repliesLayout->removeItem(lastItem);
    }

    auto *wrapper = new QWidget(m_repliesContent);
    auto *wrapperLayout = new QHBoxLayout(wrapper);
    wrapperLayout->setContentsMargins(16, 2, 4, 2);
    wrapperLayout->setSpacing(0);

    // Small thread-line indicator
    auto *threadLine = new QFrame(wrapper);
    threadLine->setFixedWidth(3);
    threadLine->setStyleSheet(QStringLiteral(
        "QFrame { background: #D0D8EE; border-radius: 2px; }"));
    wrapperLayout->addWidget(threadLine);
    wrapperLayout->addSpacing(8);

    auto *bubble = new MessageBubble(msg, wrapper);
    bubble->setSentByMe(msg.senderId == m_session->userId());
    wrapperLayout->addWidget(bubble, 1);

    m_repliesLayout->addWidget(wrapper);
    m_repliesLayout->addStretch();

    ++m_replyCount;
    updateReplyCount();
}

void ThreadViewWidget::updateReplyCount()
{
    int count = m_replyCount;
    m_replyCountLabel->setText(
        count == 0
            ? QStringLiteral("No replies")
            : count == 1
                ? QStringLiteral("1 reply")
                : QStringLiteral("%1 replies").arg(count));
}

void ThreadViewWidget::setLoading(bool loading)
{
    m_spinner->setVisible(loading);
    if (loading)
        m_spinner->start();
    else
        m_spinner->stop();
}

// ---------------------------------------------------------------------------
// Sending
// ---------------------------------------------------------------------------

void ThreadViewWidget::onSendReply()
{
    QString text = m_replyInput->toPlainText().trimmed();
    if (text.isEmpty())
        return;

    m_sendButton->setEnabled(false);
    m_replyInput->setEnabled(false);

    QJsonObject relatesTo;
    relatesTo[QStringLiteral("event_id")] = m_rootMessageId;
    relatesTo[QStringLiteral("rel_type")] = QStringLiteral("m.thread");

    QJsonObject content;
    content[QStringLiteral("msgtype")] = QStringLiteral("m.text");
    content[QStringLiteral("body")] = text;
    content[QStringLiteral("m.relates_to")] = relatesTo;

    QJsonObject body;
    body[QStringLiteral("content")] = content;
    body[QStringLiteral("type")] = QStringLiteral("m.room.message");

    QString txnId = generateTxnId();
    QString path = QStringLiteral("/_matrix/client/r0/rooms/%1/send/m.room.message/%2")
                       .arg(m_roomId, txnId);

    QNetworkReply *reply = m_nam->put(
        makeRequest(path), QJsonDocument(body).toJson(QJsonDocument::Compact));

    connect(reply, &QNetworkReply::finished, this, &ThreadViewWidget::onReplySent);
}

void ThreadViewWidget::onReplySent()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply)
        return;

    reply->deleteLater();

    m_replyInput->setEnabled(true);
    m_replyInput->clear();
    m_sendButton->setEnabled(false);

    if (reply->error() == QNetworkReply::NoError) {
        emit replySent(m_rootMessageId);
        // Trigger a poll to fetch the new reply we just sent
        pollForNewReplies();
    }
}

// ---------------------------------------------------------------------------
// Live updates
// ---------------------------------------------------------------------------

void ThreadViewWidget::onSessionMessage(const ProtocolMessage &msg)
{
    if (msg.roomId != m_roomId)
        return;

    QJsonObject relatesTo = JsonUtil::optObject(msg.metadata, QStringLiteral("m.relates_to"));
    if (relatesTo.isEmpty())
        return;

    QString relType = JsonUtil::optString(relatesTo, QStringLiteral("rel_type"));
    QString linkedEventId = JsonUtil::optString(relatesTo, QStringLiteral("event_id"));

    if (relType != QStringLiteral("m.thread") || linkedEventId != m_rootMessageId)
        return;

    QString dedupKey = msg.senderId + QStringLiteral("|") + QString::number(msg.timestamp);
    if (m_seenReplies.contains(dedupKey))
        return;

    m_seenReplies.insert(dedupKey);
    addReplyBubble(msg);
}

void ThreadViewWidget::pollForNewReplies()
{
    fetchThreadReplies();
}
