#pragma once

struct MatrixMessage : ProtocolMessage {
    QString eventId;
    QString roomId;
    QString senderId;
    QString body;
    QString formattedBody;
    QString msgType;
    qint64 originServerTs = 0;
    QJsonObject content;
    QJsonObject unsigned_data;
    QString relatesToEventId;
    QString relatesToType;
    bool isEdited = false;
    bool isEncrypted = false;

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["event_id"] = eventId;
        obj["room_id"] = roomId;
        obj["sender"] = senderId;
        obj["body"] = body;
        obj["formatted_body"] = formattedBody;
        obj["msgtype"] = msgType;
        obj["origin_server_ts"] = originServerTs;
        obj["content"] = content;
        obj["unsigned"] = unsigned_data;
        obj["relates_to_event_id"] = relatesToEventId;
        obj["relates_to_type"] = relatesToType;
        obj["is_edited"] = isEdited;
        obj["is_encrypted"] = isEncrypted;
        return obj;
    }

    static MatrixMessage fromJson(const QJsonObject& json) {
        MatrixMessage msg;
        msg.eventId = json["event_id"].toString();
        msg.roomId = json["room_id"].toString();
        msg.senderId = json["sender"].toString();
        msg.body = json["body"].toString();
        msg.formattedBody = json["formatted_body"].toString();
        msg.msgType = json["msgtype"].toString();
        msg.originServerTs = static_cast<qint64>(json["origin_server_ts"].toDouble());
        msg.content = json["content"].toObject();
        msg.unsigned_data = json["unsigned"].toObject();
        msg.relatesToEventId = json["relates_to_event_id"].toString();
        msg.relatesToType = json["relates_to_type"].toString();
        msg.isEdited = json["is_edited"].toBool();
        msg.isEncrypted = json["is_encrypted"].toBool();
        return msg;
    }

    static ProtocolMessage toProtocolMessage(const MatrixMessage& msg) {
        ProtocolMessage proto;
        proto.roomId = msg.roomId;
        proto.senderId = msg.senderId;
        proto.text = msg.body;
        proto.timestamp = msg.originServerTs;
        proto.contentType = mapMsgType(msg.msgType);
        return proto;
    }

    static ProtocolContentType mapMsgType(const QString& msgType) {
        if (msgType == "m.text") return ProtocolContentType::TEXT;
        if (msgType == "m.image") return ProtocolContentType::IMAGE;
        if (msgType == "m.audio") return ProtocolContentType::AUDIO;
        if (msgType == "m.video") return ProtocolContentType::VIDEO;
        if (msgType == "m.file") return ProtocolContentType::FILE;
        if (msgType == "m.location") return ProtocolContentType::LOCATION;
        if (msgType == "m.emote") return ProtocolContentType::ACTION;
        if (msgType == "m.notice") return ProtocolContentType::NOTICE;
        return ProtocolContentType::TEXT;
    }
};
