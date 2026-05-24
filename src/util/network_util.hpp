#pragma once

#include <QString>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

namespace NetworkUtil {

inline QString resolveWellKnown(const QString &homeserver)
{
    QUrl url(homeserver + "/.well-known/matrix/client");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.get(request);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start(10000);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return QString();
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();
    QJsonObject homeserverObj = obj["m.homeserver"].toObject();
    return homeserverObj["base_url"].toString();
}

inline bool isReachable(const QString &urlStr, int timeoutMs = 5000)
{
    QUrl url(urlStr);
    QNetworkRequest request(url);

    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.head(request);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start(timeoutMs);
    loop.exec();

    bool reachable = (reply->error() == QNetworkReply::NoError ||
                      reply->error() == QNetworkReply::ContentNotFoundError);
    reply->deleteLater();
    return reachable;
}

inline QString extractDomain(const QString &urlStr)
{
    QUrl url(urlStr);
    QString host = url.host();
    if (host.isEmpty())
        return urlStr;
    return host;
}

inline bool isValidMatrixId(const QString &id)
{
    QRegularExpression re("^@[\\w.=\\-\\/]+:[\\w.\\-]+(:\\d+)?$");
    return re.match(id).hasMatch();
}

inline bool isValidRoomId(const QString &id)
{
    QRegularExpression re("^![\\w.=\\-\\/]+:[\\w.\\-]+(:\\d+)?$");
    return re.match(id).hasMatch();
}

inline bool isValidRoomAlias(const QString &alias)
{
    QRegularExpression re("^#[\\w.=\\-\\/]+:[\\w.\\-]+(:\\d+)?$");
    return re.match(alias).hasMatch();
}

} // namespace NetworkUtil
