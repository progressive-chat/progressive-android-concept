#include "matrix_sync_manager.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <algorithm>

MatrixSyncManager::MatrixSyncManager(MatrixSession& session, QObject* parent)
    : QObject(parent)
    , m_session(session)
    , m_nam(new QNetworkAccessManager(this))
    , m_syncTimer(new QTimer(this))
    , m_retryDelayMs(0)
    , m_currentBackoffMs(1000)
    , m_running(false)
    , m_currentReply(nullptr)
{
    m_syncTimer->setSingleShot(true);
    connect(m_syncTimer, &QTimer::timeout, this, &MatrixSyncManager::onSyncTimerFired);
}

MatrixSyncManager::~MatrixSyncManager()
{
    m_running = false;
    m_syncTimer->stop();
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
}

void MatrixSyncManager::startSync()
{
    if (m_running) {
        return;
    }
    if (m_homeserverUrl.isEmpty() || m_accessToken.isEmpty()) {
        return;
    }
    m_running = true;
    resetBackoff();
    doSync();
}

void MatrixSyncManager::stopSync()
{
    m_running = false;
    m_syncTimer->stop();
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
}

void MatrixSyncManager::setHomeserverUrl(const QString& url)
{
    m_homeserverUrl = url;
    while (m_homeserverUrl.endsWith('/')) {
        m_homeserverUrl.chop(1);
    }
}

void MatrixSyncManager::setAccessToken(const QString& token)
{
    m_accessToken = token;
}

QString MatrixSyncManager::nextBatch() const
{
    return m_nextBatch;
}

void MatrixSyncManager::onSyncTimerFired()
{
    doSync();
}

void MatrixSyncManager::doSync()
{
    if (!m_running) {
        return;
    }
    if (m_homeserverUrl.isEmpty() || m_accessToken.isEmpty()) {
        return;
    }

    QUrl url(m_homeserverUrl + "/_matrix/client/v3/sync");
    QUrlQuery query;
    query.addQueryItem("timeout", QStringLiteral("30000"));

    if (!m_nextBatch.isEmpty()) {
        query.addQueryItem("since", m_nextBatch);
    }

    QJsonObject filterObj = buildSyncFilter();
    QJsonDocument filterDoc(filterObj);
    QString filterJson = QString::fromUtf8(filterDoc.toJson(QJsonDocument::Compact));
    query.addQueryItem("filter", filterJson);

    url.setQuery(query);

    QNetworkRequest req(url);
    req.setRawHeader("Authorization", ("Bearer " + m_accessToken).toUtf8());
    req.setRawHeader("Content-Type", "application/json");
    req.setTransferTimeout(90000);

    m_currentReply = m_nam->get(req);
    connect(m_currentReply, &QNetworkReply::finished, this, &MatrixSyncManager::onSyncReplyFinished);
}

void MatrixSyncManager::onSyncReplyFinished()
{
    auto reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    reply->deleteLater();

    if (!m_running) {
        m_currentReply = nullptr;
        return;
    }

    m_currentReply = nullptr;

    if (reply->error() != QNetworkReply::NoError) {
        scheduleRetry();
        return;
    }

    QByteArray data = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        scheduleRetry();
        return;
    }

    QJsonObject response = doc.object();
    processSyncResponse(response);
    resetBackoff();

    if (m_running) {
        m_syncTimer->start(0);
    }
}

void MatrixSyncManager::processSyncResponse(const QJsonObject& response)
{
    m_nextBatch = response.value("next_batch").toString();

    QJsonObject rooms = response.value("rooms").toObject();
    processJoinedRooms(rooms.value("join").toObject());
    processInvitedRooms(rooms.value("invite").toObject());
    processLeftRooms(rooms.value("leave").toObject());

    QJsonObject accountData = response.value("account_data").toObject();
    processAccountData(accountData.value("events").toArray());

    QJsonObject presence = response.value("presence").toObject();
    processPresence(presence.value("events").toArray());

    QJsonObject toDevice = response.value("to_device").toObject();
    processToDevice(toDevice);
}

