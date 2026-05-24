#pragma once

#include <QDialog>

class QTabWidget;
class QComboBox;
class QCheckBox;
class QSlider;
class QSpinBox;
class QGroupBox;
class QLineEdit;
class QTextEdit;
class QTimeEdit;
class QDialogButtonBox;
class QLabel;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog() override;

signals:
    void settingsChanged();

private slots:
    void onAccepted();
    void onApplied();

private:
    void setupUi();
    void loadSettings();
    void saveSettings();

    QWidget *createGeneralTab();
    QWidget *createAppearanceTab();
    QWidget *createNetworkTab();
    QWidget *createNotificationsTab();
    QWidget *createAboutTab();

    QTabWidget *m_tabWidget;
    QDialogButtonBox *m_buttonBox;

    // General
    QComboBox *m_languageCombo;
    QCheckBox *m_startMinimizedCheck;
    QCheckBox *m_closeToTrayCheck;
    QCheckBox *m_autoConnectCheck;

    // Appearance
    QComboBox *m_themeCombo;
    QSlider *m_fontSizeSlider;
    QLabel *m_fontSizeLabel;
    QComboBox *m_bubbleStyleCombo;
    QCheckBox *m_showAvatarsCheck;
    QCheckBox *m_showTimestampsCheck;

    // Network
    QComboBox *m_proxyTypeCombo;
    QLineEdit *m_proxyHostEdit;
    QSpinBox *m_proxyPortSpin;
    QLineEdit *m_proxyUserEdit;
    QLineEdit *m_proxyPasswordEdit;
    QGroupBox *m_proxyGroup;
    QSpinBox *m_timeoutSpin;
    QCheckBox *m_yggdrasilCheck;
    QCheckBox *m_i2pCheck;

    // Notifications
    QCheckBox *m_enableNotificationsCheck;
    QCheckBox *m_showPreviewCheck;
    QCheckBox *m_soundCheck;
    QCheckBox *m_matrixNotifyCheck;
    QCheckBox *m_ircNotifyCheck;
    QCheckBox *m_lemmyNotifyCheck;
    QTextEdit *m_keywordsEdit;
    QTimeEdit *m_quietFromEdit;
    QTimeEdit *m_quietToEdit;
};
