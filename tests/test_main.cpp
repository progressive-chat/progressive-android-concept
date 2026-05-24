#include <QtTest>

#include "protocol/protocol_type.hpp"
#include "util/json_util.hpp"
#include "util/string_util.hpp"
#include "irc/irc_message.hpp"
#include "irc/irc_room.hpp"
#include "matrix/matrix_message.hpp"
#include "lemmy/lemmy_room.hpp"
#include "lemmy/lemmy_message.hpp"
#include "protocol/protocol_manager.hpp"
#include "protocol/protocol_session.hpp"

#include <QJsonObject>

class MockSession : public IProtocolSession {
    Q_OBJECT
public:
    explicit MockSession(ProtocolType t, QObject *p = nullptr) : IProtocolSession(), m_type(t) { Q_UNUSED(p); }
    void open() override {}
    void close() override {}
    void sendMessage(const QString &, const QString &, ProtocolContentType) override {}
    QVector<ProtocolRoom> getRooms() override { return {}; }
    QVector<ProtocolMessage> getMessages(const QString &, int) override { return {}; }
    void markAsRead(const QString &) override {}
    void joinRoom(const QString &) override {}
    void leaveRoom(const QString &) override {}
    QVector<ProtocolRoom> searchRooms(const QString &) override { return {}; }
    ProtocolType protocolType() const override { return m_type; }
    ConnectionState connectionState() const override { return ConnectionState::DISCONNECTED; }
    QString userId() const override { return "mock"; }
    QString displayName() const override { return "Mock"; }
private:
    ProtocolType m_type;
};

class TestCore : public QObject {
    Q_OBJECT
private slots:
    void testEnums()
    {
        QCOMPARE(static_cast<int>(ProtocolType::MATRIX), 0);
        QCOMPARE(static_cast<int>(ProtocolType::IRC), 1);
        QCOMPARE(static_cast<int>(ProtocolType::LEMMY), 2);
        QCOMPARE(Protocol::protocolTypeToString(ProtocolType::MATRIX), QString("matrix"));
        QCOMPARE(Protocol::protocolTypeToString(ProtocolType::IRC), QString("irc"));
        QCOMPARE(Protocol::stringToProtocolType("matrix"), ProtocolType::MATRIX);
        QCOMPARE(Protocol::stringToProtocolType("irc"), ProtocolType::IRC);

        QCOMPARE(static_cast<int>(ConnectionState::DISCONNECTED), 0);
        QCOMPARE(static_cast<int>(ConnectionState::CONNECTING), 1);
        QCOMPARE(static_cast<int>(ConnectionState::CONNECTED), 2);
        QCOMPARE(static_cast<int>(ConnectionState::REGISTERED), 3);
        QCOMPARE(static_cast<int>(ConnectionState::ERROR), 4);

        QCOMPARE(static_cast<int>(ProtocolContentType::TEXT), 0);
        QCOMPARE(static_cast<int>(ProtocolContentType::ACTION), 5);
        QCOMPARE(static_cast<int>(ProtocolContentType::TOPIC), 10);
        QCOMPARE(static_cast<int>(ProtocolContentType::NOTICE), 11);
    }

    void testJsonUtil()
    {
        QJsonObject obj;
        obj["name"] = "alice";
        obj["count"] = 42;
        obj["active"] = true;
        obj["emptyStr"] = "";

        QCOMPARE(JsonUtil::optString(obj, "name"), QString("alice"));
        QCOMPARE(JsonUtil::optString(obj, "missing", "fb"), QString("fb"));
        QCOMPARE(JsonUtil::optString(QJsonObject(), "x"), QString());

        QCOMPARE(JsonUtil::optInt(obj, "count"), 42);
        QCOMPARE(JsonUtil::optInt(obj, "missing", 99), 99);

        QCOMPARE(JsonUtil::optBool(obj, "active"), true);
        QCOMPARE(JsonUtil::optBool(obj, "missing"), false);
        QCOMPARE(JsonUtil::optBool(obj, "missing", true), true);

        QJsonObject parsed = JsonUtil::parseObject(R"({"k":"v"})");
        QCOMPARE(parsed["k"].toString(), QString("v"));
        QVERIFY(JsonUtil::parseObject("bad").isEmpty());

        QString s = JsonUtil::stringify(parsed);
        QVERIFY(s.contains("\"k\":\"v\""));
    }

