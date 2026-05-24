#include "lemmy/lemmy_session.hpp"
#include "lemmy/lemmy_client.hpp"
#include "lemmy/lemmy_room.hpp"
#include "lemmy/lemmy_message.hpp"

#include <QJsonArray>
#include <QJsonObject>

LemmySession::LemmySession(const QString &instanceUrl,
                           const QString &username,
                           const QString &password,
                           QObject *parent)
    : IProtocolSession()
    , m_client(new LemmyClient(instanceUrl, this))
    , m_instanceUrl(instanceUrl)
    , m_username(username)
    , m_password(password)
{
    setParent(parent);
    connect(m_client, &LemmyClient::loginSuccess, this, [this](const QString &) {
        setState(ConnectionState::CONNECTED);
        refreshCommunities();
    });
    connect(m_client, &LemmyClient::loginFailed, this, [this](const QString &error) {
        setState(ConnectionState::ERROR);
        emit errorOccurred(error);
    });
    connect(m_client, &LemmyClient::errorOccurred, this, [this](const QString &, const QString &error) {
        emit errorOccurred(error);
    });
}

LemmySession::~LemmySession()
{
}

void LemmySession::open()
{
    setState(ConnectionState::CONNECTING);
    m_client->login(m_username, m_password);
}

void LemmySession::close()
{
    m_rooms.clear();
    m_messages.clear();
    setState(ConnectionState::DISCONNECTED);
}

void LemmySession::sendMessage(const QString &roomId, const QString &text, ProtocolContentType contentType)
{
    Q_UNUSED(contentType)

    int colonPos = roomId.indexOf(QChar(':'));
    if (colonPos != -1) {
        qint64 communityId = roomId.left(colonPos).toLongLong();
        qint64 postId = roomId.mid(colonPos + 1).toLongLong();
        QJsonObject response = m_client->createComment(static_cast<int>(postId), 0, text);
        QJsonObject commentObj = response.value(QStringLiteral("comment")).toObject();
        if (!commentObj.isEmpty()) {
            LemmyComment comment = LemmyComment::fromJson(commentObj);
            ProtocolMessage msg = comment.toProtocolMessage();
            msg.roomId = roomId;
            msg.contentType = contentType;
            emit messageReceived(msg);
            refreshPosts(communityId);
        }
    } else {
        qint64 communityId = roomId.toLongLong();
        QJsonObject response = m_client->createPost(static_cast<int>(communityId), text);
        QJsonObject postObj = response.value(QStringLiteral("post")).toObject();
        if (!postObj.isEmpty()) {
            LemmyPost post = LemmyPost::fromJson(postObj);
            ProtocolMessage msg = post.toProtocolMessage();
            msg.roomId = roomId;
            msg.contentType = contentType;
            emit messageReceived(msg);
            refreshPosts(communityId);
        }
    }
}

QVector<ProtocolRoom> LemmySession::getRooms()
{
    return m_rooms.values().toVector();
}

QVector<ProtocolMessage> LemmySession::getMessages(const QString &roomId, int limit)
{
    Q_UNUSED(limit)

    if (roomId.isEmpty()) {
        return {};
    }

    int colonPos = roomId.indexOf(QChar(':'));
    if (colonPos != -1) {
        qint64 communityId = roomId.left(colonPos).toLongLong();
        qint64 postId = roomId.mid(colonPos + 1).toLongLong();

        refreshPosts(communityId);

        QJsonObject response = m_client->getComments(static_cast<int>(postId));
        QJsonArray commentsArray = response.value(QStringLiteral("comments")).toArray();
        QVector<ProtocolMessage> result;
        for (const QJsonValue &val : commentsArray) {
            LemmyComment comment = LemmyComment::fromJson(val.toObject());
            result.append(comment.toProtocolMessage());
        }
        return result;
    }

    qint64 communityId = roomId.toLongLong();
    refreshPosts(communityId);
    return m_messages.value(communityId);
}

void LemmySession::markAsRead(const QString &roomId)
{
    Q_UNUSED(roomId)
}

void LemmySession::joinRoom(const QString &roomIdOrAlias)
{
    bool ok = false;
    qint64 communityId = roomIdOrAlias.toLongLong(&ok);
    if (!ok) {
        return;
    }
    m_client->followCommunity(static_cast<int>(communityId), true);
    refreshCommunities();
}

void LemmySession::leaveRoom(const QString &roomId)
{
    bool ok = false;
    qint64 communityId = roomId.toLongLong(&ok);
    if (!ok) {
        return;
    }
    m_client->followCommunity(static_cast<int>(communityId), false);
    m_rooms.remove(communityId);
    m_messages.remove(communityId);
    emit roomUpdated(ProtocolRoom{});
}

QVector<ProtocolRoom> LemmySession::searchRooms(const QString &query)
{
    QJsonObject response = m_client->search(query, QStringLiteral("Communities"));
    QJsonArray communitiesArray = response.value(QStringLiteral("communities")).toArray();

    QVector<ProtocolRoom> results;
    for (const QJsonValue &val : communitiesArray) {
        LemmyCommunity community = LemmyCommunity::fromJson(val.toObject());
        results.append(community.toProtocolRoom());
    }
    return results;
}

void LemmySession::refreshCommunities()
{
    QJsonObject response = m_client->getCommunities();
    QJsonArray communitiesArray = response.value(QStringLiteral("communities")).toArray();

    m_rooms.clear();
    for (const QJsonValue &val : communitiesArray) {
        LemmyCommunity community = LemmyCommunity::fromJson(val.toObject());
        ProtocolRoom room = community.toProtocolRoom();
        m_rooms.insert(community.id, room);
        emit roomUpdated(room);
    }
}

void LemmySession::refreshPosts(qint64 communityId)
{
    QJsonObject response = m_client->getPosts(static_cast<int>(communityId));
    QJsonArray postsArray = response.value(QStringLiteral("posts")).toArray();

    QVector<ProtocolMessage> msgs;
    for (const QJsonValue &val : postsArray) {
        LemmyPost post = LemmyPost::fromJson(val.toObject());
        msgs.append(post.toProtocolMessage());
    }
    m_messages.insert(communityId, msgs);
}

void LemmySession::setState(ConnectionState state)
{
    if (m_state != state) {
        m_state = state;
        emit connectionStateChanged(ProtocolType::LEMMY, m_state);
    }
}
