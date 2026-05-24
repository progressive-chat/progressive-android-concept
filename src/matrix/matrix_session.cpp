#include "matrix/matrix_session.hpp"

#include <QDateTime>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSet>
#include <QUrl>

#include "util/json_util.hpp"
#include "util/string_util.hpp"

MatrixSession::MatrixSession(const QString &userId,
                             const QString &accessToken,
                             const QString &homeserverUrl,
                             QObject *parent)
    : IProtocolSession()
    , m_userId(userId)
    , m_password(accessToken)
    , m_homeserverUrl(homeserverUrl)
    , m_nam(new QNetworkAccessManager(this))
    , m_syncTimer(new QTimer(this))
{
    setParent(parent);
    m_syncTimer->setSingleShot(true);
    connect(m_syncTimer, &QTimer::timeout, this, &MatrixSession::doSync);
}

MatrixSession::~MatrixSession()
{
    close();
}

QNetworkRequest MatrixSession::makeRequest(const QString &path) const
{
    QUrl url(m_homeserverUrl + path);
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    if (!m_accessToken.isEmpty()) {
        req.setRawHeader("Authorization", ("Bearer " + m_accessToken).toUtf8());
    }
    return req;
}

void MatrixSession::setState(ConnectionState state)
{
    if (m_state != state) {
        m_state = state;
        emit connectionStateChanged(ProtocolType::MATRIX, state);
    }
}

QString MatrixSession::generateTxnId()
{
    return QStringLiteral("txn%1_%2")
        .arg(QString::number(QDateTime::currentMSecsSinceEpoch()))
        .arg(QString::number(++m_txnCounter));
}

void MatrixSession::open()
{
    if (m_state == ConnectionState::CONNECTED || m_state == ConnectionState::CONNECTING)
        return;

    setState(ConnectionState::CONNECTING);

    if (!m_password.isEmpty()) {
        QJsonObject body;
        body[QStringLiteral("type")] = QStringLiteral("m.login.password");
        body[QStringLiteral("user")] = m_userId;
        body[QStringLiteral("password")] = m_password;

        QNetworkReply *reply = m_nam->post(
            makeRequest(QStringLiteral("/_matrix/client/r0/login")),
            QJsonDocument(body).toJson(QJsonDocument::Compact));

        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            reply->deleteLater();

            if (reply->error() != QNetworkReply::NoError) {
                emit errorOccurred(tr("Login failed: %1").arg(reply->errorString()));
                setState(ConnectionState::ERROR);
                return;
            }

            QJsonObject resp = JsonUtil::parseObject(QString::fromUtf8(reply->readAll()));
            m_accessToken = JsonUtil::optString(resp, QStringLiteral("access_token"));
            m_userId = JsonUtil::optString(resp, QStringLiteral("user_id"));
            m_homeserverUrl = JsonUtil::optString(resp, QStringLiteral("home_server"), m_homeserverUrl);
            m_displayName = JsonUtil::optString(resp, QStringLiteral("display_name"));

            setState(ConnectionState::CONNECTED);
            startSyncLoop();
        });
    } else {
        setState(ConnectionState::CONNECTED);
        startSyncLoop();
    }
}

void MatrixSession::close()
{
    m_syncActive = false;
    m_syncTimer->stop();
    m_nextBatch.clear();
    setState(ConnectionState::DISCONNECTED);
}

void MatrixSession::startSyncLoop()
{
    m_syncActive = true;
    doSync();
}

