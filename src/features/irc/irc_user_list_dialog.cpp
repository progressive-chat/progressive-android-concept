#include "features/irc/irc_user_list_dialog.hpp"

#include "irc/irc_session.hpp"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollBar>
#include <QSslSocket>
#include <QTcpSocket>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>

static const char *kIrcUserListStyleSheet = R"(
    IrcUserListDialog {
        background-color: #1e1e2e;
    }
    IrcUserListDialog QLabel {
        color: #cdd6f4;
        font-size: 12px;
        background: transparent;
    }
    IrcUserListDialog QLabel#titleLabel {
        font-size: 14px;
        font-weight: bold;
    }
    IrcUserListDialog QLabel#userCountLabel {
        color: #6c7086;
        font-size: 12px;
        padding: 4px 0px;
    }
    IrcUserListDialog QLineEdit {
        background-color: #313244;
        color: #cdd6f4;
        border: 1px solid #45475a;
        border-radius: 6px;
        padding: 6px 10px;
        font-size: 12px;
    }
    IrcUserListDialog QLineEdit:focus {
        border-color: #89b4fa;
    }
    IrcUserListDialog QComboBox {
        background-color: #313244;
        color: #cdd6f4;
        border: 1px solid #45475a;
        border-radius: 6px;
        padding: 5px 10px;
        font-size: 12px;
        min-height: 20px;
    }
    IrcUserListDialog QComboBox:hover {
        border-color: #89b4fa;
    }
    IrcUserListDialog QComboBox QAbstractItemView {
        background-color: #313244;
        color: #cdd6f4;
        selection-background-color: #45475a;
        border: 1px solid #45475a;
        outline: none;
    }
    IrcUserListDialog QListWidget {
        background-color: #1e1e2e;
        color: #cdd6f4;
        border: 1px solid #313244;
        outline: none;
        font-size: 12px;
    }
    IrcUserListDialog QListWidget::item {
        border-bottom: 1px solid #313244;
        padding: 4px;
    }
    IrcUserListDialog QListWidget::item:hover {
        background-color: #2a2a3c;
    }
    IrcUserListDialog QListWidget::item:selected {
        background-color: #313244;
    }
    IrcUserListDialog QPushButton {
        background-color: #313244;
        color: #cdd6f4;
        border: 1px solid #45475a;
        border-radius: 6px;
        padding: 6px 16px;
        font-size: 12px;
        min-height: 28px;
    }
    IrcUserListDialog QPushButton:hover {
        background-color: #45475a;
        border-color: #89b4fa;
    }
    IrcUserListDialog QPushButton:disabled {
        color: #585b70;
    }
    IrcUserListDialog QPushButton#refreshButton {
        background-color: #a6e3a1;
        color: #1e1e2e;
        font-weight: bold;
        border: none;
    }
    IrcUserListDialog QPushButton#refreshButton:hover {
        background-color: #94e2d5;
    }
    IrcUserListDialog QPushButton#refreshButton:disabled {
        background-color: #585b70;
        color: #313244;
    }
    IrcUserListDialog QProgressBar {
        background-color: #313244;
        border: none;
        border-radius: 4px;
        height: 4px;
    }
    IrcUserListDialog QProgressBar::chunk {
        background-color: #89b4fa;
        border-radius: 4px;
    }
    IrcUserListDialog QMenu {
        background-color: #313244;
        color: #cdd6f4;
        border: 1px solid #45475a;
        border-radius: 6px;
        padding: 4px;
    }
    IrcUserListDialog QMenu::item {
        padding: 5px 24px;
        border-radius: 4px;
    }
    IrcUserListDialog QMenu::item:selected {
        background-color: #45475a;
    }
    IrcUserListDialog QMenu::separator {
        height: 1px;
        background: #45475a;
        margin: 4px 8px;
    }
)";

static const char *kRankChars = "~&@%+";

static int rankSortOrder(const QString &rank)
{
    if (rank == QStringLiteral("~")) return 0;
    if (rank == QStringLiteral("&")) return 1;
    if (rank == QStringLiteral("@")) return 2;
    if (rank == QStringLiteral("%")) return 3;
    if (rank == QStringLiteral("+")) return 4;
    return 5;
}

