#include "features/matrix_chat/device_verification_widget.hpp"

#include <QFrame>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>

#include <cmath>

namespace {

QFrame *createSeparator(QWidget *parent)
{
    auto *line = new QFrame(parent);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setFixedHeight(1);
    return line;
}

constexpr int EMOJI_FONT_SIZE = 48;
constexpr int EMOJI_NAME_FONT_SIZE = 11;
constexpr int SAS_EMOJI_COUNT = 7;

const QStringList SPINNER_FRAMES = {
    QStringLiteral("\xe2\xa3\xbf"),  // ⣿
    QStringLiteral("\xe2\xa3\xbe"),  // ⣾
    QStringLiteral("\xe2\xa3\xbd"),  // ⣽
    QStringLiteral("\xe2\xa3\xbb"),  // ⣻
    QStringLiteral("\xe2\xa3\xba"),  // ⣺
    QStringLiteral("\xe2\xa3\xb9"),  // ⣹
    QStringLiteral("\xe2\xa3\xb8"),  // ⣸
    QStringLiteral("\xe2\xa3\xb7"),  // ⣷
};

} // anonymous namespace

DeviceVerificationWidget::DeviceVerificationWidget(const QString &otherUserId,
                                                     const QString &deviceId,
                                                     QWidget *parent)
    : QWidget(parent)
    , m_otherUserId(otherUserId)
    , m_deviceId(deviceId)
    , m_currentStep(Step::Requesting)
    , m_stackedWidget(nullptr)
    , m_progressBar(nullptr)
    , m_statusLabel(nullptr)
    , m_emojiLabels{}
    , m_emojiNameLabels{}
    , m_cancelButton(nullptr)
    , m_matchButton(nullptr)
    , m_noMatchButton(nullptr)
    , m_doneButton(nullptr)
    , m_spinnerLabel(nullptr)
    , m_spinnerTimer(nullptr)
    , m_spinnerFrame(0)
{
    setupUi();
}

DeviceVerificationWidget::~DeviceVerificationWidget()
{
    if (m_spinnerTimer) {
        m_spinnerTimer->stop();
    }
}

