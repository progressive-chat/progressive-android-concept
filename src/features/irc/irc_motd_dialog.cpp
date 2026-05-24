#include "irc_motd_dialog.hpp"

#include <QTcpSocket>
#include <QSslSocket>
#include <QTextBrowser>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QClipboard>
#include <QApplication>
#include <QRegularExpression>

IrcMotdDialog::IrcMotdDialog(const QString& serverHost, quint16 port, bool useTls,
                             QWidget* parent)
    : QDialog(parent)
    , m_serverHost(serverHost)
    , m_port(port)
    , m_useTls(useTls)
    , m_socket(nullptr)
    , m_motdView(nullptr)
    , m_statusLabel(nullptr)
    , m_copyButton(nullptr)
    , m_closeButton(nullptr)
    , m_timeoutTimer(nullptr)
    , m_progressBar(nullptr)
    , m_motdStarted(false)
    , m_motdEnded(false)
{
    setWindowTitle(tr("Message of the Day - %1").arg(m_serverHost));
    setMinimumSize(580, 420);
    resize(600, 480);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    m_statusLabel = new QLabel(tr("Connecting to %1:%2...").arg(m_serverHost).arg(m_port), this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_statusLabel);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 0);
    m_progressBar->setMaximumHeight(6);
    m_progressBar->setTextVisible(false);
    mainLayout->addWidget(m_progressBar);

    m_motdView = new QTextBrowser(this);
    m_motdView->setOpenExternalLinks(false);
    m_motdView->setReadOnly(true);
    QFont monoFont("monospace", 10);
    monoFont.setStyleHint(QFont::Monospace);
    m_motdView->setFont(monoFont);
    m_motdView->setVisible(false);
    mainLayout->addWidget(m_motdView, 1);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);

    m_copyButton = new QPushButton(tr("Copy"), this);
    m_copyButton->setEnabled(false);
    connect(m_copyButton, &QPushButton::clicked, this, &IrcMotdDialog::onCopyClicked);
    buttonLayout->addWidget(m_copyButton);

    buttonLayout->addStretch();

    m_closeButton = new QPushButton(tr("Close"), this);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::close);
    buttonLayout->addWidget(m_closeButton);

    mainLayout->addLayout(buttonLayout);

    m_timeoutTimer = new QTimer(this);
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout, this, &IrcMotdDialog::onTimeout);

    if (m_useTls) {
        auto* sslSock = new QSslSocket(this);
        connect(sslSock, &QSslSocket::encrypted, this, &IrcMotdDialog::onConnected);
        connect(sslSock, &QSslSocket::errorOccurred, this, &IrcMotdDialog::onErrorOccurred);
        m_socket = sslSock;
    } else {
        auto* tcpSock = new QTcpSocket(this);
        connect(tcpSock, &QTcpSocket::connected, this, &IrcMotdDialog::onConnected);
        connect(tcpSock, &QTcpSocket::errorOccurred, this, &IrcMotdDialog::onErrorOccurred);
        m_socket = tcpSock;
    }

    connect(m_socket, &QTcpSocket::readyRead, this, &IrcMotdDialog::onReadyRead);

    m_timeoutTimer->start(15000);

    if (m_useTls) {
        auto* sslSock = qobject_cast<QSslSocket*>(m_socket);
        sslSock->connectToHostEncrypted(m_serverHost, m_port);
    } else {
        m_socket->connectToHost(m_serverHost, m_port);
    }
}

IrcMotdDialog::~IrcMotdDialog()
{
    if (m_socket) {
        m_socket->disconnectFromHost();
    }
}

void IrcMotdDialog::onConnected()
{
    m_statusLabel->setText(tr("Connected. Requesting MOTD..."));
    QString nick = QStringLiteral("motd_%1").arg(QCoreApplication::applicationPid());
    sendLine(QStringLiteral("NICK %1").arg(nick));
    sendLine(QStringLiteral("USER motd 0 * :MOTD Fetcher"));
}

void IrcMotdDialog::onReadyRead()
{
    while (m_socket->canReadLine()) {
        QByteArray rawLine = m_socket->readLine();
        QString line = QString::fromUtf8(rawLine.trimmed());
        if (!line.isEmpty()) {
            parseLine(line);
        }
    }
}

