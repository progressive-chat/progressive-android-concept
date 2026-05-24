#include "features/auth/matrix_login_widget.hpp"

#include <QCheckBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProgressBar>
#include <QPushButton>
#include <QStackedWidget>
#include <QUrl>
#include <QVBoxLayout>

#include "ui/components/loading_spinner.hpp"
#include "util/json_util.hpp"

MatrixLoginWidget::MatrixLoginWidget(QWidget *parent)
    : QWidget(parent)
    , m_nam(new QNetworkAccessManager(this))
{
    setupUi();
}

void MatrixLoginWidget::setupUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(24, 24, 24, 24);
    root->setSpacing(16);

    auto *headerLayout = new QHBoxLayout;
    m_backButton = new QPushButton(tr("< Back"), this);
    m_backButton->setCursor(Qt::PointingHandCursor);
    m_backButton->setStyleSheet(QStringLiteral(
        "QPushButton { border: none; color: #1976D2; font-size: 14px; padding: 4px 8px; }"
        "QPushButton:hover { color: #1565C0; }"));
    headerLayout->addWidget(m_backButton);
    headerLayout->addStretch();
    root->addLayout(headerLayout);

    auto *title = new QLabel(tr("Sign in to Matrix"), this);
    QFont titleFont;
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignCenter);
    root->addWidget(title);

    auto *subtitle = new QLabel(tr("Connect to your Matrix homeserver"), this);
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet(QStringLiteral("color: #666; font-size: 14px;"));
    root->addWidget(subtitle);

    root->addSpacing(8);

    m_stack = new QStackedWidget(this);

    // ── Step 1: Server page ──
    m_serverPage = new QWidget(this);
    auto *serverLayout = new QVBoxLayout(m_serverPage);
    serverLayout->setContentsMargins(0, 0, 0, 0);
    serverLayout->setSpacing(12);

    auto *serverForm = new QFormLayout;
    serverForm->setSpacing(8);
    m_serverUrlInput = new QLineEdit(QStringLiteral("https://matrix.org"), this);
    m_serverUrlInput->setPlaceholderText(QStringLiteral("https://matrix.org"));
    m_serverUrlInput->setMinimumHeight(40);
    serverForm->addRow(tr("Homeserver URL:"), m_serverUrlInput);
    serverLayout->addLayout(serverForm);

    m_checkButton = new QPushButton(tr("Check Server"), this);
    m_checkButton->setCursor(Qt::PointingHandCursor);
    m_checkButton->setMinimumHeight(40);
    m_checkButton->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: #1976D2; color: white; border: none; border-radius: 6px;"
        " font-size: 14px; font-weight: bold; padding: 8px 24px; }"
        "QPushButton:hover { background-color: #1565C0; }"
        "QPushButton:disabled { background-color: #B0BEC5; }"));
    serverLayout->addWidget(m_checkButton, 0, Qt::AlignHCenter);

    auto *serverProgress = new QProgressBar(this);
    serverProgress->setRange(0, 0);
    serverProgress->setMaximumHeight(4);
    serverProgress->setTextVisible(false);
    serverProgress->setVisible(false);
    serverProgress->setObjectName(QStringLiteral("serverProgress"));
    serverLayout->addWidget(serverProgress);

    m_serverInfoLabel = new QLabel(this);
    m_serverInfoLabel->setWordWrap(true);
    m_serverInfoLabel->setAlignment(Qt::AlignCenter);
    m_serverInfoLabel->setVisible(false);
    m_serverInfoLabel->setStyleSheet(QStringLiteral(
        "background-color: #E8F5E9; border: 1px solid #A5D6A7; border-radius: 6px;"
        " padding: 12px; font-size: 13px;"));
    serverLayout->addWidget(m_serverInfoLabel);

    m_serverErrorLabel = new QLabel(this);
    m_serverErrorLabel->setWordWrap(true);
    m_serverErrorLabel->setAlignment(Qt::AlignCenter);
    m_serverErrorLabel->setVisible(false);
    m_serverErrorLabel->setStyleSheet(QStringLiteral(
        "color: #D32F2F; background-color: #FFEBEE; border: 1px solid #EF9A9A;"
        " border-radius: 6px; padding: 8px; font-size: 13px;"));
    serverLayout->addWidget(m_serverErrorLabel);

    auto *continueButton = new QPushButton(tr("Continue"), this);
    continueButton->setCursor(Qt::PointingHandCursor);
    continueButton->setMinimumHeight(40);
    continueButton->setVisible(false);
    continueButton->setObjectName(QStringLiteral("continueButton"));
    continueButton->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: #2E7D32; color: white; border: none; border-radius: 6px;"
        " font-size: 14px; font-weight: bold; padding: 8px 24px; }"
        "QPushButton:hover { background-color: #1B5E20; }"));
    serverLayout->addWidget(continueButton, 0, Qt::AlignHCenter);
    serverLayout->addStretch();

    m_stack->addWidget(m_serverPage);

    // ── Step 2: Auth page ──
    m_authPage = new QWidget(this);
    auto *authLayout = new QVBoxLayout(m_authPage);
    authLayout->setContentsMargins(0, 0, 0, 0);
    authLayout->setSpacing(12);

    m_modeTitle = new QLabel(tr("Login"), this);
    QFont modeFont;
    modeFont.setPointSize(16);
    modeFont.setBold(true);
    m_modeTitle->setFont(modeFont);
    m_modeTitle->setAlignment(Qt::AlignCenter);
    authLayout->addWidget(m_modeTitle);

    m_modeStack = new QStackedWidget(this);

    // Login form
    m_loginForm = new QWidget(this);
    auto *loginLayout = new QVBoxLayout(m_loginForm);
    loginLayout->setContentsMargins(0, 0, 0, 0);
    loginLayout->setSpacing(10);

    auto *loginFormLayout = new QFormLayout;
    loginFormLayout->setSpacing(8);
    m_loginUsername = new QLineEdit(this);
    m_loginUsername->setPlaceholderText(tr("Username or Matrix ID"));
    m_loginUsername->setMinimumHeight(40);
    m_loginPassword = new QLineEdit(this);
    m_loginPassword->setEchoMode(QLineEdit::Password);
    m_loginPassword->setPlaceholderText(tr("Password"));
    m_loginPassword->setMinimumHeight(40);
    loginFormLayout->addRow(tr("Username:"), m_loginUsername);
    loginFormLayout->addRow(tr("Password:"), m_loginPassword);
    loginLayout->addLayout(loginFormLayout);

    m_forgotPasswordLink = new QLabel(this);
    m_forgotPasswordLink->setText(QStringLiteral(
        "<a href=\"#\" style=\"color: #1976D2; text-decoration: none;\">") +
        tr("Forgot password?") + QStringLiteral("</a>"));
    m_forgotPasswordLink->setAlignment(Qt::AlignRight);
    m_forgotPasswordLink->setCursor(Qt::PointingHandCursor);
    m_forgotPasswordLink->setTextFormat(Qt::RichText);
    loginLayout->addWidget(m_forgotPasswordLink);

    m_modeStack->addWidget(m_loginForm);

    // Register form
    m_registerForm = new QWidget(this);
    auto *regLayout = new QVBoxLayout(m_registerForm);
    regLayout->setContentsMargins(0, 0, 0, 0);
    regLayout->setSpacing(10);

    auto *regFormLayout = new QFormLayout;
    regFormLayout->setSpacing(8);
    m_regUsername = new QLineEdit(this);
    m_regUsername->setPlaceholderText(tr("Choose a username"));
    m_regUsername->setMinimumHeight(40);
    m_regPassword = new QLineEdit(this);
    m_regPassword->setEchoMode(QLineEdit::Password);
    m_regPassword->setPlaceholderText(tr("Choose a password"));
    m_regPassword->setMinimumHeight(40);
    m_regEmail = new QLineEdit(this);
    m_regEmail->setPlaceholderText(tr("Email address (optional)"));
    m_regEmail->setMinimumHeight(40);
    m_regCaptcha = new QLineEdit(this);
    m_regCaptcha->setPlaceholderText(tr("Captcha token (if required)"));
    m_regCaptcha->setMinimumHeight(40);
    regFormLayout->addRow(tr("Username:"), m_regUsername);
    regFormLayout->addRow(tr("Password:"), m_regPassword);
    regFormLayout->addRow(tr("Email:"), m_regEmail);
    regFormLayout->addRow(tr("Captcha:"), m_regCaptcha);
    regLayout->addLayout(regFormLayout);

    m_modeStack->addWidget(m_registerForm);

    authLayout->addWidget(m_modeStack);

    m_authErrorLabel = new QLabel(this);
    m_authErrorLabel->setWordWrap(true);
    m_authErrorLabel->setAlignment(Qt::AlignCenter);
    m_authErrorLabel->setVisible(false);
    m_authErrorLabel->setStyleSheet(QStringLiteral(
        "color: #D32F2F; background-color: #FFEBEE; border: 1px solid #EF9A9A;"
        " border-radius: 6px; padding: 8px; font-size: 13px;"));
    authLayout->addWidget(m_authErrorLabel);

    m_spinner = new LoadingSpinner(28, QColor(25, 118, 210), this);
    m_spinner->setVisible(false);
    authLayout->addWidget(m_spinner, 0, Qt::AlignCenter);

    m_submitButton = new QPushButton(tr("Login"), this);
    m_submitButton->setCursor(Qt::PointingHandCursor);
    m_submitButton->setMinimumHeight(44);
    m_submitButton->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: #1976D2; color: white; border: none; border-radius: 6px;"
        " font-size: 14px; font-weight: bold; padding: 8px 24px; }"
        "QPushButton:hover { background-color: #1565C0; }"
        "QPushButton:disabled { background-color: #B0BEC5; }"));
    authLayout->addWidget(m_submitButton);

    m_toggleButton = new QPushButton(tr("Create an account"), this);
    m_toggleButton->setCursor(Qt::PointingHandCursor);
    m_toggleButton->setStyleSheet(QStringLiteral(
        "QPushButton { border: none; color: #1976D2; font-size: 13px; }"
        "QPushButton:hover { color: #1565C0; text-decoration: underline; }"));
    authLayout->addWidget(m_toggleButton, 0, Qt::AlignCenter);

    // SSO section
    m_ssoContainer = new QWidget(this);
    auto *ssoLayout = new QVBoxLayout(m_ssoContainer);
    ssoLayout->setContentsMargins(0, 8, 0, 0);
    ssoLayout->setSpacing(8);

    auto *ssoLabel = new QLabel(tr("Or sign in with:"), this);
    ssoLabel->setAlignment(Qt::AlignCenter);
    ssoLabel->setStyleSheet(QStringLiteral("color: #999; font-size: 13px;"));
    ssoLayout->addWidget(ssoLabel);

    auto *ssoButtonRow = new QHBoxLayout;
    ssoButtonRow->setSpacing(12);
    ssoButtonRow->setAlignment(Qt::AlignCenter);
    ssoButtonRow->setObjectName(QStringLiteral("ssoButtonRow"));
    ssoLayout->addLayout(ssoButtonRow);

    m_ssoContainer->setVisible(false);
    authLayout->addWidget(m_ssoContainer);

    authLayout->addStretch();

    m_stack->addWidget(m_authPage);
    root->addWidget(m_stack);

    // Connections
    connect(m_backButton, &QPushButton::clicked, this, [this]() {
        if (m_stack->currentIndex() == 1) {
            m_stack->setCurrentIndex(0);
        } else {
            emit backRequested();
        }
    });

    connect(m_checkButton, &QPushButton::clicked, this, &MatrixLoginWidget::onCheckServer);
    connect(continueButton, &QPushButton::clicked, this, [this]() {
        showStep(1);
    });
    connect(m_submitButton, &QPushButton::clicked, this, [this]() {
        if (m_isLoginMode)
            onLoginClicked();
        else
            onRegisterClicked();
    });
    connect(m_toggleButton, &QPushButton::clicked, this, &MatrixLoginWidget::onToggleMode);

    connect(m_serverUrlInput, &QLineEdit::returnPressed, this, &MatrixLoginWidget::onCheckServer);
    connect(m_loginPassword, &QLineEdit::returnPressed, this, &MatrixLoginWidget::onLoginClicked);
}