void DeviceVerificationWidget::setupUi()
{
    setMinimumSize(480, 420);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    auto *header = new QLabel(tr("Device Verification"), this);
    QFont headerFont = header->font();
    headerFont.setPointSize(16);
    headerFont.setBold(true);
    header->setFont(headerFont);
    header->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(header);

    auto *subtitle = new QLabel(tr("Verify device <b>%1</b>").arg(m_deviceId), this);
    subtitle->setStyleSheet(QStringLiteral("font-family: monospace; color: #888; font-size: 11px;"));
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setWordWrap(true);
    mainLayout->addWidget(subtitle);

    // Progress bar (5 steps)
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 5);
    m_progressBar->setValue(1);
    m_progressBar->setTextVisible(false);
    m_progressBar->setFixedHeight(6);
    m_progressBar->setStyleSheet(QStringLiteral(
        "QProgressBar { border: none; background-color: #e0e0e0; border-radius: 3px; } "
        "QProgressBar::chunk { background-color: #3498db; border-radius: 3px; }"));
    mainLayout->addWidget(m_progressBar);

    // Status label
    m_statusLabel = new QLabel(this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setWordWrap(true);
    QFont statusFont = m_statusLabel->font();
    statusFont.setPointSize(12);
    m_statusLabel->setFont(statusFont);
    mainLayout->addWidget(m_statusLabel);

    // Spinner
    m_spinnerLabel = new QLabel(this);
    m_spinnerLabel->setAlignment(Qt::AlignCenter);
    QFont spinnerFont = m_spinnerLabel->font();
    spinnerFont.setPointSize(32);
    m_spinnerLabel->setFont(spinnerFont);
    m_spinnerLabel->setStyleSheet(QStringLiteral("color: #3498db;"));
    mainLayout->addWidget(m_spinnerLabel);

    m_spinnerTimer = new QTimer(this);
    QObject::connect(m_spinnerTimer, &QTimer::timeout,
                     this, &DeviceVerificationWidget::onSpinnerTick);

    // Stacked widget for content
    m_stackedWidget = new QStackedWidget(this);
    m_stackedWidget->addWidget(createRequestingPage()); // index 0
    m_stackedWidget->addWidget(createReadyPage());      // index 1
    m_stackedWidget->addWidget(createEmojiDisplay(m_stackedWidget, false)); // index 2 - comparing
    m_stackedWidget->addWidget(createEmojiDisplay(m_stackedWidget, true));  // index 3 - confirming
    m_stackedWidget->addWidget(createDonePage());       // index 4
    m_stackedWidget->addWidget(createCancelledPage());  // index 5
    m_stackedWidget->setCurrentIndex(0);
    mainLayout->addWidget(m_stackedWidget, 1);

    // Bottom button bar
    auto *btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(12);
    btnLayout->addStretch();

    m_cancelButton = new QPushButton(tr("Cancel"), this);
    m_cancelButton->setStyleSheet(QStringLiteral(
        "QPushButton { padding: 8px 20px; border: 1px solid #ccc; border-radius: 4px; "
        "background-color: #f5f5f5; font-weight: bold; } "
        "QPushButton:hover { background-color: #e0e0e0; } "));
    QObject::connect(m_cancelButton, &QPushButton::clicked,
                     this, &DeviceVerificationWidget::onCancelClicked);
    btnLayout->addWidget(m_cancelButton);

    m_matchButton = new QPushButton(tr("They Match"), this);
    m_matchButton->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: #27ae60; color: white; border: none; "
        "padding: 8px 24px; border-radius: 4px; font-weight: bold; } "
        "QPushButton:hover { background-color: #219a52; } "
        "QPushButton:pressed { background-color: #1e8449; }"));
    QObject::connect(m_matchButton, &QPushButton::clicked,
                     this, &DeviceVerificationWidget::onMatchClicked);

    m_noMatchButton = new QPushButton(tr("No Match"), this);
    m_noMatchButton->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: #e74c3c; color: white; border: none; "
        "padding: 8px 24px; border-radius: 4px; font-weight: bold; } "
        "QPushButton:hover { background-color: #c0392b; } "
        "QPushButton:pressed { background-color: #a93226; }"));
    QObject::connect(m_noMatchButton, &QPushButton::clicked,
                     this, &DeviceVerificationWidget::onNoMatchClicked);

    m_doneButton = new QPushButton(tr("Done"), this);
    m_doneButton->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: #3498db; color: white; border: none; "
        "padding: 8px 24px; border-radius: 4px; font-weight: bold; } "
        "QPushButton:hover { background-color: #2980b9; } "
        "QPushButton:pressed { background-color: #2471a3; }"));
    QObject::connect(m_doneButton, &QPushButton::clicked,
                     this, &DeviceVerificationWidget::onDoneClicked);

    btnLayout->addWidget(m_matchButton);
    btnLayout->addWidget(m_noMatchButton);
    btnLayout->addWidget(m_doneButton);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    // Initial button visibility
    m_matchButton->setVisible(false);
    m_noMatchButton->setVisible(false);
    m_doneButton->setVisible(false);

    m_spinnerLabel->setVisible(true);
    m_spinnerTimer->start(120);
}

QWidget *DeviceVerificationWidget::createRequestingPage()
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(16, 8, 16, 8);
    layout->setSpacing(8);
    layout->addStretch();

    auto *info = new QLabel(
        tr("Requesting verification from <b>%1</b>...\n\n"
           "This will confirm that the device you are communicating with "
           "belongs to the person you think it does.")
            .arg(m_otherUserId.toHtmlEscaped()), page);
    info->setWordWrap(true);
    info->setAlignment(Qt::AlignCenter);
    QFont infoFont = info->font();
    infoFont.setPointSize(12);
    info->setFont(infoFont);
    layout->addWidget(info);

    layout->addStretch();
    return page;
}

QWidget *DeviceVerificationWidget::createReadyPage()
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(16, 8, 16, 8);
    layout->setSpacing(8);
    layout->addStretch();

    auto *info = new QLabel(
        tr("<b>%1</b> accepted the verification request.\n\n"
           "Generating Short Authentication String (SAS)...")
            .arg(m_otherUserId.toHtmlEscaped()), page);
    info->setWordWrap(true);
    info->setAlignment(Qt::AlignCenter);
    QFont infoFont = info->font();
    infoFont.setPointSize(12);
    info->setFont(infoFont);
    layout->addWidget(info);

    layout->addStretch();
    return page;
}