void MatrixSyncManager::processJoinedRooms(const QJsonObject& rooms)
{
    for (auto it = rooms.begin(); it != rooms.end(); ++it) {
        const QString roomId = it.key();
        QJsonObject roomData = it.value().toObject();

        emit roomJoined(roomId, roomData);

        QJsonObject timeline = roomData.value("timeline").toObject();
        QJsonArray events = timeline.value("events").toArray();
        for (const auto& ev : events) {
            QJsonObject event = ev.toObject();
            emit messageReceived(roomId, event);
        }
    }
}

void MatrixSyncManager::processInvitedRooms(const QJsonObject& rooms)
{
    for (auto it = rooms.begin(); it != rooms.end(); ++it) {
        const QString roomId = it.key();
        QJsonObject inviteData = it.value().toObject();
        emit roomInvited(roomId, inviteData);
    }
}

void MatrixSyncManager::processLeftRooms(const QJsonObject& rooms)
{
    for (auto it = rooms.begin(); it != rooms.end(); ++it) {
        const QString roomId = it.key();
        QJsonObject roomData = it.value().toObject();

        QJsonObject timeline = roomData.value("timeline").toObject();
        QJsonArray events = timeline.value("events").toArray();
        for (const auto& ev : events) {
            QJsonObject event = ev.toObject();
            emit messageReceived(roomId, event);
        }
    }
}

void MatrixSyncManager::processAccountData(const QJsonArray& events)
{
    for (const auto& ev : events) {
        QJsonObject event = ev.toObject();
        QString type = event.value("type").toString();
        if (type == "m.direct" || type == "m.push_rules") {
            emit messageReceived(QString(), event);
        }
    }
}

void MatrixSyncManager::processPresence(const QJsonArray& events)
{
    for (const auto& ev : events) {
        QJsonObject event = ev.toObject();
        QString userId = event.value("sender").toString();
        if (userId.isEmpty()) {
            userId = event.value("content").toObject().value("user_id").toString();
        }
        emit presenceChanged(userId, event);
    }
}

void MatrixSyncManager::processToDevice(const QJsonObject& toDevice)
{
    QJsonArray events = toDevice.value("events").toArray();
    for (const auto& ev : events) {
        QJsonObject event = ev.toObject();
        QString type = event.value("type").toString();
        emit deviceMessageReceived(type, event);
    }
}

void MatrixSyncManager::scheduleRetry()
{
    m_currentBackoffMs = std::min(m_currentBackoffMs * 2, 60000);
    if (m_currentBackoffMs < 1000) {
        m_currentBackoffMs = 1000;
    }
    m_syncTimer->start(m_currentBackoffMs);
}

void MatrixSyncManager::resetBackoff()
{
    m_currentBackoffMs = 1000;
}

QJsonObject MatrixSyncManager::buildSyncFilter() const
{
    QJsonObject filter;

    QJsonObject roomFilter;
    QJsonObject timelineFilter;
    timelineFilter["lazy_load_members"] = true;
    roomFilter["timeline"] = timelineFilter;

    QJsonObject stateFilter;
    stateFilter["lazy_load_members"] = true;
    roomFilter["state"] = stateFilter;

    QJsonObject ephemeralFilter;
    ephemeralFilter["types"] = QJsonArray{"m.receipt", "m.typing"};
    roomFilter["ephemeral"] = ephemeralFilter;

    filter["room"] = roomFilter;

    QJsonObject accountDataFilter;
    accountDataFilter["types"] = QJsonArray{"m.direct", "m.push_rules"};
    filter["account_data"] = accountDataFilter;

    QJsonObject presenceFilter;
    presenceFilter["types"] = QJsonArray{"m.presence"};
    filter["presence"] = presenceFilter;

    return filter;
}