void MatrixLoginWidget::showStep(int step)
{
    m_stack->setCurrentIndex(step);
    if (step == 1) {
        m_backButton->setText(tr("< Change Server"));
    } else {
        m_backButton->setText(tr("< Back"));
    }
    clearError();
}

QString MatrixLoginWidget::resolveWellKnown(const QString &inputUrl) const
{
    QString url = inputUrl.trimmed();
    if (!url.startsWith(QStringLiteral("http://")) && !url.startsWith(QStringLiteral("https://")))
        url = QStringLiteral("https://") + url;
    while (url.endsWith(QLatin1Char('/')))
        url.chop(1);
    return url;
}

void MatrixLoginWidget::onCheckServer()
{
    const QString raw = m_serverUrlInput->text();
    if (raw.trimmed().isEmpty()) {
        setError(raw);
        auto *err = m_serverPage->findChild<QLabel *>(QStringLiteral("serverErrorLabel"));
        if (auto *lbl = qobject_cast<QLabel *>(m_serverErrorLabel)) {
            lbl->setText(tr("Please enter a homeserver URL."));
            lbl->setVisible(true);
        }
        return;
    }

    const QString baseUrl = resolveWellKnown(raw);
    m_serverInfo = MatrixServerInfo{};
    m_serverInfo.baseUrl = baseUrl;

    auto *progress = m_serverPage->findChild<QProgressBar *>(QStringLiteral("serverProgress"));
    if (progress)
        progress->setVisible(true);
    m_checkButton->setEnabled(false);
    m_serverInfoLabel->setVisible(false);
    m_serverErrorLabel->setVisible(false);

    QUrl wellKnownUrl(baseUrl + QStringLiteral("/.well-known/matrix/client"));
    QNetworkRequest req(wellKnownUrl);
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    QNetworkReply *reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, baseUrl, progress]() {
        reply->deleteLater();
        if (progress)
            progress->setVisible(false);
        m_checkButton->setEnabled(true);

        if (reply->error() == QNetworkReply::NoError) {
            QJsonObject json = JsonUtil::parseObject(QString::fromUtf8(reply->readAll()));

            QJsonObject mHomeserver = JsonUtil::optObject(json, QStringLiteral("m.homeserver"));
            QString resolvedUrl = JsonUtil::optString(mHomeserver, QStringLiteral("base_url"), baseUrl);
            m_serverInfo.baseUrl = resolvedUrl;
            m_serverInfo.valid = true;

            m_serverInfoLabel->setText(
                tr("Server verified successfully.\nURL: %1").arg(resolvedUrl));
            m_serverInfoLabel->setVisible(true);

            auto *continueBtn = m_serverPage->findChild<QPushButton *>(QStringLiteral("continueButton"));
            if (continueBtn)
                continueBtn->setVisible(true);
        } else {
            m_serverInfo.valid = true;
            m_serverInfo.baseUrl = baseUrl;

            m_serverInfoLabel->setText(
                tr("No .well-known found. Using %1 directly.").arg(baseUrl));
            m_serverInfoLabel->setStyleSheet(QStringLiteral(
                "background-color: #FFF8E1; border: 1px solid #FFE082; border-radius: 6px;"
                " padding: 12px; font-size: 13px;"));
            m_serverInfoLabel->setVisible(true);

            auto *continueBtn = m_serverPage->findChild<QPushButton *>(QStringLiteral("continueButton"));
            if (continueBtn)
                continueBtn->setVisible(true);
        }

        // Fetch server version / registration info
        QUrl versionsUrl(baseUrl + QStringLiteral("/_matrix/client/versions"));
        QNetworkRequest vReq(versionsUrl);
        vReq.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
        QNetworkReply *vReply = m_nam->get(vReq);
        connect(vReply, &QNetworkReply::finished, this, [this, vReply]() {
            vReply->deleteLater();
            if (vReply->error() == QNetworkReply::NoError) {
                QJsonObject vJson = JsonUtil::parseObject(QString::fromUtf8(vReply->readAll()));
                QJsonArray versions = JsonUtil::optArray(vJson, QStringLiteral("versions"));
                if (!versions.isEmpty())
                    m_serverInfo.serverVersion = versions.last().toString();
            }
        });

        // Fetch login flows to detect SSO and registration
        QUrl loginFlowsUrl(baseUrl + QStringLiteral("/_matrix/client/r0/login"));
        QNetworkRequest lfReq(loginFlowsUrl);
        lfReq.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
        QNetworkReply *lfReply = m_nam->get(lfReq);
        connect(lfReply, &QNetworkReply::finished, this, [this, lfReply]() {
            lfReply->deleteLater();
            if (lfReply->error() == QNetworkReply::NoError) {
                QJsonObject lfJson = JsonUtil::parseObject(QString::fromUtf8(lfReply->readAll()));
                QJsonArray flows = JsonUtil::optArray(lfJson, QStringLiteral("flows"));

                m_ssoProviders.clear();
                for (const auto &flowVal : flows) {
                    QJsonObject flow = flowVal.toObject();
                    QString type = JsonUtil::optString(flow, QStringLiteral("type"));
                    if (type == QStringLiteral("m.login.sso") || type == QStringLiteral("m.login.cas")) {
                        QJsonArray idps = JsonUtil::optArray(flow, QStringLiteral("identity_providers"));
                        for (const auto &idpVal : idps) {
                            QJsonObject idp = idpVal.toObject();
                            MatrixSsoProvider prov;
                            prov.id = JsonUtil::optString(idp, QStringLiteral("id"));
                            prov.name = JsonUtil::optString(idp, QStringLiteral("name"));
                            m_ssoProviders.append(prov);
                        }
                        if (idps.isEmpty()) {
                            MatrixSsoProvider prov;
                            prov.id = QStringLiteral("sso");
                            prov.name = QStringLiteral("SSO");
                            m_ssoProviders.append(prov);
                        }
                    }
                }
                populateSsoButtons();
            }
        });
    });
}

