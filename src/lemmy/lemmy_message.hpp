#pragma once

#include <QtTypes>
#include <QString>
#include <QStringList>
#include <QJsonObject>

#include "../protocol/protocol_message.hpp"

struct LemmyPost
{
    qint64  id = 0;
    QString name;
    QString body;
    qint64  creatorId = 0;
    QString creatorName;
    QString creatorAvatar;
    qint64  communityId = 0;
    QString communityName;
    bool    removed = false;
    bool    deleted = false;
    bool    nsfw = false;
    bool    locked = false;
    bool    featuredLocal = false;
    bool    featuredCommunity = false;
    qint64  score = 0;
    qint64  upvotes = 0;
    qint64  downvotes = 0;
    qint64  comments = 0;
    QString published;
    QString updated;
    QString url;
    QString embedTitle;
    QString embedDescription;

    QJsonObject toJson() const;
    ProtocolMessage toProtocolMessage() const {
        ProtocolMessage m;
        m.id = id;
        m.text = name.isEmpty() ? body : name;
        m.senderId = QString::number(creatorId);
        m.senderName = creatorName;
        m.senderAvatar = creatorAvatar;
        m.timestamp = 0;
        m.contentType = ProtocolContentType::TEXT;
        m.isEdited = !updated.isEmpty();
        m.protocolType = ProtocolType::LEMMY;
        m.metadata["community_id"] = communityId;
        m.metadata["community_name"] = communityName;
        m.metadata["score"] = score;
        m.metadata["url"] = url;
        return m;
    }
    static LemmyPost fromJson(const QJsonObject& json) {
        LemmyPost p;
        auto j = json.contains("post") ? json["post"].toObject() : json;
        p.id = static_cast<qint64>(j["id"].toDouble());
        p.name = j["name"].toString();
        p.body = j["body"].toString();
        p.creatorId = static_cast<qint64>(j["creator_id"].toDouble());
        p.creatorName = j["creator_name"].toString();
        p.creatorAvatar = j["creator_avatar"].toString();
        p.communityId = static_cast<qint64>(j["community_id"].toDouble());
        p.communityName = j["community_name"].toString();
        p.removed = j["removed"].toBool();
        p.deleted = j["deleted"].toBool();
        p.nsfw = j["nsfw"].toBool();
        p.locked = j["locked"].toBool();
        p.featuredLocal = j["featured_local"].toBool();
        p.featuredCommunity = j["featured_community"].toBool();
        p.score = static_cast<qint64>(j["score"].toDouble());
        p.upvotes = static_cast<qint64>(j["upvotes"].toDouble());
        p.downvotes = static_cast<qint64>(j["downvotes"].toDouble());
        p.comments = static_cast<qint64>(j["comments"].toDouble());
        p.published = j["published"].toString();
        p.updated = j["updated"].toString();
        p.url = j["url"].toString();
        p.embedTitle = j["embed_title"].toString();
        p.embedDescription = j["embed_description"].toString();
        return p;
    }
};

struct LemmyComment
{
    qint64      id = 0;
    qint64      postId = 0;
    qint64      parentId = 0;
    QString     content;
    qint64      creatorId = 0;
    QString     creatorName;
    qint64      score = 0;
    bool        removed = false;
    bool        deleted = false;
    QString     published;
    QString     updated;
    int         depth = 0;
    QStringList path;

    ProtocolMessage toProtocolMessage() const { return {}; }
    static LemmyComment fromJson(const QJsonObject& json) { LemmyComment c; Q_UNUSED(json); return c; }
};