void MatrixSession::doSync()
{
    if (!m_syncActive)
        return;

    QString path = QStringLiteral("/_matrix/client/r0/sync?timeout=%1").arg(m_syncTimeoutMs);
    if (!m_nextBatch.isEmpty()) {
        path += QStringLiteral("&since=") + m_nextBatch;
    }
    path += QStringLiteral("&filter=") + QString::fromUtf8(
        QUrl::toPercentEncoding(QStringLiteral("{\"room\":{\"timeline\":{\"limit\":50}}}")));

    QNetworkReply *reply = m_nam->get(makeRequest(path));

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        if (!m_syncActive)
            return;

        if (reply->error() != QNetworkReply::NoError) {
            if (reply->error() == QNetworkReply::AuthenticationRequiredError) {
                setState(ConnectionState::ERROR);
                emit errorOccurred(tr("Authentication failed — token may have expired"));
                return;
            }
            m_syncTimer->start(5000);
            return;
        }

        QJsonObject json = JsonUtil::parseObject(QString::fromUtf8(reply->readAll()));
        m_nextBatch = JsonUtil::optString(json, QStringLiteral("next_batch"), m_nextBatch);
        processSyncResponse(json);

        m_syncTimer->start(0);
    });
}

void MatrixSession::processSyncResponse(const QJsonObject &json)
{
    QJsonObject roomsObj = JsonUtil::optObject(json, QStringLiteral("rooms"));
    QJsonObject joinedObj = JsonUtil::optObject(roomsObj, QStringLiteral("join"));
    processJoinedRooms(joinedObj);

    QJsonObject invitedObj = JsonUtil::optObject(roomsObj, QStringLiteral("invite"));
    for (auto it = invitedObj.begin(); it != invitedObj.end(); ++it) {
        QJsonObject roomObj = it.value().toObject();
        QJsonObject inviteState = JsonUtil::optObject(roomObj, QStringLiteral("invite_state"));
        QJsonArray events = JsonUtil::optArray(inviteState, QStringLiteral("events"));
        for (const auto &ev : events) {
            QJsonObject evObj = ev.toObject();
            QString type = JsonUtil::optString(evObj, QStringLiteral("type"));
            if (type == QStringLiteral("m.room.member") || type == QStringLiteral("m.room.name")) {
                ProtocolRoom room = parseRoomInfo(it.key(), roomObj);
                if (!m_rooms.contains(room.id)) {
                    m_rooms.insert(room.id, room);
                    emit roomUpdated(room);
                }
                break;
            }
        }
    }
}

void MatrixSession::processJoinedRooms(const QJsonObject &joinedObj)
{
    for (auto it = joinedObj.begin(); it != joinedObj.end(); ++it) {
        const QString &roomId = it.key();
        QJsonObject roomObj = it.value().toObject();

        ProtocolRoom room = parseRoomInfo(roomId, roomObj);
        if (!m_rooms.contains(roomId)) {
            m_rooms.insert(roomId, room);
        } else {
            ProtocolRoom &existing = m_rooms[roomId];
            existing.name = room.name;
            existing.topic = room.topic;
            existing.avatarUrl = room.avatarUrl;
            existing.unreadCount = room.unreadCount;
            existing.memberCount = room.memberCount;
            existing.isEncrypted = room.isEncrypted;
        }
        emit roomUpdated(m_rooms[roomId]);

        QJsonObject timeline = JsonUtil::optObject(roomObj, QStringLiteral("timeline"));
        QJsonArray events = JsonUtil::optArray(timeline, QStringLiteral("events"));
        processTimelineEvents(roomId, events);

        QJsonObject ephemeral = JsonUtil::optObject(roomObj, QStringLiteral("ephemeral"));
        QJsonArray ephemeralEvents = JsonUtil::optArray(ephemeral, QStringLiteral("events"));
        processRoomEvents(roomId, ephemeralEvents);

        QJsonObject accountData = JsonUtil::optObject(roomObj, QStringLiteral("account_data"));
        QJsonArray accountEvents = JsonUtil::optArray(accountData, QStringLiteral("events"));
        processRoomEvents(roomId, accountEvents);
    }
}

