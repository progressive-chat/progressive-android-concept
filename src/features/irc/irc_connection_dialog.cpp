#include "features/irc/irc_connection_dialog.hpp"

#include "irc/irc_session.hpp"
#include "protocol/protocol_session.hpp"
#include "protocol/protocol_room.hpp"
#include "protocol/protocol_type.hpp"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>
#include <QAbstractSocket>
#include <QSslSocket>
#include <QSslConfiguration>
#include <QDateTime>
#include <QRegularExpression>

// ─── Style sheet ──────────────────────────────────────────────────────────────

static const char *kConnectionDialogStyle = R"(
    IrcConnectionDialog {
        background-color: #1e1e2e;
    }
    IrcConnectionDialog QLabel {
        color: #cdd6f4;
        background: transparent;
        font-size: 12px;
    }
    IrcConnectionDialog QLabel#valueLabel {
        color: #89b4fa;
        font-family: "monospace";
    }
    IrcConnectionDialog QGroupBox {
        color: #a6adc8;
        font-weight: bold;
        font-size: 12px;
        border: 1px solid #313244;
        border-radius: 8px;
        margin-top: 12px;
        padding-top: 14px;
    }
    IrcConnectionDialog QGroupBox::title {
        subcontrol-origin: margin;
        left: 12px;
        padding: 0 6px;
    }
    IrcConnectionDialog QLineEdit {
        background-color: #313244;
        color: #cdd6f4;
        border: 1px solid #45475a;
        border-radius: 6px;
        padding: 6px 10px;
        font-size: 12px;
    }
    IrcConnectionDialog QLineEdit:focus {
        border-color: #89b4fa;
    }
    IrcConnectionDialog QPushButton {
        background-color: #313244;
        color: #cdd6f4;
        border: 1px solid #45475a;
        border-radius: 6px;
        padding: 6px 14px;
        font-size: 12px;
    }
    IrcConnectionDialog QPushButton:hover {
        background-color: #45475a;
        border-color: #89b4fa;
    }
    IrcConnectionDialog QPushButton#disconnectBtn {
        background-color: #f38ba8;
        color: #1e1e2e;
        font-weight: bold;
        border: none;
    }
    IrcConnectionDialog QPushButton#disconnectBtn:hover {
        background-color: #eba0ac;
    }
    IrcConnectionDialog QPushButton#reconnectBtn {
        background-color: #a6e3a1;
        color: #1e1e2e;
        font-weight: bold;
        border: none;
    }
    IrcConnectionDialog QPushButton#reconnectBtn:hover {
        background-color: #94e2d5;
    }
)";

// ─── Helpers ───────────────────────────────────────────────────────────────────

static QLabel *makeValueLabel(const QString &text = QString(), QWidget *parent = nullptr)
{
    auto *l = new QLabel(text, parent);
    l->setObjectName(QStringLiteral("valueLabel"));
    l->setTextInteractionFlags(Qt::TextSelectableByMouse);
    l->setWordWrap(true);
    return l;
}

static QLabel *makeTitle(const QString &text, QWidget *parent = nullptr)
{
    auto *l = new QLabel(text, parent);
    l->setStyleSheet(QStringLiteral(
        "color: #a6adc8; font-size: 11px; background: transparent;"));
    return l;
}

// ─── Constructor / Destructor ──────────────────────────────────────────────────

