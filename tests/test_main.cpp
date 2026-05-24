// Progressive Chat Test Suite
// Tests for the core SDK and application modules

#include <QTest>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSignalSpy>
#include <QNetworkAccessManager>

#include <iostream>

// ===== Protocol Type Tests =====
class ProtocolTypeTests : public QObject
{
    Q_OBJECT

private slots:
    void testProtocolTypeToString()
    {
        QCOMPARE(progressive_chat::protocolTypeToString(
            progressive_chat::ProtocolType::Matrix), QString("matrix"));
        QCOMPARE(progressive_chat::protocolTypeToString(
            progressive_chat::ProtocolType::IRC), QString("irc"));
        QCOMPARE(progressive_chat::protocolTypeToString(
            progressive_chat::ProtocolType::Lemmy), QString("lemmy"));
        QCOMPARE(progressive_chat::protocolTypeToString(
            progressive_chat::ProtocolType::Unknown), QString("unknown"));
    }

    void testProtocolTypeFromString()
    {
        QCOMPARE(progressive_chat::protocolTypeFromString("matrix"),
                 progressive_chat::ProtocolType::Matrix);
        QCOMPARE(progressive_chat::protocolTypeFromString("irc"),
                 progressive_chat::ProtocolType::IRC);
        QCOMPARE(progressive_chat::protocolTypeFromString("lemmy"),
                 progressive_chat::ProtocolType::Lemmy);
        QCOMPARE(progressive_chat::protocolTypeFromString("invalid"),
                 progressive_chat::ProtocolType::Unknown);
    }

    void testProtocolTypeRoundTrip()
    {
        auto types = {progressive_chat::ProtocolType::Matrix,
                      progressive_chat::ProtocolType::IRC,
                      progressive_chat::ProtocolType::Lemmy};

        for (auto type : types) {
            QString str = progressive_chat::protocolTypeToString(type);
            QCOMPARE(progressive_chat::protocolTypeFromString(str), type);
        }
    }
};

// ===== Protocol Message Tests =====
class ProtocolMessageTests : public QObject
{
    Q_OBJECT

private slots:
    void testDefaultMessage()
    {
        progressive_chat::ProtocolMessage msg;
        QCOMPARE(msg.protocol, progressive_chat::ProtocolType::Unknown);
        QCOMPARE(msg.type, progressive_chat::MessageType::Text);
        QVERIFY(!msg.isEncrypted());
        QVERIFY(!msg.isEdited());
        QVERIFY(!msg.isReply());
        QVERIFY(!msg.isInThread());
    }

    void testMessageReply()
    {
        progressive_chat::ProtocolMessage msg;
        msg.replyToId = "$event123";
        QVERIFY(msg.isReply());
    }

    void testMessageThread()
    {
        progressive_chat::ProtocolMessage msg;
        msg.threadRootId = "$thread456";
        QVERIFY(msg.isInThread());
    }

    void testMessageDisplayName()
    {
        progressive_chat::ProtocolMessage msg;
        msg.senderName = "Alice";
        msg.senderId = "@alice:matrix.org";
        QCOMPARE(msg.displayName(), QString("Alice"));

        progressive_chat::ProtocolMessage msg2;
        msg2.senderId = "@bob:matrix.org";
        QCOMPARE(msg2.displayName(), QString("@bob:matrix.org"));
    }
};

// ===== Protocol Room Tests =====
class ProtocolRoomTests : public QObject
{
    Q_OBJECT

private slots:
    void testRoomDefaults()
    {
        progressive_chat::ProtocolRoom room;
        QCOMPARE(room.protocol, progressive_chat::ProtocolType::Unknown);
        QCOMPARE(room.type, progressive_chat::ProtocolRoom::RoomType::Unknown);
        QCOMPARE(room.memberCount, 0);
        QCOMPARE(room.unreadCount, 0);
        QVERIFY(!room.isEncrypted);
        QVERIFY(!room.isDirect);
        QVERIFY(!room.isSpace());
    }

