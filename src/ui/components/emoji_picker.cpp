#include "emoji_picker.hpp"
#include <QShowEvent>
#include <QFocusEvent>
#include <QGuiApplication>
#include <QScreen>

QMap<QString, QStringList> EmojiPicker::s_emojis = {
    {QStringLiteral("Smileys"), {
        // Grinning / happy
        QStringLiteral("\xF0\x9F\x98\x80"), // 😀
        QStringLiteral("\xF0\x9F\x98\x83"), // 😃
        QStringLiteral("\xF0\x9F\x98\x84"), // 😄
        QStringLiteral("\xF0\x9F\x98\x81"), // 😁
        QStringLiteral("\xF0\x9F\x98\x86"), // 😆
        QStringLiteral("\xF0\x9F\x98\x85"), // 😅
        QStringLiteral("\xF0\x9F\xA4\xA3"), // 🤣
        QStringLiteral("\xF0\x9F\x98\x82"), // 😂
        QStringLiteral("\xF0\x9F\x99\x82"), // 🙂
        QStringLiteral("\xF0\x9F\x99\x83"), // 🙃
        QStringLiteral("\xF0\x9F\x98\x89"), // 😉
        QStringLiteral("\xF0\x9F\x98\x8A"), // 😊
        QStringLiteral("\xF0\x9F\x98\x87"), // 😇
        // Love
        QStringLiteral("\xF0\x9F\xA5\xB0"), // 🥰
        QStringLiteral("\xF0\x9F\x98\x8D"), // 😍
        QStringLiteral("\xF0\x9F\xA4\xA9"), // 🤩
        QStringLiteral("\xF0\x9F\x98\x98"), // 😘
        QStringLiteral("\xF0\x9F\x98\x97"), // 😗
        QStringLiteral("\xE2\x98\xBA"),       // ☺
        QStringLiteral("\xF0\x9F\x98\x9A"), // 😚
        QStringLiteral("\xF0\x9F\x98\x99"), // 😙
        // Tongue / silly
        QStringLiteral("\xF0\x9F\x98\x8B"), // 😋
        QStringLiteral("\xF0\x9F\x98\x9B"), // 😛
        QStringLiteral("\xF0\x9F\x98\x9C"), // 😜
        QStringLiteral("\xF0\x9F\xA4\xAA"), // 🤪
        QStringLiteral("\xF0\x9F\x98\x9D"), // 😝
        QStringLiteral("\xF0\x9F\xA4\x97"), // 🤑
        // Hands / hugs
        QStringLiteral("\xF0\x9F\xA4\x97"), // 🤗
        QStringLiteral("\xF0\x9F\xA4\xAD"), // 🤭
        QStringLiteral("\xF0\x9F\xA4\xAB"), // 🤫
        QStringLiteral("\xF0\x9F\xA4\x94"), // 🤔
        // Neutral / skeptical
        QStringLiteral("\xF0\x9F\xA4\x90"), // 😐
        QStringLiteral("\xF0\x9F\x98\x90"), // 😐 (neutral)
        QStringLiteral("\xF0\x9F\x98\x91"), // 😑
        QStringLiteral("\xF0\x9F\x98\xB6"), // 😶
        QStringLiteral("\xF0\x9F\x98\x8F"), // 😏
        QStringLiteral("\xF0\x9F\x98\x92"), // 😒
        QStringLiteral("\xF0\x9F\x99\x84"), // 🙄
        QStringLiteral("\xF0\x9F\x98\xAC"), // 😬
        QStringLiteral("\xF0\x9F\x98\xAE"), // 😮
        // Sleepy / sick
        QStringLiteral("\xF0\x9F\xA4\xA5"), // 🤥
        QStringLiteral("\xF0\x9F\x98\x8C"), // 😌
        QStringLiteral("\xF0\x9F\x98\x94"), // 😔
        QStringLiteral("\xF0\x9F\x98\xAA"), // 😪
        QStringLiteral("\xF0\x9F\xA4\xA4"), // 🤤
        QStringLiteral("\xF0\x9F\x98\xB4"), // 😴
        QStringLiteral("\xF0\x9F\x98\xB7"), // 😷
        QStringLiteral("\xF0\x9F\xA4\x92"), // 🤒
        QStringLiteral("\xF0\x9F\xA4\xA7"), // 🤧
        // Cool / nerdy
        QStringLiteral("\xF0\x9F\xA5\xB5"), // 🥵
        QStringLiteral("\xF0\x9F\xA5\xB6"), // 🥶
        QStringLiteral("\xF0\x9F\xA5\xB4"), // 🥴
        QStringLiteral("\xF0\x9F\x98\xB5"), // 😵
        QStringLiteral("\xF0\x9F\xA4\xAF"), // 🤯
        QStringLiteral("\xF0\x9F\xA4\xA0"), // 🤠
        QStringLiteral("\xF0\x9F\xA5\xB3"), // 🥳
        QStringLiteral("\xF0\x9F\x98\x8E"), // 😎
        // Worried / sad
        QStringLiteral("\xF0\x9F\x98\x95"), // 😕
        QStringLiteral("\xF0\x9F\x98\x9F"), // 😟
        QStringLiteral("\xF0\x9F\x99\x81"), // 🙁
        QStringLiteral("\xE2\x98\xB9"),       // ☹
        QStringLiteral("\xF0\x9F\x98\xAE"), // 😮
        QStringLiteral("\xF0\x9F\x98\xAF"), // 😯
        QStringLiteral("\xF0\x9F\x98\xB2"), // 😲
        QStringLiteral("\xF0\x9F\x98\xB3"), // 😳
        QStringLiteral("\xF0\x9F\xA5\xBA"), // 🥺
        // Crying
        QStringLiteral("\xF0\x9F\x98\xA6"), // 😦
        QStringLiteral("\xF0\x9F\x98\xA7"), // 😧
        QStringLiteral("\xF0\x9F\x98\xA8"), // 😨
        QStringLiteral("\xF0\x9F\x98\xB0"), // 😰
        QStringLiteral("\xF0\x9F\x98\xA5"), // 😥
        QStringLiteral("\xF0\x9F\x98\xA2"), // 😢
        QStringLiteral("\xF0\x9F\x98\xAD"), // 😭
        QStringLiteral("\xF0\x9F\x98\xB1"), // 😱
        // Angry
        QStringLiteral("\xF0\x9F\x98\x96"), // 😖
        QStringLiteral("\xF0\x9F\x98\xA3"), // 😣
        QStringLiteral("\xF0\x9F\x98\x9E"), // 😞
        QStringLiteral("\xF0\x9F\x98\x93"), // 😓
        QStringLiteral("\xF0\x9F\x98\xA9"), // 😩
        QStringLiteral("\xF0\x9F\x98\xAB"), // 😫
        QStringLiteral("\xF0\x9F\xA5\xB1"), // 🥱
        QStringLiteral("\xF0\x9F\x98\xA4"), // 😤
        QStringLiteral("\xF0\x9F\x98\xA0"), // 😡
        QStringLiteral("\xF0\x9F\x98\xA1"), // 😠
        QStringLiteral("\xF0\x9F\xA4\xAC"), // 🤬
    }},
    {QStringLiteral("\xF0\x9F\x91\xA8"), {
        // 👨 People
        QStringLiteral("\xF0\x9F\x91\xB6"), // 👶
        QStringLiteral("\xF0\x9F\xA7\x92"), // 🧒
        QStringLiteral("\xF0\x9F\x91\xA6"), // 👦
        QStringLiteral("\xF0\x9F\x91\xA7"), // 👧
        QStringLiteral("\xF0\x9F\xA7\x91"), // 🧑
        QStringLiteral("\xF0\x9F\x91\xB1"), // 👱
        QStringLiteral("\xF0\x9F\x91\xA8"), // 👨
        QStringLiteral("\xF0\x9F\xA7\x94"), // 🧔
        QStringLiteral("\xF0\x9F\x91\xA9"), // 👩
        QStringLiteral("\xF0\x9F\x91\xB4"), // 👴
        QStringLiteral("\xF0\x9F\x91\xB5"), // 👵
        QStringLiteral("\xF0\x9F\x99\x8D"), // 🙍
        QStringLiteral("\xF0\x9F\x99\x8E"), // 🙎
        QStringLiteral("\xF0\x9F\x99\x85"), // 🙅
        QStringLiteral("\xF0\x9F\x99\x86"), // 🙆
        QStringLiteral("\xF0\x9F\x92\x81"), // 💁
        QStringLiteral("\xF0\x9F\x99\x8B"), // 🙋
        QStringLiteral("\xF0\x9F\x99\x87"), // 🙇
        QStringLiteral("\xF0\x9F\xA7\x8F"), // 🧏
        QStringLiteral("\xF0\x9F\xA4\xA6"), // 🤦
        QStringLiteral("\xF0\x9F\xA4\xB7"), // 🤷
        QStringLiteral("\xF0\x9F\x91\xAE"), // 👮
        QStringLiteral("\xF0\x9F\x95\xB5"), // 🕵
        QStringLiteral("\xF0\x9F\x92\x82"), // 💂
        QStringLiteral("\xF0\x9F\xA5\xB7"), // 🥷
        QStringLiteral("\xF0\x9F\x91\xB7"), // 👷
        QStringLiteral("\xF0\x9F\xA4\xB4"), // 🤴
        QStringLiteral("\xF0\x9F\x91\xB8"), // 👸
        QStringLiteral("\xF0\x9F\x91\xB3"), // 👳
        QStringLiteral("\xF0\x9F\x91\xB2"), // 👲
        QStringLiteral("\xF0\x9F\xA7\x95"), // 🧕
        QStringLiteral("\xF0\x9F\xA4\xB5"), // 🤵
        QStringLiteral("\xF0\x9F\x91\xB0"), // 👰
        QStringLiteral("\xF0\x9F\xA4\xB0"), // 🤰
        QStringLiteral("\xF0\x9F\xA4\xB1"), // 🤱
        QStringLiteral("\xF0\x9F\x91\xBC"), // 👼
        QStringLiteral("\xF0\x9F\x8E\x85"), // 🎅
        QStringLiteral("\xF0\x9F\xA4\xB6"), // 🤶
        QStringLiteral("\xF0\x9F\xA6\xB8"), // 🦸
        QStringLiteral("\xF0\x9F\xA6\xB9"), // 🦹
        QStringLiteral("\xF0\x9F\xA7\x99"), // 🧙
        QStringLiteral("\xF0\x9F\xA7\x9A"), // 🧚
        QStringLiteral("\xF0\x9F\xA7\x9B"), // 🧛
        QStringLiteral("\xF0\x9F\xA7\x9C"), // 🧜
        QStringLiteral("\xF0\x9F\xA7\x9D"), // 🧝
        QStringLiteral("\xF0\x9F\xA7\x9E"), // 🧞
        QStringLiteral("\xF0\x9F\xA7\x9F"), // 🧟
        QStringLiteral("\xF0\x9F\x92\x86"), // 💆
        QStringLiteral("\xF0\x9F\x92\x87"), // 💇
        QStringLiteral("\xF0\x9F\x9A\xB6"), // 🚶
        QStringLiteral("\xF0\x9F\x8F\x83"), // 🏃
        QStringLiteral("\xF0\x9F\x92\x83"), // 💃
        QStringLiteral("\xF0\x9F\x95\xBA"), // 🕺
        QStringLiteral("\xF0\x9F\x91\xAB"), // 👫
        QStringLiteral("\xF0\x9F\x91\xAC"), // 👬
        QStringLiteral("\xF0\x9F\x91\xAD"), // 👭
        QStringLiteral("\xF0\x9F\xA7\x91\xE2\x80\x8D\xF0\x9F\xA4\x9D\xE2\x80\x8D\xF0\x9F\xA7\x91"), // 🧑‍🤝‍🧑
        QStringLiteral("\xF0\x9F\x91\xAA"), // 👪
        QStringLiteral("\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA6"), // 👨‍👩‍👦
        QStringLiteral("\xF0\x9F\x92\xAA"), // 💪
        QStringLiteral("\xF0\x9F\xA6\xB5"), // 🦵
        QStringLiteral("\xF0\x9F\xA6\xB6"), // 🦶
        QStringLiteral("\xF0\x9F\x91\x82"), // 👂
        QStringLiteral("\xF0\x9F\x91\x83"), // 👃
        QStringLiteral("\xF0\x9F\xA7\xA0"), // 🧠
        QStringLiteral("\xF0\x9F\xA6\xB7"), // 🦷
        QStringLiteral("\xF0\x9F\xA6\xB4"), // 🦴
        QStringLiteral("\xF0\x9F\x91\x80"), // 👀
        QStringLiteral("\xF0\x9F\x91\x81"), // 👁
        QStringLiteral("\xF0\x9F\x91\x85"), // 👅
        QStringLiteral("\xF0\x9F\x91\x84"), // 👄
    }},
    {QStringLiteral("Animals"), {
        QStringLiteral("\xF0\x9F\x90\xB6"), // 🐶
        QStringLiteral("\xF0\x9F\x90\xB1"), // 🐱
        QStringLiteral("\xF0\x9F\x90\xAD"), // 🐭
        QStringLiteral("\xF0\x9F\x90\xB9"), // 🐹
        QStringLiteral("\xF0\x9F\x90\xB0"), // 🐰
        QStringLiteral("\xF0\x9F\xA6\x8A"), // 🦊
        QStringLiteral("\xF0\x9F\x90\xBB"), // 🐻
        QStringLiteral("\xF0\x9F\x90\xBC"), // 🐼
        QStringLiteral("\xF0\x9F\x90\xA8"), // 🐨
        QStringLiteral("\xF0\x9F\x90\xAF"), // 🐯
        QStringLiteral("\xF0\x9F\xA6\x81"), // 🦁
        QStringLiteral("\xF0\x9F\x90\xAE"), // 🐮
        QStringLiteral("\xF0\x9F\x90\xB7"), // 🐷
        QStringLiteral("\xF0\x9F\x90\xB8"), // 🐸
        QStringLiteral("\xF0\x9F\x90\xB5"), // 🐵
        QStringLiteral("\xF0\x9F\x99\x88"), // 🙈
        QStringLiteral("\xF0\x9F\x99\x89"), // 🙉
        QStringLiteral("\xF0\x9F\x99\x8A"), // 🙊
        QStringLiteral("\xF0\x9F\x90\x92"), // 🐒
        QStringLiteral("\xF0\x9F\x90\x94"), // 🐔
        QStringLiteral("\xF0\x9F\x90\xA7"), // 🐧
        QStringLiteral("\xF0\x9F\x90\xA6"), // 🐦
        QStringLiteral("\xF0\x9F\x90\xA4"), // 🐤
        QStringLiteral("\xF0\x9F\x90\xA3"), // 🐣
        QStringLiteral("\xF0\x9F\x90\xA5"), // 🐥
        QStringLiteral("\xF0\x9F\xA6\x86"), // 🦆
        QStringLiteral("\xF0\x9F\xA6\x85"), // 🦅
        QStringLiteral("\xF0\x9F\xA6\x89"), // 🦉
        QStringLiteral("\xF0\x9F\xA6\x87"), // 🦇
        QStringLiteral("\xF0\x9F\x90\xBA"), // 🐺
        QStringLiteral("\xF0\x9F\x90\x97"), // 🐗
        QStringLiteral("\xF0\x9F\x90\xB4"), // 🐴
        QStringLiteral("\xF0\x9F\xA6\x84"), // 🦄
        QStringLiteral("\xF0\x9F\x90\x9D"), // 🐝
        QStringLiteral("\xF0\x9F\x90\x9B"), // 🐛
        QStringLiteral("\xF0\x9F\xA6\x8B"), // 🦋
        QStringLiteral("\xF0\x9F\x90\x8C"), // 🐌
        QStringLiteral("\xF0\x9F\x90\x9E"), // 🐞
        QStringLiteral("\xF0\x9F\x90\x9C"), // 🐜
        QStringLiteral("\xF0\x9F\xA6\x9F"), // 🦟
        QStringLiteral("\xF0\x9F\xA6\x97"), // 🦗
        QStringLiteral("\xF0\x9F\x95\xB7"), // 🕷
        QStringLiteral("\xF0\x9F\x95\xB8"), // 🕸
        QStringLiteral("\xF0\x9F\xA6\x82"), // 🦂
        QStringLiteral("\xF0\x9F\x90\xA2"), // 🐢
        QStringLiteral("\xF0\x9F\x90\x8D"), // 🐍
        QStringLiteral("\xF0\x9F\xA6\x8E"), // 🦎
        QStringLiteral("\xF0\x9F\xA6\x96"), // 🦖
        QStringLiteral("\xF0\x9F\xA6\x95"), // 🦕
        QStringLiteral("\xF0\x9F\x90\x99"), // 🐙
        QStringLiteral("\xF0\x9F\xA6\x91"), // 🦑
        QStringLiteral("\xF0\x9F\xA6\x90"), // 🦐
        QStringLiteral("\xF0\x9F\xA6\x9E"), // 🦞
        QStringLiteral("\xF0\x9F\xA6\x80"), // 🦀
        QStringLiteral("\xF0\x9F\x90\xA1"), // 🐡
        QStringLiteral("\xF0\x9F\x90\xA0"), // 🐠
        QStringLiteral("\xF0\x9F\x90\x9F"), // 🐟
        QStringLiteral("\xF0\x9F\x90\xAC"), // 🐬
        QStringLiteral("\xF0\x9F\x90\xB3"), // 🐳
        QStringLiteral("\xF0\x9F\x90\x8B"), // 🐋
        QStringLiteral("\xF0\x9F\xA6\x88"), // 🦈
        QStringLiteral("\xF0\x9F\x90\x8A"), // 🐊
        QStringLiteral("\xF0\x9F\x90\x85"), // 🐅
        QStringLiteral("\xF0\x9F\x90\x86"), // 🐆
        QStringLiteral("\xF0\x9F\xA6\x93"), // 🦓
        QStringLiteral("\xF0\x9F\xA6\x8D"), // 🦍
        QStringLiteral("\xF0\x9F\xA6\xA7"), // 🦧
        QStringLiteral("\xF0\x9F\x90\x98"), // 🐘
        QStringLiteral("\xF0\x9F\xA6\x9B"), // 🦛
        QStringLiteral("\xF0\x9F\xA6\x8F"), // 🦏
        QStringLiteral("\xF0\x9F\x90\xAA"), // 🐪
        QStringLiteral("\xF0\x9F\x90\xAB"), // 🐫
        QStringLiteral("\xF0\x9F\xA6\x92"), // 🦒
        QStringLiteral("\xF0\x9F\xA6\x98"), // 🦘
        QStringLiteral("\xF0\x9F\x90\x83"), // 🐃
        QStringLiteral("\xF0\x9F\x90\x82"), // 🐂
        QStringLiteral("\xF0\x9F\x90\x84"), // 🐄
        QStringLiteral("\xF0\x9F\x90\x96"), // 🐖
        QStringLiteral("\xF0\x9F\x90\x8F"), // 🐏
        QStringLiteral("\xF0\x9F\x90\x91"), // 🐑
        QStringLiteral("\xF0\x9F\xA6\x99"), // 🦙
        QStringLiteral("\xF0\x9F\x90\x90"), // 🐐
        QStringLiteral("\xF0\x9F\xA6\x8C"), // 🦌
        QStringLiteral("\xF0\x9F\x90\x95"), // 🐕
        QStringLiteral("\xF0\x9F\x90\xA9"), // 🐩
        QStringLiteral("\xF0\x9F\xA6\xAE"), // 🦮
        QStringLiteral("\xF0\x9F\x90\x88"), // 🐈
        QStringLiteral("\xF0\x9F\x90\x87"), // 🐇
        QStringLiteral("\xF0\x9F\xA6\x9D"), // 🦝
        QStringLiteral("\xF0\x9F\xA6\xA8"), // 🦨
        QStringLiteral("\xF0\x9F\xA6\xA1"), // 🦡
        QStringLiteral("\xF0\x9F\xA6\xAB"), // 🦫
        QStringLiteral("\xF0\x9F\xA6\xA6"), // 🦦
        QStringLiteral("\xF0\x9F\xA6\xA5"), // 🦥
        QStringLiteral("\xF0\x9F\x90\x81"), // 🐁
        QStringLiteral("\xF0\x9F\x90\x80"), // 🐀
        QStringLiteral("\xF0\x9F\x90\xBF"), // 🐿
        QStringLiteral("\xF0\x9F\xA6\x94"), // 🦔
        QStringLiteral("\xF0\x9F\x90\xBE"), // 🐾
        QStringLiteral("\xF0\x9F\x90\x89"), // 🐉
        QStringLiteral("\xF0\x9F\x90\xB2"), // 🐲
    }},
    {QStringLiteral("Food"), {
        QStringLiteral("\xF0\x9F\x8D\x8F"), // 🍏
        QStringLiteral("\xF0\x9F\x8D\x8E"), // 🍎
        QStringLiteral("\xF0\x9F\x8D\x90"), // 🍐
        QStringLiteral("\xF0\x9F\x8D\x8A"), // 🍊
        QStringLiteral("\xF0\x9F\x8D\x8B"), // 🍋
        QStringLiteral("\xF0\x9F\x8D\x8C"), // 🍌
        QStringLiteral("\xF0\x9F\x8D\x89"), // 🍉
        QStringLiteral("\xF0\x9F\x8D\x87"), // 🍇
        QStringLiteral("\xF0\x9F\x8D\x93"), // 🍓
        QStringLiteral("\xF0\x9F\xAB\x90"), // 🫐
        QStringLiteral("\xF0\x9F\x8D\x88"), // 🍈
        QStringLiteral("\xF0\x9F\x8D\x92"), // 🍒
        QStringLiteral("\xF0\x9F\x8D\x91"), // 🍑
        QStringLiteral("\xF0\x9F\xA5\xAD"), // 🥭
        QStringLiteral("\xF0\x9F\x8D\x8D"), // 🍍
        QStringLiteral("\xF0\x9F\xA5\xA5"), // 🥥
        QStringLiteral("\xF0\x9F\xA5\x9D"), // 🥝
        QStringLiteral("\xF0\x9F\x8D\x85"), // 🍅
        QStringLiteral("\xF0\x9F\x8D\x86"), // 🍆
        QStringLiteral("\xF0\x9F\xA5\x91"), // 🥑
        QStringLiteral("\xF0\x9F\xA5\xA6"), // 🥦
        QStringLiteral("\xF0\x9F\xA5\xAC"), // 🥬
        QStringLiteral("\xF0\x9F\xA5\x92"), // 🥒
        QStringLiteral("\xF0\x9F\x8C\xB6"), // 🌶
        QStringLiteral("\xF0\x9F\x8C\xBD"), // 🌽
        QStringLiteral("\xF0\x9F\xA5\x95"), // 🥕
        QStringLiteral("\xF0\x9F\xA7\x84"), // 🧄
        QStringLiteral("\xF0\x9F\xA7\x85"), // 🧅
        QStringLiteral("\xF0\x9F\xA5\x94"), // 🥔
        QStringLiteral("\xF0\x9F\x8D\xA0"), // 🍠
        QStringLiteral("\xF0\x9F\xA5\x90"), // 🥐
        QStringLiteral("\xF0\x9F\x8D\x9E"), // 🍞
        QStringLiteral("\xF0\x9F\xA5\x96"), // 🥖
        QStringLiteral("\xF0\x9F\xA5\xA8"), // 🥨
        QStringLiteral("\xF0\x9F\xA7\x80"), // 🧀
        QStringLiteral("\xF0\x9F\xA5\x9A"), // 🥚
        QStringLiteral("\xF0\x9F\x8D\xB3"), // 🍳
        QStringLiteral("\xF0\x9F\xA7\x88"), // 🧈
        QStringLiteral("\xF0\x9F\xA5\x9E"), // 🥞
        QStringLiteral("\xF0\x9F\xA7\x87"), // 🧇
        QStringLiteral("\xF0\x9F\xA5\x93"), // 🥓
        QStringLiteral("\xF0\x9F\xA5\xA9"), // 🥩
        QStringLiteral("\xF0\x9F\x8D\x97"), // 🍗
        QStringLiteral("\xF0\x9F\x8D\x96"), // 🍖
        QStringLiteral("\xF0\x9F\xA6\xB4"), // 🦴
        QStringLiteral("\xF0\x9F\x8C\xAD"), // 🌭
        QStringLiteral("\xF0\x9F\x8D\x94"), // 🍔
        QStringLiteral("\xF0\x9F\x8D\x9F"), // 🍟
        QStringLiteral("\xF0\x9F\x8D\x95"), // 🍕
        QStringLiteral("\xF0\x9F\xA5\xAA"), // 🥪
        QStringLiteral("\xF0\x9F\xA5\x99"), // 🥙
        QStringLiteral("\xF0\x9F\xA7\x86"), // 🧆
        QStringLiteral("\xF0\x9F\x8C\xAE"), // 🌮
        QStringLiteral("\xF0\x9F\x8C\xAF"), // 🌯
        QStringLiteral("\xF0\x9F\xA5\x98"), // 🥘
        QStringLiteral("\xF0\x9F\x8D\x9D"), // 🍝
        QStringLiteral("\xF0\x9F\x8D\x9C"), // 🍜
        QStringLiteral("\xF0\x9F\x8D\xB2"), // 🍲
        QStringLiteral("\xF0\x9F\x8D\x9B"), // 🍛
        QStringLiteral("\xF0\x9F\x8D\xA3"), // 🍣
        QStringLiteral("\xF0\x9F\x8D\xB1"), // 🍱
        QStringLiteral("\xF0\x9F\xA5\x9F"), // 🥟
        QStringLiteral("\xF0\x9F\xA6\xAA"), // 🦪
        QStringLiteral("\xF0\x9F\x8D\xA4"), // 🍤
        QStringLiteral("\xF0\x9F\x8D\x99"), // 🍙
        QStringLiteral("\xF0\x9F\x8D\x9A"), // 🍚
        QStringLiteral("\xF0\x9F\x8D\x98"), // 🍘
        QStringLiteral("\xF0\x9F\x8D\xA5"), // 🍥
        QStringLiteral("\xF0\x9F\x8D\xA2"), // 🍢
        QStringLiteral("\xF0\x9F\x8D\xA1"), // 🍡
        QStringLiteral("\xF0\x9F\x8D\xA7"), // 🍧
        QStringLiteral("\xF0\x9F\x8D\xA8"), // 🍨
        QStringLiteral("\xF0\x9F\x8D\xA6"), // 🍦
        QStringLiteral("\xF0\x9F\xA5\xA7"), // 🥧
        QStringLiteral("\xF0\x9F\xA7\x81"), // 🧁
        QStringLiteral("\xF0\x9F\x8D\xB0"), // 🍰
        QStringLiteral("\xF0\x9F\x8E\x82"), // 🎂
        QStringLiteral("\xF0\x9F\x8D\xAE"), // 🍮
        QStringLiteral("\xF0\x9F\x8D\xAD"), // 🍭
        QStringLiteral("\xF0\x9F\x8D\xAC"), // 🍬
        QStringLiteral("\xF0\x9F\x8D\xAB"), // 🍫
        QStringLiteral("\xF0\x9F\x8D\xBF"), // 🍿
        QStringLiteral("\xF0\x9F\x8D\xA9"), // 🍩
        QStringLiteral("\xF0\x9F\x8D\xAA"), // 🍪
        QStringLiteral("\xF0\x9F\x8C\xB0"), // 🌰
        QStringLiteral("\xF0\x9F\xA5\x9C"), // 🥜
        QStringLiteral("\xF0\x9F\x8D\xAF"), // 🍯
        QStringLiteral("\xF0\x9F\xA5\x9B"), // 🥛
        QStringLiteral("\xF0\x9F\x8D\xBC"), // 🍼
        QStringLiteral("\xE2\x98\x95"),       // ☕
        QStringLiteral("\xF0\x9F\x8D\xB5"), // 🍵
        QStringLiteral("\xF0\x9F\xA7\x83"), // 🧃
        QStringLiteral("\xF0\x9F\xA5\xA4"), // 🥤
        QStringLiteral("\xF0\x9F\x8D\xB6"), // 🍶
        QStringLiteral("\xF0\x9F\x8D\xBA"), // 🍺
        QStringLiteral("\xF0\x9F\x8D\xBB"), // 🍻
        QStringLiteral("\xF0\x9F\xA5\x82"), // 🥂
        QStringLiteral("\xF0\x9F\x8D\xB7"), // 🍷
        QStringLiteral("\xF0\x9F\xA5\x83"), // 🥃
        QStringLiteral("\xF0\x9F\x8D\xB8"), // 🍸
        QStringLiteral("\xF0\x9F\x8D\xB9"), // 🍹
        QStringLiteral("\xF0\x9F\xA7\x89"), // 🧉
        QStringLiteral("\xF0\x9F\x8D\xBE"), // 🍾
        QStringLiteral("\xF0\x9F\xA7\x8A"), // 🧊
        QStringLiteral("\xF0\x9F\xA5\x84"), // 🥄
        QStringLiteral("\xF0\x9F\x8D\xB4"), // 🍴
    }},
    {QStringLiteral("Travel"), {
        QStringLiteral("\xF0\x9F\x9A\x97"), // 🚗
        QStringLiteral("\xF0\x9F\x9A\x95"), // 🚕
        QStringLiteral("\xF0\x9F\x9A\x99"), // 🚙
        QStringLiteral("\xF0\x9F\x9A\x8C"), // 🚌
        QStringLiteral("\xF0\x9F\x9A\x8E"), // 🚎
        QStringLiteral("\xF0\x9F\x8F\x8E"), // 🏎
        QStringLiteral("\xF0\x9F\x9A\x93"), // 🚓
        QStringLiteral("\xF0\x9F\x9A\x91"), // 🚑
        QStringLiteral("\xF0\x9F\x9A\x92"), // 🚒
        QStringLiteral("\xF0\x9F\x9A\x90"), // 🚐
        QStringLiteral("\xF0\x9F\x9A\x9B"), // 🚛
        QStringLiteral("\xF0\x9F\x9A\x9A"), // 🚚
        QStringLiteral("\xF0\x9F\x9A\x9C"), // 🚜
        QStringLiteral("\xF0\x9F\x8F\x8D"), // 🏍
        QStringLiteral("\xF0\x9F\x9B\xB5"), // 🛵
        QStringLiteral("\xF0\x9F\x9A\xB2"), // 🚲
        QStringLiteral("\xF0\x9F\x9B\xB4"), // 🛴
        QStringLiteral("\xF0\x9F\x9A\xA8"), // 🚨
        QStringLiteral("\xF0\x9F\x9A\x94"), // 🚔
        QStringLiteral("\xF0\x9F\x9A\x8D"), // 🚍
        QStringLiteral("\xF0\x9F\x9A\x98"), // 🚘
        QStringLiteral("\xF0\x9F\x9A\x96"), // 🚖
        QStringLiteral("\xF0\x9F\x9A\xA1"), // 🚡
        QStringLiteral("\xF0\x9F\x9A\xA0"), // 🚠
        QStringLiteral("\xF0\x9F\x9A\x9F"), // 🚟
        QStringLiteral("\xF0\x9F\x9A\x83"), // 🚃
        QStringLiteral("\xF0\x9F\x9A\x8B"), // 🚋
        QStringLiteral("\xF0\x9F\x9A\x9E"), // 🚞
        QStringLiteral("\xF0\x9F\x9A\x9D"), // 🚝
        QStringLiteral("\xF0\x9F\x9A\x84"), // 🚄
        QStringLiteral("\xF0\x9F\x9A\x85"), // 🚅
        QStringLiteral("\xF0\x9F\x9A\x88"), // 🚈
        QStringLiteral("\xF0\x9F\x9A\x82"), // 🚂
        QStringLiteral("\xF0\x9F\x9A\x86"), // 🚆
        QStringLiteral("\xF0\x9F\x9A\x87"), // 🚇
        QStringLiteral("\xF0\x9F\x9A\x8A"), // 🚊
        QStringLiteral("\xF0\x9F\x9A\x89"), // 🚉
        QStringLiteral("\xE2\x9C\x88"),       // ✈
        QStringLiteral("\xF0\x9F\x9B\xAB"), // 🛫
        QStringLiteral("\xF0\x9F\x9B\xAC"), // 🛬
        QStringLiteral("\xF0\x9F\x92\xBA"), // 💺
        QStringLiteral("\xF0\x9F\x9B\xB0"), // 🛰
        QStringLiteral("\xF0\x9F\x9A\x80"), // 🚀
        QStringLiteral("\xF0\x9F\x9B\xB8"), // 🛸
        QStringLiteral("\xF0\x9F\x9A\x81"), // 🚁
        QStringLiteral("\xF0\x9F\x9B\xB6"), // 🛶
        QStringLiteral("\xE2\x9B\xB5"),       // ⛵
        QStringLiteral("\xF0\x9F\x9A\xA4"), // 🚤
        QStringLiteral("\xF0\x9F\x9B\xA5"), // 🛥
        QStringLiteral("\xF0\x9F\x9B\xB3"), // 🛳
        QStringLiteral("\xF0\x9F\x9A\xA2"), // 🚢
        QStringLiteral("\xE2\x9A\x93"),       // ⚓
        QStringLiteral("\xE2\x9B\xBD"),       // ⛽
        QStringLiteral("\xF0\x9F\x9A\xA7"), // 🚧
        QStringLiteral("\xF0\x9F\x9A\xA6"), // 🚦
        QStringLiteral("\xF0\x9F\x9A\xA5"), // 🚥
        QStringLiteral("\xF0\x9F\x97\xBA"), // 🗺
        QStringLiteral("\xF0\x9F\x97\xBF"), // 🗿
        QStringLiteral("\xF0\x9F\x97\xBD"), // 🗽
        QStringLiteral("\xF0\x9F\x97\xBC"), // 🗼
        QStringLiteral("\xF0\x9F\x8F\xB0"), // 🏰
        QStringLiteral("\xF0\x9F\x8F\xAF"), // 🏯
        QStringLiteral("\xF0\x9F\x8E\xA1"), // 🎡
        QStringLiteral("\xF0\x9F\x8E\xA2"), // 🎢
        QStringLiteral("\xF0\x9F\x8E\xA0"), // 🎠
        QStringLiteral("\xE2\x9B\xB2"),       // ⛲
        QStringLiteral("\xF0\x9F\x8F\x96"), // 🏖
        QStringLiteral("\xF0\x9F\x8F\x9D"), // 🏝
        QStringLiteral("\xF0\x9F\x8F\x9C"), // 🏜
        QStringLiteral("\xF0\x9F\x8C\x8B"), // 🌋
        QStringLiteral("\xF0\x9F\x97\xBB"), // 🗻
        QStringLiteral("\xF0\x9F\x8F\x95"), // 🏕
        QStringLiteral("\xF0\x9F\x8F\xA8"), // 🏨
        QStringLiteral("\xF0\x9F\x8F\xA6"), // 🏦
        QStringLiteral("\xF0\x9F\x8F\xA5"), // 🏥
        QStringLiteral("\xF0\x9F\x8F\xA4"), // 🏤
        QStringLiteral("\xF0\x9F\x8F\xA3"), // 🏣
        QStringLiteral("\xF0\x9F\x8F\xA2"), // 🏢
        QStringLiteral("\xF0\x9F\x8F\xAC"), // 🏬
        QStringLiteral("\xF0\x9F\x8F\xAD"), // 🏭
        QStringLiteral("\xF0\x9F\x8F\xAA"), // 🏪
        QStringLiteral("\xF0\x9F\x8F\xAB"), // 🏫
        QStringLiteral("\xF0\x9F\x8F\xA1"), // 🏡
        QStringLiteral("\xF0\x9F\x8F\xA0"), // 🏠
        QStringLiteral("\xF0\x9F\x8F\x97"), // 🏗
        QStringLiteral("\xF0\x9F\x8F\x99"), // 🏙
        QStringLiteral("\xF0\x9F\x8F\x9A"), // 🏚
        QStringLiteral("\xE2\x9B\xAA"),       // ⛪
        QStringLiteral("\xF0\x9F\x95\x8C"), // 🕌
        QStringLiteral("\xF0\x9F\x95\x8D"), // 🕍
        QStringLiteral("\xF0\x9F\x9B\x95"), // 🛕
        QStringLiteral("\xF0\x9F\x95\x8B"), // 🕋
    }},
    {QStringLiteral("Activities"), {
        QStringLiteral("\xE2\x9A\xBD"),       // ⚽
        QStringLiteral("\xF0\x9F\x8F\x80"), // 🏀
        QStringLiteral("\xF0\x9F\x8F\x88"), // 🏈
        QStringLiteral("\xE2\x9A\xBE"),       // ⚾
        QStringLiteral("\xF0\x9F\xA5\x8E"), // 🥎
        QStringLiteral("\xF0\x9F\x8E\xBE"), // 🎾
        QStringLiteral("\xF0\x9F\x8F\x90"), // 🏐
        QStringLiteral("\xF0\x9F\x8F\x89"), // 🏉
        QStringLiteral("\xF0\x9F\xA5\x8F"), // 🥏
        QStringLiteral("\xF0\x9F\x8E\xB1"), // 🎱
        QStringLiteral("\xF0\x9F\x8F\x93"), // 🏓
        QStringLiteral("\xF0\x9F\x8F\xB8"), // 🏸
        QStringLiteral("\xF0\x9F\x8F\x92"), // 🏒
        QStringLiteral("\xF0\x9F\x8F\x91"), // 🏑
        QStringLiteral("\xF0\x9F\xA5\x8D"), // 🥍
        QStringLiteral("\xF0\x9F\x8F\x8F"), // 🏏
        QStringLiteral("\xF0\x9F\x8E\xB3"), // 🎳
        QStringLiteral("\xF0\x9F\xA5\x8C"), // 🥌
        QStringLiteral("\xE2\x9B\xB3"),       // ⛳
        QStringLiteral("\xF0\x9F\x8F\xB9"), // 🏹
        QStringLiteral("\xF0\x9F\x8E\xA3"), // 🎣
        QStringLiteral("\xF0\x9F\xA4\xBF"), // 🤿
        QStringLiteral("\xF0\x9F\xA5\x8A"), // 🥊
        QStringLiteral("\xF0\x9F\xA5\x8B"), // 🥋
        QStringLiteral("\xF0\x9F\x8E\xBD"), // 🎽
        QStringLiteral("\xF0\x9F\x9B\xB7"), // 🛷
        QStringLiteral("\xF0\x9F\xA5\x8C"), // 🥌
        QStringLiteral("\xF0\x9F\xAA\x81"), // 🪁
        QStringLiteral("\xF0\x9F\x8E\xBF"), // 🎿
        QStringLiteral("\xE2\x9B\xB7"),       // ⛷
        QStringLiteral("\xF0\x9F\x8F\x82"), // 🏂
        QStringLiteral("\xF0\x9F\xAA\x82"), // 🪂
        QStringLiteral("\xF0\x9F\x8F\x8B"), // 🏋
        QStringLiteral("\xF0\x9F\xA4\xBC"), // 🤼
        QStringLiteral("\xF0\x9F\xA4\xB8"), // 🤸
        QStringLiteral("\xE2\x9B\xB9"),       // ⛹
        QStringLiteral("\xF0\x9F\xA4\xBE"), // 🤾
        QStringLiteral("\xF0\x9F\x8F\x8C"), // 🏌
        QStringLiteral("\xF0\x9F\x8F\x87"), // 🏇
        QStringLiteral("\xF0\x9F\xA4\xBA"), // 🤺
        QStringLiteral("\xF0\x9F\x8F\x8A"), // 🏊
        QStringLiteral("\xF0\x9F\x8F\x84"), // 🏄
        QStringLiteral("\xF0\x9F\xA7\x97"), // 🧗
        QStringLiteral("\xF0\x9F\x9A\xB4"), // 🚴
        QStringLiteral("\xF0\x9F\x9A\xB5"), // 🚵
        QStringLiteral("\xF0\x9F\xA4\xBD"), // 🤽
        QStringLiteral("\xF0\x9F\xA4\xB9"), // 🤹
        QStringLiteral("\xF0\x9F\xA7\x98"), // 🧘
        QStringLiteral("\xF0\x9F\x8E\xAF"), // 🎯
        QStringLiteral("\xF0\x9F\x8E\xAE"), // 🎮
        QStringLiteral("\xF0\x9F\x95\xB9"), // 🕹
        QStringLiteral("\xF0\x9F\x8E\xB0"), // 🎰
        QStringLiteral("\xF0\x9F\x8E\xB2"), // 🎲
        QStringLiteral("\xF0\x9F\xA7\xA9"), // 🧩
        QStringLiteral("\xE2\x99\x9F"),       // ♟
        QStringLiteral("\xF0\x9F\x8E\xAD"), // 🎭
        QStringLiteral("\xF0\x9F\x8E\xA8"), // 🎨
        QStringLiteral("\xF0\x9F\xA7\xB5"), // 🧵
        QStringLiteral("\xF0\x9F\xA7\xB6"), // 🧶
        QStringLiteral("\xF0\x9F\x8E\xBC"), // 🎼
        QStringLiteral("\xF0\x9F\x8E\xB5"), // 🎵
        QStringLiteral("\xF0\x9F\x8E\xB6"), // 🎶
        QStringLiteral("\xF0\x9F\x8E\xA4"), // 🎤
        QStringLiteral("\xF0\x9F\x8E\xA7"), // 🎧
        QStringLiteral("\xF0\x9F\x93\xAF"), // 📯
        QStringLiteral("\xF0\x9F\xA5\x81"), // 🥁
        QStringLiteral("\xF0\x9F\x8E\xB9"), // 🎹
        QStringLiteral("\xF0\x9F\x8E\xB8"), // 🎸
        QStringLiteral("\xF0\x9F\x8E\xBB"), // 🎻
        QStringLiteral("\xF0\x9F\xAA\x95"), // 🪕
        QStringLiteral("\xF0\x9F\x8E\xB7"), // 🎷
        QStringLiteral("\xF0\x9F\x8E\xBA"), // 🎺
        QStringLiteral("\xF0\x9F\x8E\xB4"), // 🎴
        QStringLiteral("\xF0\x9F\x83\x8F"), // 🀄
        QStringLiteral("\xF0\x9F\x8E\xAF"), // 🎯
        QStringLiteral("\xF0\x9F\xA5\x85"), // 🥅
        QStringLiteral("\xF0\x9F\xA5\x8B"), // 🥋
        QStringLiteral("\xF0\x9F\x92\xAA"), // 💪
        QStringLiteral("\xF0\x9F\x8F\x86"), // 🏆
        QStringLiteral("\xF0\x9F\x8F\x85"), // 🏅
        QStringLiteral("\xF0\x9F\xA5\x87"), // 🥇
        QStringLiteral("\xF0\x9F\xA5\x88"), // 🥈
        QStringLiteral("\xF0\x9F\xA5\x89"), // 🥉
        QStringLiteral("\xF0\x9F\x8F\x88"), // 🏈
    }},
    {QStringLiteral("Objects"), {
        QStringLiteral("\xF0\x9F\x92\xBB"), // 💻
        QStringLiteral("\xF0\x9F\x96\xA5"), // 🖥
        QStringLiteral("\xF0\x9F\x96\xA8"), // 🖨
        QStringLiteral("\xF0\x9F\x96\xB1"), // 🖱
        QStringLiteral("\xF0\x9F\x96\xB2"), // 🖲
        QStringLiteral("\xF0\x9F\x92\xBD"), // 💽
        QStringLiteral("\xF0\x9F\x92\xBE"), // 💾
        QStringLiteral("\xF0\x9F\x92\xBF"), // 💿
        QStringLiteral("\xF0\x9F\x93\x80"), // 📀
        QStringLiteral("\xF0\x9F\x93\xB1"), // 📱
        QStringLiteral("\xF0\x9F\x93\xB2"), // 📲
        QStringLiteral("\xE2\x98\x8E"),       // ☎
        QStringLiteral("\xF0\x9F\x93\x9E"), // 📞
        QStringLiteral("\xF0\x9F\x93\x9F"), // 📟
        QStringLiteral("\xF0\x9F\x93\xA0"), // 📠
        QStringLiteral("\xF0\x9F\x94\x8B"), // 🔋
        QStringLiteral("\xF0\x9F\x94\x8C"), // 🔌
        QStringLiteral("\xF0\x9F\x92\xA1"), // 💡
        QStringLiteral("\xF0\x9F\x94\xA6"), // 🔦
        QStringLiteral("\xF0\x9F\x95\xAF"), // 🕯
        QStringLiteral("\xF0\x9F\xA7\xA8"), // 🧨
        QStringLiteral("\xF0\x9F\x92\xA3"), // 💣
        QStringLiteral("\xF0\x9F\x93\x9C"), // 📜
        QStringLiteral("\xF0\x9F\x93\x84"), // 📄
        QStringLiteral("\xF0\x9F\x93\x83"), // 📃
        QStringLiteral("\xF0\x9F\x93\x91"), // 📑
        QStringLiteral("\xF0\x9F\x93\x8A"), // 📊
        QStringLiteral("\xF0\x9F\x93\x88"), // 📈
        QStringLiteral("\xF0\x9F\x93\x89"), // 📉
        QStringLiteral("\xF0\x9F\x93\x81"), // 📁
        QStringLiteral("\xF0\x9F\x93\x82"), // 📂
        QStringLiteral("\xF0\x9F\x97\x82"), // 🗂
        QStringLiteral("\xF0\x9F\x93\x85"), // 📅
        QStringLiteral("\xF0\x9F\x93\x86"), // 📆
        QStringLiteral("\xF0\x9F\x97\x92"), // 🗒
        QStringLiteral("\xF0\x9F\x97\x93"), // 🗓
        QStringLiteral("\xF0\x9F\x93\x87"), // 📇
        QStringLiteral("\xF0\x9F\x93\x8B"), // 📋
        QStringLiteral("\xF0\x9F\x93\x8C"), // 📌
        QStringLiteral("\xF0\x9F\x93\x8D"), // 📍
        QStringLiteral("\xF0\x9F\x93\x8E"), // 📎
        QStringLiteral("\xF0\x9F\x96\x87"), // 🖇
        QStringLiteral("\xF0\x9F\x93\x8F"), // 📏
        QStringLiteral("\xF0\x9F\x93\x90"), // 📐
        QStringLiteral("\xE2\x9C\x82"),       // ✂
        QStringLiteral("\xF0\x9F\x97\x83"), // 🗃
        QStringLiteral("\xF0\x9F\x97\x84"), // 🗄
        QStringLiteral("\xF0\x9F\x97\x91"), // 🗑
        QStringLiteral("\xF0\x9F\x94\x92"), // 🔒
        QStringLiteral("\xF0\x9F\x94\x93"), // 🔓
        QStringLiteral("\xF0\x9F\x94\x8F"), // 🔏
        QStringLiteral("\xF0\x9F\x94\x90"), // 🔐
        QStringLiteral("\xF0\x9F\x94\x8F"), // 🔏
        QStringLiteral("\xF0\x9F\x94\x91"), // 🔑
        QStringLiteral("\xF0\x9F\x97\x9D"), // 🗝
        QStringLiteral("\xF0\x9F\x94\xA8"), // 🔨
        QStringLiteral("\xE2\x9B\x8F"),       // ⛏
        QStringLiteral("\xE2\x9A\x92"),       // ⚒
        QStringLiteral("\xF0\x9F\xA7\xB0"), // 🧰
        QStringLiteral("\xF0\x9F\xAA\x93"), // 🪓
        QStringLiteral("\xF0\x9F\x94\xA7"), // 🔧
        QStringLiteral("\xF0\x9F\x94\xA9"), // 🔩
        QStringLiteral("\xE2\x9A\x99"),       // ⚙
        QStringLiteral("\xF0\x9F\x97\x9C"), // 🗜
        QStringLiteral("\xE2\x9A\x96"),       // ⚖
        QStringLiteral("\xF0\x9F\xA6\xAF"), // 🦯
        QStringLiteral("\xF0\x9F\x94\x97"), // 🔗
        QStringLiteral("\xE2\x9B\x93"),       // ⛓
        QStringLiteral("\xF0\x9F\xA7\xB2"), // 🧲
        QStringLiteral("\xF0\x9F\xA7\xAA"), // 🧪
        QStringLiteral("\xF0\x9F\xA7\xAB"), // 🧫
        QStringLiteral("\xF0\x9F\xA7\xAC"), // 🧬
        QStringLiteral("\xF0\x9F\x94\xAD"), // 🔭
        QStringLiteral("\xF0\x9F\x94\xAC"), // 🔬
        QStringLiteral("\xF0\x9F\x93\xA1"), // 📡
        QStringLiteral("\xF0\x9F\x92\x89"), // 💉
        QStringLiteral("\xF0\x9F\xA9\xB8"), // 🩸
        QStringLiteral("\xF0\x9F\x92\x8A"), // 💊
        QStringLiteral("\xF0\x9F\xA9\xB9"), // 🩹
        QStringLiteral("\xF0\x9F\xA9\xBA"), // 🩺
        QStringLiteral("\xF0\x9F\x9A\xBF"), // 🚿
        QStringLiteral("\xF0\x9F\x9B\x81"), // 🛁
        QStringLiteral("\xF0\x9F\x9A\xBD"), // 🚽
        QStringLiteral("\xF0\x9F\xA7\xBB"), // 🧻
        QStringLiteral("\xF0\x9F\xAA\xA0"), // 🪠
        QStringLiteral("\xF0\x9F\xA7\xB4"), // 🧴
        QStringLiteral("\xF0\x9F\xA7\xB7"), // 🧷
        QStringLiteral("\xF0\x9F\xA7\xB9"), // 🧹
        QStringLiteral("\xF0\x9F\xA7\xBA"), // 🧺
        QStringLiteral("\xF0\x9F\xA7\xB3"), // 🧳
    }},
    {QStringLiteral("Symbols"), {
        QStringLiteral("\xE2\x9D\xA4"),       // ❤
        QStringLiteral("\xF0\x9F\xA7\xA1"), // 🧡
        QStringLiteral("\xF0\x9F\x92\x9B"), // 💛
        QStringLiteral("\xF0\x9F\x92\x9A"), // 💚
        QStringLiteral("\xF0\x9F\x92\x99"), // 💙
        QStringLiteral("\xF0\x9F\x92\x9C"), // 💜
        QStringLiteral("\xF0\x9F\xA4\x8E"), // 🤎
        QStringLiteral("\xF0\x9F\x96\xA4"), // 🖤
        QStringLiteral("\xF0\x9F\xA4\x8D"), // 🤍
        QStringLiteral("\xF0\x9F\x92\x94"), // 💔
        QStringLiteral("\xE2\x9D\xA3"),       // ❣
        QStringLiteral("\xF0\x9F\x92\x95"), // 💕
        QStringLiteral("\xF0\x9F\x92\x96"), // 💖
        QStringLiteral("\xF0\x9F\x92\x97"), // 💗
        QStringLiteral("\xF0\x9F\x92\x98"), // 💘
        QStringLiteral("\xF0\x9F\x92\x93"), // 💓
        QStringLiteral("\xF0\x9F\x92\x9F"), // 💟
        QStringLiteral("\xE2\x98\xAE"),       // ☮
        QStringLiteral("\xE2\x9C\x9D"),       // ✝
        QStringLiteral("\xE2\x98\xAA"),       // ☪
        QStringLiteral("\xF0\x9F\x95\x89"), // 🕉
        QStringLiteral("\xE2\x9C\xA1"),       // ✡
        QStringLiteral("\xE2\x98\xB8"),       // ☸
        QStringLiteral("\xE2\x98\xAF"),       // ☯
        QStringLiteral("\xE2\x9C\x9D"),       // ✝
        QStringLiteral("\xE2\x98\xA6"),       // ☦
        QStringLiteral("\xF0\x9F\x9B\x90"), // 🛐
        QStringLiteral("\xE2\x99\x88"),       // ♈
        QStringLiteral("\xE2\x99\x89"),       // ♉
        QStringLiteral("\xE2\x99\x8A"),       // ♊
        QStringLiteral("\xE2\x99\x8B"),       // ♋
        QStringLiteral("\xE2\x99\x8C"),       // ♌
        QStringLiteral("\xE2\x99\x8D"),       // ♍
        QStringLiteral("\xE2\x99\x8E"),       // ♎
        QStringLiteral("\xE2\x99\x8F"),       // ♏
        QStringLiteral("\xE2\x99\x90"),       // ♐
        QStringLiteral("\xE2\x99\x91"),       // ♑
        QStringLiteral("\xE2\x99\x92"),       // ♒
        QStringLiteral("\xE2\x99\x93"),       // ♓
        QStringLiteral("\xE2\x9B\x8E"),       // ⛎
        QStringLiteral("\xF0\x9F\x94\x80"), // 🔀
        QStringLiteral("\xF0\x9F\x94\x81"), // 🔁
        QStringLiteral("\xF0\x9F\x94\x82"), // 🔂
        QStringLiteral("\xE2\x96\xB6"),       // ▶
        QStringLiteral("\xE2\x8F\xB8"),       // ⏸
        QStringLiteral("\xE2\x8F\xAF"),       // ⏯
        QStringLiteral("\xE2\x8F\xB9"),       // ⏹
        QStringLiteral("\xE2\x8F\xAE"),       // ⏮
        QStringLiteral("\xE2\x8F\xAD"),       // ⏭
        QStringLiteral("\xE2\x8F\xB4"),       // ⏴
        QStringLiteral("\xE2\x8F\xB5"),       // ⏵
        QStringLiteral("\xE2\x97\x80"),       // ◀
        QStringLiteral("\xF0\x9F\x94\x83"), // 🔃
        QStringLiteral("\xF0\x9F\x94\x84"), // 🔄
        QStringLiteral("\xF0\x9F\x94\x85"), // 🔅
        QStringLiteral("\xF0\x9F\x94\x86"), // 🔆
        QStringLiteral("\xF0\x9F\x94\x87"), // 🔇
        QStringLiteral("\xF0\x9F\x94\x88"), // 🔈
        QStringLiteral("\xF0\x9F\x94\x89"), // 🔉
        QStringLiteral("\xF0\x9F\x94\x8A"), // 🔊
        QStringLiteral("\xF0\x9F\x94\x94"), // 🔔
        QStringLiteral("\xF0\x9F\x94\x95"), // 🔕
        QStringLiteral("\xF0\x9F\x92\xAC"), // 💬
        QStringLiteral("\xF0\x9F\x97\xA8"), // 🗨
        QStringLiteral("\xF0\x9F\x92\xAD"), // 💭
        QStringLiteral("\xF0\x9F\x97\xAF"), // 🗯
        QStringLiteral("\xF0\x9F\x92\xA4"), // 💤
        QStringLiteral("\xF0\x9F\x92\xA2"), // 💢
        QStringLiteral("\xF0\x9F\x92\xA5"), // 💥
        QStringLiteral("\xF0\x9F\x92\xA6"), // 💦
        QStringLiteral("\xF0\x9F\x92\xA8"), // 💨
        QStringLiteral("\xF0\x9F\x92\xA9"), // 💩
        QStringLiteral("\xF0\x9F\x92\xAB"), // 💫
        QStringLiteral("\xF0\x9F\x92\xAE"), // 💮
        QStringLiteral("\xF0\x9F\x92\xAF"), // 💯
        QStringLiteral("\xF0\x9F\x94\xB4"), // 🔴
        QStringLiteral("\xF0\x9F\x9F\xA0"), // 🟠
        QStringLiteral("\xF0\x9F\x9F\xA1"), // 🟡
        QStringLiteral("\xF0\x9F\x9F\xA2"), // 🟢
        QStringLiteral("\xF0\x9F\x94\xB5"), // 🔵
        QStringLiteral("\xF0\x9F\x9F\xA3"), // 🟣
        QStringLiteral("\xF0\x9F\x9F\xA4"), // 🟤
        QStringLiteral("\xE2\x9A\xAB"),       // ⚫
        QStringLiteral("\xE2\x9A\xAA"),       // ⚪
        QStringLiteral("\xF0\x9F\x9F\xA5"), // 🟥
        QStringLiteral("\xF0\x9F\x9F\xA7"), // 🟧
        QStringLiteral("\xF0\x9F\x9F\xA8"), // 🟨
        QStringLiteral("\xF0\x9F\x9F\xA9"), // 🟩
        QStringLiteral("\xF0\x9F\x9F\xA6"), // 🟦
        QStringLiteral("\xF0\x9F\x9F\xAA"), // 🟪
        QStringLiteral("\xF0\x9F\x9F\xAB"), // 🟫
        QStringLiteral("\xE2\x9C\xB3"),       // ✳
        QStringLiteral("\xE2\x9C\xB4"),       // ✴
        QStringLiteral("\xE2\x9D\x87"),       // ❇
        QStringLiteral("\xE2\x80\xBC"),       // ‼
        QStringLiteral("\xE2\x81\x89"),       // ⁉
        QStringLiteral("\xE2\x9D\x93"),       // ❓
        QStringLiteral("\xE2\x9D\x94"),       // ❔
        QStringLiteral("\xE2\x9D\x95"),       // ❕
        QStringLiteral("\xE2\x9D\x97"),       // ❗
        QStringLiteral("\xE2\x81\x83"),       // 〰
        QStringLiteral("\xC2\xA9"),           // ©
        QStringLiteral("\xC2\xAE"),           // ®
        QStringLiteral("\xE2\x84\xA2"),       // ™
        QStringLiteral("\xF0\x9F\x94\xB0"), // 🔰
    }},
    {QStringLiteral("Flags"), {
        QStringLiteral("\xF0\x9F\x8F\xB3"), // 🏳
        QStringLiteral("\xF0\x9F\x8F\xB4"), // 🏴
        QStringLiteral("\xF0\x9F\x9A\xA9"), // 🚩
        QStringLiteral("\xF0\x9F\x8E\x8C"), // 🎌
        QStringLiteral("\xF0\x9F\x8F\xB4\xE2\x80\x8D\xE2\x98\xA0\xEF\xB8\x8F"), // 🏴‍☠️
        QStringLiteral("\xF0\x9F\x87\xA6\xF0\x9F\x87\xAB"), // 🇦🇫
        QStringLiteral("\xF0\x9F\x87\xA6\xF0\x9F\x87\xB1"), // 🇦🇱
        QStringLiteral("\xF0\x9F\x87\xA9\xF0\x9F\x87\xBF"), // 🇩🇿
        QStringLiteral("\xF0\x9F\x87\xA6\xF0\x9F\x87\xB8"), // 🇦🇸
        QStringLiteral("\xF0\x9F\x87\xA6\xF0\x9F\x87\xA9"), // 🇦🇩
        QStringLiteral("\xF0\x9F\x87\xA6\xF0\x9F\x87\xB4"), // 🇦🇴
        QStringLiteral("\xF0\x9F\x87\xA6\xF0\x9F\x87\xAE"), // 🇦🇮
        QStringLiteral("\xF0\x9F\x87\xA6\xF0\x9F\x87\xB6"), // 🇦🇶
        QStringLiteral("\xF0\x9F\x87\xA6\xF0\x9F\x87\xB7"), // 🇦🇷
        QStringLiteral("\xF0\x9F\x87\xA6\xF0\x9F\x87\xB2"), // 🇦🇲
        QStringLiteral("\xF0\x9F\x87\xA6\xF0\x9F\x87\xBC"), // 🇦🇼
        QStringLiteral("\xF0\x9F\x87\xA6\xF0\x9F\x87\xBA"), // 🇦🇺
        QStringLiteral("\xF0\x9F\x87\xA6\xF0\x9F\x87\xB9"), // 🇦🇹
        QStringLiteral("\xF0\x9F\x87\xA6\xF0\x9F\x87\xBF"), // 🇦🇿
        QStringLiteral("\xF0\x9F\x87\xA7\xF0\x9F\x87\xB8"), // 🇧🇸
        QStringLiteral("\xF0\x9F\x87\xA7\xF0\x9F\x87\xAD"), // 🇧🇭
        QStringLiteral("\xF0\x9F\x87\xA7\xF0\x9F\x87\xA9"), // 🇧🇩
        QStringLiteral("\xF0\x9F\x87\xA7\xF0\x9F\x87\xA7"), // 🇧🇧
        QStringLiteral("\xF0\x9F\x87\xA7\xF0\x9F\x87\xBE"), // 🇧🇾
        QStringLiteral("\xF0\x9F\x87\xA7\xF0\x9F\x87\xAA"), // 🇧🇪
        QStringLiteral("\xF0\x9F\x87\xA7\xF0\x9F\x87\xBF"), // 🇧🇿
        QStringLiteral("\xF0\x9F\x87\xA7\xF0\x9F\x87\xAF"), // 🇧🇯
        QStringLiteral("\xF0\x9F\x87\xA7\xF0\x9F\x87\xB2"), // 🇧🇲
        QStringLiteral("\xF0\x9F\x87\xA7\xF0\x9F\x87\xB9"), // 🇧🇹
        QStringLiteral("\xF0\x9F\x87\xA7\xF0\x9F\x87\xB4"), // 🇧🇴
        QStringLiteral("\xF0\x9F\x87\xA7\xF0\x9F\x87\xB6"), // 🇧🇶
        QStringLiteral("\xF0\x9F\x87\xA7\xF0\x9F\x87\xBC"), // 🇧🇼
        QStringLiteral("\xF0\x9F\x87\xA7\xF0\x9F\x87\xB7"), // 🇧🇷
        QStringLiteral("\xF0\x9F\x87\xAE\xF0\x9F\x87\xB4"), // 🇮🇴
        QStringLiteral("\xF0\x9F\x87\xBB\xF0\x9F\x87\xB3"), // 🇻🇳
        QStringLiteral("\xF0\x9F\x87\xA7\xF0\x9F\x87\xAC"), // 🇧🇬
        QStringLiteral("\xF0\x9F\x87\xA7\xF0\x9F\x87\xAB"), // 🇧🇫
        QStringLiteral("\xF0\x9F\x87\xA7\xF0\x9F\x87\xAE"), // 🇧🇮
        QStringLiteral("\xF0\x9F\x87\xB0\xF0\x9F\x87\xAD"), // 🇰🇭
        QStringLiteral("\xF0\x9F\x87\xA8\xF0\x9F\x87\xB2"), // 🇨🇲
        QStringLiteral("\xF0\x9F\x87\xA8\xF0\x9F\x87\xA6"), // 🇨🇦
        QStringLiteral("\xF0\x9F\x87\xA8\xF0\x9F\x87\xBB"), // 🇨🇻
        QStringLiteral("\xF0\x9F\x87\xB0\xF0\x9F\x87\xBE"), // 🇰🇾
        QStringLiteral("\xF0\x9F\x87\xA8\xF0\x9F\x87\xAB"), // 🇨🇫
        QStringLiteral("\xF0\x9F\x87\xB9\xF0\x9F\x87\xA9"), // 🇹🇩
        QStringLiteral("\xF0\x9F\x87\xA8\xF0\x9F\x87\xB1"), // 🇨🇱
        QStringLiteral("\xF0\x9F\x87\xA8\xF0\x9F\x87\xB3"), // 🇨🇳
        QStringLiteral("\xF0\x9F\x87\xA8\xF0\x9F\x87\xB4"), // 🇨🇴
        QStringLiteral("\xF0\x9F\x87\xB0\xF0\x9F\x87\xB2"), // 🇰🇲
        QStringLiteral("\xF0\x9F\x87\xA8\xF0\x9F\x87\xAC"), // 🇨🇬
        QStringLiteral("\xF0\x9F\x87\xA8\xF0\x9F\x87\xA9"), // 🇨🇩
        QStringLiteral("\xF0\x9F\x87\xA8\xF0\x9F\x87\xB0"), // 🇨🇰
        QStringLiteral("\xF0\x9F\x87\xA8\xF0\x9F\x87\xB7"), // 🇨🇷
        QStringLiteral("\xF0\x9F\x87\xAD\xF0\x9F\x87\xB7"), // 🇭🇷
        QStringLiteral("\xF0\x9F\x87\xA8\xF0\x9F\x87\xBA"), // 🇨🇺
        QStringLiteral("\xF0\x9F\x87\xA8\xF0\x9F\x87\xBC"), // 🇨🇼
        QStringLiteral("\xF0\x9F\x87\xA8\xF0\x9F\x87\xBE"), // 🇨🇾
        QStringLiteral("\xF0\x9F\x87\xA8\xF0\x9F\x87\xBF"), // 🇨🇿
        QStringLiteral("\xF0\x9F\x87\xA9\xF0\x9F\x87\xB0"), // 🇩🇰
        QStringLiteral("\xF0\x9F\x87\xA9\xF0\x9F\x87\xAF"), // 🇩🇯
        QStringLiteral("\xF0\x9F\x87\xA9\xF0\x9F\x87\xB2"), // 🇩🇲
        QStringLiteral("\xF0\x9F\x87\xA9\xF0\x9F\x87\xB4"), // 🇩🇴
        QStringLiteral("\xF0\x9F\x87\xAA\xF0\x9F\x87\xA8"), // 🇪🇨
        QStringLiteral("\xF0\x9F\x87\xAA\xF0\x9F\x87\xAC"), // 🇪🇬
        QStringLiteral("\xF0\x9F\x87\xB8\xF0\x9F\x87\xBB"), // 🇸🇻
        QStringLiteral("\xF0\x9F\x87\xAC\xF0\x9F\x87\xB6"), // 🇬🇶
        QStringLiteral("\xF0\x9F\x87\xAA\xF0\x9F\x87\xB7"), // 🇪🇷
        QStringLiteral("\xF0\x9F\x87\xAA\xF0\x9F\x87\xAA"), // 🇪🇪
        QStringLiteral("\xF0\x9F\x87\xB8\xF0\x9F\x87\xBF"), // 🇸🇿
        QStringLiteral("\xF0\x9F\x87\xAA\xF0\x9F\x87\xB9"), // 🇪🇹
        QStringLiteral("\xF0\x9F\x87\xAB\xF0\x9F\x87\xAE"), // 🇫🇮
        QStringLiteral("\xF0\x9F\x87\xAB\xF0\x9F\x87\xB7"), // 🇫🇷
        QStringLiteral("\xF0\x9F\x87\xAC\xF0\x9F\x87\xA6"), // 🇬🇦
        QStringLiteral("\xF0\x9F\x87\xAC\xF0\x9F\x87\xB2"), // 🇬🇲
        QStringLiteral("\xF0\x9F\x87\xAC\xF0\x9F\x87\xAA"), // 🇬🇪
        QStringLiteral("\xF0\x9F\x87\xA9\xF0\x9F\x87\xAA"), // 🇩🇪
        QStringLiteral("\xF0\x9F\x87\xAC\xF0\x9F\x87\xAD"), // 🇬🇭
        QStringLiteral("\xF0\x9F\x87\xAC\xF0\x9F\x87\xAE"), // 🇬🇮
        QStringLiteral("\xF0\x9F\x87\xAC\xF0\x9F\x87\xB7"), // 🇬🇷
        QStringLiteral("\xF0\x9F\x87\xAC\xF0\x9F\x87\xB1"), // 🇬🇱
        QStringLiteral("\xF0\x9F\x87\xAC\xF0\x9F\x87\xA9"), // 🇬🇩
        QStringLiteral("\xF0\x9F\x87\xAC\xF0\x9F\x87\xB8"), // 🇬🇸
        QStringLiteral("\xF0\x9F\x87\xAC\xF0\x9F\x87\xB9"), // 🇬🇹
        QStringLiteral("\xF0\x9F\x87\xAC\xF0\x9F\x87\xB3"), // 🇬🇳
        QStringLiteral("\xF0\x9F\x87\xAC\xF0\x9F\x87\xBC"), // 🇬🇼
        QStringLiteral("\xF0\x9F\x87\xAC\xF0\x9F\x87\xBE"), // 🇬🇾
        QStringLiteral("\xF0\x9F\x87\xAD\xF0\x9F\x87\xB9"), // 🇭🇹
        QStringLiteral("\xF0\x9F\x87\xAD\xF0\x9F\x87\xB3"), // 🇭🇳
        QStringLiteral("\xF0\x9F\x87\xAD\xF0\x9F\x87\xB0"), // 🇭🇰
        QStringLiteral("\xF0\x9F\x87\xAD\xF0\x9F\x87\xBA"), // 🇭🇺
        QStringLiteral("\xF0\x9F\x87\xAE\xF0\x9F\x87\xB8"), // 🇮🇸
        QStringLiteral("\xF0\x9F\x87\xAE\xF0\x9F\x87\xB3"), // 🇮🇳
        QStringLiteral("\xF0\x9F\x87\xAE\xF0\x9F\x87\xA9"), // 🇮🇩
        QStringLiteral("\xF0\x9F\x87\xAE\xF0\x9F\x87\xB7"), // 🇮🇷
        QStringLiteral("\xF0\x9F\x87\xAE\xF0\x9F\x87\xB6"), // 🇮🇶
        QStringLiteral("\xF0\x9F\x87\xAE\xF0\x9F\x87\xAA"), // 🇮🇪
        QStringLiteral("\xF0\x9F\x87\xAE\xF0\x9F\x87\xB1"), // 🇮🇱
        QStringLiteral("\xF0\x9F\x87\xAE\xF0\x9F\x87\xB9"), // 🇮🇹
        QStringLiteral("\xF0\x9F\x87\xAF\xF0\x9F\x87\xB2"), // 🇯🇲
        QStringLiteral("\xF0\x9F\x87\xAF\xF0\x9F\x87\xB5"), // 🇯🇵
        QStringLiteral("\xF0\x9F\x87\xB0\xF0\x9F\x87\xAA"), // 🇰🇪
        QStringLiteral("\xF0\x9F\x87\xB0\xF0\x9F\x87\xAE"), // 🇰🇮
        QStringLiteral("\xF0\x9F\x87\xB0\xF0\x9F\x87\xB5"), // 🇰🇵
        QStringLiteral("\xF0\x9F\x87\xB0\xF0\x9F\x87\xB7"), // 🇰🇷
        QStringLiteral("\xF0\x9F\x87\xB1\xF0\x9F\x87\xBB"), // 🇱🇻
        QStringLiteral("\xF0\x9F\x87\xB1\xF0\x9F\x87\xA7"), // 🇱🇧
        QStringLiteral("\xF0\x9F\x87\xB1\xF0\x9F\x87\xB8"), // 🇱🇸
        QStringLiteral("\xF0\x9F\x87\xB1\xF0\x9F\x87\xB7"), // 🇱🇷
        QStringLiteral("\xF0\x9F\x87\xB1\xF0\x9F\x87\xBE"), // 🇱🇾
        QStringLiteral("\xF0\x9F\x87\xB1\xF0\x9F\x87\xAE"), // 🇱🇮
        QStringLiteral("\xF0\x9F\x87\xB1\xF0\x9F\x87\xB9"), // 🇱🇹
        QStringLiteral("\xF0\x9F\x87\xB1\xF0\x9F\x87\xBA"), // 🇱🇺
        QStringLiteral("\xF0\x9F\x87\xB2\xF0\x9F\x87\xB4"), // 🇲🇴
        QStringLiteral("\xF0\x9F\x87\xB2\xF0\x9F\x87\xA6"), // 🇲🇬
        QStringLiteral("\xF0\x9F\x87\xB2\xF0\x9F\x87\xBC"), // 🇲🇼
        QStringLiteral("\xF0\x9F\x87\xB2\xF0\x9F\x87\xBE"), // 🇲🇾
        QStringLiteral("\xF0\x9F\x87\xB2\xF0\x9F\x87\xBB"), // 🇲🇻
        QStringLiteral("\xF0\x9F\x87\xB2\xF0\x9F\x87\xB1"), // 🇲🇱
        QStringLiteral("\xF0\x9F\x87\xB2\xF0\x9F\x87\xB9"), // 🇲🇹
        QStringLiteral("\xF0\x9F\x87\xB2\xF0\x9F\x87\xAD"), // 🇲🇭
        QStringLiteral("\xF0\x9F\x87\xB2\xF0\x9F\x87\xB7"), // 🇲🇷
        QStringLiteral("\xF0\x9F\x87\xB2\xF0\x9F\x87\xBA"), // 🇲🇺
        QStringLiteral("\xF0\x9F\x87\xB2\xF0\x9F\x87\xBD"), // 🇲🇽
        QStringLiteral("\xF0\x9F\x87\xB2\xF0\x9F\x87\xA9"), // 🇲🇩
        QStringLiteral("\xF0\x9F\x87\xB2\xF0\x9F\x87\xA8"), // 🇲🇨
        QStringLiteral("\xF0\x9F\x87\xB2\xF0\x9F\x87\xB3"), // 🇲🇳
        QStringLiteral("\xF0\x9F\x87\xB2\xF0\x9F\x87\xAA"), // 🇲🇪
        QStringLiteral("\xF0\x9F\x87\xB2\xF0\x9F\x87\xA6"), // 🇲🇦
        QStringLiteral("\xF0\x9F\x87\xB2\xF0\x9F\x87\xBF"), // 🇲🇿
        QStringLiteral("\xF0\x9F\x87\xB3\xF0\x9F\x87\xA6"), // 🇳🇦
        QStringLiteral("\xF0\x9F\x87\xB3\xF0\x9F\x87\xB7"), // 🇳🇷
        QStringLiteral("\xF0\x9F\x87\xB3\xF0\x9F\x87\xB5"), // 🇳🇵
        QStringLiteral("\xF0\x9F\x87\xB3\xF0\x9F\x87\xB1"), // 🇳🇱
        QStringLiteral("\xF0\x9F\x87\xB3\xF0\x9F\x87\xBF"), // 🇳🇿
        QStringLiteral("\xF0\x9F\x87\xB3\xF0\x9F\x87\xAE"), // 🇳🇮
        QStringLiteral("\xF0\x9F\x87\xB3\xF0\x9F\x87\xAA"), // 🇳🇪
        QStringLiteral("\xF0\x9F\x87\xB3\xF0\x9F\x87\xAC"), // 🇳🇬
        QStringLiteral("\xF0\x9F\x87\xB3\xF0\x9F\x87\xBA"), // 🇳🇺
        QStringLiteral("\xF0\x9F\x87\xB3\xF0\x9F\x87\xAB"), // 🇳🇫
        QStringLiteral("\xF0\x9F\x87\xB2\xF0\x9F\x87\xB0"), // 🇲🇰
        QStringLiteral("\xF0\x9F\x87\xB3\xF0\x9F\x87\xB4"), // 🇳🇴
        QStringLiteral("\xF0\x9F\x87\xB4\xF0\x9F\x87\xB2"), // 🇴🇲
        QStringLiteral("\xF0\x9F\x87\xB5\xF0\x9F\x87\xB0"), // 🇵🇰
        QStringLiteral("\xF0\x9F\x87\xB5\xF0\x9F\x87\xBC"), // 🇵🇼
        QStringLiteral("\xF0\x9F\x87\xB5\xF0\x9F\x87\xB8"), // 🇵🇸
        QStringLiteral("\xF0\x9F\x87\xB5\xF0\x9F\x87\xA6"), // 🇵🇦
        QStringLiteral("\xF0\x9F\x87\xB5\xF0\x9F\x87\xAC"), // 🇵🇬
        QStringLiteral("\xF0\x9F\x87\xB5\xF0\x9F\x87\xBE"), // 🇵🇾
        QStringLiteral("\xF0\x9F\x87\xB5\xF0\x9F\x87\xAA"), // 🇵🇪
        QStringLiteral("\xF0\x9F\x87\xB5\xF0\x9F\x87\xAD"), // 🇵🇭
        QStringLiteral("\xF0\x9F\x87\xB5\xF0\x9F\x87\xB1"), // 🇵🇱
        QStringLiteral("\xF0\x9F\x87\xB5\xF0\x9F\x87\xB9"), // 🇵🇹
        QStringLiteral("\xF0\x9F\x87\xB5\xF0\x9F\x87\xB7"), // 🇵🇷
        QStringLiteral("\xF0\x9F\x87\xB6\xF0\x9F\x87\xA6"), // 🇶🇦
        QStringLiteral("\xF0\x9F\x87\xB7\xF0\x9F\x87\xB4"), // 🇷🇴
        QStringLiteral("\xF0\x9F\x87\xB7\xF0\x9F\x87\xBA"), // 🇷🇺
        QStringLiteral("\xF0\x9F\x87\xB7\xF0\x9F\x87\xBC"), // 🇷🇼
        QStringLiteral("\xF0\x9F\x87\xB8\xF0\x9F\x87\xA6"), // 🇸🇦
        QStringLiteral("\xF0\x9F\x87\xB8\xF0\x9F\x87\xB3"), // 🇸🇳
        QStringLiteral("\xF0\x9F\x87\xB7\xF0\x9F\x87\xB8"), // 🇷🇸
        QStringLiteral("\xF0\x9F\x87\xB8\xF0\x9F\x87\xA8"), // 🇸🇨
        QStringLiteral("\xF0\x9F\x87\xB8\xF0\x9F\x87\xAC"), // 🇸🇬
        QStringLiteral("\xF0\x9F\x87\xB8\xF0\x9F\x87\xB0"), // 🇸🇰
        QStringLiteral("\xF0\x9F\x87\xB8\xF0\x9F\x87\xAE"), // 🇸🇮
        QStringLiteral("\xF0\x9F\x87\xB8\xF0\x9F\x87\xA7"), // 🇸🇧
        QStringLiteral("\xF0\x9F\x87\xB8\xF0\x9F\x87\xB4"), // 🇸🇴
        QStringLiteral("\xF0\x9F\x87\xBF\xF0\x9F\x87\xA6"), // 🇿🇦
        QStringLiteral("\xF0\x9F\x87\xB0\xF0\x9F\x87\xB7"), // 🇰🇷
        QStringLiteral("\xF0\x9F\x87\xB8\xF0\x9F\x87\xB8"), // 🇸🇸
        QStringLiteral("\xF0\x9F\x87\xAA\xF0\x9F\x87\xB8"), // 🇪🇸
        QStringLiteral("\xF0\x9F\x87\xB1\xF0\x9F\x87\xB0"), // 🇱🇰
        QStringLiteral("\xF0\x9F\x87\xB8\xF0\x9F\x87\xA9"), // 🇸🇩
        QStringLiteral("\xF0\x9F\x87\xB8\xF0\x9F\x87\xB7"), // 🇸🇷
        QStringLiteral("\xF0\x9F\x87\xB8\xF0\x9F\x87\xAA"), // 🇸🇪
        QStringLiteral("\xF0\x9F\x87\xA8\xF0\x9F\x87\xAD"), // 🇨🇭
        QStringLiteral("\xF0\x9F\x87\xB8\xF0\x9F\x87\xBE"), // 🇸🇾
        QStringLiteral("\xF0\x9F\x87\xB9\xF0\x9F\x87\xBC"), // 🇹🇼
        QStringLiteral("\xF0\x9F\x87\xB9\xF0\x9F\x87\xAF"), // 🇹🇯
        QStringLiteral("\xF0\x9F\x87\xB9\xF0\x9F\x87\xBF"), // 🇹🇿
        QStringLiteral("\xF0\x9F\x87\xB9\xF0\x9F\x87\xAD"), // 🇹🇭
        QStringLiteral("\xF0\x9F\x87\xB9\xF0\x9F\x87\xB1"), // 🇹🇱
        QStringLiteral("\xF0\x9F\x87\xB9\xF0\x9F\x87\xAC"), // 🇹🇬
        QStringLiteral("\xF0\x9F\x87\xB9\xF0\x9F\x87\xB4"), // 🇹🇴
        QStringLiteral("\xF0\x9F\x87\xB9\xF0\x9F\x87\xB9"), // 🇹🇹
        QStringLiteral("\xF0\x9F\x87\xB9\xF0\x9F\x87\xB3"), // 🇹🇳
        QStringLiteral("\xF0\x9F\x87\xB9\xF0\x9F\x87\xB7"), // 🇹🇷
        QStringLiteral("\xF0\x9F\x87\xB9\xF0\x9F\x87\xB2"), // 🇹🇲
        QStringLiteral("\xF0\x9F\x87\xB9\xF0\x9F\x87\xBB"), // 🇹🇻
        QStringLiteral("\xF0\x9F\x87\xBA\xF0\x9F\x87\xAC"), // 🇺🇬
        QStringLiteral("\xF0\x9F\x87\xBA\xF0\x9F\x87\xA6"), // 🇺🇦
        QStringLiteral("\xF0\x9F\x87\xA6\xF0\x9F\x87\xAA"), // 🇦🇪
        QStringLiteral("\xF0\x9F\x87\xAC\xF0\x9F\x87\xA7"), // 🇬🇧
        QStringLiteral("\xF0\x9F\x87\xBA\xF0\x9F\x87\xB8"), // 🇺🇸
        QStringLiteral("\xF0\x9F\x87\xBA\xF0\x9F\x87\xBE"), // 🇺🇾
        QStringLiteral("\xF0\x9F\x87\xBA\xF0\x9F\x87\xBF"), // 🇺🇿
        QStringLiteral("\xF0\x9F\x87\xBB\xF0\x9F\x87\xBA"), // 🇻🇺
        QStringLiteral("\xF0\x9F\x87\xBB\xF0\x9F\x87\xA6"), // 🇻🇦
        QStringLiteral("\xF0\x9F\x87\xBB\xF0\x9F\x87\xAA"), // 🇻🇪
        QStringLiteral("\xF0\x9F\x87\xBB\xF0\x9F\x87\xB3"), // 🇻🇳
        QStringLiteral("\xF0\x9F\x87\xBC\xF0\x9F\x87\xAB"), // 🇼🇫
        QStringLiteral("\xF0\x9F\x87\xAA\xF0\x9F\x87\xAD"), // 🇪🇭
        QStringLiteral("\xF0\x9F\x87\xBE\xF0\x9F\x87\xAA"), // 🇾🇪
        QStringLiteral("\xF0\x9F\x87\xBF\xF0\x9F\x87\xB2"), // 🇿🇲
        QStringLiteral("\xF0\x9F\x87\xBF\xF0\x9F\x87\xBC"), // 🇿🇼
        QStringLiteral("\xF0\x9F\x87\xA6\xF0\x9F\x87\xBD"), // 🇦🇽
        QStringLiteral("\xF0\x9F\x87\xA6\xF0\x9F\x87\xB3"), // 🇦🇳
        QStringLiteral("\xF0\x9F\x87\xA6\xF0\x9F\x87\xAC"), // 🇦🇬
    }},
};

