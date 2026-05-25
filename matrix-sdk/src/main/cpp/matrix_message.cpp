#include "matrix_message.hpp"

namespace matrix_sdk {

MessageContent MessageBuilder::buildText(const std::string& body) {
    return {MessageType::TEXT, body};
}

MessageContent MessageBuilder::buildNotice(const std::string& body) {
    return {MessageType::NOTICE, body};
}

MessageContent MessageBuilder::buildEmote(const std::string& body) {
    MessageContent mc{MessageType::EMOTE, body};
    mc.formattedBody = "* " + body + " *";
    return mc;
}

MessageContent MessageBuilder::buildHtml(const std::string& body, const std::string& html) {
    MessageContent mc{MessageType::TEXT, body};
    mc.formattedBody = html;
    mc.format = "org.matrix.custom.html";
    return mc;
}

MessageContent MessageBuilder::buildImage(const std::string& url, int w, int h, int size,
                                            const std::string& mimeType, const std::string& thumbUrl) {
    MessageContent mc{MessageType::IMAGE, ""};
    mc.url = url;
    mc.mimeType = mimeType;
    mc.width = w;
    mc.height = h;
    mc.size = size;
    mc.info = {{"w", w}, {"h", h}, {"size", size}, {"mimetype", mimeType}};
    if (!thumbUrl.empty()) mc.thumbnailUrl = thumbUrl;
    return mc;
}

MessageContent MessageBuilder::buildVideo(const std::string& url, int w, int h, int duration,
                                            int size, const std::string& mimeType) {
    MessageContent mc{MessageType::VIDEO, ""};
    mc.url = url;
    mc.width = w;
    mc.height = h;
    mc.duration = duration;
    mc.size = size;
    mc.mimeType = mimeType;
    mc.info = {{"w", w}, {"h", h}, {"duration", duration}, {"size", size}, {"mimetype", mimeType}};
    return mc;
}

MessageContent MessageBuilder::buildAudio(const std::string& url, int duration, int size,
                                            const std::string& mimeType) {
    MessageContent mc{MessageType::AUDIO, ""};
    mc.url = url;
    mc.duration = duration;
    mc.size = size;
    mc.mimeType = mimeType;
    mc.info = {{"duration", duration}, {"size", size}, {"mimetype", mimeType}};
    return mc;
}

MessageContent MessageBuilder::buildFile(const std::string& url, const std::string& filename,
                                           int64_t size, const std::string& mimeType) {
    MessageContent mc{MessageType::FILE, filename};
    mc.url = url;
    mc.filename = filename;
    mc.size = size;
    mc.mimeType = mimeType;
    mc.info = {{"size", size}, {"mimetype", mimeType}};
    return mc;
}

MessageContent MessageBuilder::buildLocation(const std::string& geoUri, const std::string& description) {
    MessageContent mc{MessageType::LOCATION, description};
    mc.geoUri = geoUri;
    return mc;
}

MessageContent MessageBuilder::buildReply(const std::string& body, const std::string& eventId) {
    MessageContent mc{MessageType::TEXT, body};
    mc.relatesTo = {{"m.in_reply_to", {{"event_id", eventId}}}};
    return mc;
}

MessageContent MessageBuilder::buildEdit(const std::string& newBody, const std::string& originalEventId) {
    MessageContent mc{MessageType::TEXT, newBody};
    mc.relatesTo = {
        {"rel_type", "m.replace"},
        {"event_id", originalEventId}
    };
    return mc;
}

json MessageBuilder::toContentJson(const MessageContent& content) {
    json j;
    j["msgtype"] = msgTypeToString(content.msgType);
    if (!content.body.empty()) j["body"] = content.body;
    if (!content.formattedBody.empty()) {
        j["formatted_body"] = content.formattedBody;
        j["format"] = content.format;
    }
    if (!content.url.empty()) j["url"] = content.url;
    if (!content.mimeType.empty()) j["mimetype"] = content.mimeType;
    if (content.size > 0) j["size"] = content.size;
    if (content.width > 0) j["w"] = content.width;
    if (content.height > 0) j["h"] = content.height;
    if (content.duration > 0) j["duration"] = content.duration;
    if (!content.filename.empty()) j["filename"] = content.filename;
    if (!content.geoUri.empty()) j["geo_uri"] = content.geoUri;
    if (!content.info.empty()) j["info"] = content.info;
    if (!content.relatesTo.empty()) j["m.relates_to"] = content.relatesTo;
    return j;
}

std::string MessageBuilder::msgTypeToString(MessageType type) {
    switch (type) {
        case MessageType::TEXT: return "m.text";
        case MessageType::NOTICE: return "m.notice";
        case MessageType::EMOTE: return "m.emote";
        case MessageType::IMAGE: return "m.image";
        case MessageType::AUDIO: return "m.audio";
        case MessageType::VIDEO: return "m.video";
        case MessageType::FILE: return "m.file";
        case MessageType::LOCATION: return "m.location";
        case MessageType::STICKER: return "m.sticker";
        case MessageType::CUSTOM: return "m.custom";
        default: return "m.text";
    }
}

MessageType MessageBuilder::stringToMsgType(const std::string& s) {
    if (s == "m.text") return MessageType::TEXT;
    if (s == "m.notice") return MessageType::NOTICE;
    if (s == "m.emote") return MessageType::EMOTE;
    if (s == "m.image") return MessageType::IMAGE;
    if (s == "m.audio") return MessageType::AUDIO;
    if (s == "m.video") return MessageType::VIDEO;
    if (s == "m.file") return MessageType::FILE;
    if (s == "m.location") return MessageType::LOCATION;
    if (s == "m.sticker") return MessageType::STICKER;
    return MessageType::CUSTOM;
}

MessageContent MessageBuilder::fromContentJson(const json& j) {
    MessageContent mc;
    if (j.contains("msgtype")) mc.msgType = stringToMsgType(j["msgtype"]);
    mc.body = j.value("body", "");
    mc.formattedBody = j.value("formatted_body", "");
    mc.format = j.value("format", "");
    mc.url = j.value("url", "");
    mc.mimeType = j.value("mimetype", "");
    mc.size = j.value("size", 0);
    mc.width = j.value("w", j.value("info", json::object()).value("w", 0));
    mc.height = j.value("h", j.value("info", json::object()).value("h", 0));
    mc.duration = j.value("duration", 0);
    mc.filename = j.value("filename", "");
    mc.geoUri = j.value("geo_uri", "");
    mc.info = j.value("info", json::object());
    mc.relatesTo = j.value("m.relates_to", json::object());
    return mc;
}

} // namespace matrix_sdk
