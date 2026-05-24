#pragma once

#include <QtTypes>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include "../protocol/protocol_room.hpp"
#include "../protocol/protocol_type.hpp"

struct LemmyCommunity
{
    qint64  id = 0;
    QString name;
    QString title;
    QString description;
    bool    removed = false;
    bool    deleted = false;
    bool    nsfw = false;
    QString actorId;
    QString icon;
    QString banner;
    qint64  subscribers = 0;
    qint64  posts = 0;
    qint64  comments = 0;
    bool    userIsFollowing = false;

    ProtocolRoom toProtocolRoom() const {
        ProtocolRoom r;
        r.id = QString::number(id);
        r.name = name;
        r.topic = title.isEmpty() ? description : title;
        r.protocolType = ProtocolType::LEMMY;
        r.memberCount = static_cast<int>(subscribers);
        r.isPublic = true;
        r.metadata["community_id"] = id;
        r.metadata["actor_id"] = actorId;
        r.metadata["nsfw"] = nsfw;
        return r;
    }

    static LemmyCommunity fromJson(const QJsonObject& json) {
        LemmyCommunity c;
        c.id = static_cast<qint64>(json["id"].toDouble());
        c.name = json["name"].toString();
        c.title = json["title"].toString();
        c.description = json["description"].toString();
        c.removed = json["removed"].toBool();
        c.deleted = json["deleted"].toBool();
        c.nsfw = json["nsfw"].toBool();
        c.actorId = json["actor_id"].toString();
        c.icon = json["icon"].toString();
        c.banner = json["banner"].toString();
        c.subscribers = static_cast<qint64>(json["subscribers"].toDouble());
        c.posts = static_cast<qint64>(json["posts"].toDouble());
        c.comments = static_cast<qint64>(json["comments"].toDouble());
        c.userIsFollowing = json.value("user_is_following").toBool();
        return c;
    }
};
