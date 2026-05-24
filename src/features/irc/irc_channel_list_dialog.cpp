#include "features/irc/irc_channel_list_dialog.hpp"

#include "irc/irc_session.hpp"

#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QTcpSocket>
#include <QSslSocket>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QRegularExpression>
#include <QStyle>

#include <algorithm>

static const char *kIrcChannelListStyleSheet = R"(
    IrcChannelListDialog {
        background-color: #1e1e2e;
    }
    IrcChannelListDialog QLineEdit {
        background-color: #313244;
        color: #cdd6f4;
        border: 1px solid #45475a;
        border-radius: 6px;
        padding: 6px 10px;
        font-size: 12px;
    }
    IrcChannelListDialog QLineEdit:focus {
        border-color: #89b4fa;
    }
    IrcChannelListDialog QListWidget {
        background-color: #1e1e2e;
        color: #cdd6f4;
        border: 1px solid #313244;
        outline: none;
        font-size: 12px;
    }
    IrcChannelListDialog QListWidget::item {
        border-bottom: 1px solid #313244;
        padding: 4px;
    }
    IrcChannelListDialog QListWidget::item:hover {
        background-color: #2a2a3c;
    }
    IrcChannelListDialog QListWidget::item:selected {
        background-color: #313244;
    }
    IrcChannelListDialog QPushButton {
        background-color: #313244;
        color: #cdd6f4;
        border: 1px solid #45475a;
        border-radius: 6px;
        padding: 4px 12px;
        font-size: 12px;
    }
    IrcChannelListDialog QPushButton:hover {
        background-color: #45475a;
        border-color: #89b4fa;
    }
    IrcChannelListDialog QPushButton:disabled {
        color: #585b70;
    }
    IrcChannelListDialog QPushButton#joinButton {
        background-color: #a6e3a1;
        color: #1e1e2e;
        font-weight: bold;
        border: none;
        padding: 6px 20px;
    }
    IrcChannelListDialog QPushButton#joinButton:hover {
        background-color: #94e2d5;
    }
    IrcChannelListDialog QPushButton#joinButton:disabled {
        background-color: #585b70;
        color: #313244;
    }
    IrcChannelListDialog QPushButton.sortedBy {
        color: #89b4fa;
        border-color: #89b4fa;
    }
    IrcChannelListDialog QProgressBar {
        background-color: #313244;
        border: none;
        border-radius: 4px;
        height: 4px;
        text-align: center;
        font-size: 11px;
        color: #6c7086;
    }
    IrcChannelListDialog QProgressBar::chunk {
        background-color: #89b4fa;
        border-radius: 4px;
    }
)";

IrcChannelListDialog::IrcChannelListDialog(QWidget *parent)
    : QDialog(parent)
    , m_filterBar(nullptr)
    , m_channelList(nullptr)
    , m_refreshButton(nullptr)
    , m_joinButton(nullptr)
    , m_loadingBar(nullptr)
    , m_emptyLabel(nullptr)
    , m_manualChannelInput(nullptr)
    , m_manualJoinButton(nullptr)
    , m_nameHeader(nullptr)
    , m_usersHeader(nullptr)
    , m_topicHeader(nullptr)
    , m_session(nullptr)
    , m_tempSocket(nullptr)
    , m_tempTimeout(nullptr)
    , m_tempRegistered(false)
    , m_listInProgress(false)
    , m_sortColumn(0)
    , m_sortAscending(true)
    , m_port(6667)
    , m_useSsl(false)
{
    setMinimumSize(560, 480);
    setStyleSheet(QString::fromLatin1(kIrcChannelListStyleSheet));
    setupUi();
}

IrcChannelListDialog::~IrcChannelListDialog()
{
    closeTempConnection();
}

void IrcChannelListDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto *contentLayout = new QVBoxLayout();
    contentLayout->setContentsMargins(10, 10, 10, 10);
    contentLayout->setSpacing(8);

    auto *topBar = new QHBoxLayout();
    topBar->setSpacing(8);

    m_filterBar = new QLineEdit(this);
    m_filterBar->setPlaceholderText(tr("Filter channels..."));
    m_filterBar->setClearButtonEnabled(true);
    m_filterBar->setMinimumHeight(30);
    topBar->addWidget(m_filterBar, 1);

    m_refreshButton = new QPushButton(tr("Refresh"), this);
    m_refreshButton->setMinimumHeight(30);
    topBar->addWidget(m_refreshButton);

    contentLayout->addLayout(topBar);

    m_loadingBar = new QProgressBar(this);
    m_loadingBar->setRange(0, 0);
    m_loadingBar->setFixedHeight(4);
    m_loadingBar->setVisible(false);
    contentLayout->addWidget(m_loadingBar);

    auto *headerRow = new QHBoxLayout();
    headerRow->setSpacing(0);
    headerRow->setContentsMargins(0, 0, 0, 0);

    m_nameHeader = new QPushButton(tr("Channel"), this);
    m_nameHeader->setFlat(true);
    m_nameHeader->setCursor(Qt::PointingHandCursor);
    m_nameHeader->setStyleSheet(QStringLiteral(
        "QPushButton { color: #cdd6f4; font-weight: bold; font-size: 12px; text-align: left; padding: 4px 8px; border: none; background: transparent; }"
        "QPushButton:hover { color: #89b4fa; }"));
    headerRow->addWidget(m_nameHeader, 3);

    m_usersHeader = new QPushButton(tr("Users"), this);
    m_usersHeader->setFlat(true);
    m_usersHeader->setCursor(Qt::PointingHandCursor);
    m_usersHeader->setStyleSheet(QStringLiteral(
        "QPushButton { color: #cdd6f4; font-weight: bold; font-size: 12px; padding: 4px 8px; border: none; background: transparent; }"
        "QPushButton:hover { color: #89b4fa; }"));
    m_usersHeader->setFixedWidth(80);
    headerRow->addWidget(m_usersHeader);

    m_topicHeader = new QPushButton(tr("Topic"), this);
    m_topicHeader->setFlat(true);
    m_topicHeader->setCursor(Qt::PointingHandCursor);
    m_topicHeader->setStyleSheet(QStringLiteral(
        "QPushButton { color: #cdd6f4; font-weight: bold; font-size: 12px; text-align: left; padding: 4px 8px; border: none; background: transparent; }"
        "QPushButton:hover { color: #89b4fa; }"));
    headerRow->addWidget(m_topicHeader, 5);

    contentLayout->addLayout(headerRow);

    m_emptyLabel = new QLabel(tr("No channels loaded. Press Refresh to fetch the channel list."), this);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet(QStringLiteral(
        "color: #6c7086; font-size: 13px; padding: 40px;"));
    contentLayout->addWidget(m_emptyLabel);

    m_channelList = new QListWidget(this);
    m_channelList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_channelList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_channelList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_channelList->setVisible(false);
    m_channelList->setMinimumHeight(200);
    contentLayout->addWidget(m_channelList, 1);

    auto *buttonRow = new QHBoxLayout();
    buttonRow->setSpacing(8);

    m_joinButton = new QPushButton(tr("Join Channel"), this);
    m_joinButton->setObjectName(QStringLiteral("joinButton"));
    m_joinButton->setCursor(Qt::PointingHandCursor);
    m_joinButton->setMinimumHeight(34);
    m_joinButton->setEnabled(false);
    buttonRow->addWidget(m_joinButton);

    buttonRow->addStretch();

    contentLayout->addLayout(buttonRow);

    contentLayout->addSpacing(8);

    auto *manualRow = new QHBoxLayout();
    manualRow->setSpacing(8);

    auto *manualLabel = new QLabel(tr("Join:"), this);
    manualLabel->setStyleSheet(QStringLiteral(
        "color: #cdd6f4; font-size: 12px; background: transparent;"));
    manualRow->addWidget(manualLabel);

    m_manualChannelInput = new QLineEdit(this);
    m_manualChannelInput->setPlaceholderText(tr("#channel"));
    m_manualChannelInput->setMinimumHeight(30);
    manualRow->addWidget(m_manualChannelInput, 1);

    m_manualJoinButton = new QPushButton(tr("Join"), this);
    m_manualJoinButton->setMinimumHeight(30);
    manualRow->addWidget(m_manualJoinButton);

    contentLayout->addLayout(manualRow);

    mainLayout->addLayout(contentLayout);

    connect(m_filterBar, &QLineEdit::textChanged,
            this, &IrcChannelListDialog::onFilterTextChanged);
    connect(m_refreshButton, &QPushButton::clicked,
            this, &IrcChannelListDialog::onRefreshClicked);
    connect(m_channelList, &QListWidget::itemDoubleClicked,
            this, &IrcChannelListDialog::onResultItemDoubleClicked);
    connect(m_joinButton, &QPushButton::clicked,
            this, &IrcChannelListDialog::onJoinClicked);
    connect(m_manualJoinButton, &QPushButton::clicked,
            this, &IrcChannelListDialog::onJoinManualClicked);
    connect(m_manualChannelInput, &QLineEdit::returnPressed,
            this, &IrcChannelListDialog::onJoinManualClicked);

    connect(m_nameHeader, &QPushButton::clicked,
            this, [this]() { onSortColumnClicked(0); });
    connect(m_usersHeader, &QPushButton::clicked,
            this, [this]() { onSortColumnClicked(1); });
    connect(m_topicHeader, &QPushButton::clicked,
            this, [this]() { onSortColumnClicked(2); });

    update();
}

