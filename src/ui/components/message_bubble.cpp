#include "ui/components/message_bubble.hpp"
#include "ui/components/avatar_widget.hpp"

#include <QPainter>
#include <QPaintEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QDateTime>
#include <QRegularExpression>
#include <QFontMetrics>
#include <QJsonObject>

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

MessageBubble::MessageBubble(const ProtocolMessage &msg, QWidget *parent)
    : QWidget(parent)
    , m_sentByMe(false)
    , m_mainLayout(nullptr)
    , m_systemLabel(nullptr)
    , m_normalRow(nullptr)
    , m_normalLayout(nullptr)
    , m_avatar(nullptr)
    , m_contentColumn(nullptr)
    , m_contentLayout(nullptr)
    , m_senderLabel(nullptr)
    , m_bubbleBackground(nullptr)
    , m_bubbleLayout(nullptr)
    , m_bodyLabel(nullptr)
    , m_timestampLabel(nullptr)
    , m_editedLabel(nullptr)
    , m_encryptedLabel(nullptr)
    , m_metadataLayout(nullptr)
{
    setupUi();
    rebuild(msg);
}

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------

void MessageBubble::setupUi()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    // --- System message label (centered, gray, no bubble) ---
    m_systemLabel = new QLabel(this);
    m_systemLabel->setAlignment(Qt::AlignCenter);
    m_systemLabel->setWordWrap(true);
    m_systemLabel->setTextFormat(Qt::RichText);
    m_systemLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_systemLabel->setStyleSheet(QStringLiteral(
        "QLabel { color: #888; font-size: 12px; padding: 4px 16px; }"));
    m_systemLabel->hide();
    m_mainLayout->addWidget(m_systemLabel);

    // --- Normal row: avatar | contentColumn ---
    m_normalRow = new QWidget(this);
    m_normalLayout = new QHBoxLayout(m_normalRow);
    m_normalLayout->setContentsMargins(4, 2, 4, 2);
    m_normalLayout->setSpacing(8);

    m_avatar = new AvatarWidget(QString(), QString(), 28, m_normalRow);

    // --- Content column: senderName, bubbleBackground ---
    m_contentColumn = new QWidget(m_normalRow);
    m_contentLayout = new QVBoxLayout(m_contentColumn);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    m_contentLayout->setSpacing(2);

    m_senderLabel = new QLabel(m_contentColumn);
    m_senderLabel->setStyleSheet(QStringLiteral(
        "QLabel { color: #555; font-size: 11px; font-weight: bold; }"));
    m_contentLayout->addWidget(m_senderLabel);

    // Bubble background widget – transparent so paintEvent rectangle shows through
    m_bubbleBackground = new QWidget(m_contentColumn);
    m_bubbleBackground->setAttribute(Qt::WA_TranslucentBackground);
    m_bubbleBackground->setStyleSheet(QStringLiteral("background: transparent;"));
    m_bubbleBackground->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    m_bubbleLayout = new QVBoxLayout(m_bubbleBackground);
    m_bubbleLayout->setContentsMargins(10, 6, 10, 4);
    m_bubbleLayout->setSpacing(2);

    m_bodyLabel = new QLabel(m_bubbleBackground);
    m_bodyLabel->setWordWrap(true);
    m_bodyLabel->setTextFormat(Qt::RichText);
    m_bodyLabel->setOpenExternalLinks(true);
    m_bodyLabel->setTextInteractionFlags(
        Qt::TextBrowserInteraction | Qt::LinksAccessibleByMouse);
    m_bodyLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    m_bodyLabel->setMaximumWidth(kMaxBubbleWidth - 20);
    m_bubbleLayout->addWidget(m_bodyLabel);

    // Metadata row (timestamp, edited, encrypted)
    m_metadataLayout = new QHBoxLayout();
    m_metadataLayout->setContentsMargins(0, 0, 0, 0);
    m_metadataLayout->setSpacing(4);

    m_timestampLabel = new QLabel(m_bubbleBackground);
    m_timestampLabel->setStyleSheet(QStringLiteral(
        "QLabel { color: #999; font-size: 10px; }"));
    m_metadataLayout->addWidget(m_timestampLabel);

    m_editedLabel = new QLabel(QStringLiteral("(edited)"), m_bubbleBackground);
    m_editedLabel->setStyleSheet(QStringLiteral(
        "QLabel { color: #aaa; font-size: 10px; font-style: italic; }"));
    m_editedLabel->hide();
    m_metadataLayout->addWidget(m_editedLabel);

    m_encryptedLabel = new QLabel(QStringLiteral("\xF0\x9F\x94\x92"), m_bubbleBackground);
    m_encryptedLabel->setToolTip(QObject::tr("Encrypted"));
    m_encryptedLabel->setStyleSheet(QStringLiteral(
        "QLabel { font-size: 10px; color: #aaa; }"));
    m_encryptedLabel->hide();
    m_metadataLayout->addWidget(m_encryptedLabel);

    m_metadataLayout->addStretch();
    m_bubbleLayout->addLayout(m_metadataLayout);

    m_contentLayout->addWidget(m_bubbleBackground);

    m_mainLayout->addWidget(m_normalRow);
}