static QString rankBadge(const QString &rank)
{
    if (rank == QStringLiteral("~"))  return QStringLiteral("~");
    if (rank == QStringLiteral("&"))  return QStringLiteral("&");
    if (rank == QStringLiteral("@"))  return QStringLiteral("@");
    if (rank == QStringLiteral("%"))  return QStringLiteral("%%");
    if (rank == QStringLiteral("+"))  return QStringLiteral("+");
    return {};
}

static QString rankLabel(const QString &rank)
{
    if (rank == QStringLiteral("~"))  return QStringLiteral("Owner");
    if (rank == QStringLiteral("&"))  return QStringLiteral("Admin");
    if (rank == QStringLiteral("@"))  return QStringLiteral("Op");
    if (rank == QStringLiteral("%"))  return QStringLiteral("Half-op");
    if (rank == QStringLiteral("+"))  return QStringLiteral("Voice");
    return QStringLiteral("User");
}

static QString rankStyleSheet(const QString &rank)
{
    if (rank == QStringLiteral("~"))
        return QStringLiteral("color: #f9e2af; font-weight: bold; font-size: 13px; background: transparent; padding: 2px 6px;");
    if (rank == QStringLiteral("&"))
        return QStringLiteral("color: #f38ba8; font-weight: bold; font-size: 13px; background: transparent; padding: 2px 6px;");
    if (rank == QStringLiteral("@"))
        return QStringLiteral("color: #cba6f7; font-weight: bold; font-size: 13px; background: transparent; padding: 2px 6px;");
    if (rank == QStringLiteral("%"))
        return QStringLiteral("color: #89b4fa; font-weight: bold; font-size: 13px; background: transparent; padding: 2px 6px;");
    if (rank == QStringLiteral("+"))
        return QStringLiteral("color: #a6e3a1; font-weight: bold; font-size: 13px; background: transparent; padding: 2px 6px;");
    return QStringLiteral("color: transparent; font-size: 13px; background: transparent; padding: 2px 6px;");
}

IrcUserListDialog::IrcUserListDialog(const QString &channel, IrcSession *session, QWidget *parent)
    : QDialog(parent)
    , m_channel(channel)
    , m_session(session)
{
    setMinimumSize(340, 440);
    setStyleSheet(QString::fromLatin1(kIrcUserListStyleSheet));

    if (m_session) {
        m_nick = m_session->userId();

        QAbstractSocket *mainSocket = m_session->findChild<QAbstractSocket *>(QString(),
            Qt::FindDirectChildrenOnly);
        if (mainSocket) {
            m_host = mainSocket->peerName();
            m_port = static_cast<quint16>(mainSocket->peerPort());
            QSslSocket *sslSock = qobject_cast<QSslSocket *>(mainSocket);
            m_useSsl = (sslSock != nullptr);
        }
    }

    setupUi();
    fetchUserList();
}

IrcUserListDialog::~IrcUserListDialog()
{
    closeTempConnection();
}

