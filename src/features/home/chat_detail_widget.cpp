#include "features/home/chat_detail_widget.hpp"

#include "protocol/protocol_manager.hpp"
#include "protocol/protocol_session.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QDateTime>
#include <QFrame>
#include <QRegularExpression>
#include <QSizePolicy>
#include <QApplication>

ChatDetailWidget::ChatDetailWidget(QWidget *parent)
    : QWidget(parent)
    , m_scrollArea(nullptr)
    , m_messageContainer(nullptr)
    , m_messageLayout(nullptr)
    , m_loadEarlierButton(nullptr)
    , m_typingIndicator(nullptr)
    , m_inputEdit(nullptr)
    , m_sendButton(nullptr)
    , m_emojiButton(nullptr)
    , m_typingTimer(nullptr)
    , m_protocolType(ProtocolType::MATRIX)
    , m_lastDisplayedTimestamp(0)
{
    setupUi();
}

ChatDetailWidget::~ChatDetailWidget() = default;

void ChatDetailWidget::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_messageContainer = new QWidget();
    m_messageLayout = new QVBoxLayout(m_messageContainer);
    m_messageLayout->setContentsMargins(8, 8, 8, 8);
    m_messageLayout->setSpacing(4);

    m_loadEarlierButton = new QPushButton(tr("Load earlier messages"), m_messageContainer);
    m_loadEarlierButton->setFlat(true);
    m_loadEarlierButton->setStyleSheet(QStringLiteral(
        "QPushButton { color: #888; font-style: italic; "
        "border: none; padding: 8px; } "
        "QPushButton:hover { color: #333; }"));
    m_messageLayout->addWidget(m_loadEarlierButton);

    m_messageLayout->addStretch(1);

    m_scrollArea->setWidget(m_messageContainer);

    m_typingIndicator = new QLabel(this);
    m_typingIndicator->setStyleSheet(QStringLiteral(
        "color: #888; font-style: italic; padding: 2px 12px; font-size: 12px;"));
    m_typingIndicator->hide();

    auto *inputBar = new QHBoxLayout();
    inputBar->setContentsMargins(8, 4, 8, 8);
    inputBar->setSpacing(6);

    m_inputEdit = new QTextEdit(this);
    m_inputEdit->setPlaceholderText(tr("Type a message..."));
    m_inputEdit->setMaximumHeight(120);
    m_inputEdit->setMinimumHeight(36);
    m_inputEdit->setAcceptRichText(false);
    m_inputEdit->setStyleSheet(QStringLiteral(
        "QTextEdit { border: 1px solid #ccc; border-radius: 8px; "
        "padding: 6px 10px; font-size: 13px; } "
        "QTextEdit:focus { border-color: #2b579a; }"));

    m_emojiButton = new QPushButton(QStringLiteral("\xF0\x9F\x98\x8A"), this);
    m_emojiButton->setFixedSize(36, 36);
    m_emojiButton->setToolTip(tr("Insert emoji"));
    m_emojiButton->setStyleSheet(QStringLiteral(
        "QPushButton { border: none; font-size: 18px; } "
        "QPushButton:hover { background-color: #e0e0e0; border-radius: 18px; }"));

    m_sendButton = new QPushButton(tr("Send"), this);
    m_sendButton->setMinimumWidth(64);
    m_sendButton->setMinimumHeight(36);
    m_sendButton->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: #2b579a; color: white; "
        "border: none; border-radius: 8px; padding: 6px 16px; "
        "font-weight: bold; font-size: 13px; } "
        "QPushButton:hover { background-color: #3a6bb5; } "
        "QPushButton:pressed { background-color: #1e4278; }"));

    inputBar->addWidget(m_inputEdit, 1);
    inputBar->addWidget(m_emojiButton);
    inputBar->addWidget(m_sendButton);

    mainLayout->addWidget(m_scrollArea, 1);
    mainLayout->addWidget(m_typingIndicator);
    mainLayout->addLayout(inputBar);

    m_typingTimer = new QTimer(this);
    m_typingTimer->setSingleShot(true);
    m_typingTimer->setInterval(3000);

    QObject::connect(m_sendButton, &QPushButton::clicked,
                     this, &ChatDetailWidget::onSendClicked);
    QObject::connect(m_emojiButton, &QPushButton::clicked,
                     this, &ChatDetailWidget::onEmojiClicked);
    QObject::connect(m_inputEdit, &QTextEdit::textChanged,
                     this, &ChatDetailWidget::onInputChanged);
    QObject::connect(m_typingTimer, &QTimer::timeout,
                     this, &ChatDetailWidget::onTypingTimeout);
    QObject::connect(m_scrollArea->verticalScrollBar(), &QScrollBar::valueChanged,
                     this, &ChatDetailWidget::onScrollChanged);
    QObject::connect(m_loadEarlierButton, &QPushButton::clicked,
                     this, &ChatDetailWidget::onLoadEarlierClicked);
}