EmojiPicker::EmojiPicker(QWidget *parent)
    : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint)
    , m_animated(false)
    , m_settings(QStringLiteral("MatrixClient"), QStringLiteral("EmojiPicker"))
{
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_TranslucentBackground);
    setFocusPolicy(Qt::StrongFocus);

    buildUi();
    buildCategoryTabs();
    loadRecent();

    setMinimumSize(420, 340);
    resize(460, 380);
}

EmojiPicker::~EmojiPicker()
{
    saveRecent();
}

void EmojiPicker::buildUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    m_searchBar = new QLineEdit(this);
    m_searchBar->setPlaceholderText(QStringLiteral("Search emoji..."));
    m_searchBar->setClearButtonEnabled(true);
    m_searchBar->setStyleSheet(QStringLiteral(
        "QLineEdit {"
        "  border: 1px solid palette(mid);"
        "  border-radius: 6px;"
        "  padding: 6px 10px;"
        "  font-size: 14px;"
        "  background: palette(base);"
        "  color: palette(text);"
        "}"
        "QLineEdit:focus {"
        "  border-color: palette(highlight);"
        "}"
    ));

    connect(m_searchBar, &QLineEdit::textChanged, this, &EmojiPicker::onSearchTextChanged);

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setDocumentMode(true);

    auto *recentContainer = new QWidget(this);
    auto *recentLayout = new QVBoxLayout(recentContainer);
    recentLayout->setContentsMargins(4, 4, 4, 4);

    auto *recentLabel = new QLabel(QStringLiteral("Recently Used"), this);
    QFont labelFont = recentLabel->font();
    labelFont.setBold(true);
    labelFont.setPointSize(labelFont.pointSize() - 1);
    recentLabel->setFont(labelFont);

    auto *recentScroll = new QScrollArea(this);
    recentScroll->setWidgetResizable(true);
    recentScroll->setFrameShape(QFrame::NoFrame);
    recentScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_recentWidget = new QWidget(this);
    m_recentGrid = new QGridLayout(m_recentWidget);
    m_recentGrid->setContentsMargins(0, 0, 0, 0);
    m_recentGrid->setSpacing(1);
    m_recentGrid->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    recentScroll->setWidget(m_recentWidget);

    recentLayout->addWidget(recentLabel);
    recentLayout->addWidget(recentScroll);

    m_tabWidget->addTab(recentContainer, QStringLiteral("\xF0\x9F\x95\x93")); // 🕓

    mainLayout->addWidget(m_searchBar);
    mainLayout->addWidget(m_tabWidget);
}