QWidget *DeviceVerificationWidget::createEmojiDisplay(QWidget *parent, bool showButtons)
{
    auto *page = new QWidget(parent);
    auto *outerLayout = new QVBoxLayout(page);
    outerLayout->setContentsMargins(8, 4, 8, 4);
    outerLayout->setSpacing(6);
    outerLayout->addStretch();

    // Emoji cards: 2 rows of 4+3
    auto *row1 = new QHBoxLayout;
    row1->setSpacing(10);
    row1->addStretch();

    auto *row2 = new QHBoxLayout;
    row2->setSpacing(10);
    row2->addStretch();

    for (int i = 0; i < SAS_EMOJI_COUNT; ++i) {
        auto *cardFrame = new QFrame(page);
        cardFrame->setFixedSize(90, 110);
        cardFrame->setStyleSheet(QStringLiteral(
            "QFrame { background-color: #ffffff; border: 1px solid #e0e0e0; "
            "border-radius: 8px; } "
            "QFrame:hover { border-color: #3498db; }"));
        auto *cardLayout = new QVBoxLayout(cardFrame);
        cardLayout->setContentsMargins(4, 8, 4, 4);
        cardLayout->setAlignment(Qt::AlignCenter);
        cardLayout->setSpacing(4);

        auto *emojiLabel = new QLabel(cardFrame);
        emojiLabel->setAlignment(Qt::AlignCenter);
        QFont emojiFont = emojiLabel->font();
        emojiFont.setPointSize(EMOJI_FONT_SIZE);
        emojiLabel->setFont(emojiFont);
        cardLayout->addWidget(emojiLabel);

        auto *nameLabel = new QLabel(cardFrame);
        nameLabel->setAlignment(Qt::AlignCenter);
        nameLabel->setWordWrap(true);
        QFont nameFont = nameLabel->font();
        nameFont.setPointSize(EMOJI_NAME_FONT_SIZE);
        nameLabel->setFont(nameFont);
        nameLabel->setStyleSheet(QStringLiteral("color: #555; background: transparent; border: none;"));
        cardLayout->addWidget(nameLabel);

        m_emojiLabels[i] = emojiLabel;
        m_emojiNameLabels[i] = nameLabel;

        if (i < 4) {
            row1->addWidget(cardFrame);
        } else {
            row2->addWidget(cardFrame);
        }
    }

    row1->addStretch();
    row2->addStretch();

    outerLayout->addLayout(row1);
    outerLayout->addLayout(row2);
    outerLayout->addStretch();

    return page;
}

QWidget *DeviceVerificationWidget::createDonePage()
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);
    layout->addStretch();

    auto *checkLabel = new QLabel(QStringLiteral("\xE2\x9C\x85"), page); // ✅
    checkLabel->setAlignment(Qt::AlignCenter);
    QFont checkFont = checkLabel->font();
    checkFont.setPointSize(64);
    checkLabel->setFont(checkFont);
    layout->addWidget(checkLabel);

    auto *doneLabel = new QLabel(tr("<b>Verified!</b>"), page);
    doneLabel->setAlignment(Qt::AlignCenter);
    QFont doneFont = doneLabel->font();
    doneFont.setPointSize(18);
    doneFont.setBold(true);
    doneLabel->setFont(doneFont);
    doneLabel->setStyleSheet(QStringLiteral("color: #27ae60;"));
    layout->addWidget(doneLabel);

    auto *detailLabel = new QLabel(
        tr("Device <b>%1</b> belonging to <b>%2</b> has been verified.\n"
           "Your messages are now end-to-end encrypted and safe.")
            .arg(m_deviceId.toHtmlEscaped(), m_otherUserId.toHtmlEscaped()), page);
    detailLabel->setWordWrap(true);
    detailLabel->setAlignment(Qt::AlignCenter);
    QFont detailFont = detailLabel->font();
    detailFont.setPointSize(11);
    detailLabel->setFont(detailFont);
    layout->addWidget(detailLabel);

    layout->addStretch();
    return page;
}

QWidget *DeviceVerificationWidget::createCancelledPage()
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);
    layout->addStretch();

    auto *crossLabel = new QLabel(QStringLiteral("\xE2\x9D\x8C"), page); // ❌
    crossLabel->setAlignment(Qt::AlignCenter);
    QFont crossFont = crossLabel->font();
    crossFont.setPointSize(64);
    crossLabel->setFont(crossFont);
    layout->addWidget(crossLabel);

    auto *cancelledLabel = new QLabel(tr("<b>Verification Cancelled</b>"), page);
    cancelledLabel->setAlignment(Qt::AlignCenter);
    QFont cancelledFont = cancelledLabel->font();
    cancelledFont.setPointSize(18);
    cancelledFont.setBold(true);
    cancelledLabel->setFont(cancelledFont);
    cancelledLabel->setStyleSheet(QStringLiteral("color: #e74c3c;"));
    layout->addWidget(cancelledLabel);

    auto *detailLabel = new QLabel(
        tr("The verification process was cancelled. You can try again later.\n"
           "Until verification completes, this device will remain untrusted."), page);
    detailLabel->setWordWrap(true);
    detailLabel->setAlignment(Qt::AlignCenter);
    QFont detailFont = detailLabel->font();
    detailFont.setPointSize(11);
    detailLabel->setFont(detailFont);
    layout->addWidget(detailLabel);

    layout->addStretch();
    return page;
}