    void testRoomDisplayName()
    {
        progressive_chat::ProtocolRoom room;
        room.name = "Test Room";
        room.canonicalAlias = "#test:matrix.org";
        QCOMPARE(room.displayName(), QString("Test Room"));

        progressive_chat::ProtocolRoom room2;
        room2.canonicalAlias = "#onlyalias:matrix.org";
        QCOMPARE(room2.displayName(), QString("#onlyalias:matrix.org"));
    }

    void testSpaceRoom()
    {
        progressive_chat::ProtocolRoom room;
        room.type = progressive_chat::ProtocolRoom::RoomType::Space;
        QVERIFY(room.isSpace());
    }
};

// ===== String Utility Tests =====
class StringUtilTests : public QObject
{
    Q_OBJECT

private slots:
    void testEllipsize()
    {
        QCOMPARE(progressive_chat::util::ellipsize("Short", 10), QString("Short"));
        QCOMPARE(progressive_chat::util::ellipsize("This is a long text", 10),
                 QString("This is..."));
    }

    void testStripHtml()
    {
        QCOMPARE(progressive_chat::util::stripHtml("<b>Hello</b> <i>World</i>"),
                 QString("Hello World"));
    }

    void testSplitLines()
    {
        QStringList lines = progressive_chat::util::splitLines("line1\\nline2\\r\\nline3");
        QCOMPARE(lines.size(), 3);
        QCOMPARE(lines[0], QString("line1"));
        QCOMPARE(lines[1], QString("line2"));
        QCOMPARE(lines[2], QString("line3"));
    }

    void testIsValidMatrixId()
    {
        QVERIFY(progressive_chat::util::isValidMatrixId("@user:matrix.org"));
        QVERIFY(progressive_chat::util::isValidMatrixId("!room:matrix.org"));
        QVERIFY(progressive_chat::util::isValidMatrixId("#alias:matrix.org"));
        QVERIFY(!progressive_chat::util::isValidMatrixId("invalid"));
        QVERIFY(!progressive_chat::util::isValidMatrixId(""));
    }

    void testExtractServerName()
    {
        QCOMPARE(progressive_chat::util::extractServerName("@user:matrix.org"),
                 QString("matrix.org"));
        QCOMPARE(progressive_chat::util::extractServerName("@user:example.com:443"),
                 QString("443"));
    }

    void testFormatBytes()
    {
        QCOMPARE(progressive_chat::util::formatBytes(500), QString("500 B"));
        QCOMPARE(progressive_chat::util::formatBytes(1500), QString("1.5 KB"));
        QCOMPARE(progressive_chat::util::formatBytes(1500000), QString("1.4 MB"));
        QCOMPARE(progressive_chat::util::formatBytes(1500000000LL), QString("1.4 GB"));
    }
};

// ===== Date Utility Tests =====
class DateUtilTests : public QObject
{
    Q_OBJECT

private slots:
    void testFormatTimestamp()
    {
        QDateTime now = QDateTime::currentDateTime();
        QString today = progressive_chat::util::formatTimestamp(now);
        QVERIFY(!today.isEmpty());
    }

    void testFormatRelativeTime()
    {
        QDateTime now = QDateTime::currentDateTime();
        QDateTime tenSecAgo = now.addSecs(-10);
        QString relative = progressive_chat::util::formatRelativeTime(tenSecAgo);
        QVERIFY(relative.contains("ago"));
    }

    void testFormatDuration()
    {
        QCOMPARE(progressive_chat::util::formatDuration(5000), QString("0:05"));
        QCOMPARE(progressive_chat::util::formatDuration(65000), QString("1:05"));
        QCOMPARE(progressive_chat::util::formatDuration(3665000), QString("1:01:05"));
    }
};