IrcConnectionDialog::IrcConnectionDialog(IrcSession *session, QWidget *parent)
    : QDialog(parent)
    , m_session(session)
{
    setMinimumSize(520, 680);
    setWindowTitle(tr("IRC Connection Details"));
    setStyleSheet(QString::fromLatin1(kConnectionDialogStyle));

    setupUi();

    if (m_session) {
        m_socket = m_session->findChild<QAbstractSocket *>(QString(),
            Qt::FindDirectChildrenOnly);

        if (m_socket) {
            connect(m_socket, &QAbstractSocket::bytesWritten,
                    this, &IrcConnectionDialog::onSocketBytesWritten);
        }

        connect(m_session, &IrcSession::connectionStateChanged,
                this, &IrcConnectionDialog::onConnectionStateChanged);

        connect(m_session, &IrcSession::messageReceived, this,
                [this](const ProtocolMessage &msg) {
                    if (msg.contentType == ProtocolContentType::SYSTEM &&
                        msg.roomId == QStringLiteral("*status")) {
                        m_bytesReceived += static_cast<quint64>(
                            qMax(0, msg.text.toUtf8().size()));
                    }
                });

        refreshInfo();
    }

    onConnectionStateChanged();

    m_durationTimer = new QTimer(this);
    connect(m_durationTimer, &QTimer::timeout,
            this, &IrcConnectionDialog::onUpdateDuration);
    m_durationTimer->start(1000);

    m_statsTimer = new QTimer(this);
    connect(m_statsTimer, &QTimer::timeout,
            this, &IrcConnectionDialog::onUpdateStats);
    m_statsTimer->start(5000);

    // Latency probe every 30 seconds
    m_latencyTimer = new QTimer(this);
    connect(m_latencyTimer, &QTimer::timeout, this, [this]() {
        if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
            m_latencyTimerElapsed.start();
            m_socket->write(QStringLiteral("PING LAG%1\r\n")
                                .arg(QDateTime::currentMSecsSinceEpoch())
                                .toUtf8());
        }
    });
    m_latencyTimer->start(30000);

    // Fire one initial latency probe after 2 seconds
    QTimer::singleShot(2000, this, [this]() {
        if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
            m_latencyTimerElapsed.start();
            m_socket->write(QStringLiteral("PING LAG%1\r\n")
                                .arg(QDateTime::currentMSecsSinceEpoch())
                                .toUtf8());
        }
    });

    // Watch socket for PONG replies to measure latency
    if (m_socket) {
        connect(m_socket, &QAbstractSocket::readyRead,
                this, &IrcConnectionDialog::onLatencyReply);
    }
}

IrcConnectionDialog::~IrcConnectionDialog() = default;

// ─── UI construction ──────────────────────────────────────────────────────────

void IrcConnectionDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(10);

    // ── Server info group ─────────────────────────────────────────────────

    auto *serverGroup = new QGroupBox(tr("Server"), this);
    auto *serverForm = new QFormLayout(serverGroup);
    serverForm->setSpacing(6);

    m_serverLabel = makeValueLabel(tr("—"), this);
    serverForm->addRow(makeTitle(tr("Host:Port"), this), m_serverLabel);

    m_durationLabel = makeValueLabel(tr("—"), this);
    serverForm->addRow(makeTitle(tr("Connected"), this), m_durationLabel);

    m_sslLabel = makeValueLabel(tr("—"), this);
    serverForm->addRow(makeTitle(tr("TLS / SSL"), this), m_sslLabel);

    mainLayout->addWidget(serverGroup);

    // ── Identity group ────────────────────────────────────────────────────

    auto *identityGroup = new QGroupBox(tr("Identity"), this);
    auto *identityForm = new QFormLayout(identityGroup);
    identityForm->setSpacing(6);

    m_nickLabel = makeValueLabel(tr("—"), this);
    identityForm->addRow(makeTitle(tr("Nick"), this), m_nickLabel);

    m_usernameLabel = makeValueLabel(tr("—"), this);
    identityForm->addRow(makeTitle(tr("Username"), this), m_usernameLabel);

    m_realnameLabel = makeValueLabel(tr("—"), this);
    identityForm->addRow(makeTitle(tr("Real name"), this), m_realnameLabel);

    m_channelsCountLabel = makeValueLabel(tr("0"), this);
    identityForm->addRow(makeTitle(tr("Channels joined"), this), m_channelsCountLabel);

    mainLayout->addWidget(identityGroup);

    // ── Network stats group ───────────────────────────────────────────────

    auto *netGroup = new QGroupBox(tr("Network"), this);
    auto *netForm = new QFormLayout(netGroup);
    netForm->setSpacing(6);

    m_bytesSentLabel = makeValueLabel(QStringLiteral("0 B"), this);
    netForm->addRow(makeTitle(tr("Bytes sent"), this), m_bytesSentLabel);

    m_bytesReceivedLabel = makeValueLabel(QStringLiteral("0 B"), this);
    netForm->addRow(makeTitle(tr("Bytes received"), this), m_bytesReceivedLabel);

    m_latencyLabel = makeValueLabel(tr("—"), this);
    netForm->addRow(makeTitle(tr("Latency (CTCP PING)"), this), m_latencyLabel);

    mainLayout->addWidget(netGroup);

    // ── Server features group ─────────────────────────────────────────────

    auto *featuresGroup = new QGroupBox(tr("Server Features (ISUPPORT)"), this);
    auto *featuresForm = new QFormLayout(featuresGroup);
    featuresForm->setSpacing(6);

    m_chantypesLabel = makeValueLabel(tr("—"), this);
    featuresForm->addRow(makeTitle(tr("CHANTYPES"), this), m_chantypesLabel);

    m_prefixLabel = makeValueLabel(tr("—"), this);
    featuresForm->addRow(makeTitle(tr("PREFIX"), this), m_prefixLabel);

    m_chanmodesLabel = makeValueLabel(tr("—"), this);
    featuresForm->addRow(makeTitle(tr("CHANMODES"), this), m_chanmodesLabel);

    m_networkLabel = makeValueLabel(tr("—"), this);
    featuresForm->addRow(makeTitle(tr("NETWORK"), this), m_networkLabel);

    m_maxchannelsLabel = makeValueLabel(tr("—"), this);
    featuresForm->addRow(makeTitle(tr("MAXCHANNELS"), this), m_maxchannelsLabel);

    m_nicklenLabel = makeValueLabel(tr("—"), this);
    featuresForm->addRow(makeTitle(tr("NICKLEN"), this), m_nicklenLabel);

    mainLayout->addWidget(featuresGroup);

    // ── Change nick row ───────────────────────────────────────────────────

    auto *nickRow = new QHBoxLayout();
    nickRow->setSpacing(8);

    m_nickEdit = new QLineEdit(this);
    m_nickEdit->setPlaceholderText(tr("New nickname"));
    nickRow->addWidget(m_nickEdit, 1);

    m_changeNickBtn = new QPushButton(tr("Change Nick"), this);
    nickRow->addWidget(m_changeNickBtn);

    mainLayout->addLayout(nickRow);

    // ── Raw command row ───────────────────────────────────────────────────

    auto *rawRow = new QHBoxLayout();
    rawRow->setSpacing(8);

    m_rawCommandEdit = new QLineEdit(this);
    m_rawCommandEdit->setPlaceholderText(tr("Raw IRC command…"));
    rawRow->addWidget(m_rawCommandEdit, 1);

    m_sendRawBtn = new QPushButton(tr("Send"), this);
    rawRow->addWidget(m_sendRawBtn);

    mainLayout->addLayout(rawRow);

    // ── Action buttons ────────────────────────────────────────────────────

    auto *actionRow = new QHBoxLayout();
    actionRow->setSpacing(8);

    m_disconnectBtn = new QPushButton(tr("Disconnect"), this);
    m_disconnectBtn->setObjectName(QStringLiteral("disconnectBtn"));
    m_disconnectBtn->setMinimumHeight(34);
    actionRow->addWidget(m_disconnectBtn);

    m_reconnectBtn = new QPushButton(tr("Reconnect"), this);
    m_reconnectBtn->setObjectName(QStringLiteral("reconnectBtn"));
    m_reconnectBtn->setMinimumHeight(34);
    actionRow->addWidget(m_reconnectBtn);

    m_copyInfoBtn = new QPushButton(tr("Copy Server Info"), this);
    m_copyInfoBtn->setMinimumHeight(34);
    actionRow->addWidget(m_copyInfoBtn);

    actionRow->addStretch();

    mainLayout->addLayout(actionRow);
    mainLayout->addStretch();

    // ── Connections ───────────────────────────────────────────────────────

    connect(m_changeNickBtn, &QPushButton::clicked,
            this, &IrcConnectionDialog::onChangeNick);
    connect(m_nickEdit, &QLineEdit::returnPressed,
            this, &IrcConnectionDialog::onChangeNick);
    connect(m_sendRawBtn, &QPushButton::clicked,
            this, &IrcConnectionDialog::onSendRaw);
    connect(m_rawCommandEdit, &QLineEdit::returnPressed,
            this, &IrcConnectionDialog::onSendRaw);
    connect(m_disconnectBtn, &QPushButton::clicked,
            this, &IrcConnectionDialog::onDisconnect);
    connect(m_reconnectBtn, &QPushButton::clicked,
            this, &IrcConnectionDialog::onReconnect);
    connect(m_copyInfoBtn, &QPushButton::clicked,
            this, &IrcConnectionDialog::onCopyServerInfo);
}