void ChatDetailWidget::loadRoom(const ProtocolRoom &room)
{
    m_currentRoomId = room.id;
    m_protocolType = room.protocolType;
    m_lastDisplayedTimestamp = 0;

    auto *session = ProtocolManager::instance().getSession(m_protocolType);
    if (session) {
        m_currentUserId = session->userId();
    }

    while (m_messageLayout->count() > 2) {
        QLayoutItem *item = m_messageLayout->takeAt(0);
        if (item->widget() && item->widget() != m_loadEarlierButton) {
            delete item->widget();
        }
        delete item;
    }

    const auto messages = ProtocolManager::instance().getAllMessages(m_currentRoomId);
    for (const auto &msg : messages) {
        addMessage(msg);
    }

    scrollToBottom();
}

void ChatDetailWidget::addMessage(const ProtocolMessage &msg)
{
    if (msg.roomId != m_currentRoomId) {
        return;
    }

    bool wasNearBottom = isNearBottom();

    if (shouldShowTimestampDivider(msg.timestamp, m_lastDisplayedTimestamp)) {
        appendTimestampDivider(msg.timestamp);
    }
    m_lastDisplayedTimestamp = msg.timestamp;

    switch (msg.contentType) {
    case ProtocolContentType::JOIN:
    case ProtocolContentType::PART:
    case ProtocolContentType::KICK:
    case ProtocolContentType::BAN: {
        QString actionVerb;
        switch (msg.contentType) {
        case ProtocolContentType::JOIN: actionVerb = tr("joined"); break;
        case ProtocolContentType::PART: actionVerb = tr("left");   break;
        case ProtocolContentType::KICK: actionVerb = tr("was kicked from"); break;
        case ProtocolContentType::BAN:  actionVerb = tr("was banned from"); break;
        default: break;
        }
        appendSystemMessage(
            QStringLiteral("%1 %2 %3").arg(msg.senderName, actionVerb, tr("the room")),
            msg.timestamp);
        break;
    }
    case ProtocolContentType::TOPIC:
        appendSystemMessage(
            QStringLiteral("%1 %2: %3").arg(msg.senderName, tr("changed the topic to"), msg.text),
            msg.timestamp);
        break;
    case ProtocolContentType::SYSTEM:
    case ProtocolContentType::NOTICE:
        appendSystemMessage(msg.text, msg.timestamp);
        break;
    default: {
        QWidget *bubble = createMessageBubble(msg);
        int stretchIndex = m_messageLayout->count() - 1;
        m_messageLayout->insertWidget(stretchIndex, bubble);
        break;
    }
    }

    if (wasNearBottom) {
        scrollToBottom();
    }
}