void MatrixLoginWidget::onLoginClicked()
{
    const QString user = m_loginUsername->text().trimmed();
    const QString pass = m_loginPassword->text();

    if (user.isEmpty() || pass.isEmpty()) {
        setError(tr("Please enter both username and password."));
        return;
    }

    if (!user.contains(QLatin1Char(':')))
        return; // Matrix requires full MXID

    clearError();
    setLoading(true);

    QJsonObject body;
    body[QStringLiteral("type")] = QStringLiteral("m.login.password");
    body[QStringLiteral("user")] = user;
    body[QStringLiteral("password")] = pass;

    QUrl loginUrl(m_serverInfo.baseUrl + QStringLiteral("/_matrix/client/r0/login"));
    QNetworkRequest req(loginUrl);
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    QNetworkReply *reply = m_nam->post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        setLoading(false);

        if (reply->error() != QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QString detail;
            if (!responseData.isEmpty()) {
                QJsonObject errJson = JsonUtil::parseObject(QString::fromUtf8(responseData));
                detail = JsonUtil::optString(errJson, QStringLiteral("error"),
                                              reply->errorString());
            } else {
                detail = reply->errorString();
            }
            setError(tr("Login failed: %1").arg(detail));
            return;
        }

        QJsonObject resp = JsonUtil::parseObject(QString::fromUtf8(reply->readAll()));
        QString userId = JsonUtil::optString(resp, QStringLiteral("user_id"));
        QString accessToken = JsonUtil::optString(resp, QStringLiteral("access_token"));
        QString homeserver = JsonUtil::optString(resp, QStringLiteral("home_server"), m_serverInfo.baseUrl);

        if (userId.isEmpty() || accessToken.isEmpty()) {
            setError(tr("Login response missing credentials."));
            return;
        }

        emit loginSuccess(userId, accessToken, homeserver);
    });
}

