#pragma once

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QSettings>
#include <QNetworkProxy>
#include <QGroupBox>

class ProxyConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProxyConfigDialog(QWidget *parent = nullptr);

    void loadFromSettings();
    void saveToSettings();

private slots:
    void onProxyTypeChanged(int index);
    void onTestConnection();
    void onApply();
    void onAccepted();

private:
    void setupUi();
    void updateProxyStatus();
    void updateOverrideGroupState();

    QComboBox *m_proxyTypeCombo;
    QLineEdit *m_hostEdit;
    QSpinBox *m_portSpin;
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QCheckBox *m_useForAllProtocols;

    QCheckBox *m_matrixOverride;
    QCheckBox *m_ircOverride;
    QCheckBox *m_lemmyOverride;

    QLabel *m_proxyStatusLabel;
    QPushButton *m_testButton;
    QPushButton *m_applyButton;

    QGroupBox *m_overrideGroup;
};
