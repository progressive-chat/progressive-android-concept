#include "proxy_config_dialog.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QTcpSocket>
#include <QNetworkProxy>
#include <QTimer>

ProxyConfigDialog::ProxyConfigDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Proxy Configuration"));
    setMinimumWidth(480);
    setupUi();
    loadFromSettings();
}

void ProxyConfigDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);

    auto *typeLayout = new QFormLayout;
    m_proxyTypeCombo = new QComboBox(this);
    m_proxyTypeCombo->addItem(tr("None"), QVariant::fromValue(QNetworkProxy::NoProxy));
    m_proxyTypeCombo->addItem(tr("SOCKS5"), QVariant::fromValue(QNetworkProxy::Socks5Proxy));
    m_proxyTypeCombo->addItem(tr("HTTP"), QVariant::fromValue(QNetworkProxy::HttpProxy));
    m_proxyTypeCombo->addItem(tr("SOCKS4"), QVariant::fromValue(static_cast<int>(QNetworkProxy::Socks5Proxy)));
    typeLayout->addRow(tr("Proxy Type:"), m_proxyTypeCombo);

    m_hostEdit = new QLineEdit(this);
    m_hostEdit->setPlaceholderText(tr("proxy.example.com"));
    typeLayout->addRow(tr("Host:"), m_hostEdit);

    m_portSpin = new QSpinBox(this);
    m_portSpin->setRange(1, 65535);
    m_portSpin->setValue(1080);
    typeLayout->addRow(tr("Port:"), m_portSpin);

    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText(tr("username (optional)"));
    typeLayout->addRow(tr("Username:"), m_usernameEdit);

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(tr("password (optional)"));
    typeLayout->addRow(tr("Password:"), m_passwordEdit);

    mainLayout->addLayout(typeLayout);

    m_useForAllProtocols = new QCheckBox(tr("Use proxy for all protocols"), this);
    m_useForAllProtocols->setChecked(true);
    mainLayout->addWidget(m_useForAllProtocols);

    m_overrideGroup = new QGroupBox(tr("Per-Protocol Proxy Overrides"), this);
    auto *overrideLayout = new QVBoxLayout(m_overrideGroup);

    m_matrixOverride = new QCheckBox(tr("Matrix (use global proxy)"), this);
    m_matrixOverride->setChecked(true);
    overrideLayout->addWidget(m_matrixOverride);

    m_ircOverride = new QCheckBox(tr("IRC (use global proxy)"), this);
    m_ircOverride->setChecked(true);
    overrideLayout->addWidget(m_ircOverride);

    m_lemmyOverride = new QCheckBox(tr("Lemmy (use global proxy)"), this);
    m_lemmyOverride->setChecked(true);
    overrideLayout->addWidget(m_lemmyOverride);

    mainLayout->addWidget(m_overrideGroup);

    m_proxyStatusLabel = new QLabel(tr("Proxy Status: Inactive"), this);
    mainLayout->addWidget(m_proxyStatusLabel);

    auto *buttonRow = new QHBoxLayout;
    m_testButton = new QPushButton(tr("Test Connection"), this);
    buttonRow->addWidget(m_testButton);
    buttonRow->addStretch();

    m_applyButton = new QPushButton(tr("Apply"), this);
    buttonRow->addWidget(m_applyButton);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttonRow->addWidget(buttonBox);

    mainLayout->addLayout(buttonRow);

    connect(m_proxyTypeCombo, &QComboBox::currentIndexChanged, this, &ProxyConfigDialog::onProxyTypeChanged);
    connect(m_testButton, &QPushButton::clicked, this, &ProxyConfigDialog::onTestConnection);
    connect(m_applyButton, &QPushButton::clicked, this, &ProxyConfigDialog::onApply);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ProxyConfigDialog::onAccepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_useForAllProtocols, &QCheckBox::toggled, this, &ProxyConfigDialog::updateOverrideGroupState);

    onProxyTypeChanged(0);
}

void ProxyConfigDialog::onProxyTypeChanged(int index)
{
    QNetworkProxy::ProxyType type = m_proxyTypeCombo->currentData().value<QNetworkProxy::ProxyType>();
    bool enabled = (type != QNetworkProxy::NoProxy);

    m_hostEdit->setEnabled(enabled);
    m_portSpin->setEnabled(enabled);
    m_usernameEdit->setEnabled(enabled);
    m_passwordEdit->setEnabled(enabled);
    m_testButton->setEnabled(enabled);
    m_useForAllProtocols->setEnabled(enabled);
    updateOverrideGroupState();
    updateProxyStatus();
}

void ProxyConfigDialog::updateOverrideGroupState()
{
    bool globalEnabled = m_proxyTypeCombo->currentData().value<QNetworkProxy::ProxyType>() != QNetworkProxy::NoProxy;
    bool useAll = m_useForAllProtocols->isChecked();
    m_overrideGroup->setEnabled(globalEnabled && !useAll);

    if (useAll) {
        m_matrixOverride->setChecked(true);
        m_ircOverride->setChecked(true);
        m_lemmyOverride->setChecked(true);
    }
}

