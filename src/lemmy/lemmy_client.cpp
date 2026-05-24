#include "lemmy_client.hpp"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>

LemmyClient::LemmyClient(const QString &instanceUrl, QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
    , m_instanceUrl(instanceUrl)
{
}

void LemmyClient::setAuthToken(const QString &jwt)
{
    m_jwt = jwt;
}

QNetworkRequest LemmyClient::buildRequest(const QString &path, const QUrlQuery &query) const
{
    QUrl url(m_instanceUrl + path);
    if (!query.isEmpty()) {
        url.setQuery(query);
    }

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    if (!m_jwt.isEmpty()) {
        request.setRawHeader("Authorization", ("Bearer " + m_jwt).toUtf8());
    }

    return request;
}

QJsonObject LemmyClient::processReply(QNetworkReply *reply, const QString &endpoint)
{
    if (reply->error() != QNetworkReply::NoError) {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QString errorMsg = reply->errorString();

        QString body = reply->readAll();
        if (!body.isEmpty()) {
            QJsonDocument doc = QJsonDocument::fromJson(body.toUtf8());
            if (doc.isObject() && doc.object().contains(QStringLiteral("error"))) {
                errorMsg = doc.object().value(QStringLiteral("error")).toString();
            }
        }

        emit errorOccurred(endpoint, errorMsg);
        reply->deleteLater();

        QJsonObject errObj;
        errObj[QStringLiteral("_error")] = errorMsg;
        errObj[QStringLiteral("_statusCode")] = statusCode;
        return errObj;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit errorOccurred(endpoint, parseError.errorString());
        QJsonObject errObj;
        errObj[QStringLiteral("_error")] = parseError.errorString();
        return errObj;
    }

    QJsonObject response = doc.object();
    emit dataReady(endpoint, response);
    return response;
}

QJsonObject LemmyClient::getRequest(const QString &path, const QUrlQuery &query)
{
    QNetworkRequest request = buildRequest(path, query);
    QNetworkReply *reply = m_manager->get(request);

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    return processReply(reply, path);
}

QJsonObject LemmyClient::postRequest(const QString &path, const QJsonObject &body)
{
    QNetworkRequest request = buildRequest(path);
    QByteArray payload = QJsonDocument(body).toJson(QJsonDocument::Compact);

    QNetworkReply *reply = m_manager->post(request, payload);

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    return processReply(reply, path);
}

QJsonObject LemmyClient::putRequest(const QString &path, const QJsonObject &body)
{
    QNetworkRequest request = buildRequest(path);
    QByteArray payload = QJsonDocument(body).toJson(QJsonDocument::Compact);

    QNetworkReply *reply = m_manager->put(request, payload);

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    return processReply(reply, path);
}

// --- Auth ---

QJsonObject LemmyClient::login(const QString &username, const QString &password)
{
    QJsonObject body;
    body[QStringLiteral("username_or_email")] = username;
    body[QStringLiteral("password")]           = password;

    QJsonObject response = postRequest(QStringLiteral("/api/v3/user/login"), body);

    if (response.contains(QStringLiteral("jwt"))) {
        m_jwt = response.value(QStringLiteral("jwt")).toString();
        emit loginSuccess(m_jwt);
    } else if (response.contains(QStringLiteral("_error"))) {
        emit loginFailed(response.value(QStringLiteral("_error")).toString());
    } else {
        emit loginFailed(QStringLiteral("No JWT token in response"));
    }

    return response;
}

QJsonObject LemmyClient::registerUser(const QString &username, const QString &password,
                                       const QString &email, const QString &captchaUuid,
                                       const QString &captchaAnswer)
{
    QJsonObject body;
    body[QStringLiteral("username")]        = username;
    body[QStringLiteral("password")]        = password;
    body[QStringLiteral("password_verify")] = password;
    body[QStringLiteral("email")]           = email;
    body[QStringLiteral("show_nsfw")]       = false;

    if (!captchaUuid.isEmpty()) {
        body[QStringLiteral("captcha_uuid")] = captchaUuid;
    }
    if (!captchaAnswer.isEmpty()) {
        body[QStringLiteral("captcha_answer")] = captchaAnswer;
    }

    return postRequest(QStringLiteral("/api/v3/user/register"), body);
}

QJsonObject LemmyClient::getCaptcha()
{
    return getRequest(QStringLiteral("/api/v3/user/get_captcha"));
}

// --- Communities ---

QJsonObject LemmyClient::getCommunities(const QString &type, const QString &sort,
                                         int page, int limit)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("type_"), type);
    query.addQueryItem(QStringLiteral("sort"), sort);
    query.addQueryItem(QStringLiteral("page"), QString::number(page));
    query.addQueryItem(QStringLiteral("limit"), QString::number(limit));

    return getRequest(QStringLiteral("/api/v3/community/list"), query);
}