void MatrixLoginWidget::onRegisterClicked()
{
    const QString user = m_regUsername->text().trimmed();
    const QString pass = m_regPassword->text();
    const QString email = m_regEmail->text().trimmed();
    const QString captcha = m_regCaptcha->text().trimmed();

    if (user.isEmpty() || pass.isEmpty()) {
        setError(tr("Please enter a username and password."));
        return;
    }

    clearError();
    setLoading(true);

    QString fullUser = user;
    if (!fullUser.startsWith(QLatin1Char('@'))) {
        QUrl url(m_serverInfo.baseUrl);
        fullUser = QStringLiteral("@%1:%2").arg(user, url.host());
    }

    QJsonObject auth;
    auth[QStringLiteral("type")] = QStringLiteral("m.login.dummy");

    QJsonObject body;
    body[QStringLiteral("username")] = user;
    body[QStringLiteral("password")] = pass;
    body[QStringLiteral("auth")] = auth;

    if (!email.isEmpty())
        body[QStringLiteral("email")] = email;
    if (!captcha.isEmpty()) {
        QJsonObject captchaObj;
        captchaObj[QStringLiteral("type")] = QStringLiteral("m.login.recaptcha");
        captchaObj[QStringLiteral("response")] = captcha;
        body[QStringLiteral("auth")] = captchaObj;
    }

    QUrl regUrl(m_serverInfo.baseUrl + QStringLiteral("/_matrix/client/r0/register?kind=user"));
    QNetworkRequest req(regUrl);
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    QNetworkReply *reply = m_nam->post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        setLoading(false);

        QByteArray responseData = reply->readAll();
        QJsonObject resp = JsonUtil::parseObject(QString::fromUtf8(responseData));

        if (reply->error() != QNetworkReply::NoError) {
            QString detail = JsonUtil::optString(resp, QStringLiteral("error"), reply->errorString());

            // Check for registration flows (captcha, terms, etc.)
            QJsonArray flows = JsonUtil::optArray(resp, QStringLiteral("flows"));
            QString completed = JsonUtil::optString(resp, QStringLiteral("completed"), QString());
            if (!flows.isEmpty() && completed != QStringLiteral("m.login.dummy")) {
                setError(tr("Registration requires additional steps: %1").arg(detail));
            } else {
                setError(tr("Registration failed: %1").arg(detail));
            }
            return;
        }

        QString userId = JsonUtil::optString(resp, QStringLiteral("user_id"));
        QString accessToken = JsonUtil::optString(resp, QStringLiteral("access_token"));
        QString homeserver = JsonUtil::optString(resp, QStringLiteral("home_server"), m_serverInfo.baseUrl);

        if (userId.isEmpty() || accessToken.isEmpty()) {
            // Check for registration token flow
            QString session = JsonUtil::optString(resp, QStringLiteral("session"));
            if (!session.isEmpty()) {
                setError(tr("Registration requires additional verification: %1").arg(session));
            } else {
                setError(tr("Registration response missing credentials."));
            }
            return;
        }

        emit loginSuccess(userId, accessToken, homeserver);
    });
}