void IrcUserListDialog::setupUi()
{
    setWindowTitle(tr("Users in %1").arg(m_channel));
    setModal(true);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    auto *titleLabel = new QLabel(tr("Users in %1").arg(m_channel), this);
    titleLabel->setObjectName(QStringLiteral("titleLabel"));
    mainLayout->addWidget(titleLabel);

    auto *filterRow = new QHBoxLayout;
    filterRow->setSpacing(8);

    m_searchBar = new QLineEdit(this);
    m_searchBar->setPlaceholderText(tr("Search users..."));
    m_searchBar->setClearButtonEnabled(true);
    filterRow->addWidget(m_searchBar, 2);

    m_rankFilter = new QComboBox(this);
    m_rankFilter->addItem(tr("All"),            FilterAll);
    m_rankFilter->addItem(tr("Op / Owner"),     FilterOps);
    m_rankFilter->addItem(tr("Voiced"),         FilterVoiced);
    m_rankFilter->addItem(tr("Normal Users"),   FilterNormal);
    filterRow->addWidget(m_rankFilter);

    mainLayout->addLayout(filterRow);

    auto *progressBar = new QProgressBar(this);
    progressBar->setObjectName(QStringLiteral("progressBar"));
    progressBar->setRange(0, 0);
    progressBar->setVisible(false);
    progressBar->setFixedHeight(4);
    mainLayout->addWidget(progressBar);

    m_userList = new QListWidget(this);
    m_userList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_userList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_userList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_userList->setContextMenuPolicy(Qt::CustomContextMenu);
    mainLayout->addWidget(m_userList, 1);

    m_userCountLabel = new QLabel(this);
    m_userCountLabel->setObjectName(QStringLiteral("userCountLabel"));
    m_userCountLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_userCountLabel);

    auto *buttonRow = new QHBoxLayout;
    buttonRow->setSpacing(8);
    buttonRow->addStretch();

    m_refreshButton = new QPushButton(tr("Refresh"), this);
    m_refreshButton->setObjectName(QStringLiteral("refreshButton"));
    m_refreshButton->setCursor(Qt::PointingHandCursor);
    buttonRow->addWidget(m_refreshButton);

    m_closeButton = new QPushButton(tr("Close"), this);
    buttonRow->addWidget(m_closeButton);

    mainLayout->addLayout(buttonRow);

    connect(m_searchBar, &QLineEdit::textChanged,
            this, &IrcUserListDialog::onSearchChanged);
    connect(m_rankFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &IrcUserListDialog::onRankFilterChanged);
    connect(m_userList, &QListWidget::customContextMenuRequested,
            this, &IrcUserListDialog::onUserContextMenu);
    connect(m_refreshButton, &QPushButton::clicked,
            this, &IrcUserListDialog::onRefreshClicked);
    connect(m_closeButton, &QPushButton::clicked,
            this, &QDialog::close);
}

void IrcUserListDialog::onRefreshClicked()
{
    fetchUserList();
}

void IrcUserListDialog::onSearchChanged(const QString &text)
{
    Q_UNUSED(text)
    populateList();
}

void IrcUserListDialog::onRankFilterChanged(int index)
{
    Q_UNUSED(index)
    populateList();
}

void IrcUserListDialog::fetchUserList()
{
    if (m_host.isEmpty()) {
        if (m_session) {
            QAbstractSocket *mainSocket = m_session->findChild<QAbstractSocket *>(QString(),
                Qt::FindDirectChildrenOnly);
            if (mainSocket) {
                m_host = mainSocket->peerName();
                m_port = static_cast<quint16>(mainSocket->peerPort());
                QSslSocket *sslSock = qobject_cast<QSslSocket *>(mainSocket);
                m_useSsl = (sslSock != nullptr);
            }
        }
        if (m_host.isEmpty()) {
            QMessageBox::warning(this, tr("No Connection"),
                                 tr("No active IRC session to fetch user list from."));
            return;
        }
    }

    closeTempConnection();

    auto *progressBar = findChild<QProgressBar *>(QStringLiteral("progressBar"));
    if (progressBar) {
        progressBar->setVisible(true);
    }
    m_refreshButton->setEnabled(false);

    m_allUsers.clear();
    m_userList->clear();
    updateUserCount();
    m_namesInProgress = true;

    startTempConnection();
}

void IrcUserListDialog::startTempConnection()
{
    m_tempBuffer.clear();
    m_tempRegistered = false;
    m_namesInProgress = true;

    if (m_useSsl) {
        QSslSocket *ssl = new QSslSocket(this);
        ssl->ignoreSslErrors();
        connect(ssl, &QSslSocket::encrypted, this, &IrcUserListDialog::onTempConnected);
        connect(ssl, &QSslSocket::errorOccurred, this, &IrcUserListDialog::onTempError);
        connect(ssl, &QSslSocket::readyRead, this, &IrcUserListDialog::onTempReadyRead);
        m_tempSocket = ssl;
        ssl->connectToHostEncrypted(m_host, m_port);
    } else {
        QTcpSocket *tcp = new QTcpSocket(this);
        connect(tcp, &QTcpSocket::connected, this, &IrcUserListDialog::onTempConnected);
        connect(tcp, &QTcpSocket::errorOccurred, this, &IrcUserListDialog::onTempError);
        connect(tcp, &QTcpSocket::readyRead, this, &IrcUserListDialog::onTempReadyRead);
        m_tempSocket = tcp;
        tcp->connectToHost(m_host, m_port);
    }

    m_tempTimeout = new QTimer(this);
    m_tempTimeout->setSingleShot(true);
    connect(m_tempTimeout, &QTimer::timeout, this, [this]() {
        closeTempConnection();
        auto *progressBar = findChild<QProgressBar *>(QStringLiteral("progressBar"));
        if (progressBar) progressBar->setVisible(false);
        m_refreshButton->setEnabled(true);
        updateUserCount();
    });
    m_tempTimeout->start(20000);
}