// ---------------------------------------------------------------------------
// Rebuild
// ---------------------------------------------------------------------------

void MessageBubble::rebuild(const ProtocolMessage &msg)
{
    m_message = msg;

    const bool isSystemLike =
        msg.contentType == ProtocolContentType::SYSTEM ||
        msg.contentType == ProtocolContentType::NOTICE ||
        msg.contentType == ProtocolContentType::JOIN ||
        msg.contentType == ProtocolContentType::PART ||
        msg.contentType == ProtocolContentType::KICK ||
        msg.contentType == ProtocolContentType::BAN ||
        msg.contentType == ProtocolContentType::TOPIC;

    if (isSystemLike) {
        QString text = msg.text;
        if (msg.contentType == ProtocolContentType::JOIN) {
            text = QStringLiteral("%1 joined the room").arg(msg.senderName);
        } else if (msg.contentType == ProtocolContentType::PART) {
            text = QStringLiteral("%1 left the room").arg(msg.senderName);
        } else if (msg.contentType == ProtocolContentType::KICK) {
            text = QStringLiteral("%1 was kicked from the room").arg(msg.senderName);
        } else if (msg.contentType == ProtocolContentType::BAN) {
            text = QStringLiteral("%1 was banned from the room").arg(msg.senderName);
        } else if (msg.contentType == ProtocolContentType::TOPIC) {
            text = QStringLiteral("%1 changed the topic to: %2")
                       .arg(msg.senderName, msg.text);
        }
        rebuildSystem(text);
    } else {
        rebuildNormal();
    }
}

void MessageBubble::rebuildSystem(const QString &text)
{
    m_normalRow->hide();
    m_systemLabel->setText(
        QStringLiteral("<span>%1</span>").arg(text.toHtmlEscaped()));
    m_systemLabel->show();
    updateGeometry();
}