void MatrixLoginWidget::onToggleMode()
{
    m_isLoginMode = !m_isLoginMode;
    if (m_isLoginMode) {
        m_modeStack->setCurrentIndex(0);
        m_modeTitle->setText(tr("Login"));
        m_submitButton->setText(tr("Login"));
        m_toggleButton->setText(tr("Create an account"));
        m_forgotPasswordLink->setVisible(true);
    } else {
        m_modeStack->setCurrentIndex(1);
        m_modeTitle->setText(tr("Create Account"));
        m_submitButton->setText(tr("Register"));
        m_toggleButton->setText(tr("Already have an account? Sign in"));
        m_forgotPasswordLink->setVisible(false);
    }
    clearError();
}

void MatrixLoginWidget::onSsoClicked(const QString &providerId)
{
    Q_UNUSED(providerId)
}

void MatrixLoginWidget::populateSsoButtons()
{
    auto *row = m_ssoContainer->findChild<QHBoxLayout *>(QStringLiteral("ssoButtonRow"));
    if (!row)
        return;

    while (row->count() > 0) {
        QLayoutItem *item = row->takeAt(0);
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }

    static const QStringList knownProviders = {
        QStringLiteral("github"), QStringLiteral("google"), QStringLiteral("gitlab"),
        QStringLiteral("apple"), QStringLiteral("facebook")
    };

    for (const auto &prov : m_ssoProviders) {
        Q_UNUSED(prov)
    }

    bool anyVisible = false;
    for (const auto &known : knownProviders) {
        bool found = false;
        QString pid;
        for (const auto &prov : m_ssoProviders) {
            if (prov.id.contains(known, Qt::CaseInsensitive) ||
                prov.name.contains(known, Qt::CaseInsensitive)) {
                found = true;
                pid = prov.id;
                break;
            }
        }

        auto *btn = new QPushButton(this);
        btn->setText(known.at(0).toUpper() + known.mid(1));
        btn->setMinimumSize(80, 36);
        btn->setEnabled(found);
        btn->setCursor(found ? Qt::PointingHandCursor : Qt::ForbiddenCursor);
        btn->setToolTip(found ? tr("Sign in with %1").arg(known)
                              : tr("%1 not available on this server").arg(known));

        if (found) {
            btn->setStyleSheet(QStringLiteral(
                "QPushButton { background-color: #ECEFF1; border: 1px solid #CFD8DC; border-radius: 6px;"
                " font-size: 13px; padding: 6px 14px; }"
                "QPushButton:hover { background-color: #CFD8DC; }"));
            connect(btn, &QPushButton::clicked, this, [this, pid]() {
                onSsoClicked(pid);
            });
        } else {
            btn->setStyleSheet(QStringLiteral(
                "QPushButton { background-color: #F5F5F5; border: 1px solid #E0E0E0; border-radius: 6px;"
                " color: #BDBDBD; font-size: 13px; padding: 6px 14px; }"));
        }
        row->addWidget(btn);
        anyVisible = true;
    }

    m_ssoContainer->setVisible(anyVisible);
}