void MatrixSession::processRoomEvents(const QString &roomId, const QJsonArray &events)
{
    auto &cache = m_messages[roomId];
    for (const auto &ev : events) {
        QJsonObject evObj = ev.toObject();
        QString type = JsonUtil::optString(evObj, QStringLiteral("type"));
        if (type.startsWith(QStringLiteral("m."))) {
            ProtocolMessage msg = parseMessageEvent(roomId, evObj);
            if (!msg.text.isEmpty() || msg.contentType != ProtocolContentType::TEXT) {
                cache.append(msg);
                if (cache.size() > 500)
                    cache.removeFirst();
                if (!m_rooms.contains(roomId)) {
                    ProtocolRoom room;
                    room.id = roomId;
                    m_rooms.insert(roomId, room);
                }
                m_rooms[roomId].lastMessageTimestamp = msg.timestamp;
                m_rooms[roomId].lastMessagePreview = msg.text.left(120);
                emit messageReceived(msg);
            }
        }
    }
}

void MatrixSession::processTimelineEvents(const QString &roomId, const QJsonArray &events)
{
    processRoomEvents(roomId, events);
}

ProtocolMessage MatrixSession::parseMessageEvent(const QString &roomId, const QJsonObject &event)
{
    ProtocolMessage msg;
    msg.roomId = roomId;
    msg.protocolType = ProtocolType::MATRIX;
    msg.timestamp = JsonUtil::optLong(event, QStringLiteral("origin_server_ts"));
    msg.senderId = JsonUtil::optString(event, QStringLiteral("sender"));
    msg.id = static_cast<qint64>(JsonUtil::optString(event, QStringLiteral("event_id")).toUtf8().size())
             | (msg.timestamp << 16);

    QString eventType = JsonUtil::optString(event, QStringLiteral("type"));

    QJsonObject content = JsonUtil::optObject(event, QStringLiteral("content"));
    msg.metadata = content;

    if (eventType == QStringLiteral("m.room.message")) {
        QString msgType = JsonUtil::optString(content, QStringLiteral("msgtype"));
        msg.text = JsonUtil::optString(content, QStringLiteral("body"));
        QString formatted = JsonUtil::optString(content, QStringLiteral("formatted_body"));

        if (msgType == QStringLiteral("m.text"))         msg.contentType = ProtocolContentType::TEXT;
        else if (msgType == QStringLiteral("m.emote"))   msg.contentType = ProtocolContentType::ACTION;
        else if (msgType == QStringLiteral("m.notice"))  msg.contentType = ProtocolContentType::NOTICE;
        else if (msgType == QStringLiteral("m.image"))   msg.contentType = ProtocolContentType::IMAGE;
        else if (msgType == QStringLiteral("m.file"))    msg.contentType = ProtocolContentType::FILE;
        else if (msgType == QStringLiteral("m.audio"))   msg.contentType = ProtocolContentType::AUDIO;
        else if (msgType == QStringLiteral("m.video"))   msg.contentType = ProtocolContentType::VIDEO;
        else if (msgType == QStringLiteral("m.location")) msg.contentType = ProtocolContentType::LOCATION;
        else msg.contentType = ProtocolContentType::TEXT;

        if (!formatted.isEmpty() && msg.contentType == ProtocolContentType::TEXT) {
            msg.text = formatted;
        }
    } else if (eventType == QStringLiteral("m.room.member")) {
        QString membership = JsonUtil::optString(content, QStringLiteral("membership"));
        msg.text = JsonUtil::optString(content, QStringLiteral("displayname"), msg.senderId);
        if (membership == QStringLiteral("join")) {
            msg.contentType = ProtocolContentType::JOIN;
            msg.text = msg.text + QStringLiteral(" joined");
        } else if (membership == QStringLiteral("leave")) {
            msg.contentType = ProtocolContentType::PART;
            msg.text = msg.text + QStringLiteral(" left");
        } else if (membership == QStringLiteral("invite")) {
            msg.contentType = ProtocolContentType::SYSTEM;
            msg.text = QStringLiteral("Invited ") + msg.text;
        } else if (membership == QStringLiteral("ban")) {
            msg.contentType = ProtocolContentType::BAN;
            msg.text = msg.text + QStringLiteral(" was banned");
        } else {
            msg.contentType = ProtocolContentType::SYSTEM;
        }
    } else if (eventType == QStringLiteral("m.room.topic")) {
        msg.text = QStringLiteral("Topic changed: ") + JsonUtil::optString(content, QStringLiteral("topic"));
        msg.contentType = ProtocolContentType::TOPIC;
    } else if (eventType == QStringLiteral("m.room.name")) {
        msg.text = QStringLiteral("Room renamed: ") + JsonUtil::optString(content, QStringLiteral("name"));
        msg.contentType = ProtocolContentType::SYSTEM;
    } else if (eventType == QStringLiteral("m.room.encrypted")) {
        msg.contentType = ProtocolContentType::SYSTEM;
        msg.isEncrypted = true;
        msg.text = QStringLiteral("Encrypted message");
    } else if (eventType == QStringLiteral("m.sticker")) {
        msg.text = JsonUtil::optString(content, QStringLiteral("body"),
                                        QStringLiteral("Sticker"));
        msg.contentType = ProtocolContentType::STICKER;
    } else if (eventType == QStringLiteral("m.reaction")) {
        QString relatesTo = JsonUtil::optString(
            JsonUtil::optObject(content, QStringLiteral("m.relates_to")),
            QStringLiteral("key"));
        msg.text = QStringLiteral("Reacted with ") + relatesTo;
        msg.contentType = ProtocolContentType::EMOJI;
    } else {
        msg.text = JsonUtil::optString(content, QStringLiteral("body"));
        msg.contentType = ProtocolContentType::SYSTEM;
    }

    return msg;
}