void EmojiPicker::buildCategoryTabs()
{
    const QStringList categories = {
        QStringLiteral("Smileys"),
        QStringLiteral("\xF0\x9F\x91\xA8"),   // People tab label
        QStringLiteral("Animals"),
        QStringLiteral("Food"),
        QStringLiteral("Travel"),
        QStringLiteral("Activities"),
        QStringLiteral("Objects"),
        QStringLiteral("Symbols"),
        QStringLiteral("Flags"),
    };

    struct CategoryInfo {
        QString key;
        QString label;
    };

    QList<CategoryInfo> info = {
        {QStringLiteral("Smileys"), categories[0]},
        {QStringLiteral("\xF0\x9F\x91\xA8"), categories[1]},
        {QStringLiteral("Animals"), categories[2]},
        {QStringLiteral("Food"), categories[3]},
        {QStringLiteral("Travel"), categories[4]},
        {QStringLiteral("Activities"), categories[5]},
        {QStringLiteral("Objects"), categories[6]},
        {QStringLiteral("Symbols"), categories[7]},
        {QStringLiteral("Flags"), categories[8]},
    };

    for (const auto &cat : info) {
        auto it = s_emojis.constFind(cat.key);
        if (it != s_emojis.constEnd()) {
            populateTab(cat.key, it.value());

            m_tabWidget->setTabToolTip(m_tabWidget->count() - 1, cat.key);
        }
    }

    connect(m_tabWidget, &QTabWidget::currentChanged, this, &EmojiPicker::onCategoryChanged);

    m_tabWidget->setStyleSheet(QStringLiteral(
        "QTabWidget::pane {"
        "  border: 1px solid palette(mid);"
        "  border-radius: 6px;"
        "  background: palette(window);"
        "}"
        "QTabBar::tab {"
        "  padding: 4px 10px;"
        "  font-size: 16px;"
        "}"
        "QTabBar::tab:selected {"
        "  font-weight: bold;"
        "}"
    ));
}