    void testStringUtil()
    {
        QVERIFY(StringUtil::isEmpty(QString()));
        QVERIFY(StringUtil::isEmpty(""));
        QVERIFY(!StringUtil::isEmpty("x"));

        QCOMPARE(StringUtil::trim("  hi  "), QString("hi"));
        QCOMPARE(StringUtil::trim(""), QString(""));

        QStringList parts = StringUtil::split("a,b,c", ',');
        QCOMPARE(parts.size(), 3);
        QCOMPARE(parts.at(0), QString("a"));

        QCOMPARE(StringUtil::join({"x", "y"}, "-"), QString("x-y"));

        QVERIFY(StringUtil::contains("Hello", "ell"));
        QVERIFY(!StringUtil::contains("Hello", "X"));

        QByteArray data("test123");
        QString b64 = StringUtil::base64Encode(data);
        QCOMPARE(StringUtil::base64Decode(b64), data);
    }

    void testIrcMessage()
    {
        auto msg = IrcMessage::fromRawLine(":nick!user@host PRIVMSG #chan :Hello");
        QCOMPARE(msg.prefix, QString("nick!user@host"));
        QCOMPARE(msg.nick(), QString("nick"));
        QCOMPARE(msg.user(), QString("user"));
        QCOMPARE(msg.host(), QString("host"));
        QCOMPARE(msg.command, QString("PRIVMSG"));
        QCOMPARE(msg.target, QString("#chan"));
        QCOMPARE(msg.text, QString("Hello"));

        auto msg2 = IrcMessage::fromRawLine("JOIN #room");
        QCOMPARE(msg2.command, QString("JOIN"));
        QVERIFY(msg2.prefix.isEmpty());

        auto msg3 = IrcMessage::fromRawLine("");
        QVERIFY(msg3.command.isEmpty());

        QCOMPARE(IrcMessage::commandToContentType("PRIVMSG"), ProtocolContentType::TEXT);
        QCOMPARE(IrcMessage::commandToContentType("NOTICE"), ProtocolContentType::NOTICE);
        QCOMPARE(IrcMessage::commandToContentType("JOIN"), ProtocolContentType::JOIN);
        QCOMPARE(IrcMessage::commandToContentType("PART"), ProtocolContentType::PART);
        QCOMPARE(IrcMessage::commandToContentType("KICK"), ProtocolContentType::KICK);
        QCOMPARE(IrcMessage::commandToContentType("TOPIC"), ProtocolContentType::TOPIC);
        QCOMPARE(IrcMessage::commandToContentType("ACTION"), ProtocolContentType::ACTION);
    }

    void testIrcRoom()
    {
        IrcRoom r = IrcRoom::fromChannel("linux");
        QCOMPARE(r.channel, QString("#linux"));

        IrcRoom r2 = IrcRoom::fromChannel("#debian");
        QCOMPARE(r2.channel, QString("#debian"));

        ProtocolRoom p = r.toProtocolRoom();
        QCOMPARE(p.protocolType, ProtocolType::IRC);
    }

    void testMatrixMessage()
    {
        QJsonObject j;
        j["event_id"] = "$evt1";
        j["room_id"] = "!room:matrix.org";
        j["sender"] = "@alice:matrix.org";
        j["body"] = "Hello!";
        j["formatted_body"] = "<b>H</b>";
        j["msgtype"] = "m.text";
        j["origin_server_ts"] = 1710000000000.0;
        j["is_edited"] = false;
        j["is_encrypted"] = false;

        auto msg = MatrixMessage::fromJson(j);
        QCOMPARE(msg.eventId, QString("$evt1"));
        QCOMPARE(msg.roomId, QString("!room:matrix.org"));
        QCOMPARE(msg.senderId, QString("@alice:matrix.org"));
        QCOMPARE(msg.body, QString("Hello!"));
        QCOMPARE(msg.msgType, QString("m.text"));
        QCOMPARE(msg.originServerTs, static_cast<qint64>(1710000000000));
        QCOMPARE(msg.isEdited, false);
        QCOMPARE(msg.isEncrypted, false);

        ProtocolMessage proto = MatrixMessage::toProtocolMessage(msg);
        QCOMPARE(proto.text, QString("Hello!"));

        QCOMPARE(MatrixMessage::mapMsgType("m.text"), ProtocolContentType::TEXT);
        QCOMPARE(MatrixMessage::mapMsgType("m.image"), ProtocolContentType::IMAGE);
        QCOMPARE(MatrixMessage::mapMsgType("m.video"), ProtocolContentType::VIDEO);
    }