ProtocolRoom MatrixSession::parseRoomInfo(const QString &roomId, const QJsonObject &roomObj)
{
    ProtocolRoom room;
    room.id = roomId;
    room.protocolType = ProtocolType::MATRIX;

    QJsonObject state = JsonUtil::optObject(roomObj, QStringLiteral("state"));
    QJsonArray stateEvents = JsonUtil::optArray(state, QStringLiteral("events"));

    for (const auto &ev : stateEvents) {
        QJsonObject evObj = ev.toObject();
        QString type = JsonUtil::optString(evObj, QStringLiteral("type"));
        QJsonObject content = JsonUtil::optObject(evObj, QStringLiteral("content"));

        if (type == QStringLiteral("m.room.name")) {
            room.name = JsonUtil::optString(content, QStringLiteral("name"));
        } else if (type == QStringLiteral("m.room.topic")) {
            room.topic = JsonUtil::optString(content, QStringLiteral("topic"));
        } else if (type == QStringLiteral("m.room.avatar")) {
            room.avatarUrl = JsonUtil::optString(content, QStringLiteral("url"));
        } else if (type == QStringLiteral("m.room.member")) {
            room.memberCount++;
        } else if (type == QStringLiteral("m.room.join_rules")) {
            room.isPublic = JsonUtil::optString(content, QStringLiteral("join_rule"))
                                != QStringLiteral("invite");
        } else if (type == QStringLiteral("m.room.encryption")) {
            room.isEncrypted = true;
        }
    }

    QJsonObject summary = JsonUtil::optObject(roomObj, QStringLiteral("summary"));
    if (!summary.isEmpty()) {
        room.memberCount = JsonUtil::optInt(summary, QStringLiteral("m.joined_member_count"));
        room.memberCount += JsonUtil::optInt(summary, QStringLiteral("m.invited_member_count"));
    }

    QJsonObject unread = JsonUtil::optObject(roomObj, QStringLiteral("unread_notifications"));
    room.unreadCount = JsonUtil::optInt(unread, QStringLiteral("notification_count"));
    room.unreadCount += JsonUtil::optInt(unread, QStringLiteral("highlight_count"));

    QJsonObject timeline = JsonUtil::optObject(roomObj, QStringLiteral("timeline"));
    QJsonArray events = JsonUtil::optArray(timeline, QStringLiteral("events"));
    if (!events.isEmpty()) {
        QJsonObject lastEvent = events.last().toObject();
        room.lastMessageTimestamp = JsonUtil::optLong(lastEvent, QStringLiteral("origin_server_ts"));
        QJsonObject lastContent = JsonUtil::optObject(lastEvent, QStringLiteral("content"));
        room.lastMessagePreview = JsonUtil::optString(lastContent, QStringLiteral("body")).left(120);
    }

    if (room.name.isEmpty()) {
        room.name = roomId;
    }

    return room;
}