QWidget *ChatDetailWidget::createMessageBubble(const ProtocolMessage &msg)
{
    const bool isSent = (msg.senderId == m_currentUserId);
    const QString initials = initialsFromName(msg.senderName);
    const QString color = avatarColorForSender(msg.senderId);

    auto *wrapper = new QWidget();
    auto *row = new QHBoxLayout(wrapper);
    row->setContentsMargins(4, 2, 4, 2);
    row->setSpacing(8);

    auto *avatarLabel = new QLabel();
    avatarLabel->setFixedSize(36, 36);
    avatarLabel->setAlignment(Qt::AlignCenter);
    avatarLabel->setText(initials);
    avatarLabel->setStyleSheet(QStringLiteral(
        "QLabel { background-color: %1; color: white; "
        "border-radius: 18px; font-weight: bold; font-size: 14px; "
        "min-width: 36px; min-height: 36px; }").arg(color));

    auto *textColumn = new QVBoxLayout();
    textColumn->setSpacing(2);

    auto *headerRow = new QHBoxLayout();
    headerRow->setSpacing(6);

    auto *nameLabel = new QLabel(msg.senderName);
    nameLabel->setStyleSheet(QStringLiteral(
        "font-weight: bold; font-size: 12px; color: #555;"));

    auto *timeLabel = new QLabel(formatTimestamp(msg.timestamp));
    timeLabel->setStyleSheet(QStringLiteral(
        "font-size: 11px; color: #999;"));

    if (isSent) {
        headerRow->addStretch();
        headerRow->addWidget(timeLabel);
        headerRow->addWidget(nameLabel);
    } else {
        headerRow->addWidget(nameLabel);
        headerRow->addWidget(timeLabel);
        headerRow->addStretch();
    }

    textColumn->addLayout(headerRow);

    QLabel *bodyLabel = nullptr;
    QString bubbleStyle;

    switch (msg.contentType) {
    case ProtocolContentType::IMAGE:
    case ProtocolContentType::VIDEO: {
        auto *placeholder = new QLabel(QStringLiteral("\xF0\x9F\x96\xBC\xEF\xB8\x8F  %1")
                                       .arg(msg.contentType == ProtocolContentType::IMAGE
                                            ? tr("Image") : tr("Video")));
        placeholder->setAlignment(Qt::AlignCenter);
        placeholder->setMinimumSize(160, 100);
        placeholder->setStyleSheet(QStringLiteral(
            "QLabel { background-color: #eee; border: 1px dashed #ccc; "
            "border-radius: 8px; color: #888; font-size: 14px; padding: 16px; }"));
        bodyLabel = placeholder;
        bubbleStyle = QStringLiteral(
            "background-color: #f0f0f0; border-radius: 12px; padding: 4px;");
        break;
    }
    case ProtocolContentType::ACTION: {
        bodyLabel = new QLabel(QStringLiteral("<i>* %1 %2</i>")
                               .arg(msg.senderName.toHtmlEscaped(),
                                    msg.text.toHtmlEscaped()));
        bodyLabel->setStyleSheet(QStringLiteral("color: #666; font-size: 13px; padding: 6px 10px;"));
        bubbleStyle = QStringLiteral(
            "background-color: #f5f0ff; border-radius: 12px; padding: 2px 4px;");
        break;
    }
    case ProtocolContentType::STICKER:
    case ProtocolContentType::EMOJI: {
        bodyLabel = new QLabel(msg.text);
        bodyLabel->setStyleSheet(QStringLiteral("font-size: 32px; padding: 4px 8px;"));
        bubbleStyle = QStringLiteral("background-color: transparent; padding: 0;");
        break;
    }
    default: {
        QString html = simpleMarkdownToHtml(msg.text);
        bodyLabel = new QLabel(html);
        bodyLabel->setTextFormat(Qt::RichText);
        bodyLabel->setWordWrap(true);
        bodyLabel->setOpenExternalLinks(true);
        bodyLabel->setTextInteractionFlags(
            Qt::TextBrowserInteraction | Qt::LinksAccessibleByMouse);
        bodyLabel->setMaximumWidth(520);
        bodyLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

        if (isSent) {
            bubbleStyle = QStringLiteral(
                "QLabel { background-color: #2b579a; color: white; "
                "border-radius: 12px; padding: 8px 12px; font-size: 13px; "
                "selection-background-color: #4a7dc4; }");
        } else {
            bubbleStyle = QStringLiteral(
                "QLabel { background-color: #e8e8e8; color: #333; "
                "border-radius: 12px; padding: 8px 12px; font-size: 13px; }");
        }
        break;
    }
    }

    if (bodyLabel) {
        bodyLabel->setStyleSheet(bubbleStyle);
    }

    auto *bodyRow = new QHBoxLayout();
    if (isSent) {
        bodyRow->addStretch();
        bodyRow->addWidget(bodyLabel);
    } else {
        bodyRow->addWidget(bodyLabel);
        bodyRow->addStretch();
    }
    textColumn->addLayout(bodyRow);

    if (isSent) {
        row->addStretch();
        row->addLayout(textColumn);
        row->addWidget(avatarLabel);
    } else {
        row->addWidget(avatarLabel);
        row->addLayout(textColumn);
        row->addStretch();
    }

    if (msg.isEdited) {
        auto *editedLabel = new QLabel(tr("(edited)"));
        editedLabel->setStyleSheet(QStringLiteral(
            "color: #aaa; font-size: 10px; font-style: italic;"));
        auto *editedRow = new QHBoxLayout();
        if (isSent) {
            editedRow->addStretch();
            editedRow->addWidget(editedLabel);
        } else {
            editedRow->addWidget(editedLabel);
            editedRow->addStretch();
        }
        textColumn->addLayout(editedRow);
    }

    if (msg.isEncrypted) {
        auto *lockLabel = new QLabel(QStringLiteral("\xF0\x9F\x94\x92"));
        lockLabel->setToolTip(tr("Encrypted"));
        lockLabel->setStyleSheet(QStringLiteral("font-size: 10px; color: #aaa;"));
        auto *lockRow = new QHBoxLayout();
        if (isSent) {
            lockRow->addStretch();
            lockRow->addWidget(lockLabel);
        } else {
            lockRow->addWidget(lockLabel);
            lockRow->addStretch();
        }
        textColumn->addLayout(lockRow);
    }

    return wrapper;
}