// ===== Color Utility Tests =====
class ColorUtilTests : public QObject
{
    Q_OBJECT

private slots:
    void testColorFromUserId()
    {
        QColor color1 = progressive_chat::util::colorFromUserId("@alice:matrix.org");
        QColor color2 = progressive_chat::util::colorFromUserId("@alice:matrix.org");
        QVERIFY(color1.isValid());
        // Same user should get same color
        QCOMPARE(color1.name(), color2.name());
    }

    void testIsDarkColor()
    {
        QVERIFY(progressive_chat::util::isDarkColor(QColor("#000000")));
        QVERIFY(!progressive_chat::util::isDarkColor(QColor("#ffffff")));
    }

    void testTextColorForBackground()
    {
        QCOMPARE(progressive_chat::util::textColorForBackground(QColor("#000000")),
                 QColor("#ffffff"));
        QCOMPARE(progressive_chat::util::textColorForBackground(QColor("#ffffff")),
                 QColor("#212121"));
    }
};

// ===== JSON Utility Tests =====
class JsonUtilTests : public QObject
{
    Q_OBJECT

private slots:
    void testParseJson()
    {
        QByteArray valid = R"({"key": "value"})";
        auto result = progressive_chat::util::parseJson(valid);
        QVERIFY(result.has_value());
        QCOMPARE((*result)["key"].toString(), QString("value"));

        QByteArray invalid = "not json";
        auto badResult = progressive_chat::util::parseJson(invalid);
        QVERIFY(!badResult.has_value());
    }

    void testSafeObject()
    {
        QJsonObject obj;
        obj["nested"] = QJsonObject{{"key", "val"}};
        QJsonObject nested = progressive_chat::util::safeObject(obj["nested"]);
        QCOMPARE(nested["key"].toString(), QString("val"));

        QJsonObject empty = progressive_chat::util::safeObject(obj["nonexistent"]);
        QVERIFY(empty.isEmpty());
    }

    void testSafeArray()
    {
        QJsonObject obj;
        obj["list"] = QJsonArray{1, 2, 3};
        QJsonArray arr = progressive_chat::util::safeArray(obj["list"]);
        QCOMPARE(arr.size(), 3);
    }

    void testSafeString()
    {
        QJsonObject obj;
        obj["name"] = "test";
        QCOMPARE(progressive_chat::util::safeString(obj["name"]), QString("test"));
        QCOMPARE(progressive_chat::util::safeString(obj["missing"], "default"),
                 QString("default"));
    }

    void testSafeInt()
    {
        QJsonObject obj;
        obj["count"] = 42;
        QCOMPARE(progressive_chat::util::safeInt(obj["count"]), 42);
        QCOMPARE(progressive_chat::util::safeInt(obj["missing"], -1), -1);
    }

    void testCanonicalJson()
    {
        QJsonObject obj;
        obj["b"] = 2;
        obj["a"] = 1;
        QString canonical = progressive_chat::util::canonicalJson(obj);
        QVERIFY(canonical.contains("\"a\""));
        QVERIFY(canonical.contains("\"b\""));
    }

    void testMergeObjects()
    {
        QJsonObject base;
        base["a"] = 1;
        base["b"] = 2;
        QJsonObject overlay;
        overlay["b"] = 3;
        overlay["c"] = 4;

        QJsonObject merged = progressive_chat::util::mergeObjects(base, overlay);
        QCOMPARE(merged["a"].toInt(), 1);
        QCOMPARE(merged["b"].toInt(), 3);
        QCOMPARE(merged["c"].toInt(), 4);
    }
};

// ===== Network Utility Tests =====
class NetworkUtilTests : public QObject
{
    Q_OBJECT

private slots:
    void testIsValidUrl()
    {
        QVERIFY(progressive_chat::util::isValidUrl("https://matrix.org"));
        QVERIFY(progressive_chat::util::isValidUrl("http://localhost:8008"));
        QVERIFY(!progressive_chat::util::isValidUrl("not a url"));
        QVERIFY(!progressive_chat::util::isValidUrl(""));
    }

