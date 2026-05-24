#pragma once

#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>

class LemmyClient : public QObject
{
    Q_OBJECT

public:
    explicit LemmyClient(const QString &instanceUrl, QObject *parent = nullptr);

    void setAuthToken(const QString &jwt);

    // Auth
    QJsonObject login(const QString &username, const QString &password);
    QJsonObject registerUser(const QString &username, const QString &password,
                             const QString &email, const QString &captchaUuid = QString(),
                             const QString &captchaAnswer = QString());
    QJsonObject getCaptcha();

    // Communities
    QJsonObject getCommunities(const QString &type = QStringLiteral("Local"),
                               const QString &sort = QStringLiteral("TopAll"),
                               int page = 1, int limit = 50);
    QJsonObject getCommunity(int id);
    QJsonObject followCommunity(int communityId, bool follow);
    QJsonObject blockCommunity(int communityId, bool block);

    // Posts
    QJsonObject getPosts(int communityId = 0, const QString &sort = QStringLiteral("Active"),
                         int page = 1, int limit = 50);
    QJsonObject getPost(int id);
    QJsonObject createPost(int communityId, const QString &name,
                           const QString &body = QString(),
                           const QString &url = QString(),
                           bool nsfw = false);
    QJsonObject editPost(int postId, const QString &name,
                         const QString &body = QString(),
                         const QString &url = QString(),
                         bool nsfw = false);
    QJsonObject deletePost(int postId, bool deleted);
    QJsonObject likePost(int postId, int score);
    QJsonObject savePost(int postId, bool save);
    QJsonObject lockPost(int postId, bool locked);
    QJsonObject featurePost(int postId, bool featured,
                            const QString &featureType = QStringLiteral("Community"));

    // Comments
    QJsonObject getComments(int postId, const QString &sort = QStringLiteral("Hot"),
                            int page = 1, int limit = 50);
    QJsonObject createComment(int postId, int parentId, const QString &content);
    QJsonObject editComment(int commentId, const QString &content);
    QJsonObject deleteComment(int commentId, bool deleted);
    QJsonObject likeComment(int commentId, int score);
    QJsonObject saveComment(int commentId, bool save);

    // Private Messages
    QJsonObject getPrivateMessages(int page = 1, int limit = 50);
    QJsonObject sendPrivateMessage(int recipientId, const QString &content);

    // Search
    QJsonObject search(const QString &query, const QString &type = QStringLiteral("All"),
                       int page = 1, int limit = 50);

    // User
    QJsonObject getUser(int personId);
    QJsonObject blockUser(int personId, bool block);

    // Site & Misc
    QJsonObject getSite();
    QJsonObject getUnreadCount();

signals:
    void loginSuccess(const QString &jwt);
    void loginFailed(const QString &error);
    void dataReady(const QString &endpoint, const QJsonObject &response);
    void errorOccurred(const QString &endpoint, const QString &error);

private:
    QNetworkRequest buildRequest(const QString &path, const QUrlQuery &query = QUrlQuery()) const;
    QJsonObject processReply(QNetworkReply *reply, const QString &endpoint);
    QJsonObject getRequest(const QString &path, const QUrlQuery &query = QUrlQuery());
    QJsonObject postRequest(const QString &path, const QJsonObject &body);
    QJsonObject putRequest(const QString &path, const QJsonObject &body);

    QNetworkAccessManager *m_manager;
    QString m_instanceUrl;
    QString m_jwt;
};