void IrcUserListDialog::closeTempConnection()
{
    if (m_tempTimeout) {
        m_tempTimeout->stop();
        delete m_tempTimeout;
        m_tempTimeout = nullptr;
    }

    if (m_tempSocket) {
        m_tempSocket->disconnect();
        if (m_tempSocket->state() != QAbstractSocket::UnconnectedState) {
            m_tempSocket->abort();
        }
        m_tempSocket->deleteLater();
        m_tempSocket = nullptr;
    }

    m_tempBuffer.clear();
    m_tempRegistered = false;
    m_namesInProgress = false;
    m_whoisInProgress = false;
    m_whoisBuffer.clear();
}

void IrcUserListDialog::sendLine(const QString &line)
{
    if (!m_tempSocket || m_tempSocket->state() != QAbstractSocket::ConnectedState) {
        return;
    }
    m_tempSocket->write(line.toUtf8() + "\r\n");
}

void IrcUserListDialog::onTempConnected()
{
    QString listNick = m_nick;
    if (listNick.isEmpty()) {
        listNick = QStringLiteral("client");
    }
    if (listNick.length() > 24) {
        listNick = listNick.left(24);
    }

    QString listUser = listNick;
    if (listUser.length() > 9) {
        listUser = listUser.left(9);
    }

    sendLine(QStringLiteral("NICK %1").arg(listNick));
    sendLine(QStringLiteral("USER %1 0 * :%2").arg(listUser, listNick));
}