void ChatDetailWidget::appendSystemMessage(const QString &text, qint64 timestamp)
{
    auto *label = new QLabel(
        QStringLiteral("<span style='color: #999; font-size: 11px;'>%1</span>")
            .arg(text.toHtmlEscaped()));
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
    label->setTextFormat(Qt::RichText);

    int stretchIndex = m_messageLayout->count() - 1;
    m_messageLayout->insertWidget(stretchIndex, label);
    Q_UNUSED(timestamp);
}

void ChatDetailWidget::appendTimestampDivider(qint64 timestamp)
{
    auto *divider = new QWidget();
    auto *row = new QHBoxLayout(divider);
    row->setContentsMargins(16, 4, 16, 4);

    auto *lineLeft = new QFrame();
    lineLeft->setFrameShape(QFrame::HLine);
    lineLeft->setStyleSheet(QStringLiteral("QFrame { color: #ddd; }"));
    auto *timeLabel = new QLabel(formatTimestamp(timestamp));
    timeLabel->setStyleSheet(QStringLiteral(
        "color: #aaa; font-size: 11px; padding: 0 8px;"));
    auto *lineRight = new QFrame();
    lineRight->setFrameShape(QFrame::HLine);
    lineRight->setStyleSheet(QStringLiteral("QFrame { color: #ddd; }"));

    row->addWidget(lineLeft, 1);
    row->addWidget(timeLabel);
    row->addWidget(lineRight, 1);

    int stretchIndex = m_messageLayout->count() - 1;
    m_messageLayout->insertWidget(stretchIndex, divider);
}

bool ChatDetailWidget::shouldShowTimestampDivider(qint64 current, qint64 previous) const
{
    if (previous == 0) {
        return true;
    }
    return (current - previous) > 300000;
}

QString ChatDetailWidget::formatTimestamp(qint64 timestamp) const
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

QString ChatDetailWidget::initialsFromName(const QString &name) const
{
    if (name.isEmpty()) {
        return QStringLiteral("?");
    }

    QStringList parts = name.split(' ', Qt::SkipEmptyParts);
    if (parts.isEmpty()) {
        return name.left(1).toUpper();
    }

    QString result;
    result += parts.first().at(0).toUpper();
    if (parts.size() >= 2) {
        result += parts.last().at(0).toUpper();
    }
    return result;
}

QString ChatDetailWidget::avatarColorForSender(const QString &senderId) const
{
    static const QStringList palette = {
        QStringLiteral("#E53935"), QStringLiteral("#D81B60"), QStringLiteral("#8E24AA"),
        QStringLiteral("#5E35B1"), QStringLiteral("#3949AB"), QStringLiteral("#1E88E5"),
        QStringLiteral("#039BE5"), QStringLiteral("#00ACC1"), QStringLiteral("#00897B"),
        QStringLiteral("#43A047"), QStringLiteral("#7CB342"), QStringLiteral("#C0CA33"),
        QStringLiteral("#FDD835"), QStringLiteral("#FFB300"), QStringLiteral("#FB8C00"),
        QStringLiteral("#F4511E")
    };
    uint hash = qHash(senderId);
    return palette.at(static_cast<int>(hash) % palette.size());
}