void MessageBubble::rebuildNormal()
{
    m_systemLabel->hide();
    m_normalRow->show();

    m_avatar->setName(m_message.senderName);
    m_senderLabel->setText(m_message.senderName);
    m_timestampLabel->setText(formatTimestamp(m_message.timestamp));
    m_editedLabel->setVisible(m_message.isEdited);
    m_encryptedLabel->setVisible(m_message.isEncrypted);

    // Determine text styling based on content type
    const bool isSent = m_sentByMe;
    const QString sentBubbleBg   = QStringLiteral("#0084FF");
    const QString recvBubbleBg   = QStringLiteral("#E4E6EB");
    const QString sentTextColor  = QStringLiteral("white");
    const QString recvTextColor  = QStringLiteral("black");
    const QString sentMetaColor  = QStringLiteral("rgba(255,255,255,0.7)");
    const QString recvMetaColor  = QStringLiteral("#999");

    QString bodyHtml;
    QString bodyTextColor;
    QString metaColor;
    bool useLargeFont = false;
    bool transparentBg = false;

    switch (m_message.contentType) {
    case ProtocolContentType::STICKER:
    case ProtocolContentType::EMOJI:
        bodyHtml = m_message.text.toHtmlEscaped();
        bodyTextColor = QStringLiteral("#333");
        metaColor = recvMetaColor;
        useLargeFont = true;
        transparentBg = true;
        break;

    case ProtocolContentType::IMAGE:
    case ProtocolContentType::VIDEO: {
        const QString label = m_message.contentType == ProtocolContentType::IMAGE
                                  ? QObject::tr("Image")
                                  : QObject::tr("Video");
        bodyHtml = QStringLiteral("<div style='text-align:center;'>"
                                  "\xF0\x9F\x96\xBC  %1</div>")
                       .arg(label);
        bodyTextColor = QStringLiteral("#888");
        metaColor = recvMetaColor;
        break;
    }

    case ProtocolContentType::AUDIO:
        bodyHtml = QStringLiteral("<div style='text-align:center;'>"
                                  "\xF0\x9F\x8E\xB5  %1</div>")
                       .arg(QObject::tr("Audio"));
        bodyTextColor = QStringLiteral("#888");
        metaColor = recvMetaColor;
        break;

    case ProtocolContentType::FILE: {
        const QString filename =
            m_message.metadata.value(QStringLiteral("filename"))
                .toString(QObject::tr("Unknown file"));
        bodyHtml = QStringLiteral("\xF0\x9F\x93\x81  %1").arg(filename.toHtmlEscaped());
        bodyTextColor = QStringLiteral("#333");
        metaColor = recvMetaColor;
        break;
    }

    case ProtocolContentType::ACTION:
        bodyHtml = QStringLiteral("<i>* %1 %2</i>")
                       .arg(m_message.senderName.toHtmlEscaped(),
                            m_message.text.toHtmlEscaped());
        bodyTextColor = QStringLiteral("#6C3A9E");
        metaColor = recvMetaColor;
        break;

    default:
        bodyHtml = simpleMarkdownToHtml(m_message.text);
        bodyTextColor = isSent ? sentTextColor : recvTextColor;
        metaColor = isSent ? sentMetaColor : recvMetaColor;
        break;
    }

    // Apply body styling
    QString bodyStylesheet;
    if (transparentBg) {
        bodyStylesheet = QStringLiteral(
            "QLabel { color: %1; font-size: 32px; background: transparent; }")
            .arg(bodyTextColor);
    } else if (useLargeFont) {
        bodyStylesheet = QStringLiteral(
            "QLabel { color: %1; font-size: 28px; background: transparent; }")
            .arg(bodyTextColor);
    } else if (m_message.contentType == ProtocolContentType::IMAGE ||
               m_message.contentType == ProtocolContentType::VIDEO ||
               m_message.contentType == ProtocolContentType::AUDIO) {
        bodyStylesheet = QStringLiteral(
            "QLabel { color: %1; font-size: 14px; background: transparent;"
            " min-width: 140px; min-height: 60px; }")
            .arg(bodyTextColor);
    } else {
        bodyStylesheet = QStringLiteral(
            "QLabel { color: %1; font-size: 13px; background: transparent; }")
            .arg(bodyTextColor);
    }
    m_bodyLabel->setStyleSheet(bodyStylesheet);

    // Apply timestamp / meta styling
    QString metaStylesheet = QStringLiteral(
        "QLabel { color: %1; font-size: 10px; }").arg(metaColor);
    m_timestampLabel->setStyleSheet(metaStylesheet);
    m_editedLabel->setStyleSheet(QStringLiteral(
        "QLabel { color: %1; font-size: 10px; font-style: italic; }")
        .arg(metaColor));
    m_encryptedLabel->setStyleSheet(QStringLiteral(
        "QLabel { color: %1; font-size: 10px; }").arg(metaColor));

    // Apply content to body
    if (m_message.contentType == ProtocolContentType::IMAGE ||
        m_message.contentType == ProtocolContentType::VIDEO ||
        m_message.contentType == ProtocolContentType::AUDIO) {
        // Dashed placeholder
        m_bubbleBackground->setStyleSheet(QStringLiteral(
            "QWidget { background: transparent; "
            "border: 2px dashed #ccc; border-radius: 12px; }"));
        m_bodyLabel->setAlignment(Qt::AlignCenter);
    } else {
        // No border on the background widget; the rounded rect is drawn in paintEvent
        m_bubbleBackground->setStyleSheet(QStringLiteral("background: transparent;"));
        m_bodyLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    }

    m_bodyLabel->setText(bodyHtml);

    // Re-layout avatar position
    QLayoutItem *avatarItem = nullptr;
    QLayoutItem *contentItem = nullptr;
    for (int i = 0; i < m_normalLayout->count(); ++i) {
        QLayoutItem *item = m_normalLayout->itemAt(i);
        if (item->widget() == m_avatar) {
            avatarItem = item;
        } else if (item->widget() == m_contentColumn) {
            contentItem = item;
        }
    }

    if (isSent) {
        m_senderLabel->setAlignment(Qt::AlignRight);
        m_senderLabel->setStyleSheet(QStringLiteral(
            "QLabel { color: #555; font-size: 11px; font-weight: bold; }"));
        if (avatarItem && contentItem) {
            int avatarIdx = m_normalLayout->indexOf(m_avatar);
            int contentIdx = m_normalLayout->indexOf(m_contentColumn);
            if (avatarIdx < contentIdx) {
                m_normalLayout->removeWidget(m_avatar);
                m_normalLayout->removeWidget(m_contentColumn);
                m_normalLayout->addWidget(m_contentColumn);
                m_normalLayout->addWidget(m_avatar);
            }
        }
    } else {
        m_senderLabel->setAlignment(Qt::AlignLeft);
        m_senderLabel->setStyleSheet(QStringLiteral(
            "QLabel { color: #555; font-size: 11px; font-weight: bold; }"));
        if (avatarItem && contentItem) {
            int avatarIdx = m_normalLayout->indexOf(m_avatar);
            int contentIdx = m_normalLayout->indexOf(m_contentColumn);
            if (avatarIdx > contentIdx) {
                m_normalLayout->removeWidget(m_contentColumn);
                m_normalLayout->removeWidget(m_avatar);
                m_normalLayout->addWidget(m_avatar);
                m_normalLayout->addWidget(m_contentColumn);
            }
        }
    }

    updateGeometry();
    update();
}