void IrcUserListDialog::onTempReadyRead()
{
    QByteArray data = m_tempSocket->readAll();
    m_tempBuffer.append(data);

    while (true) {
        int eol = m_tempBuffer.indexOf('\n');
        if (eol == -1)
            break;

        QByteArray line = m_tempBuffer.left(eol);
        m_tempBuffer.remove(0, eol + 1);
        line = line.trimmed();

        if (line.isEmpty())
            continue;

        QString lineStr = QString::fromUtf8(line);

        static QRegularExpression numericRe(QStringLiteral(
            R"(^(?::\S+ )?(\d{3})\s)"));
        QRegularExpressionMatch match = numericRe.match(lineStr);

        if (match.hasMatch()) {
            int numeric = match.captured(1).toInt();

            if (!m_tempRegistered) {
                if (numeric == 1) {
                    m_tempRegistered = true;
                    sendLine(QStringLiteral("NAMES %1").arg(m_channel));
                }
                continue;
            }

            if (m_namesInProgress && numeric == 353) {
                static QRegularExpression namesRe(QStringLiteral(
                    R"(^(?::\S+ )?\d{3}\s\S+\s[=@*]\s(\S+)\s:(.*))"));
                QRegularExpressionMatch nm = namesRe.match(lineStr);
                if (nm.hasMatch()) {
                    QStringList names = nm.captured(2).split(QStringLiteral(" "), Qt::SkipEmptyParts);
                    for (const QString &entry : names) {
                        QString nick = cleanNick(entry);
                        if (nick.isEmpty())
                            continue;

                        QChar first = entry.at(0);
                        QString rank;
                        for (const char *p = kRankChars; *p; ++p) {
                            if (first == QLatin1Char(*p)) {
                                rank = QChar::fromLatin1(*p);
                                break;
                            }
                        }

                        IrcUserInfo info;
                        info.nick = nick;
                        info.rank = rank;
                        m_allUsers.append(info);
                    }
                }
            } else if (m_namesInProgress && numeric == 366) {
                m_namesInProgress = false;

                std::sort(m_allUsers.begin(), m_allUsers.end(),
                          [](const IrcUserInfo &a, const IrcUserInfo &b) {
                              int ra = rankSortOrder(a.rank);
                              int rb = rankSortOrder(b.rank);
                              if (ra != rb)
                                  return ra < rb;
                              return a.nick.toLower() < b.nick.toLower();
                          });

                populateList();

                auto *progressBar = findChild<QProgressBar *>(QStringLiteral("progressBar"));
                if (progressBar)
                    progressBar->setVisible(false);
                m_refreshButton->setEnabled(true);

                if (!m_whoisTarget.isEmpty()) {
                    m_whoisInProgress = true;
                    sendLine(QStringLiteral("WHOIS %1 %1").arg(m_whoisTarget));
                } else {
                    closeTempConnection();
                }
            } else if (m_whoisInProgress && (numeric == 311 || numeric == 312 ||
                        numeric == 313 || numeric == 317 || numeric == 319 ||
                        numeric == 301 || numeric == 338 || numeric == 330)) {
                QString text = lineStr.section(QStringLiteral(" :"), 1);
                if (!text.isEmpty()) {
                    m_whoisBuffer += text + QStringLiteral("\n");
                }
            } else if (m_whoisInProgress && numeric == 318) {
                m_whoisInProgress = false;
                m_whoisTarget.clear();

                if (!m_whoisBuffer.isEmpty()) {
                    QMessageBox::information(this, tr("WhoIs — %1").arg(m_contextNick),
                                             m_whoisBuffer.trimmed());
                }
                m_whoisBuffer.clear();

                auto *progressBar = findChild<QProgressBar *>(QStringLiteral("progressBar"));
                if (progressBar)
                    progressBar->setVisible(false);
                m_refreshButton->setEnabled(true);
                closeTempConnection();
            }
        } else {
            static QRegularExpression prefixRe(QStringLiteral(R"(^(:\S+)\s)"));
            QRegularExpressionMatch pm = prefixRe.match(lineStr);
            if (pm.hasMatch()) {
                QString prefix = pm.captured(1).mid(1);
                QString rest = lineStr.mid(pm.capturedLength()).trimmed();
                QString cmd = rest.section(QStringLiteral(" "), 0, 0);
                QString trailing = rest.section(QStringLiteral(" :"), 1);

                if (cmd == QStringLiteral("MODE")) {
                    QStringList parts = rest.split(QStringLiteral(" "));
                    if (parts.size() >= 3) {
                        QString chan = parts[1];
                        QString modes = parts[2];
                        QString target = parts.size() > 3 ? parts[3] : QString();
                        if (target.isEmpty() && trailing.isEmpty())
                            target = trailing;
                    }
                } else if (cmd == QStringLiteral("KICK")) {
                    QStringList parts = rest.split(QStringLiteral(" "));
                    if (parts.size() >= 2) {
                        QString kickedNick = parts[1];
                        for (int i = 0; i < m_allUsers.size(); ++i) {
                            if (m_allUsers[i].nick.toLower() == kickedNick.toLower()) {
                                m_allUsers.remove(i);
                                populateList();
                                break;
                            }
                        }
                    }
                } else if (cmd == QStringLiteral("JOIN") || cmd == QStringLiteral("PART") ||
                           cmd == QStringLiteral("QUIT")) {
                    onRefreshClicked();
                }
            }
        }
    }
}

void IrcUserListDialog::onTempError()
{
    closeTempConnection();

    auto *progressBar = findChild<QProgressBar *>(QStringLiteral("progressBar"));
    if (progressBar)
        progressBar->setVisible(false);
    m_refreshButton->setEnabled(true);
    updateUserCount();
}