    void testIsMxcUrl()
    {
        QVERIFY(progressive_chat::util::isMxcUrl("mxc://matrix.org/abc123"));
        QVERIFY(!progressive_chat::util::isMxcUrl("https://matrix.org/abc123"));
    }

    void testParseMxcUrl()
    {
        auto [server, mediaId] = progressive_chat::util::parseMxcUrl(
            "mxc://matrix.org/abc123def");
        QCOMPARE(server, QString("matrix.org"));
        QCOMPARE(mediaId, QString("abc123def"));
    }

    void testMxcToHttp()
    {
        QString http = progressive_chat::util::mxcToHttp(
            "mxc://matrix.org/abc123", "https://matrix.org");
        QVERIFY(http.contains("_matrix/media/v3/download"));
        QVERIFY(http.contains("matrix.org"));
        QVERIFY(http.contains("abc123"));
    }
};

// ===== File Utility Tests =====
class FileUtilTests : public QObject
{
    Q_OBJECT

private slots:
    void testFileExtension()
    {
        QCOMPARE(progressive_chat::util::fileExtension("/path/to/file.jpg"),
                 QString("jpg"));
        QCOMPARE(progressive_chat::util::fileExtension("test.PNG"),
                 QString("png"));
    }

    void testFileName()
    {
        QCOMPARE(progressive_chat::util::fileName("/path/to/document.pdf"),
                 QString("document.pdf"));
    }

    void testSanitizeFilename()
    {
        QCOMPARE(progressive_chat::util::sanitizeFilename("file<name>:with?chars"),
                 QString("file_name__with_chars"));
    }
};

// ===== Matrix Push Rules Tests =====
class MatrixPushRulesTests : public QObject
{
    Q_OBJECT

private slots:
    void testParseRule()
    {
        QJsonObject ruleObj;
        ruleObj["rule_id"] = ".m.rule.contains_user_name";
        ruleObj["default"] = true;
        ruleObj["enabled"] = true;
        ruleObj["priority_class"] = 3;
        ruleObj["actions"] = QJsonArray{"notify", QJsonObject{{"set_tweak", "sound"}, {"value", "default"}}};

        auto rule = progressive_chat::MatrixPushRules::parseRule(ruleObj);
        QCOMPARE(rule.ruleId, QString(".m.rule.contains_user_name"));
        QVERIFY(rule.isDefault);
        QVERIFY(rule.enabled);
        QCOMPARE(rule.priority, 3);
        QVERIFY(rule.actions.contains(progressive_chat::PushAction::Notify));
        QCOMPARE(rule.tweaks["sound"], QString("default"));
    }

    void testSerializeRule()
    {
        progressive_chat::PushRule rule;
        rule.ruleId = ".m.rule.test";
        rule.actions.append(progressive_chat::PushAction::Notify);
        rule.tweaks["sound"] = "custom.wav";

        QJsonObject obj = progressive_chat::MatrixPushRules::serializeRule(rule);
        QCOMPARE(obj["rule_id"].toString(), QString(".m.rule.test"));
    }

    void testShouldNotify()
    {
        progressive_chat::PushRule notifyRule;
        notifyRule.actions.append(progressive_chat::PushAction::Notify);

        progressive_chat::MatrixPushRules rules;
        QVERIFY(rules.shouldNotify(notifyRule));

        progressive_chat::PushRule silentRule;
        silentRule.actions.append(progressive_chat::PushAction::DontNotify);
        QVERIFY(!rules.shouldNotify(silentRule));
    }
};

// ===== Matrix Session Tests =====
class MatrixSessionTests : public QObject
{
    Q_OBJECT

private slots:
    void testSessionDefaults()
    {
        QNetworkAccessManager network;
        progressive_chat::MatrixSession session(&network);
        QCOMPARE(session.protocolType(), progressive_chat::ProtocolType::Matrix);
        QVERIFY(!session.isConnected());
        QVERIFY(session.homeserver().isEmpty());
        QVERIFY(session.accessToken().isEmpty());
    }