QString ChatDetailWidget::simpleMarkdownToHtml(const QString &text) const
{
    QString html = text.toHtmlEscaped();

    QRegularExpression boldRe(QStringLiteral(R"(\*\*(.+?)\*\*|__(.+?)__)"));
    html.replace(boldRe, QStringLiteral(R"(<b>\1\2</b>)"));

    QRegularExpression italicRe(QStringLiteral(R"(\*(.+?)\*|_(.+?)_)"));
    html.replace(italicRe, QStringLiteral(R"(<i>\1\2</i>)"));

    QRegularExpression strikeRe(QStringLiteral(R"(~~(.+?)~~)"));
    html.replace(strikeRe, QStringLiteral(R"(<s>\1</s>)"));

    QRegularExpression codeRe(QStringLiteral(R"(`(.+?)`)"));
    html.replace(codeRe, QStringLiteral(R"(<code style='background-color:#f0f0f0; padding:1px 4px; border-radius:3px; font-family:monospace;'>\1</code>)"));

    QRegularExpression urlRe(QStringLiteral(R"((https?://[^\s<>"]+))"));
    html.replace(urlRe, QStringLiteral(R"(<a href="\1" style="color:#2b579a;">\1</a>)"));

    html.replace(QStringLiteral("\n"), QStringLiteral("<br>"));

    return html;
}

void ChatDetailWidget::scrollToBottom()
{
    auto *bar = m_scrollArea->verticalScrollBar();
    if (bar) {
        bar->setValue(bar->maximum());
    }
}

bool ChatDetailWidget::isNearBottom() const
{
    auto *bar = m_scrollArea->verticalScrollBar();
    if (!bar) {
        return true;
    }
    return (bar->value() >= bar->maximum() - 20);
}

void ChatDetailWidget::onSendClicked()
{
    const QString text = m_inputEdit->toPlainText().trimmed();
    if (text.isEmpty()) {
        return;
    }

    emit sendMessage(m_currentRoomId, text, ProtocolContentType::TEXT);
    m_inputEdit->clear();
    m_inputEdit->setFocus();
}

void ChatDetailWidget::onEmojiClicked()
{
    static const QStringList commonEmoji = {
        QStringLiteral("\xF0\x9F\x98\x80"), QStringLiteral("\xF0\x9F\x98\x82"),
        QStringLiteral("\xF0\x9F\x98\x8D"), QStringLiteral("\xF0\x9F\x98\xAD"),
        QStringLiteral("\xF0\x9F\x98\xA1"), QStringLiteral("\xF0\x9F\x98\x8E"),
        QStringLiteral("\xF0\x9F\x91\x8D"), QStringLiteral("\xF0\x9F\x91\x8E"),
        QStringLiteral("\xF0\x9F\x8E\x89"), QStringLiteral("\xE2\x9D\xA4"),
        QStringLiteral("\xF0\x9F\x94\xA5"), QStringLiteral("\xF0\x9F\x92\xAF"),
    };

    QString emojiStr = commonEmoji.at(
        static_cast<int>(qHash(m_currentRoomId + QString::number(m_inputEdit->toPlainText().size()))
                         % commonEmoji.size()));
    m_inputEdit->insertPlainText(emojiStr);
    m_inputEdit->setFocus();
}

void ChatDetailWidget::onInputChanged()
{
    if (!m_inputEdit->toPlainText().isEmpty()) {
        m_typingIndicator->show();
        m_typingTimer->start();
    }
}

void ChatDetailWidget::onTypingTimeout()
{
    m_typingIndicator->hide();
}

void ChatDetailWidget::onScrollChanged(int value)
{
    m_typingIndicator->setVisible(m_typingTimer->isActive() && !m_inputEdit->toPlainText().isEmpty());
    m_loadEarlierButton->setVisible(value == 0);

    if (m_typingTimer->isActive()) {
        m_typingIndicator->setText(
            tr("%1 is typing...").arg(m_inputEdit->toPlainText().isEmpty()
                                      ? QString() : tr("You")));
    }
}

void ChatDetailWidget::onLoadEarlierClicked()
{
    auto *session = ProtocolManager::instance().getSession(m_protocolType);
    if (!session || m_currentRoomId.isEmpty()) {
        return;
    }

    m_loadEarlierButton->setText(tr("Loading..."));
    m_loadEarlierButton->setEnabled(false);

    const int existingCount = m_messageLayout->count() - 2;
    const auto messages = session->getMessages(m_currentRoomId, existingCount + 50);

    while (m_messageLayout->count() > 2) {
        QLayoutItem *item = m_messageLayout->takeAt(1);
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

    m_lastDisplayedTimestamp = 0;
    for (const auto &msg : messages) {
        addMessage(msg);
    }

    m_loadEarlierButton->setText(tr("Load earlier messages"));
    m_loadEarlierButton->setEnabled(true);
    scrollToBottom();
}