// ---------------------------------------------------------------------------
// Step management
// ---------------------------------------------------------------------------

void DeviceVerificationWidget::setStep(Step step)
{
    m_currentStep = step;
    updateProgressBar(step);

    // Hide all conditional buttons first
    m_matchButton->setVisible(false);
    m_noMatchButton->setVisible(false);
    m_doneButton->setVisible(false);

    switch (step) {
    case Step::Requesting:
        m_stackedWidget->setCurrentIndex(0);
        m_statusLabel->setText(tr("Waiting for %1 to accept...").arg(m_otherUserId));
        m_spinnerLabel->setVisible(true);
        m_spinnerTimer->start(120);
        m_cancelButton->setVisible(true);
        break;

    case Step::Ready:
        m_stackedWidget->setCurrentIndex(1);
        m_statusLabel->setText(tr("Generating verification emojis..."));
        m_spinnerLabel->setVisible(true);
        m_spinnerTimer->start(120);
        m_cancelButton->setVisible(true);
        break;

    case Step::Comparing:
        m_stackedWidget->setCurrentIndex(2);
        m_statusLabel->setText(tr("Comparing emojis with %1...").arg(m_otherUserId));
        m_spinnerLabel->setVisible(false);
        m_spinnerTimer->stop();
        m_cancelButton->setVisible(true);
        break;

    case Step::Confirming:
        m_stackedWidget->setCurrentIndex(3);
        m_statusLabel->setText(tr("<b>Do the emojis match?</b>"));
        m_spinnerLabel->setVisible(false);
        m_spinnerTimer->stop();
        m_cancelButton->setVisible(true);
        m_matchButton->setVisible(true);
        m_noMatchButton->setVisible(true);
        break;

    case Step::Done:
        m_stackedWidget->setCurrentIndex(4);
        m_statusLabel->setText(tr("Verified!"));
        m_progressBar->setValue(5);
        m_progressBar->setStyleSheet(QStringLiteral(
            "QProgressBar { border: none; background-color: #e0e0e0; border-radius: 3px; } "
            "QProgressBar::chunk { background-color: #27ae60; border-radius: 3px; }"));
        m_spinnerLabel->setVisible(false);
        m_spinnerTimer->stop();
        m_cancelButton->setVisible(false);
        m_doneButton->setVisible(true);
        break;

    case Step::Cancelled:
        m_stackedWidget->setCurrentIndex(5);
        m_statusLabel->setText(tr("Cancelled"));
        m_progressBar->setStyleSheet(QStringLiteral(
            "QProgressBar { border: none; background-color: #e0e0e0; border-radius: 3px; } "
            "QProgressBar::chunk { background-color: #e74c3c; border-radius: 3px; }"));
        m_spinnerLabel->setVisible(false);
        m_spinnerTimer->stop();
        m_cancelButton->setVisible(false);
        m_doneButton->setVisible(true);
        break;
    }
}