    void testSessionConfiguration()
    {
        QNetworkAccessManager network;
        progressive_chat::MatrixSession session(&network);
        session.setHomeserver("https://matrix.org");
        session.setAccessToken("test_token");
        session.setDeviceId("test_device");

        QCOMPARE(session.homeserver(), QString("https://matrix.org"));
        QCOMPARE(session.accessToken(), QString("test_token"));
    }

    void testSessionSignals()
    {
        QNetworkAccessManager network;
        progressive_chat::MatrixSession session(&network);

        QSignalSpy connectedSpy(&session, &progressive_chat::MatrixSession::connected);
        QSignalSpy disconnectedSpy(&session, &progressive_chat::MatrixSession::disconnected);
        QSignalSpy errorSpy(&session, &progressive_chat::MatrixSession::connectionError);

        QCOMPARE(connectedSpy.count(), 0);
        QCOMPARE(disconnectedSpy.count(), 0);
        QCOMPARE(errorSpy.count(), 0);
    }
};

// ===== Theme Manager Tests =====
class ThemeManagerTests : public QObject
{
    Q_OBJECT

private slots:
    void testDefaultTheme()
    {
        progressive_chat::ThemeManager manager;
        QCOMPARE(manager.activeThemeName(), QString("Light"));
    }

    void testThemeSwitch()
    {
        progressive_chat::ThemeManager manager;
        QSignalSpy themeSpy(&manager, &progressive_chat::ThemeManager::themeChanged);

        manager.applyTheme(progressive_chat::ThemeManager::Theme::Dark);
        QCOMPARE(themeSpy.count(), 1);
        QCOMPARE(manager.activeThemeName(), QString("Dark"));
    }

    void testColorsAfterThemeSwitch()
    {
        progressive_chat::ThemeManager manager;
        manager.applyTheme(progressive_chat::ThemeManager::Theme::Dark);

        QColor bg = manager.backgroundColor();
        QVERIFY(progressive_chat::util::isDarkColor(bg));
    }
};

// ===== Main Test Runner =====
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);

    int failures = 0;

    auto runTest = [&](QObject *test) {
        int result = QTest::qExec(test, argc, argv);
        if (result != 0) failures++;
        delete test;
    };

    std::cout << "=== Protocol Type Tests ===" << std::endl;
    runTest(new ProtocolTypeTests());

    std::cout << "\n=== Protocol Message Tests ===" << std::endl;
    runTest(new ProtocolMessageTests());

    std::cout << "\n=== Protocol Room Tests ===" << std::endl;
    runTest(new ProtocolRoomTests());

    std::cout << "\n=== String Utility Tests ===" << std::endl;
    runTest(new StringUtilTests());

    std::cout << "\n=== Date Utility Tests ===" << std::endl;
    runTest(new DateUtilTests());

    std::cout << "\n=== Color Utility Tests ===" << std::endl;
    runTest(new ColorUtilTests());

    std::cout << "\n=== JSON Utility Tests ===" << std::endl;
    runTest(new JsonUtilTests());

    std::cout << "\n=== Network Utility Tests ===" << std::endl;
    runTest(new NetworkUtilTests());

    std::cout << "\n=== File Utility Tests ===" << std::endl;
    runTest(new FileUtilTests());

    std::cout << "\n=== Matrix Push Rules Tests ===" << std::endl;
    runTest(new MatrixPushRulesTests());

    std::cout << "\n=== Matrix Session Tests ===" << std::endl;
    runTest(new MatrixSessionTests());

    std::cout << "\n=== Theme Manager Tests ===" << std::endl;
    runTest(new ThemeManagerTests());

    std::cout << "\n============================" << std::endl;
    if (failures == 0) {
        std::cout << "All tests passed!" << std::endl;
        return 0;
    } else {
        std::cout << failures << " test suite(s) failed." << std::endl;
        return 1;
    }
}

#include "test_main.moc"