// ---------------------------------------------------------------------------
// setSentByMe
// ---------------------------------------------------------------------------

void MessageBubble::setSentByMe(bool sent)
{
    if (m_sentByMe == sent)
        return;
    m_sentByMe = sent;
    rebuild(m_message);
}

bool MessageBubble::isSentByMe() const
{
    return m_sentByMe;
}

// ---------------------------------------------------------------------------
// paintEvent – draws the rounded-rect bubble behind m_bubbleBackground
// ---------------------------------------------------------------------------

void MessageBubble::paintEvent(QPaintEvent * /*event*/)
{
    if (!m_bubbleBackground || !m_bubbleBackground->isVisible())
        return;

    const bool isMediaPlaceholder =
        m_message.contentType == ProtocolContentType::IMAGE ||
        m_message.contentType == ProtocolContentType::VIDEO ||
        m_message.contentType == ProtocolContentType::AUDIO;

    // Dashed-border placeholders are drawn via stylesheet on m_bubbleBackground
    if (isMediaPlaceholder)
        return;

    const bool isAction = m_message.contentType == ProtocolContentType::ACTION;
    const bool isStickerOrEmoji =
        m_message.contentType == ProtocolContentType::STICKER ||
        m_message.contentType == ProtocolContentType::EMOJI;

    if (isStickerOrEmoji)
        return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);

    const QRect bubbleGeo = m_bubbleBackground->geometry();
    QRect drawRect = bubbleGeo;

    if (isAction) {
        painter.setBrush(QColor(0xF5, 0xF0, 0xFF));
    } else if (m_sentByMe) {
        painter.setBrush(QColor(0x00, 0x84, 0xFF));
    } else {
        painter.setBrush(QColor(0xE4, 0xE6, 0xEB));
    }

    painter.drawRoundedRect(drawRect, 12, 12);
}