void IrcChannelListDialog::setSession(IrcSession *session)
{
    m_session = session;
    if (!m_session) {
        return;
    }

    QAbstractSocket *mainSocket = m_session->findChild<QAbstractSocket *>(QString(),
        Qt::FindDirectChildrenOnly);
    if (!mainSocket) {
        return;
    }

    m_host = mainSocket->peerName();
    m_port = static_cast<quint16>(mainSocket->peerPort());

    QSslSocket *sslSock = qobject_cast<QSslSocket *>(mainSocket);
    m_useSsl = (sslSock != nullptr);

    m_nick = m_session->userId();
}

void IrcChannelListDialog::fetchChannelList()
{
    if (m_host.isEmpty()) {
        if (m_session) {
            setSession(m_session);
        }
        if (m_host.isEmpty()) {
            QMessageBox::warning(this, tr("No Connection"),
                                 tr("No active IRC session to fetch channels from."));
            return;
        }
    }

    closeTempConnection();
    m_allChannels.clear();
    m_filteredChannels.clear();
    m_channelList->clear();
    m_channelList->setVisible(false);
    m_emptyLabel->setText(tr("Fetching channel list..."));
    m_emptyLabel->setVisible(true);
    m_joinButton->setEnabled(false);
    showLoading(true);

    startTempConnection();
}

void IrcChannelListDialog::startTempConnection()
{
    m_tempBuffer.clear();
    m_tempRegistered = false;
    m_listInProgress = false;

    if (m_useSsl) {
        QSslSocket *ssl = new QSslSocket(this);
        ssl->ignoreSslErrors();
        connect(ssl, &QSslSocket::encrypted, this, &IrcChannelListDialog::onTempConnected);
        connect(ssl, &QSslSocket::errorOccurred, this, &IrcChannelListDialog::onTempError);
        connect(ssl, &QSslSocket::readyRead, this, &IrcChannelListDialog::onTempReadyRead);
        m_tempSocket = ssl;
        ssl->connectToHostEncrypted(m_host, m_port);
    } else {
        QTcpSocket *tcp = new QTcpSocket(this);
        connect(tcp, &QTcpSocket::connected, this, &IrcChannelListDialog::onTempConnected);
        connect(tcp, &QTcpSocket::errorOccurred, this, &IrcChannelListDialog::onTempError);
        connect(tcp, &QTcpSocket::readyRead, this, &IrcChannelListDialog::onTempReadyRead);
        m_tempSocket = tcp;
        tcp->connectToHost(m_host, m_port);
    }

    m_tempTimeout = new QTimer(this);
    m_tempTimeout->setSingleShot(true);
    connect(m_tempTimeout, &QTimer::timeout, this, [this]() {
        closeTempConnection();
        m_emptyLabel->setText(tr("Timed out while fetching channel list."));
        m_emptyLabel->setVisible(true);
        m_channelList->setVisible(false);
        showLoading(false);
    });
    m_tempTimeout->start(30000);
}

void IrcChannelListDialog::closeTempConnection()
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
    m_listInProgress = false;
}

void IrcChannelListDialog::onTempConnected()
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

    m_tempSocket->write(
        QStringLiteral("NICK %1\r\nUSER %2 0 * :%3\r\n")
            .arg(listNick, listUser, listNick).toUtf8());
}