// ─── Refresh ───────────────────────────────────────────────────────────────────

void IrcConnectionDialog::refreshInfo()
{
    if (!m_session || !m_socket) {
        return;
    }

    const QString host = m_socket->peerName();
    const quint16 port = m_socket->peerPort();
    m_serverLabel->setText(QStringLiteral("%1:%2").arg(host).arg(port));

    QSslSocket *sslSock = qobject_cast<QSslSocket *>(m_socket);
    if (sslSock && sslSock->isEncrypted()) {
        QSslConfiguration cfg = sslSock->sslConfiguration();
        QSslCertificate cert = cfg.peerCertificate();
        if (!cert.isNull()) {
            m_sslLabel->setText(QStringLiteral("\xF0\x9F\x94\x92 %1\nCN: %2\nIssuer: %3")
                .arg(tr("Encrypted"), cert.subjectInfo(QSslCertificate::CommonName)
                                           .join(QStringLiteral(", ")),
                     cert.issuerInfo(QSslCertificate::CommonName)
                         .join(QStringLiteral(", "))));
        } else {
            m_sslLabel->setText(QStringLiteral("\xF0\x9F\x94\x92 %1").arg(tr("Encrypted")));
        }
    } else {
        m_sslLabel->setText(tr("\xE2\x9A\xA0 Not encrypted"));
    }

    m_nickLabel->setText(m_session->userId());
    m_realnameLabel->setText(m_session->displayName());

    auto rooms = m_session->getRooms();
    m_channelsCountLabel->setText(QString::number(rooms.size()));

    // Try to capture server features from ISUPPORT messages in *status room
    auto msgs = m_session->getMessages(QStringLiteral("*status"), 200);
    for (const auto &msg : msgs) {
        if (msg.contentType != ProtocolContentType::SYSTEM) {
            continue;
        }

        QString text = msg.text;
        static QRegularExpression tokenRe(QStringLiteral(
            R"(([A-Z]+)=(\S+))"));
        auto it = tokenRe.globalMatch(text);
        while (it.hasNext()) {
            auto m = it.next();
            m_features[m.captured(1).toUpper()] = m.captured(2);
        }
    }

    auto setFeat = [this](QLabel *label, const QString &key) {
        auto it = m_features.constFind(key);
        label->setText(it != m_features.constEnd() ? it.value() : tr("—"));
    };

    setFeat(m_chantypesLabel,   QStringLiteral("CHANTYPES"));
    setFeat(m_prefixLabel,      QStringLiteral("PREFIX"));
    setFeat(m_chanmodesLabel,   QStringLiteral("CHANMODES"));
    setFeat(m_networkLabel,     QStringLiteral("NETWORK"));
    setFeat(m_maxchannelsLabel, QStringLiteral("MAXCHANNELS"));
    setFeat(m_nicklenLabel,     QStringLiteral("NICKLEN"));
}

// ─── Duration ──────────────────────────────────────────────────────────────────

void IrcConnectionDialog::onUpdateDuration()
{
    m_durationLabel->setText(formatDuration());
}

QString IrcConnectionDialog::formatDuration() const
{
    if (!m_connectedSince.isValid() || !m_session) {
        return tr("—");
    }

    const qint64 elapsed = m_connectedSince.elapsed() / 1000;
    if (elapsed < 0) {
        return tr("—");
    }

    const auto h = elapsed / 3600;
    const auto m = (elapsed % 3600) / 60;
    const auto s = elapsed % 60;
    return QStringLiteral("%1h %2m %3s").arg(h).arg(m, 2, 10, QLatin1Char('0'))
                                        .arg(s, 2, 10, QLatin1Char('0'));
}

// ─── Stats ─────────────────────────────────────────────────────────────────────

void IrcConnectionDialog::onUpdateStats()
{
    m_bytesSentLabel->setText(formatBytes(static_cast<qint64>(m_bytesSent)));
    m_bytesReceivedLabel->setText(formatBytes(static_cast<qint64>(m_bytesReceived)));

    if (m_session) {
        m_channelsCountLabel->setText(
            QString::number(m_session->getRooms().size()));
    }
}

void IrcConnectionDialog::onSocketBytesWritten(qint64 bytes)
{
    m_bytesSent += static_cast<quint64>(qMax(0LL, bytes));
}