void IrcUserListDialog::populateList()
{
    QString searchText = m_searchBar->text().trimmed();
    int filterMode = m_rankFilter->currentData().toInt();
    m_userList->clear();

    for (int i = 0; i < m_allUsers.size(); ++i) {
        const IrcUserInfo &user = m_allUsers[i];

        if (filterMode == FilterOps) {
            if (user.rank != QStringLiteral("~") && user.rank != QStringLiteral("&") &&
                user.rank != QStringLiteral("@") && user.rank != QStringLiteral("%")) {
                continue;
            }
        } else if (filterMode == FilterVoiced) {
            if (user.rank != QStringLiteral("+")) {
                continue;
            }
        } else if (filterMode == FilterNormal) {
            if (!user.rank.isEmpty()) {
                continue;
            }
        }

        if (!searchText.isEmpty() &&
            !user.nick.contains(searchText, Qt::CaseInsensitive) &&
            !user.user.contains(searchText, Qt::CaseInsensitive) &&
            !user.host.contains(searchText, Qt::CaseInsensitive)) {
            continue;
        }

        auto *item = new QListWidgetItem();
        item->setData(Qt::UserRole, user.nick);

        auto *container = new QWidget();
        container->setStyleSheet(QStringLiteral("background: transparent;"));

        auto *row = new QHBoxLayout(container);
        row->setContentsMargins(8, 5, 8, 5);
        row->setSpacing(8);

        QString badge = rankBadge(user.rank);
        auto *badgeLabel = new QLabel(badge.isEmpty() ? QStringLiteral("\xE2\x80\xA2") : badge, container);
        badgeLabel->setStyleSheet(rankStyleSheet(user.rank));
        badgeLabel->setFixedWidth(24);
        badgeLabel->setAlignment(Qt::AlignCenter);
        row->addWidget(badgeLabel);

        auto *nickLabel = new QLabel(user.nick, container);
        nickLabel->setStyleSheet(QStringLiteral(
            "color: #cdd6f4; font-weight: bold; font-size: 13px; background: transparent;"));
        nickLabel->setFixedWidth(110);
        row->addWidget(nickLabel);

        QString hostInfo;
        if (!user.user.isEmpty() || !user.host.isEmpty()) {
            hostInfo = user.user + QStringLiteral("@") + user.host;
        } else {
            hostInfo = rankLabel(user.rank);
        }
        auto *hostLabel = new QLabel(hostInfo, container);
        hostLabel->setStyleSheet(QStringLiteral(
            "color: #6c7086; font-size: 11px; background: transparent;"));
        hostLabel->setWordWrap(false);
        row->addWidget(hostLabel, 1);

        item->setSizeHint(QSize(0, 34));
        m_userList->addItem(item);
        m_userList->setItemWidget(item, container);
    }

    updateUserCount();
}

void IrcUserListDialog::updateUserCount()
{
    int total = m_allUsers.size();
    int ops = 0;
    int voiced = 0;
    for (const auto &u : m_allUsers) {
        if (u.rank == QStringLiteral("~") || u.rank == QStringLiteral("&") ||
            u.rank == QStringLiteral("@") || u.rank == QStringLiteral("%")) {
            ++ops;
        } else if (u.rank == QStringLiteral("+")) {
            ++voiced;
        }
    }

    m_userCountLabel->setText(
        tr("%1 users, %2 ops, %3 voiced").arg(total).arg(ops).arg(voiced));
}

int IrcUserListDialog::rankValue(const QString &rank) const
{
    static const QMap<QString, int> map = {
        {QStringLiteral("~"), 0},
        {QStringLiteral("&"), 1},
        {QStringLiteral("@"), 2},
        {QStringLiteral("%"), 3},
        {QStringLiteral("+"), 4},
        {QString(),          5}
    };
    return map.value(rank, 5);
}

