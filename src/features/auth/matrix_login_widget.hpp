#pragma once

#include <QWidget>
#include <QVector>

class QLineEdit;
class QPushButton;
class QLabel;
class QStackedWidget;
class QProgressBar;
class QCheckBox;
class QNetworkAccessManager;
class LoadingSpinner;

struct MatrixServerInfo {
    QString baseUrl;
    QString serverName;
    QString serverVersion;
    bool registrationEnabled = false;
    bool valid = false;
};

struct MatrixSsoProvider {
    QString id;
    QString name;
    QByteArray iconSvg;
};

class MatrixLoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MatrixLoginWidget(QWidget *parent = nullptr);
    ~MatrixLoginWidget() override = default;

signals:
    void loginSuccess(const QString &userId,
                      const QString &accessToken,
                      const QString &homeserverUrl);
    void backRequested();

private slots:
    void onCheckServer();
    void onLoginClicked();
    void onRegisterClicked();
    void onToggleMode();
    void onSsoClicked(const QString &providerId);

private:
    void setupUi();
    void setLoading(bool loading);
    void setError(const QString &message);
    void clearError();
    void showStep(int step);
    void populateSsoButtons();
    QString resolveWellKnown(const QString &inputUrl) const;

    static QByteArray ssoIconSvg(const QString &provider);

    QStackedWidget *m_stack;

    // Step 1: Server selection
    QWidget *m_serverPage;
    QLineEdit *m_serverUrlInput;
    QPushButton *m_checkButton;
    QLabel *m_serverInfoLabel;
    QLabel *m_serverErrorLabel;

    // Step 2: Login / Register
    QWidget *m_authPage;
    QStackedWidget *m_modeStack;
    QPushButton *m_toggleButton;
    QLabel *m_modeTitle;
    QLabel *m_authErrorLabel;
    LoadingSpinner *m_spinner;

    // Login form
    QWidget *m_loginForm;
    QLineEdit *m_loginUsername;
    QLineEdit *m_loginPassword;

    // Register form
    QWidget *m_registerForm;
    QLineEdit *m_regUsername;
    QLineEdit *m_regPassword;
    QLineEdit *m_regEmail;
    QLineEdit *m_regCaptcha;

    // Shared
    QPushButton *m_submitButton;
    QLabel *m_forgotPasswordLink;
    QWidget *m_ssoContainer;

    QNetworkAccessManager *m_nam;
    MatrixServerInfo m_serverInfo;
    QVector<MatrixSsoProvider> m_ssoProviders;
    bool m_isLoginMode = true;

    QPushButton *m_backButton;
};