void IrcMotdDialog::onErrorOccurred()
{
    if (m_motdEnded) {
        return;
    }
    showError(tr("Connection error: %1").arg(m_socket->errorString()));
}

void IrcMotdDialog::onTimeout()
{
    if (m_motdEnded) {
        return;
    }
    showError(tr("Failed to load MOTD: connection timed out"));
}

void IrcMotdDialog::onCopyClicked()
{
    QString plainText = m_motdView->toPlainText();
    QApplication::clipboard()->setText(plainText);
}

void IrcMotdDialog::sendLine(const QString& line)
{
    m_socket->write(line.toUtf8() + "\r\n");
}

void IrcMotdDialog::parseLine(const QString& line)
{
    static const QRegularExpression numericRegex(R"(^:\S+\s+(\d{3})\s)");
    auto match = numericRegex.match(line);

    if (match.hasMatch()) {
        QString code = match.captured(1);

        if (code == QStringLiteral("375")) {
            handleMotdStart(line);
        } else if (code == QStringLiteral("372")) {
            handleMotdLine(line);
        } else if (code == QStringLiteral("376")) {
            handleMotdEnd();
        } else if (code == QStringLiteral("422")) {
            handleNoMotd();
        }
    }
}

void IrcMotdDialog::handleMotdStart(const QString& line)
{
    m_motdStarted = true;
    QString display = line.section(":-", 1).trimmed();
    if (display.isEmpty()) {
        display = line.section(':', 2).trimmed();
    }
    if (!display.isEmpty()) {
        m_motdLines.append(ansiToHtml(display));
    }
}

void IrcMotdDialog::handleMotdLine(const QString& line)
{
    m_motdStarted = true;
    QString display = line.section(":-", 1).trimmed();
    if (display.isEmpty()) {
        display = line.section(':', 2).trimmed();
    }
    m_motdLines.append(ansiToHtml(display));
}

void IrcMotdDialog::handleMotdEnd()
{
    finalizeMotd();
}

void IrcMotdDialog::handleNoMotd()
{
    m_motdLines.clear();
    m_motdLines.append(QStringLiteral("<i>%1</i>").arg(tr("No MOTD set on this server.")));
    finalizeMotd();
}

void IrcMotdDialog::finalizeMotd()
{
    if (m_motdEnded) {
        return;
    }
    m_motdEnded = true;

    m_timeoutTimer->stop();
    m_socket->disconnectFromHost();

    m_progressBar->setVisible(false);
    m_statusLabel->setText(tr("MOTD for %1").arg(m_serverHost));

    m_motdView->setHtml(m_motdLines.join(QStringLiteral("<br>")));
    m_motdView->setVisible(true);

    m_copyButton->setEnabled(true);
}

void IrcMotdDialog::showError(const QString& message)
{
    m_motdEnded = true;
    m_timeoutTimer->stop();

    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
    }

    m_progressBar->setVisible(false);
    m_statusLabel->setText(message);
    m_statusLabel->setStyleSheet(QStringLiteral("color: red; font-weight: bold;"));

    m_motdView->setPlainText(message);
    m_motdView->setVisible(true);

    m_copyButton->setEnabled(false);
}

QString IrcMotdDialog::ansiToHtml(const QString& text) const
{
    QString result = text;
    result.replace('&', QStringLiteral("&amp;"));
    result.replace('<', QStringLiteral("&lt;"));
    result.replace('>', QStringLiteral("&gt;"));
    result.replace('\"', QStringLiteral("&quot;"));

    static const QRegularExpression ansiPattern(QStringLiteral("\x1B\\[[0-9;]*m"));
    result.replace(ansiPattern, QString());

    result.replace(QStringLiteral("\x02"), QStringLiteral("<b>"));
    result.replace(QStringLiteral("\x1D"), QStringLiteral("<i>"));
    result.replace(QStringLiteral("\x1F"), QStringLiteral("<u>"));
    result.replace(QStringLiteral("\x0F"), QString());

    return result;
}