void IrcUserListDialog::onUserContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = m_userList->itemAt(pos);
    if (!item) return;

    m_contextNick = item->data(Qt::UserRole).toString();

    IrcUserInfo currentUser;
    bool found = false;
    for (const auto &u : m_allUsers) {
        if (u.nick == m_contextNick) {
            currentUser = u;
            found = true;
            break;
        }
    }
    if (!found) return;

    bool isSelf = (m_contextNick.toLower() == m_nick.toLower());
    QString targetRank = currentUser.rank;

    QMenu menu(this);

    QAction *whoisAction = menu.addAction(tr("WhoIs"));
    menu.addSeparator();
    QAction *chatAction = menu.addAction(tr("Start Private Chat"));
    menu.addSeparator();

    QAction *kickAction = nullptr;
    QAction *banAction = nullptr;
    QAction *opAction = nullptr;
    QAction *deopAction = nullptr;

    if (!isSelf) {
        kickAction = menu.addAction(tr("Kick %1").arg(m_contextNick));
        banAction = menu.addAction(tr("Ban %1").arg(m_contextNick));
        menu.addSeparator();

        bool isOp = (targetRank == QStringLiteral("~") || targetRank == QStringLiteral("&") ||
                     targetRank == QStringLiteral("@") || targetRank == QStringLiteral("%"));
        if (isOp) {
            deopAction = menu.addAction(tr("Deop %1").arg(m_contextNick));
        } else {
            opAction = menu.addAction(tr("Op %1").arg(m_contextNick));
        }
    }

    QAction *chosen = menu.exec(m_userList->viewport()->mapToGlobal(pos));
    if (!chosen) return;

    if (chosen == whoisAction) {
        onWhoIs();
    } else if (chosen == chatAction) {
        onStartPrivateChat();
    } else if (kickAction && chosen == kickAction) {
        onKickUser();
    } else if (banAction && chosen == banAction) {
        onBanUser();
    } else if (opAction && chosen == opAction) {
        executeMode(m_contextNick, QStringLiteral("+o"), true);
    } else if (deopAction && chosen == deopAction) {
        executeMode(m_contextNick, QStringLiteral("-o"), true);
    }
}

void IrcUserListDialog::onWhoIs()
{
    if (m_contextNick.isEmpty()) return;
    executeWhoIs(m_contextNick);
}

void IrcUserListDialog::onStartPrivateChat()
{
    if (m_contextNick.isEmpty()) return;
    if (m_session) {
        m_session->joinRoom(m_contextNick);
    }
    close();
}

void IrcUserListDialog::onKickUser()
{
    if (m_contextNick.isEmpty()) return;

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("Confirm Kick"),
        tr("Are you sure you want to kick %1 from %2?")
            .arg(m_contextNick, m_channel),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    executeKick(m_contextNick, tr("Kicked"));
}

void IrcUserListDialog::onBanUser()
{
    if (m_contextNick.isEmpty()) return;

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("Confirm Ban"),
        tr("Are you sure you want to ban %1 from %2?")
            .arg(m_contextNick, m_channel),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    executeBan(m_contextNick);
}

void IrcUserListDialog::onToggleOp()
{
    if (m_contextNick.isEmpty()) return;

    bool isOp = false;
    for (const auto &u : m_allUsers) {
        if (u.nick == m_contextNick) {
            isOp = (u.rank == QStringLiteral("@") || u.rank == QStringLiteral("~") ||
                    u.rank == QStringLiteral("&") || u.rank == QStringLiteral("%"));
            break;
        }
    }

    executeMode(m_contextNick, isOp ? QStringLiteral("-o") : QStringLiteral("+o"), true);
}

void IrcUserListDialog::executeMode(const QString &nick, const QString &mode, bool add)
{
    Q_UNUSED(add)

    if (m_host.isEmpty()) {
        QMessageBox::warning(this, tr("Not Connected"), tr("No IRC server connection."));
        return;
    }

    closeTempConnection();
    m_tempRegistered = false;
    m_namesInProgress = false;
    m_whoisInProgress = false;

    startTempConnection();

    auto *timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [this, nick, mode]() {
        if (m_tempSocket && m_tempSocket->state() == QAbstractSocket::ConnectedState &&
            m_tempRegistered) {
            sendLine(QStringLiteral("MODE %1 %2 %3").arg(m_channel, mode, nick));
            QTimer::singleShot(500, this, [this]() {
                closeTempConnection();
                auto *progressBar = findChild<QProgressBar *>(QStringLiteral("progressBar"));
                if (progressBar) progressBar->setVisible(false);
                m_refreshButton->setEnabled(true);
                onRefreshClicked();
            });
        }
    });
    timer->start(3000);
}