// ---------------------------------------------------------------------------
// sizeHint
// ---------------------------------------------------------------------------

QSize MessageBubble::sizeHint() const
{
    if (m_systemLabel->isVisible()) {
        return m_systemLabel->sizeHint() + QSize(0, 4);
    }

    int w = 0;
    int h = 0;

    if (m_avatar->isVisible()) {
        w += m_avatar->sizeHint().width() + m_normalLayout->spacing();
        h = qMax(h, m_avatar->sizeHint().height());
    }

    int contentW = 10;
    if (m_senderLabel->isVisible()) {
        contentW = qMax(contentW, m_senderLabel->sizeHint().width());
        h += m_senderLabel->sizeHint().height() + m_contentLayout->spacing();
    }

    if (m_bubbleBackground->isVisible()) {
        QFontMetrics fm(m_bodyLabel->font());
        QRect textRect = fm.boundingRect(
            QRect(0, 0, kMaxBubbleWidth - 20, 0),
            Qt::AlignLeft | Qt::TextWordWrap,
            m_bodyLabel->text());
        int bubbleW = qMin(textRect.width() + 24, kMaxBubbleWidth);
        contentW = qMax(contentW, bubbleW);
        h += m_bubbleBackground->sizeHint().height();
    }

    w += contentW + 8;
    h += 4;

    return QSize(w, h);
}

// ---------------------------------------------------------------------------
// Static helpers
// ---------------------------------------------------------------------------

QString MessageBubble::formatTimestamp(qint64 timestamp)
{
    auto dt = QDateTime::fromMSecsSinceEpoch(timestamp);
    auto now = QDateTime::currentDateTime();

    if (dt.date() == now.date()) {
        return dt.toString(QStringLiteral("HH:mm"));
    }
    if (dt.date().year() == now.date().year()) {
        return dt.toString(QStringLiteral("MMM d, HH:mm"));
    }
    return dt.toString(QStringLiteral("MMM d yyyy, HH:mm"));
}

QString MessageBubble::simpleMarkdownToHtml(const QString &text)
{
    QString html = text.toHtmlEscaped();

    // Bold: **text** or __text__
    QRegularExpression boldRe(QStringLiteral(R"(\*\*(.+?)\*\*|__(.+?)__)"));
    html.replace(boldRe, QStringLiteral(R"(<b>\1\2</b>)"));

    // Italic: *text* or _text_
    QRegularExpression italicRe(QStringLiteral(R"(\*(.+?)\*|_(.+?)_)"));
    html.replace(italicRe, QStringLiteral(R"(<i>\1\2</i>)"));

    // Strikethrough: ~~text~~
    QRegularExpression strikeRe(QStringLiteral(R"(~~(.+?)~~)"));
    html.replace(strikeRe, QStringLiteral(R"(<s>\1</s>)"));

    // Inline code: `text`
    QRegularExpression codeRe(QStringLiteral(R"(`(.+?)`)"));
    html.replace(codeRe,
                 QStringLiteral("<code style='background-color:#f0f0f0; "
                                "padding:1px 4px; border-radius:3px; "
                                "font-family:monospace;'>\\1</code>"));

    // URLs
    QRegularExpression urlRe(
        QStringLiteral(R"((https?://[^\s<>"']+))"));
    html.replace(urlRe,
                 QStringLiteral("<a href=\"\\1\" style=\"color:#2b579a;\">\\1</a>"));

    html.replace(QStringLiteral("\n"), QStringLiteral("<br>"));

    return html;
}