void EmojiPicker::populateTab(const QString &category, const QStringList &emojis)
{
    auto *grid = createEmojiGrid(emojis, category);
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setWidget(grid);

    auto it = s_emojis.constFind(category);
    int idx = m_tabWidget->addTab(scroll, QString());
    if (idx >= 0) {
        m_tabWidget->setTabToolTip(idx, category);
    }
}

QWidget *EmojiPicker::createEmojiGrid(const QStringList &emojis, const QString &category)
{
    auto *container = new QWidget(this);
    auto *grid = new QGridLayout(container);
    grid->setContentsMargins(4, 4, 4, 4);
    grid->setSpacing(1);
    grid->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    const int cols = 8;
    QFont emojiFont;
    emojiFont.setPointSize(18);
    emojiFont.setStyleHint(QFont::SansSerif);

    for (int i = 0; i < emojis.size(); ++i) {
        auto *btn = new QPushButton(emojis[i], this);
        btn->setFixedSize(44, 40);
        btn->setFont(emojiFont);
        btn->setFlat(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setToolTip(emojis[i]);
        btn->setFocusPolicy(Qt::NoFocus);

        btn->setStyleSheet(QStringLiteral(
            "QPushButton {"
            "  border: none;"
            "  border-radius: 4px;"
            "  background: transparent;"
            "  padding: 0px;"
            "}"
            "QPushButton:hover {"
            "  background: palette(highlight);"
            "}"
        ));

        connect(btn, &QPushButton::clicked, this, &EmojiPicker::onEmojiClicked);
        m_allButtons.append(btn);

        int row = i / cols;
        int col = i % cols;
        grid->addWidget(btn, row, col);
    }

    return container;
}

void EmojiPicker::rebuildFilteredTabs(const QString &filter)
{
    const int currentIdx = m_tabWidget->currentIndex();

    // Remove all tabs except the first (Recent)
    while (m_tabWidget->count() > 1) {
        QWidget *w = m_tabWidget->widget(1);
        m_tabWidget->removeTab(1);
        delete w;
    }

    m_allButtons.clear();

    if (filter.isEmpty()) {
        buildCategoryTabs();
        if (currentIdx > 0 && currentIdx <= m_tabWidget->count()) {
            m_tabWidget->setCurrentIndex(currentIdx);
        }
        return;
    }

    const QString lowerFilter = filter.toLower();
    QStringList filteredEmojis;

    for (auto it = s_emojis.constBegin(); it != s_emojis.constEnd(); ++it) {
        for (const QString &emoji : it.value()) {
            if (emoji.contains(lowerFilter, Qt::CaseInsensitive)) {
                filteredEmojis.append(emoji);
            }
        }
    }

    if (!filteredEmojis.isEmpty()) {
        auto *grid = createEmojiGrid(filteredEmojis, QStringLiteral("search"));
        auto *scroll = new QScrollArea(this);
        scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scroll->setWidget(grid);

        m_tabWidget->addTab(scroll, QStringLiteral("Results"));
        m_tabWidget->setCurrentIndex(1);
    }
}

void EmojiPicker::onEmojiClicked()
{
    auto *btn = qobject_cast<QPushButton *>(sender());
    if (!btn)
        return;

    const QString emoji = btn->text();
    addToRecent(emoji);
    emit emojiSelected(emoji);
    close();
}

void EmojiPicker::onSearchTextChanged(const QString &text)
{
    rebuildFilteredTabs(text);
}

void EmojiPicker::onCategoryChanged(int index)
{
    Q_UNUSED(index)
    if (!m_recentGrid)
        return;

    QLayoutItem *child;
    while ((child = m_recentGrid->takeAt(0)) != nullptr) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }

    const int cols = 8;
    QFont emojiFont;
    emojiFont.setPointSize(18);
    emojiFont.setStyleHint(QFont::SansSerif);

    for (int i = 0; i < m_recent.size(); ++i) {
        auto *btn = new QPushButton(m_recent[i], this);
        btn->setFixedSize(44, 40);
        btn->setFont(emojiFont);
        btn->setFlat(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFocusPolicy(Qt::NoFocus);

        btn->setStyleSheet(QStringLiteral(
            "QPushButton {"
            "  border: none;"
            "  border-radius: 4px;"
            "  background: transparent;"
            "  padding: 0px;"
            "}"
            "QPushButton:hover {"
            "  background: palette(highlight);"
            "}"
        ));

        connect(btn, &QPushButton::clicked, this, &EmojiPicker::onEmojiClicked);

        int row = i / cols;
        int col = i % cols;
        m_recentGrid->addWidget(btn, row, col);
    }
}

void EmojiPicker::addToRecent(const QString &emoji)
{
    m_recent.removeAll(emoji);
    m_recent.prepend(emoji);
    if (m_recent.size() > 20)
        m_recent = m_recent.mid(0, 20);
}

void EmojiPicker::loadRecent()
{
    m_recent = m_settings.value(QStringLiteral("recentEmojis")).toStringList();
}

void EmojiPicker::saveRecent()
{
    m_settings.setValue(QStringLiteral("recentEmojis"), m_recent);
}

void EmojiPicker::removeOldRecent()
{
    // Already handled in addToRecent by truncating
}

void EmojiPicker::showAbove(QWidget *anchor)
{
    if (!anchor)
        return;

    QPoint pos = anchor->mapToGlobal(QPoint(0, 0));
    int x = pos.x() + (anchor->width() - width()) / 2;
    int y = pos.y() - height() - 4;

    if (y < 0)
        y = pos.y() + anchor->height() + 4;

    move(x, y);
    show();
}

void EmojiPicker::showBelow(QWidget *anchor)
{
    if (!anchor)
        return;

    QPoint pos = anchor->mapToGlobal(QPoint(0, 0));
    int x = pos.x() + (anchor->width() - width()) / 2;
    int y = pos.y() + anchor->height() + 4;

    QScreen *screen = QGuiApplication::screenAt(pos);
    if (screen) {
        QRect avail = screen->availableGeometry();
        if (y + height() > avail.bottom())
            y = pos.y() - height() - 4;
    }

    move(x, y);
    show();
}

void EmojiPicker::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    if (m_animated)
        return;
    m_animated = true;

    setWindowOpacity(0.0);

    auto *opacityEffect = new QGraphicsOpacityEffect(this);
    opacityEffect->setOpacity(0.0);
    setGraphicsEffect(opacityEffect);

    auto *fadeIn = new QPropertyAnimation(opacityEffect, "opacity", this);
    fadeIn->setDuration(180);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::OutCubic);

    QRect startGeom = geometry();
    QRect endGeom = geometry();
    int dw = static_cast<int>(width() * 0.04);
    int dh = static_cast<int>(height() * 0.04);
    startGeom.setRect(startGeom.x() + dw, startGeom.y() + dh,
                      startGeom.width() - 2 * dw, startGeom.height() - 2 * dh);

    setGeometry(startGeom);

    auto *scaleAnim = new QPropertyAnimation(this, "geometry", this);
    scaleAnim->setDuration(180);
    scaleAnim->setStartValue(startGeom);
    scaleAnim->setEndValue(endGeom);
    scaleAnim->setEasingCurve(QEasingCurve::OutCubic);

    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    scaleAnim->start(QAbstractAnimation::DeleteWhenStopped);

    QTimer::singleShot(180, this, [this]() {
        setWindowOpacity(1.0);
        setGraphicsEffect(nullptr);
    });
}

void EmojiPicker::focusOutEvent(QFocusEvent *event)
{
    QWidget::focusOutEvent(event);
    close();
}