void IrcUserListDialog::executeKick(const QString &nick, const QString &reason)
{
    if (m_host.isEmpty()) {
        QMessageBox::warning(this, tr("Not Connected"), tr("No IRC server connection."));
        return;
    }

    closeTempConnection();
    m_tempRegistered = false;
    m_namesInProgress = false;
    m_whoisInProgress = false;

    startTempConnection();

    auto *timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [this, nick, reason]() {
        if (m_tempSocket && m_tempSocket->state() == QAbstractSocket::ConnectedState &&
            m_tempRegistered) {
            sendLine(QStringLiteral("KICK %1 %2 :%3").arg(m_channel, nick, reason));
            QTimer::singleShot(500, this, [this]() {
                closeTempConnection();
                auto *progressBar = findChild<QProgressBar *>(QStringLiteral("progressBar"));
                if (progressBar) progressBar->setVisible(false);
                m_refreshButton->setEnabled(true);
                onRefreshClicked();
            });
        }
    });
    timer->start(3000);
}

void IrcUserListDialog::executeBan(const QString &nick)
{
    if (m_host.isEmpty()) {
        QMessageBox::warning(this, tr("Not Connected"), tr("No IRC server connection."));
        return;
    }

    QString banMask = nick + QStringLiteral("!*@*");

    closeTempConnection();
    m_tempRegistered = false;
    m_namesInProgress = false;
    m_whoisInProgress = false;

    startTempConnection();

    auto *timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [this, nick, banMask]() {
        if (m_tempSocket && m_tempSocket->state() == QAbstractSocket::ConnectedState &&
            m_tempRegistered) {
            sendLine(QStringLiteral("MODE %1 +b %2").arg(m_channel, banMask));
            sendLine(QStringLiteral("KICK %1 %2 :Banned").arg(m_channel, nick));
            QTimer::singleShot(500, this, [this]() {
                closeTempConnection();
                auto *progressBar = findChild<QProgressBar *>(QStringLiteral("progressBar"));
                if (progressBar) progressBar->setVisible(false);
                m_refreshButton->setEnabled(true);
                onRefreshClicked();
            });
        }
    });
    timer->start(3000);
}

void IrcUserListDialog::executeWhoIs(const QString &nick)
{
    if (m_host.isEmpty()) {
        QMessageBox::warning(this, tr("Not Connected"), tr("No IRC server connection."));
        return;
    }

    closeTempConnection();
    m_tempRegistered = false;
    m_namesInProgress = false;
    m_whoisInProgress = false;

    m_whoisTarget = nick;
    m_whoisBuffer.clear();

    auto *progressBar = findChild<QProgressBar *>(QStringLiteral("progressBar"));
    if (progressBar) progressBar->setVisible(true);
    m_refreshButton->setEnabled(false);

    startTempConnection();
}

QString IrcUserListDialog::extractNick(const QString &prefix) const
{
    int excl = prefix.indexOf(QLatin1Char('!'));
    if (excl != -1) {
        return prefix.left(excl);
    }
    int at = prefix.indexOf(QLatin1Char('@'));
    if (at != -1) {
        return prefix;
    }
    return prefix;
}

QString IrcUserListDialog::extractUser(const QString &prefix) const
{
    int excl = prefix.indexOf(QLatin1Char('!'));
    if (excl == -1) return {};
    int at = prefix.indexOf(QLatin1Char('@'), excl);
    if (at == -1) return prefix.mid(excl + 1);
    return prefix.mid(excl + 1, at - excl - 1);
}

QString IrcUserListDialog::extractHost(const QString &prefix) const
{
    int at = prefix.indexOf(QLatin1Char('@'));
    if (at == -1) return {};
    return prefix.mid(at + 1);
}

QString IrcUserListDialog::cleanNick(const QString &nickWithRank)
{
    QString result = nickWithRank;
    while (!result.isEmpty()) {
        QChar first = result.at(0);
        bool isRank = false;
        for (const char *p = kRankChars; *p; ++p) {
            if (first == QLatin1Char(*p)) {
                isRank = true;
                break;
            }
        }
        if (isRank) {
            result = result.mid(1);
        } else {
            break;
        }
    }
    return result;
}