void MatrixSession::sendMessage(const QString &roomId, const QString &text, ProtocolContentType contentType)
{
    if (m_state != ConnectionState::CONNECTED)
        return;

    QString matrixMsgType;
    switch (contentType) {
    case ProtocolContentType::TEXT:     matrixMsgType = QStringLiteral("m.text"); break;
    case ProtocolContentType::ACTION:   matrixMsgType = QStringLiteral("m.emote"); break;
    case ProtocolContentType::NOTICE:   matrixMsgType = QStringLiteral("m.notice"); break;
    case ProtocolContentType::IMAGE:    matrixMsgType = QStringLiteral("m.image"); break;
    case ProtocolContentType::FILE:     matrixMsgType = QStringLiteral("m.file"); break;
    case ProtocolContentType::AUDIO:    matrixMsgType = QStringLiteral("m.audio"); break;
    case ProtocolContentType::VIDEO:    matrixMsgType = QStringLiteral("m.video"); break;
    default:                            matrixMsgType = QStringLiteral("m.text"); break;
    }

    QJsonObject content;
    content[QStringLiteral("msgtype")] = matrixMsgType;
    content[QStringLiteral("body")] = text;

    QJsonObject body;
    body[QStringLiteral("content")] = content;
    body[QStringLiteral("type")] = QStringLiteral("m.room.message");

    QString txnId = generateTxnId();
    QString path = QStringLiteral("/_matrix/client/r0/rooms/%1/send/m.room.message/%2")
                       .arg(roomId, txnId);

    QNetworkReply *reply = m_nam->put(
        makeRequest(path),
        QJsonDocument(body).toJson(QJsonDocument::Compact));

    connect(reply, &QNetworkReply::finished, this, [this, reply, roomId]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred(tr("Send failed: %1").arg(reply->errorString()));
        }
    });
}

QVector<ProtocolRoom> MatrixSession::getRooms()
{
    QVector<ProtocolRoom> result;
    result.reserve(m_rooms.size());
    for (auto it = m_rooms.begin(); it != m_rooms.end(); ++it)
        result.append(it.value());
    return result;
}

QVector<ProtocolMessage> MatrixSession::getMessages(const QString &roomId, int limit)
{
    if (m_state != ConnectionState::CONNECTED)
        return {};

    QNetworkReply *reply = m_nam->get(
        makeRequest(QStringLiteral("/_matrix/client/r0/rooms/%1/messages?dir=b&limit=%2")
                        .arg(roomId)
                        .arg(limit)));

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QVector<ProtocolMessage> result;
    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return result;
    }

    QJsonObject json = JsonUtil::parseObject(QString::fromUtf8(reply->readAll()));
    reply->deleteLater();

    QJsonArray chunk = JsonUtil::optArray(json, QStringLiteral("chunk"));
    for (const auto &ev : chunk) {
        QJsonObject evObj = ev.toObject();
        ProtocolMessage msg = parseMessageEvent(roomId, evObj);
        if (!msg.senderId.isEmpty())
            result.append(msg);
    }

    if (!m_messages.contains(roomId))
        m_messages.insert(roomId, result);
    else
        m_messages[roomId] = result;

    return result;
}