QJsonObject LemmyClient::getCommunity(int id)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("id"), QString::number(id));

    return getRequest(QStringLiteral("/api/v3/community"), query);
}

QJsonObject LemmyClient::followCommunity(int communityId, bool follow)
{
    QJsonObject body;
    body[QStringLiteral("community_id")] = communityId;
    body[QStringLiteral("follow")]       = follow;

    return postRequest(QStringLiteral("/api/v3/community/follow"), body);
}

QJsonObject LemmyClient::blockCommunity(int communityId, bool block)
{
    QJsonObject body;
    body[QStringLiteral("community_id")] = communityId;
    body[QStringLiteral("block")]        = block;

    return postRequest(QStringLiteral("/api/v3/community/block"), body);
}

// --- Posts ---

QJsonObject LemmyClient::getPosts(int communityId, const QString &sort,
                                   int page, int limit)
{
    QUrlQuery query;
    if (communityId > 0) {
        query.addQueryItem(QStringLiteral("community_id"), QString::number(communityId));
    }
    query.addQueryItem(QStringLiteral("sort"), sort);
    query.addQueryItem(QStringLiteral("page"), QString::number(page));
    query.addQueryItem(QStringLiteral("limit"), QString::number(limit));

    return getRequest(QStringLiteral("/api/v3/post/list"), query);
}

QJsonObject LemmyClient::getPost(int id)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("id"), QString::number(id));

    return getRequest(QStringLiteral("/api/v3/post"), query);
}

QJsonObject LemmyClient::createPost(int communityId, const QString &name,
                                     const QString &body, const QString &url, bool nsfw)
{
    QJsonObject req;
    req[QStringLiteral("community_id")] = communityId;
    req[QStringLiteral("name")]         = name;

    if (!body.isEmpty()) {
        req[QStringLiteral("body")] = body;
    }
    if (!url.isEmpty()) {
        req[QStringLiteral("url")] = url;
    }
    req[QStringLiteral("nsfw")] = nsfw;

    return postRequest(QStringLiteral("/api/v3/post"), req);
}

QJsonObject LemmyClient::editPost(int postId, const QString &name,
                                   const QString &body, const QString &url, bool nsfw)
{
    QJsonObject req;
    req[QStringLiteral("post_id")] = postId;
    req[QStringLiteral("name")]    = name;

    if (!body.isEmpty()) {
        req[QStringLiteral("body")] = body;
    }
    if (!url.isEmpty()) {
        req[QStringLiteral("url")] = url;
    }
    req[QStringLiteral("nsfw")] = nsfw;

    return putRequest(QStringLiteral("/api/v3/post"), req);
}

QJsonObject LemmyClient::deletePost(int postId, bool deleted)
{
    QJsonObject body;
    body[QStringLiteral("post_id")] = postId;
    body[QStringLiteral("deleted")] = deleted;

    return postRequest(QStringLiteral("/api/v3/post/delete"), body);
}

QJsonObject LemmyClient::likePost(int postId, int score)
{
    QJsonObject body;
    body[QStringLiteral("post_id")] = postId;
    body[QStringLiteral("score")]   = score;

    return postRequest(QStringLiteral("/api/v3/post/like"), body);
}

QJsonObject LemmyClient::savePost(int postId, bool save)
{
    QJsonObject body;
    body[QStringLiteral("post_id")] = postId;
    body[QStringLiteral("save")]    = save;

    return putRequest(QStringLiteral("/api/v3/post/save"), body);
}

QJsonObject LemmyClient::lockPost(int postId, bool locked)
{
    QJsonObject body;
    body[QStringLiteral("post_id")] = postId;
    body[QStringLiteral("locked")]  = locked;

    return postRequest(QStringLiteral("/api/v3/post/lock"), body);
}