void IrcConnectionDialog::onLatencyReply()
{
    if (!m_socket) {
        return;
    }

    while (m_socket->canReadLine()) {
        QByteArray line = m_socket->readLine().trimmed();
        QString lineStr = QString::fromUtf8(line);

        if (lineStr.startsWith(QStringLiteral("PONG "))) {
            QString tail = lineStr.mid(5).trimmed();
            if (tail.startsWith(QStringLiteral("LAG"))) {
                qint64 elapsed = m_latencyTimerElapsed.elapsed();
                m_latencyLabel->setText(QStringLiteral("%1 ms").arg(elapsed));
            }
        }
    }
}

QString IrcConnectionDialog::formatBytes(qint64 bytes) const
{
    if (bytes < 1024) {
        return QStringLiteral("%1 B").arg(bytes);
    }
    if (bytes < 1024 * 1024) {
        return QStringLiteral("%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
    }
    if (bytes < 1024LL * 1024 * 1024) {
        return QStringLiteral("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 1);
    }
    return QStringLiteral("%1 GB").arg(bytes / (1024.0 * 1024.0 * 1024.0), 0, 'f', 2);
}

// ─── Slots ─────────────────────────────────────────────────────────────────────

void IrcConnectionDialog::onChangeNick()
{
    if (!m_session || !m_socket) {
        return;
    }

    const QString newNick = m_nickEdit->text().trimmed();
    if (newNick.isEmpty()) {
        return;
    }

    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->write(QStringLiteral("NICK %1\r\n").arg(newNick).toUtf8());
        m_nickEdit->clear();
    }
}

void IrcConnectionDialog::onSendRaw()
{
    if (!m_session || !m_socket) {
        return;
    }

    const QString cmd = m_rawCommandEdit->text().trimmed();
    if (cmd.isEmpty()) {
        return;
    }

    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->write(cmd.toUtf8() + "\r\n");
        m_rawCommandEdit->clear();
    }
}

void IrcConnectionDialog::onDisconnect()
{
    if (!m_session) {
        return;
    }

    m_session->close();
}

void IrcConnectionDialog::onReconnect()
{
    if (!m_session) {
        return;
    }

    m_session->close();
    m_session->open();
}

void IrcConnectionDialog::onCopyServerInfo()
{
    const QString host = m_socket ? m_socket->peerName() : tr("unknown");
    const quint16 port = m_socket ? m_socket->peerPort() : 0;
    const QString nick = m_session ? m_session->userId() : tr("unknown");

    const QString info = QStringLiteral(
        "Server: %1:%2\n"
        "Nick: %3\n"
        "Connected: %4\n"
        "TLS: %5\n"
        "CHANTYPES: %6\n"
        "PREFIX: %7\n"
        "CHANMODES: %8\n"
        "NETWORK: %9\n"
        "MAXCHANNELS: %10\n"
        "NICKLEN: %11")
        .arg(host, QString::number(port), nick,
             m_durationLabel->text(), m_sslLabel->text(),
             m_chantypesLabel->text(), m_prefixLabel->text(),
             m_chanmodesLabel->text(), m_networkLabel->text(),
             m_maxchannelsLabel->text(), m_nicklenLabel->text());

    QClipboard *clipboard = QApplication::clipboard();
    if (clipboard) {
        clipboard->setText(info);
    }
}

void IrcConnectionDialog::onConnectionStateChanged()
{
    if (!m_session) {
        return;
    }

    const auto state = m_session->connectionState();
    const bool connected = (state == ConnectionState::REGISTERED);

    if (connected) {
        if (!m_connectedSince.isValid()) {
            m_connectedSince.start();
        }
    } else {
        m_connectedSince.invalidate();
        m_durationLabel->setText(tr("—"));
    }

    m_disconnectBtn->setEnabled(connected);
    m_reconnectBtn->setEnabled(!connected ||
                               state == ConnectionState::ERROR);
    m_changeNickBtn->setEnabled(connected);
    m_nickEdit->setEnabled(connected);
    m_sendRawBtn->setEnabled(connected);
    m_rawCommandEdit->setEnabled(connected);

    if (connected) {
        // Re-parse server features in case new ISUPPORT came in
        m_features.clear();
        refreshInfo();
    }
}