void IrcChannelListDialog::onTempReadyRead()
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
        if (!match.hasMatch())
            continue;

        int numeric = match.captured(1).toInt();

        if (!m_tempRegistered) {
            if (numeric == 1) {
                m_tempRegistered = true;
            }
            continue;
        }

        if (numeric == 322) {
            static QRegularExpression listRe(QStringLiteral(
                R"(^(?::\S+ )?\d{3}\s\S+\s(\S+)\s(\d+)\s:(.*))"));
            QRegularExpressionMatch lm = listRe.match(lineStr);
            if (lm.hasMatch()) {
                IrcChannelInfo info;
                info.name = lm.captured(1);
                info.userCount = lm.captured(2).toInt();
                info.topic = lm.captured(3);
                m_allChannels.append(info);
            }
        } else if (numeric == 323) {
            m_listInProgress = false;
            if (m_tempTimeout) {
                m_tempTimeout->stop();
            }
            applyFilter();
            showLoading(false);
            closeTempConnection();
        }
    }
}

void IrcChannelListDialog::onTempError()
{
    closeTempConnection();
    showLoading(false);
    m_emptyLabel->setText(tr("Connection error while fetching channel list."));
    m_emptyLabel->setVisible(true);
    m_channelList->setVisible(false);
}

void IrcChannelListDialog::onSortColumnClicked(int column)
{
    if (m_sortColumn == column) {
        m_sortAscending = !m_sortAscending;
    } else {
        m_sortColumn = column;
        m_sortAscending = true;
    }

    m_nameHeader->setProperty("sortedBy", false);
    m_usersHeader->setProperty("sortedBy", false);
    m_topicHeader->setProperty("sortedBy", false);

    QPushButton *active = nullptr;
    switch (column) {
    case ColName:   active = m_nameHeader;   break;
    case ColUsers:  active = m_usersHeader;  break;
    case ColTopic:  active = m_topicHeader;  break;
    default: break;
    }

    if (active) {
        active->setProperty("sortedBy", true);
        active->style()->unpolish(active);
        active->style()->polish(active);

        QString arrow = m_sortAscending ? QStringLiteral(" \xE2\x96\xB2") : QStringLiteral(" \xE2\x96\xBC");
        if (column == ColName) {
            active->setText(tr("Channel") + arrow);
        } else if (column == ColUsers) {
            active->setText(tr("Users") + arrow);
        } else {
            active->setText(tr("Topic") + arrow);
        }
    }

    applyFilter();
}

void IrcChannelListDialog::onFilterTextChanged(const QString &text)
{
    Q_UNUSED(text)
    applyFilter();
}

void IrcChannelListDialog::onRefreshClicked()
{
    fetchChannelList();
}

void IrcChannelListDialog::onJoinClicked()
{
    QListWidgetItem *current = m_channelList->currentItem();
    if (!current)
        return;

    bool ok = false;
    int idx = current->data(static_cast<int>(ChannelIndexRole)).toInt(&ok);
    if (!ok || idx < 0 || idx >= m_filteredChannels.size())
        return;

    QString channel = m_filteredChannels[idx].name;
    emit channelJoined(channel);
    accept();
}

void IrcChannelListDialog::onResultItemDoubleClicked(QListWidgetItem *item)
{
    if (!item)
        return;

    bool ok = false;
    int idx = item->data(static_cast<int>(ChannelIndexRole)).toInt(&ok);
    if (!ok || idx < 0 || idx >= m_filteredChannels.size())
        return;

    QString channel = m_filteredChannels[idx].name;
    emit channelJoined(channel);
    accept();
}

void IrcChannelListDialog::onJoinManualClicked()
{
    QString channel = m_manualChannelInput->text().trimmed();
    if (channel.isEmpty())
        return;

    if (!channel.startsWith(QLatin1Char('#')) && !channel.startsWith(QLatin1Char('&'))) {
        channel.prepend(QLatin1Char('#'));
    }

    emit channelJoined(channel);
    accept();
}