void DeviceVerificationWidget::updateProgressBar(Step step)
{
    switch (step) {
    case Step::Requesting:  m_progressBar->setValue(1); break;
    case Step::Ready:       m_progressBar->setValue(2); break;
    case Step::Comparing:   m_progressBar->setValue(3); break;
    case Step::Confirming:  m_progressBar->setValue(4); break;
    case Step::Done:        m_progressBar->setValue(5); break;
    case Step::Cancelled:   /* keep current */          break;
    }
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void DeviceVerificationWidget::startVerification()
{
    m_currentStep = Step::Requesting;
    setStep(Step::Requesting);
    m_progressBar->setStyleSheet(QStringLiteral(
        "QProgressBar { border: none; background-color: #e0e0e0; border-radius: 3px; } "
        "QProgressBar::chunk { background-color: #3498db; border-radius: 3px; }"));
}

void DeviceVerificationWidget::onVerificationRequestAccepted()
{
    if (m_currentStep == Step::Requesting) {
        setStep(Step::Ready);
    }
}

void DeviceVerificationWidget::onSasReceived(const QString &sasKey)
{
    if (m_currentStep == Step::Ready) {
        generateEmojis(sasKey);
        setStep(Step::Comparing);
    }
}

void DeviceVerificationWidget::onVerificationSuccessful()
{
    setStep(Step::Done);
    emit verificationComplete(true, m_deviceId);
}

void DeviceVerificationWidget::onVerificationFailed(const QString &reason)
{
    Q_UNUSED(reason)
    setStep(Step::Cancelled);
}

// ---------------------------------------------------------------------------
// SAS Emoji Generation
// ---------------------------------------------------------------------------

void DeviceVerificationWidget::generateEmojis(const QString &sasKey)
{
    const auto &table = emojiTable();

    // Derive 7 emoji indices from the SAS key bytes.
    // Each byte contributes one index (mod 64).
    const QByteArray keyBytes = sasKey.toUtf8();
    for (int i = 0; i < SAS_EMOJI_COUNT && i < m_emojiLabels.size(); ++i) {
        const int idx = (i < keyBytes.size())
            ? (static_cast<unsigned char>(keyBytes.at(i)) % 64)
            : ((i * 17 + 3) % 64); // fallback deterministic values

        const auto &entry = table.at(idx);
        m_emojiLabels[i]->setText(QString::fromUtf8(entry.codepoint));
        m_emojiNameLabels[i]->setText(QString::fromUtf8(entry.name));
    }
}

// ---------------------------------------------------------------------------
// Emoji Table (Matrix SAS standard, sorted by number, indices 0-63)
// ---------------------------------------------------------------------------

const std::array<EmojiEntry, 64> &DeviceVerificationWidget::emojiTable()
{
    static const std::array<EmojiEntry, 64> table = {{
        { "\xF0\x9F\x90\xB6", "Dog" },            // 🐶 0
        { "\xF0\x9F\x90\xB1", "Cat" },            // 🐱 1
        { "\xF0\x9F\xA6\x81", "Lion" },           // 🦁 2
        { "\xF0\x9F\x90\x8E", "Horse" },          // 🐎 3
        { "\xF0\x9F\xA6\x84", "Unicorn" },        // 🦄 4
        { "\xF0\x9F\x90\xB7", "Pig" },            // 🐷 5
        { "\xF0\x9F\x90\x98", "Elephant" },       // 🐘 6
        { "\xF0\x9F\x90\xB0", "Rabbit" },         // 🐰 7
        { "\xF0\x9F\x90\xBC", "Panda" },          // 🐼 8
        { "\xF0\x9F\x90\x93", "Rooster" },        // 🐓 9
        { "\xF0\x9F\x90\xA7", "Penguin" },        // 🐧 10
        { "\xF0\x9F\x90\xA2", "Turtle" },         // 🐢 11
        { "\xF0\x9F\x90\x9F", "Fish" },           // 🐟 12
        { "\xF0\x9F\x90\x99", "Octopus" },        // 🐙 13
        { "\xF0\x9F\xA6\x8B", "Butterfly" },      // 🦋 14
        { "\xF0\x9F\x8C\xB7", "Tulip" },          // 🌷 15
        { "\xF0\x9F\x8C\xB3", "Evergreen" },      // 🌳 16
        { "\xF0\x9F\x8C\xB5", "Cactus" },         // 🌵 17
        { "\xF0\x9F\x8D\x84", "Mushroom" },       // 🍄 18
        { "\xF0\x9F\x8C\x8D", "Globe" },          // 🌍 19
        { "\xF0\x9F\x8C\x99", "Crescent Moon" },  // 🌙 20
        { "\xE2\x98\x81\xEF\xB8\x8F", "Cloud" },  // ☁️ 21
        { "\xF0\x9F\x8C\x88", "Rainbow" },        // 🌈 22
        { "\xE2\x9A\xA1", "Lightning" },          // ⚡ 23
        { "\xF0\x9F\x92\xA7", "Droplet" },        // 💧 24
        { "\xF0\x9F\x94\xA5", "Fire" },           // 🔥 25
        { "\xF0\x9F\x8E\xBE", "Tennis" },         // 🎾 26
        { "\xF0\x9F\x8F\x88", "Football" },       // 🏈 27
        { "\xE2\x9A\xBE", "Baseball" },           // ⚾ 28
        { "\xF0\x9F\x8F\x80", "Basketball" },     // 🏀 29
        { "\xF0\x9F\x8E\xB1", "8 Ball" },         // 🎱 30
        { "\xE2\x98\x95", "Coffee" },             // ☕ 31
        { "\xF0\x9F\x8D\xB5", "Tea" },            // 🍵 32
        { "\xF0\x9F\x8D\xB8", "Cocktail" },       // 🍸 33
        { "\xF0\x9F\x8D\x93", "Strawberry" },     // 🍓 34
        { "\xF0\x9F\x8D\x95", "Pizza" },          // 🍕 35
        { "\xF0\x9F\x8D\xB0", "Shortcake" },      // 🍰 36
        { "\xF0\x9F\x8E\x82", "Birthday Cake" },  // 🎂 37
        { "\xF0\x9F\x8D\x94", "Hamburger" },      // 🍔 38
        { "\xF0\x9F\x8D\x9F", "Fries" },          // 🍟 39
        { "\xF0\x9F\x8D\x89", "Watermelon" },     // 🍉 40
        { "\xF0\x9F\x8D\x8C", "Banana" },         // 🍌 41
        { "\xF0\x9F\x8E\xB8", "Guitar" },         // 🎸 42
        { "\xF0\x9F\x8E\xB9", "Piano" },          // 🎹 43
        { "\xF0\x9F\x8E\xBA", "Trumpet" },        // 🎺 44
        { "\xF0\x9F\x8E\xBB", "Violin" },         // 🎻 45
        { "\xF0\x9F\xA5\x81", "Drum" },           // 🥁 46
        { "\xF0\x9F\x8E\xA7", "Headphone" },      // 🎧 47
        { "\xF0\x9F\x9A\x80", "Rocket" },         // 🚀 48
        { "\xE2\x9C\x88\xEF\xB8\x8F", "Airplane" }, // ✈️ 49
        { "\xF0\x9F\x9A\x97", "Car" },            // 🚗 50
        { "\xF0\x9F\x9A\xB2", "Bicycle" },        // 🚲 51
        { "\xE2\x9B\xB5", "Sailboat" },           // ⛵ 52
        { "\xF0\x9F\x9A\x82", "Locomotive" },     // 🚂 53
        { "\xF0\x9F\x8F\xA0", "House" },          // 🏠 54
        { "\xE2\x8F\xB0", "Alarm Clock" },        // ⏰ 55
        { "\xF0\x9F\x94\x91", "Key" },            // 🔑 56
        { "\xF0\x9F\x91\x91", "Crown" },          // 👑 57
        { "\xF0\x9F\x8E\xA9", "Top Hat" },        // 🎩 58
        { "\xF0\x9F\x93\x95", "Book" },           // 📕 59
        { "\xE2\x9C\x8F\xEF\xB8\x8F", "Pencil" }, // ✏️ 60
        { "\xF0\x9F\x93\x8C", "Pushpin" },        // 📌 61
        { "\xF0\x9F\x93\x8E", "Paperclip" },      // 📎 62
        { "\xE2\x9C\x82\xEF\xB8\x8F", "Scissors" }, // ✂️ 63
    }};
    return table;
}

// ---------------------------------------------------------------------------
// Slots
// ---------------------------------------------------------------------------

void DeviceVerificationWidget::onCancelClicked()
{
    if (m_spinnerTimer) {
        m_spinnerTimer->stop();
    }
    setStep(Step::Cancelled);
    emit verificationCancelled();
}

void DeviceVerificationWidget::onMatchClicked()
{
    // The emojis match — proceed to success
    if (m_spinnerTimer) {
        m_spinnerTimer->stop();
    }
    setStep(Step::Done);
    emit verificationComplete(true, m_deviceId);
}

void DeviceVerificationWidget::onNoMatchClicked()
{
    // The emojis don't match — likely MITM attack
    if (m_spinnerTimer) {
        m_spinnerTimer->stop();
    }
    setStep(Step::Cancelled);
    m_statusLabel->setText(tr("Emojis do not match — verification failed!"));
    emit verificationCancelled();
}

void DeviceVerificationWidget::onDoneClicked()
{
    // Emit final signal and allow parent to close/remove widget
    if (m_currentStep == Step::Done) {
        emit verificationComplete(true, m_deviceId);
    } else {
        emit verificationCancelled();
    }
}

void DeviceVerificationWidget::onSpinnerTick()
{
    m_spinnerFrame = (m_spinnerFrame + 1) % SPINNER_FRAMES.size();
    m_spinnerLabel->setText(SPINNER_FRAMES.at(m_spinnerFrame));
}