QJsonObject LemmyClient::featurePost(int postId, bool featured, const QString &featureType)
{
    QJsonObject body;
    body[QStringLiteral("post_id")]      = postId;
    body[QStringLiteral("featured")]     = featured;
    body[QStringLiteral("feature_type")] = featureType;

    return postRequest(QStringLiteral("/api/v3/post/feature"), body);
}

// --- Comments ---

QJsonObject LemmyClient::getComments(int postId, const QString &sort, int page, int limit)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("post_id"), QString::number(postId));
    query.addQueryItem(QStringLiteral("sort"), sort);
    query.addQueryItem(QStringLiteral("page"), QString::number(page));
    query.addQueryItem(QStringLiteral("limit"), QString::number(limit));

    return getRequest(QStringLiteral("/api/v3/comment/list"), query);
}

QJsonObject LemmyClient::createComment(int postId, int parentId, const QString &content)
{
    QJsonObject body;
    body[QStringLiteral("post_id")]   = postId;
    body[QStringLiteral("parent_id")] = parentId;
    body[QStringLiteral("content")]   = content;

    return postRequest(QStringLiteral("/api/v3/comment"), body);
}

QJsonObject LemmyClient::editComment(int commentId, const QString &content)
{
    QJsonObject body;
    body[QStringLiteral("comment_id")] = commentId;
    body[QStringLiteral("content")]    = content;

    return putRequest(QStringLiteral("/api/v3/comment"), body);
}

QJsonObject LemmyClient::deleteComment(int commentId, bool deleted)
{
    QJsonObject body;
    body[QStringLiteral("comment_id")] = commentId;
    body[QStringLiteral("deleted")]    = deleted;

    return postRequest(QStringLiteral("/api/v3/comment/delete"), body);
}

QJsonObject LemmyClient::likeComment(int commentId, int score)
{
    QJsonObject body;
    body[QStringLiteral("comment_id")] = commentId;
    body[QStringLiteral("score")]      = score;

    return postRequest(QStringLiteral("/api/v3/comment/like"), body);
}

QJsonObject LemmyClient::saveComment(int commentId, bool save)
{
    QJsonObject body;
    body[QStringLiteral("comment_id")] = commentId;
    body[QStringLiteral("save")]       = save;

    return putRequest(QStringLiteral("/api/v3/comment/save"), body);
}

// --- Private Messages ---

QJsonObject LemmyClient::getPrivateMessages(int page, int limit)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("page"), QString::number(page));
    query.addQueryItem(QStringLiteral("limit"), QString::number(limit));

    return getRequest(QStringLiteral("/api/v3/private_message/list"), query);
}

QJsonObject LemmyClient::sendPrivateMessage(int recipientId, const QString &content)
{
    QJsonObject body;
    body[QStringLiteral("recipient_id")] = recipientId;
    body[QStringLiteral("content")]      = content;

    return postRequest(QStringLiteral("/api/v3/private_message"), body);
}

// --- Search ---

QJsonObject LemmyClient::search(const QString &query, const QString &type, int page, int limit)
{
    QUrlQuery params;
    params.addQueryItem(QStringLiteral("q"), query);
    params.addQueryItem(QStringLiteral("type_"), type);
    params.addQueryItem(QStringLiteral("page"), QString::number(page));
    params.addQueryItem(QStringLiteral("limit"), QString::number(limit));

    return getRequest(QStringLiteral("/api/v3/search"), params);
}

// --- User ---

QJsonObject LemmyClient::getUser(int personId)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("person_id"), QString::number(personId));

    return getRequest(QStringLiteral("/api/v3/user"), query);
}

QJsonObject LemmyClient::blockUser(int personId, bool block)
{
    QJsonObject body;
    body[QStringLiteral("person_id")] = personId;
    body[QStringLiteral("block")]     = block;

    return postRequest(QStringLiteral("/api/v3/user/block"), body);
}

// --- Site & Misc ---

QJsonObject LemmyClient::getSite()
{
    return getRequest(QStringLiteral("/api/v3/site"));
}

QJsonObject LemmyClient::getUnreadCount()
{
    return getRequest(QStringLiteral("/api/v3/user/mention"));
}