void IrcChannelListDialog::applyFilter()
{
    QString filter = m_filterBar->text().trimmed();

    m_filteredChannels.clear();
    for (const auto &ch : m_allChannels) {
        if (filter.isEmpty() ||
            ch.name.contains(filter, Qt::CaseInsensitive) ||
            ch.topic.contains(filter, Qt::CaseInsensitive)) {
            m_filteredChannels.append(ch);
        }
    }

    using Comparator = bool (*)(const IrcChannelInfo &, const IrcChannelInfo &);

    Comparator cmp = nullptr;
    switch (m_sortColumn) {
    case ColName:
        cmp = m_sortAscending ? compareNameAsc : compareNameDesc;
        break;
    case ColUsers:
        cmp = m_sortAscending ? compareUsersAsc : compareUsersDesc;
        break;
    case ColTopic: {
        static auto topicAsc = +[](const IrcChannelInfo &a, const IrcChannelInfo &b) -> bool {
            return a.topic.toLower() < b.topic.toLower();
        };
        static auto topicDesc = +[](const IrcChannelInfo &a, const IrcChannelInfo &b) -> bool {
            return a.topic.toLower() > b.topic.toLower();
        };
        cmp = m_sortAscending ? topicAsc : topicDesc;
        break;
    }
    default:
        break;
    }

    if (cmp) {
        std::sort(m_filteredChannels.begin(), m_filteredChannels.end(), cmp);
    }

    populateList();
}

void IrcChannelListDialog::populateList()
{
    m_channelList->clear();

    if (m_filteredChannels.isEmpty()) {
        m_channelList->setVisible(false);
        if (m_allChannels.isEmpty()) {
            m_emptyLabel->setText(tr("No channels loaded. Press Refresh to fetch the channel list."));
        } else {
            m_emptyLabel->setText(tr("No channels match your filter."));
        }
        m_emptyLabel->setVisible(true);
        m_joinButton->setEnabled(false);
        return;
    }

    m_emptyLabel->setVisible(false);
    m_channelList->setVisible(true);

    for (int i = 0; i < m_filteredChannels.size(); ++i) {
        const auto &ch = m_filteredChannels[i];

        auto *item = new QListWidgetItem();
        item->setData(static_cast<int>(ChannelIndexRole), i);

        auto *container = new QWidget();
        container->setStyleSheet(QStringLiteral("background: transparent;"));

        auto *row = new QHBoxLayout(container);
        row->setContentsMargins(8, 6, 8, 6);
        row->setSpacing(8);

        auto *nameLabel = new QLabel(ch.name, container);
        nameLabel->setStyleSheet(QStringLiteral(
            "color: #89b4fa; font-weight: bold; font-size: 13px; background: transparent;"));
        nameLabel->setMinimumWidth(100);
        row->addWidget(nameLabel, 3);

        auto *usersLabel = new QLabel(QString::number(ch.userCount), container);
        usersLabel->setStyleSheet(QStringLiteral(
            "color: #a6e3a1; font-size: 13px; font-weight: bold; background: transparent;"));
        usersLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        usersLabel->setFixedWidth(80);
        row->addWidget(usersLabel);

        QString topicText = ch.topic;
        if (topicText.length() > 80) {
            topicText = topicText.left(77) + QStringLiteral("...");
        }
        auto *topicLabel = new QLabel(topicText, container);
        topicLabel->setStyleSheet(QStringLiteral(
            "color: #9399b2; font-size: 12px; background: transparent;"));
        topicLabel->setWordWrap(false);
        row->addWidget(topicLabel, 5);

        item->setSizeHint(QSize(0, 36));
        m_channelList->addItem(item);
        m_channelList->setItemWidget(item, container);
    }

    m_joinButton->setEnabled(true);
}

void IrcChannelListDialog::showLoading(bool loading)
{
    m_loadingBar->setVisible(loading);
    m_refreshButton->setEnabled(!loading);
}

bool IrcChannelListDialog::compareNameAsc(const IrcChannelInfo &a, const IrcChannelInfo &b)
{
    return a.name.toLower() < b.name.toLower();
}

bool IrcChannelListDialog::compareNameDesc(const IrcChannelInfo &a, const IrcChannelInfo &b)
{
    return a.name.toLower() > b.name.toLower();
}

bool IrcChannelListDialog::compareUsersAsc(const IrcChannelInfo &a, const IrcChannelInfo &b)
{
    return a.userCount < b.userCount;
}

bool IrcChannelListDialog::compareUsersDesc(const IrcChannelInfo &a, const IrcChannelInfo &b)
{
    return a.userCount > b.userCount;
}