void ProxyConfigDialog::updateProxyStatus()
{
    QNetworkProxy::ProxyType type = m_proxyTypeCombo->currentData().value<QNetworkProxy::ProxyType>();
    if (type == QNetworkProxy::NoProxy) {
        m_proxyStatusLabel->setText(tr("Proxy Status: Inactive"));
    } else {
        m_proxyStatusLabel->setText(tr("Proxy Status: Active (%1:%2)")
                                        .arg(m_hostEdit->text(), QString::number(m_portSpin->value())));
    }
}

void ProxyConfigDialog::onTestConnection()
{
    m_testButton->setEnabled(false);
    m_testButton->setText(tr("Testing..."));

    QNetworkProxy::ProxyType type = m_proxyTypeCombo->currentData().value<QNetworkProxy::ProxyType>();
    QNetworkProxy proxy;
    proxy.setType(type);
    proxy.setHostName(m_hostEdit->text());
    proxy.setPort(static_cast<quint16>(m_portSpin->value()));

    if (!m_usernameEdit->text().isEmpty()) {
        proxy.setUser(m_usernameEdit->text());
        proxy.setPassword(m_passwordEdit->text());
    }

    auto *socket = new QTcpSocket(this);
    socket->setProxy(proxy);

    connect(socket, &QTcpSocket::connected, this, [this, socket]() {
        m_proxyStatusLabel->setText(tr("Proxy Status: Active (connection OK)"));
        m_testButton->setText(tr("Test Connection"));
        m_testButton->setEnabled(true);
        socket->deleteLater();
    });

    connect(socket, &QTcpSocket::errorOccurred, this, [this, socket](QAbstractSocket::SocketError error) {
        Q_UNUSED(error)
        m_proxyStatusLabel->setText(
            tr("Proxy Status: Active (%1:%2) - Test failed: %3")
                .arg(m_hostEdit->text(),
                     QString::number(m_portSpin->value()),
                     socket->errorString()));
        m_testButton->setText(tr("Test Connection"));
        m_testButton->setEnabled(true);
        socket->deleteLater();
    });

    socket->connectToHost(m_hostEdit->text(), static_cast<quint16>(m_portSpin->value()));
}

void ProxyConfigDialog::onApply()
{
    saveToSettings();

    QNetworkProxy::ProxyType type = m_proxyTypeCombo->currentData().value<QNetworkProxy::ProxyType>();

    QNetworkProxy proxy;
    proxy.setType(type);

    if (type != QNetworkProxy::NoProxy) {
        proxy.setHostName(m_hostEdit->text());
        proxy.setPort(static_cast<quint16>(m_portSpin->value()));
        if (!m_usernameEdit->text().isEmpty()) {
            proxy.setUser(m_usernameEdit->text());
            proxy.setPassword(m_passwordEdit->text());
        }
    }

    QNetworkProxy::setApplicationProxy(proxy);
}

void ProxyConfigDialog::onAccepted()
{
    onApply();
    accept();
}

void ProxyConfigDialog::loadFromSettings()
{
    QSettings settings;

    settings.beginGroup("Proxy");
    int typeIdx = settings.value("type", 0).toInt();
    if (typeIdx >= 0 && typeIdx < m_proxyTypeCombo->count())
        m_proxyTypeCombo->setCurrentIndex(typeIdx);

    m_hostEdit->setText(settings.value("host").toString());
    m_portSpin->setValue(settings.value("port", 1080).toInt());
    m_usernameEdit->setText(settings.value("username").toString());
    m_passwordEdit->setText(settings.value("password").toString());
    m_useForAllProtocols->setChecked(settings.value("useForAll", true).toBool());
    m_matrixOverride->setChecked(settings.value("matrixOverride", true).toBool());
    m_ircOverride->setChecked(settings.value("ircOverride", true).toBool());
    m_lemmyOverride->setChecked(settings.value("lemmyOverride", true).toBool());
    settings.endGroup();

    onProxyTypeChanged(m_proxyTypeCombo->currentIndex());
}

void ProxyConfigDialog::saveToSettings()
{
    QSettings settings;

    settings.beginGroup("Proxy");
    settings.setValue("type", m_proxyTypeCombo->currentIndex());
    settings.setValue("host", m_hostEdit->text());
    settings.setValue("port", m_portSpin->value());
    settings.setValue("username", m_usernameEdit->text());
    settings.setValue("password", m_passwordEdit->text());
    settings.setValue("useForAll", m_useForAllProtocols->isChecked());
    settings.setValue("matrixOverride", m_matrixOverride->isChecked());
    settings.setValue("ircOverride", m_ircOverride->isChecked());
    settings.setValue("lemmyOverride", m_lemmyOverride->isChecked());
    settings.endGroup();
}
