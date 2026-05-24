#pragma once

#include <QWidget>
#include <QString>
#include <QVector>
#include <array>

class QLabel;
class QPushButton;
class QProgressBar;
class QStackedWidget;
class QTimer;
class QFrame;
class QHBoxLayout;
class QVBoxLayout;

struct EmojiEntry
{
    const char *codepoint;
    const char *name;
};

class DeviceVerificationWidget : public QWidget
{
    Q_OBJECT

public:
    enum class Step {
        Requesting,
        Ready,
        Comparing,
        Confirming,
        Done,
        Cancelled
    };

    explicit DeviceVerificationWidget(const QString &otherUserId,
                                      const QString &deviceId,
                                      QWidget *parent = nullptr);
    ~DeviceVerificationWidget() override;

    void startVerification();
    void onVerificationRequestAccepted();
    void onSasReceived(const QString &sasKey);
    void onVerificationSuccessful();
    void onVerificationFailed(const QString &reason);

signals:
    void verificationComplete(bool success, const QString &deviceId);
    void verificationCancelled();

private slots:
    void onCancelClicked();
    void onMatchClicked();
    void onNoMatchClicked();
    void onDoneClicked();
    void onSpinnerTick();

private:
    void setupUi();
    void setStep(Step step);
    QWidget *createRequestingPage();
    QWidget *createReadyPage();
    QWidget *createEmojiDisplay(QWidget *parent, bool showButtons);
    QWidget *createDonePage();
    QWidget *createCancelledPage();
    void generateEmojis(const QString &sasKey);
    void updateProgressBar(Step step);
    static const std::array<EmojiEntry, 64> &emojiTable();

    QString m_otherUserId;
    QString m_deviceId;
    Step m_currentStep;

    QStackedWidget *m_stackedWidget;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;

    std::array<QLabel *, 7> m_emojiLabels;
    std::array<QLabel *, 7> m_emojiNameLabels;

    QPushButton *m_cancelButton;
    QPushButton *m_matchButton;
    QPushButton *m_noMatchButton;
    QPushButton *m_doneButton;

    QLabel *m_spinnerLabel;
    QTimer *m_spinnerTimer;
    int m_spinnerFrame;
};
