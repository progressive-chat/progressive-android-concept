#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace matrix_sdk {

using json = nlohmann::json;

enum class MessageType {
    TEXT,
    NOTICE,
    EMOTE,
    IMAGE,
    AUDIO,
    VIDEO,
    FILE,
    LOCATION,
    STICKER,
    CUSTOM
};

enum class MessageFormat {
    PLAIN,
    HTML,
    MARKDOWN
};

struct MessageContent {
    MessageType msgType = MessageType::TEXT;
    std::string body;
    std::string formattedBody;
    std::string format = "org.matrix.custom.html";
    std::string url;
    std::string thumbnailUrl;
    std::string mimeType;
    int64_t size = 0;
    int width = 0, height = 0;
    int duration = 0;
    std::string filename;
    std::string geoUri;
    json info;
    json relatesTo;
};

struct MatrixMessage {
    std::string eventId;
    std::string roomId;
    std::string sender;
    std::string senderName;
    std::string senderAvatar;
    uint64_t timestamp = 0;
    uint64_t age = 0;
    MessageContent content;
    bool isEdited = false;
    bool isRedacted = false;
    std::string replacesEventId;
    std::string replyToEventId;
    std::string threadRoot;
    bool isEncrypted = false;
    std::string decryptionError;
};

class MessageBuilder {
public:
    static MessageContent buildText(const std::string& body);
    static MessageContent buildNotice(const std::string& body);
    static MessageContent buildEmote(const std::string& body);
    static MessageContent buildHtml(const std::string& body, const std::string& html);
    static MessageContent buildImage(const std::string& url, int w, int h, int size,
                                      const std::string& mimeType, const std::string& thumbUrl = "");
    static MessageContent buildVideo(const std::string& url, int w, int h, int duration,
                                      int size, const std::string& mimeType);
    static MessageContent buildAudio(const std::string& url, int duration, int size,
                                      const std::string& mimeType);
    static MessageContent buildFile(const std::string& url, const std::string& filename,
                                     int64_t size, const std::string& mimeType);
    static MessageContent buildLocation(const std::string& geoUri, const std::string& description = "");
    static MessageContent buildReply(const std::string& body, const std::string& eventId);
    static MessageContent buildEdit(const std::string& newBody, const std::string& originalEventId);

    static json toContentJson(const MessageContent& content);
    static MessageContent fromContentJson(const json& j);
    static MessageType stringToMsgType(const std::string& s);
    static std::string msgTypeToString(MessageType type);
};

} // namespace matrix_sdk