void MatrixSession::markAsRead(const QString &roomId)
{
    if (m_state != ConnectionState::CONNECTED)
        return;

    m_rooms[roomId].unreadCount = 0;

    QJsonObject body;
    body[QStringLiteral("m.fully_read")] = QJsonObject{{QStringLiteral("event_id"),
                                                         QJsonValue::Null}};

    QNetworkReply *reply = m_nam->put(
        makeRequest(QStringLiteral("/_matrix/client/r0/rooms/%1/read_markers").arg(roomId)),
        QJsonDocument(body).toJson(QJsonDocument::Compact));

    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
}

void MatrixSession::joinRoom(const QString &roomIdOrAlias)
{
    if (m_state != ConnectionState::CONNECTED)
        return;

    QString encoded = QString::fromUtf8(QUrl::toPercentEncoding(roomIdOrAlias));
    QNetworkReply *reply = m_nam->post(
        makeRequest(QStringLiteral("/_matrix/client/r0/join/%1").arg(encoded)),
        QJsonDocument(QJsonObject()).toJson(QJsonDocument::Compact));

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred(tr("Join failed: %1").arg(reply->errorString()));
            return;
        }
        QJsonObject json = JsonUtil::parseObject(QString::fromUtf8(reply->readAll()));
        QString roomId = JsonUtil::optString(json, QStringLiteral("room_id"));
        if (!roomId.isEmpty() && !m_rooms.contains(roomId)) {
            ProtocolRoom room;
            room.id = roomId;
            m_rooms.insert(roomId, room);
            emit roomUpdated(room);
        }
    });
}

void MatrixSession::leaveRoom(const QString &roomId)
{
    if (m_state != ConnectionState::CONNECTED)
        return;

    QNetworkReply *reply = m_nam->post(
        makeRequest(QStringLiteral("/_matrix/client/r0/rooms/%1/leave").arg(roomId)),
        QJsonDocument(QJsonObject()).toJson(QJsonDocument::Compact));

    connect(reply, &QNetworkReply::finished, this, [this, reply, roomId]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred(tr("Leave failed: %1").arg(reply->errorString()));
            return;
        }
        m_rooms.remove(roomId);
        m_messages.remove(roomId);
    });
}

QVector<ProtocolRoom> MatrixSession::searchRooms(const QString &query)
{
    if (m_state != ConnectionState::CONNECTED || query.isEmpty())
        return {};

    QJsonObject filter;
    filter[QStringLiteral("generic_search_term")] = query;

    QJsonObject body;
    body[QStringLiteral("search_categories")] = QJsonObject{
        {QStringLiteral("room_events"), QJsonObject{
            {QStringLiteral("filter"), filter},
            {QStringLiteral("order_by"), QStringLiteral("recent")},
        }}};

    QNetworkReply *reply = m_nam->post(
        makeRequest(QStringLiteral("/_matrix/client/r0/search")),
        QJsonDocument(body).toJson(QJsonDocument::Compact));

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QVector<ProtocolRoom> result;
    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return result;
    }

    QJsonObject json = JsonUtil::parseObject(QString::fromUtf8(reply->readAll()));
    reply->deleteLater();

    QJsonObject categories = JsonUtil::optObject(json, QStringLiteral("search_categories"));
    QJsonObject roomEvents = JsonUtil::optObject(categories, QStringLiteral("room_events"));
    QJsonArray results = JsonUtil::optArray(roomEvents, QStringLiteral("results"));

    QSet<QString> seenRooms;
    for (const auto &r : results) {
        QJsonObject resultObj = r.toObject();
        QJsonObject resultContent = JsonUtil::optObject(resultObj, QStringLiteral("result"));
        QString roomId = JsonUtil::optString(resultContent, QStringLiteral("room_id"));
        if (!roomId.isEmpty() && !seenRooms.contains(roomId)) {
            seenRooms.insert(roomId);
            if (m_rooms.contains(roomId))
                result.append(m_rooms[roomId]);
        }
    }

    return result;
}