    void testLemmyCommunity()
    {
        QJsonObject j;
        j["id"] = static_cast<qint64>(42);
        j["name"] = "linux";
        j["title"] = "Linux Community";
        j["description"] = "FLOSS";
        j["nsfw"] = false;
        j["removed"] = false;
        j["deleted"] = false;
        j["actor_id"] = "https://inst/c/linux";
        j["subscribers"] = static_cast<qint64>(1000);
        j["posts"] = static_cast<qint64>(50);
        j["comments"] = static_cast<qint64>(200);
        j["user_is_following"] = true;

        auto c = LemmyCommunity::fromJson(j);
        QCOMPARE(c.id, static_cast<qint64>(42));
        QCOMPARE(c.name, QString("linux"));
        QCOMPARE(c.title, QString("Linux Community"));
        QCOMPARE(c.nsfw, false);
        QCOMPARE(c.actorId, QString("https://inst/c/linux"));
        QCOMPARE(c.subscribers, static_cast<qint64>(1000));
        QCOMPARE(c.userIsFollowing, true);

        ProtocolRoom p = c.toProtocolRoom();
        QCOMPARE(p.protocolType, ProtocolType::LEMMY);
    }

    void testLemmyPost()
    {
        QJsonObject j;
        j["id"] = static_cast<qint64>(100);
        j["name"] = "Qt6 Release";
        j["body"] = "Now available!";
        j["creator_id"] = static_cast<qint64>(7);
        j["creator_name"] = "dev";
        j["community_id"] = static_cast<qint64>(42);
        j["community_name"] = "linux";
        j["removed"] = false;
        j["deleted"] = false;
        j["nsfw"] = false;
        j["locked"] = false;
        j["featured_local"] = false;
        j["featured_community"] = true;
        j["score"] = static_cast<qint64>(15);
        j["upvotes"] = static_cast<qint64>(20);
        j["downvotes"] = static_cast<qint64>(5);
        j["comments"] = static_cast<qint64>(8);
        j["published"] = "2024-01-15T10:30:00Z";

        auto p = LemmyPost::fromJson(j);
        QCOMPARE(p.id, static_cast<qint64>(100));
        QCOMPARE(p.name, QString("Qt6 Release"));
        QCOMPARE(p.body, QString("Now available!"));
        QCOMPARE(p.creatorName, QString("dev"));
        QCOMPARE(p.communityId, static_cast<qint64>(42));
        QCOMPARE(p.score, static_cast<qint64>(15));
        QCOMPARE(p.comments, static_cast<qint64>(8));
        QCOMPARE(p.published, QString("2024-01-15T10:30:00Z"));
        QCOMPARE(p.featuredCommunity, true);
        QCOMPARE(p.nsfw, false);

        ProtocolMessage msg = p.toProtocolMessage();
        QCOMPARE(msg.senderName, QString("dev"));
        QCOMPARE(msg.protocolType, ProtocolType::LEMMY);
    }

    void testProtocolManager()
    {
        ProtocolManager &a = ProtocolManager::instance();
        ProtocolManager &b = ProtocolManager::instance();
        QCOMPARE(&a, &b);

        auto *irc = new MockSession(ProtocolType::IRC, &a);
        a.registerSession(ProtocolType::IRC, irc);
        QCOMPARE(a.getSession(ProtocolType::IRC), irc);
        QCOMPARE(a.getSession(ProtocolType::MATRIX), nullptr);
        QVERIFY(a.getAllSessions().contains(irc));

        auto *lemmy = new MockSession(ProtocolType::LEMMY, &a);
        a.registerSession(ProtocolType::LEMMY, lemmy);
        QCOMPARE(a.getAllSessions().size(), 2);

        a.unregisterSession(ProtocolType::IRC);
        QCOMPARE(a.getSession(ProtocolType::IRC), nullptr);
        a.unregisterSession(ProtocolType::LEMMY);
    }
};

QTEST_MAIN(TestCore)

#include "test_main.moc"