void MatrixLoginWidget::setLoading(bool loading)
{
    if (m_spinner) {
        m_spinner->setVisible(loading);
        if (loading)
            m_spinner->start();
        else
            m_spinner->stop();
    }
    m_submitButton->setEnabled(!loading);
    m_toggleButton->setEnabled(!loading);
}

void MatrixLoginWidget::setError(const QString &message)
{
    m_authErrorLabel->setText(message);
    m_authErrorLabel->setVisible(true);
}

void MatrixLoginWidget::clearError()
{
    m_authErrorLabel->setVisible(false);
    m_authErrorLabel->clear();
}

QByteArray MatrixLoginWidget::ssoIconSvg(const QString &provider)
{
    // Minimal placeholder SVG icons
    if (provider == QStringLiteral("github")) {
        return QByteArrayLiteral(
            "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' fill='#24292e'>"
            "<path d='M12 0C5.37 0 0 5.37 0 12c0 5.31 3.435 9.795 8.205 11.385.6.105.825-.255.825-.57"
            " 0-.285-.015-1.23-.015-2.235-3.015.555-3.795-.735-4.035-1.41-.135-.345-.72-1.41-1.23-1.695"
            "-.42-.225-1.02-.78-.015-.795.945-.015 1.62.87 1.845 1.23 1.08 1.815 2.805 1.305 3.495.99"
            ".105-.78.42-1.305.765-1.605-2.67-.3-5.46-1.335-5.46-5.925 0-1.305.465-2.385 1.23-3.225"
            "-.12-.3-.54-1.53.12-3.18 0 0 1.005-.315 3.3 1.23.96-.27 1.98-.405 3-.405s2.04.135 3 .405"
            "c2.295-1.56 3.3-1.23 3.3-1.23.66 1.65.24 2.88.12 3.18.765.84 1.23 1.905 1.23 3.225 0 4.605"
            "-2.805 5.625-5.475 5.925.435.375.81 1.095.81 2.22 0 1.605-.015 2.895-.015 3.3 0 .315.225.69"
            ".825.57A12.02 12.02 0 0024 12c0-6.63-5.37-12-12-12z'/></svg>");
    }
    return {};
}
