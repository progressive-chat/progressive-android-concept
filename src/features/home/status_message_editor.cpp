#include "status_message_editor.hpp"

#include <QApplication>
#include <QEvent>
#include <QGraphicsOpacityEffect>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QStyle>

#include "matrix/matrix_session.hpp"

StatusMessageEditor::StatusMessageEditor(MatrixSession* session,
                                           QWidget* parent)
    : QWidget(parent)
    , m_session(session)
    , m_statusEdit(nullptr)
    , m_charCounter(nullptr)
    , m_presencePicker(nullptr)
    , m_setStatusButton(nullptr)
    , m_clearStatusButton(nullptr)
    , m_recentLayout(nullptr)
    , m_recentContainer(nullptr)
    , m_backdropWidget(nullptr)
    , m_sheetWidget(nullptr)
    , m_animation(nullptr)
    , m_slideOffset(0)
{
    setupUi();
    loadRecentStatuses();
    animateIn();
}

void StatusMessageEditor::setupUi()
{
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setFixedWidth(360);

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ---------- Semi-transparent backdrop ----------
    m_backdropWidget = new QWidget(this);
    m_backdropWidget->setObjectName(QStringLiteral("statusBackdrop"));
    m_backdropWidget->setStyleSheet(QStringLiteral("background: rgba(0, 0, 0, 0.3);"));
    m_backdropWidget->installEventFilter(this);
    rootLayout->addWidget(m_backdropWidget);

    // ---------- Sheet ----------
    m_sheetWidget = new QWidget;
    m_sheetWidget->setObjectName(QStringLiteral("statusSheet"));
    m_sheetWidget->setStyleSheet(QStringLiteral(
        "background: #ffffff; border-top-left-radius: 16px; border-top-right-radius: 16px;"));

    auto* sheetLayout = new QVBoxLayout(m_sheetWidget);
    sheetLayout->setContentsMargins(20, 16, 20, 20);
    sheetLayout->setSpacing(12);

    // ---- Drag handle ----
    auto* handle = new QLabel;
    handle->setFixedSize(40, 4);
    handle->setObjectName(QStringLiteral("sheetHandle"));
    handle->setAlignment(Qt::AlignCenter);
    handle->setStyleSheet(QStringLiteral(
        "background: #cccccc; border-radius: 2px;"));
    auto* handleCenter = new QHBoxLayout;
    handleCenter->addStretch();
    handleCenter->addWidget(handle);
    handleCenter->addStretch();
    sheetLayout->addLayout(handleCenter);

    // ---- Title ----
    auto* title = new QLabel(tr("Set Status"));
    title->setObjectName(QStringLiteral("statusTitle"));
    QFont titleFont = title->font();
    titleFont.setBold(true);
    titleFont.setPointSize(13);
    title->setFont(titleFont);
    sheetLayout->addWidget(title);

    // ---- Status input row ----
    auto* inputRow = new QHBoxLayout;
    inputRow->setSpacing(8);

    m_statusEdit = new QLineEdit;
    m_statusEdit->setObjectName(QStringLiteral("statusEdit"));
    m_statusEdit->setMaxLength(kMaxStatusLength);
    m_statusEdit->setPlaceholderText(tr("What's on your mind?"));
    m_statusEdit->setStyleSheet(QStringLiteral(
        "padding: 10px; border: 1px solid #ccc; border-radius: 8px; font-size: 14px;"));
    connect(m_statusEdit, &QLineEdit::textChanged,
            this, &StatusMessageEditor::onStatusTextChanged);
    inputRow->addWidget(m_statusEdit, 1);

    m_charCounter = new QLabel(QStringLiteral("0/%1").arg(kMaxStatusLength));
    m_charCounter->setObjectName(QStringLiteral("charCounter"));
    m_charCounter->setStyleSheet(QStringLiteral("color: #888; font-size: 11px;"));
    m_charCounter->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_charCounter->setFixedWidth(60);
    inputRow->addWidget(m_charCounter);

    sheetLayout->addLayout(inputRow);

    // ---- Presence picker ----
    auto* presenceRow = new QHBoxLayout;
    presenceRow->setSpacing(8);

    auto* presenceLabel = new QLabel(tr("Presence:"));
    presenceLabel->setStyleSheet(QStringLiteral("font-size: 13px; color: #555;"));
    presenceRow->addWidget(presenceLabel);

    m_presencePicker = new QComboBox;
    m_presencePicker->setObjectName(QStringLiteral("presencePicker"));
    m_presencePicker->setStyleSheet(QStringLiteral(
        "padding: 6px 10px; border: 1px solid #ccc; border-radius: 6px; font-size: 13px;"
        "background: white; min-width: 140px;"));

    m_presencePicker->addItem(QIcon(createPresencePixmap(QStringLiteral("#4CAF50"))),
                              tr("  Online"),    QStringLiteral("online"));
    m_presencePicker->addItem(QIcon(createPresencePixmap(QStringLiteral("#FFC107"))),
                              tr("  Away"),      QStringLiteral("unavailable"));
    m_presencePicker->addItem(QIcon(createPresencePixmap(QStringLiteral("#F44336"))),
                              tr("  Busy"),      QStringLiteral("busy"));
    m_presencePicker->addItem(QIcon(createPresencePixmap(QStringLiteral("#9E9E9E"))),
                              tr("  Invisible"), QStringLiteral("invisible"));

    m_presencePicker->setIconSize(QSize(10, 10));
    presenceRow->addWidget(m_presencePicker, 1);
    sheetLayout->addLayout(presenceRow);

    // ---- Recent statuses ----
    auto* recentHeader = new QLabel(tr("Recent Statuses"));
    recentHeader->setObjectName(QStringLiteral("recentHeader"));
    recentHeader->setStyleSheet(QStringLiteral("font-size: 12px; font-weight: bold; color: #555; margin-top: 6px;"));
    sheetLayout->addWidget(recentHeader);

    auto* scrollArea = new QScrollArea;
    scrollArea->setObjectName(QStringLiteral("recentScrollArea"));
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFixedHeight(42);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet(QStringLiteral("background: transparent;"));

    m_recentContainer = new QWidget;
    m_recentContainer->setObjectName(QStringLiteral("recentContainer"));
    m_recentLayout = new QHBoxLayout(m_recentContainer);
    m_recentLayout->setContentsMargins(0, 0, 0, 0);
    m_recentLayout->setSpacing(6);
    m_recentLayout->addStretch();

    scrollArea->setWidget(m_recentContainer);
    sheetLayout->addWidget(scrollArea);

    // ---- Action buttons ----
    auto* buttonRow = new QHBoxLayout;
    buttonRow->setSpacing(10);

    m_clearStatusButton = new QPushButton(tr("Clear Status"));
    m_clearStatusButton->setObjectName(QStringLiteral("clearStatusButton"));
    m_clearStatusButton->setCursor(Qt::PointingHandCursor);
    m_clearStatusButton->setStyleSheet(QStringLiteral(
        "background: #f5f5f5; border: 1px solid #ddd; border-radius: 6px;"
        "padding: 8px 16px; font-size: 13px; color: #555;"));
    connect(m_clearStatusButton, &QPushButton::clicked,
            this, &StatusMessageEditor::onClearStatusClicked);
    buttonRow->addWidget(m_clearStatusButton);

    buttonRow->addStretch();

    m_setStatusButton = new QPushButton(tr("Set Status"));
    m_setStatusButton->setObjectName(QStringLiteral("setStatusButton"));
    m_setStatusButton->setCursor(Qt::PointingHandCursor);
    m_setStatusButton->setStyleSheet(QStringLiteral(
        "background: #1976D2; color: white; border: none; border-radius: 6px;"
        "padding: 8px 22px; font-size: 13px; font-weight: bold;"));
    connect(m_setStatusButton, &QPushButton::clicked,
            this, &StatusMessageEditor::onSetStatusClicked);
    buttonRow->addWidget(m_setStatusButton);

    sheetLayout->addLayout(buttonRow);

    rootLayout->addWidget(m_sheetWidget);

    // Set initial height: let the sheet size itself, then fix the widget height
    m_sheetWidget->adjustSize();
    const int sheetHeight = m_sheetWidget->sizeHint().height();
    setFixedHeight(sheetHeight);

    // Position over parent
    if (parentWidget()) {
        QPoint parentCenter = parentWidget()->mapToGlobal(
            QPoint(parentWidget()->width() / 2, parentWidget()->height()));
        move(parentCenter.x() - width() / 2,
             parentCenter.y() - height());
    }
}

QPixmap StatusMessageEditor::createPresencePixmap(const QString& color) const
{
    QPixmap pix(16, 16);
    pix.fill(Qt::transparent);
    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor(color));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(2, 2, 12, 12);
    painter.end();
    return pix;
}

QLabel* StatusMessageEditor::createPresenceDot(const QString& color) const
{
    auto* dot = new QLabel;
    dot->setFixedSize(10, 10);
    dot->setStyleSheet(QStringLiteral(
        "background: %1; border-radius: 5px; min-width: 10px; max-width: 10px;"
        "min-height: 10px; max-height: 10px;").arg(color));
    return dot;
}

void StatusMessageEditor::loadRecentStatuses()
{
    // Would load from QSettings / local db; stub with sample data
    QStringList recent = {
        QStringLiteral("In a meeting"),
        QStringLiteral("Working remotely"),
        QStringLiteral("On vacation"),
        QStringLiteral("Available for calls"),
    };

    // Clear existing chips
    while (m_recentLayout->count() > 1) { // leave the trailing stretch
        QLayoutItem* item = m_recentLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    for (const QString& status : recent) {
        auto* chip = new QPushButton(status);
        chip->setObjectName(QStringLiteral("recentChip"));
        chip->setCursor(Qt::PointingHandCursor);
        chip->setStyleSheet(QStringLiteral(
            "background: #e3f2fd; color: #1565C0; border: none; border-radius: 14px;"
            "padding: 4px 14px; font-size: 12px; white-space: nowrap;"));
        chip->setToolTip(status);
        connect(chip, &QPushButton::clicked, this, [this, status]() {
            onRecentStatusClicked(status);
        });
        m_recentLayout->insertWidget(m_recentLayout->count() - 1, chip);
    }
}

void StatusMessageEditor::saveRecentStatus(const QString& status)
{
    // Stub: would persist to QSettings or equivalent
    Q_UNUSED(status);
}

void StatusMessageEditor::updateCharCounter()
{
    const int len = m_statusEdit->text().length();
    m_charCounter->setText(QStringLiteral("%1/%2").arg(len).arg(kMaxStatusLength));

    if (len >= kMaxStatusLength) {
        m_charCounter->setStyleSheet(QStringLiteral("color: #F44336; font-size: 11px; font-weight: bold;"));
    } else if (len >= kMaxStatusLength * 80 / 100) {
        m_charCounter->setStyleSheet(QStringLiteral("color: #FF9800; font-size: 11px;"));
    } else {
        m_charCounter->setStyleSheet(QStringLiteral("color: #888; font-size: 11px;"));
    }
}

void StatusMessageEditor::onStatusTextChanged(const QString& text)
{
    Q_UNUSED(text);
    updateCharCounter();
}

void StatusMessageEditor::onRecentStatusClicked(const QString& status)
{
    m_statusEdit->setText(status);
}

void StatusMessageEditor::onSetStatusClicked()
{
    const QString text = m_statusEdit->text().trimmed();
    const QString pres = m_presencePicker->currentData().toString();

    if (m_session) {
        // Real implementation:
        // m_session->setPresence(m_session->userId(), pres, text);
    }

    if (!text.isEmpty()) {
        saveRecentStatus(text);
    }

    emit statusUpdated(text, pres);
    animateOut();
}

void StatusMessageEditor::onClearStatusClicked()
{
    m_statusEdit->clear();

    if (m_session) {
        // Real implementation:
        // m_session->setPresence(m_session->userId(),
        //                        m_presencePicker->currentData().toString(),
        //                        QString());
    }

    emit statusUpdated({}, m_presencePicker->currentData().toString());
    animateOut();
}

void StatusMessageEditor::animateIn()
{
    // Slide up from bottom
    m_slideOffset = m_sheetWidget->height();
    setSlideOffset(m_slideOffset);

    m_animation = new QPropertyAnimation(this, "slideOffset", this);
    m_animation->setDuration(280);
    m_animation->setStartValue(m_sheetWidget->height());
    m_animation->setEndValue(0);
    m_animation->setEasingCurve(QEasingCurve::OutCubic);
    m_animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void StatusMessageEditor::animateOut()
{
    m_animation = new QPropertyAnimation(this, "slideOffset", this);
    m_animation->setDuration(220);
    m_animation->setStartValue(0);
    m_animation->setEndValue(m_sheetWidget->height());
    m_animation->setEasingCurve(QEasingCurve::InCubic);
    connect(m_animation, &QPropertyAnimation::finished, this, [this]() {
        emit dismissed();
        deleteLater();
    });
    m_animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void StatusMessageEditor::setSlideOffset(int offset)
{
    m_slideOffset = offset;
    m_sheetWidget->move(0, offset);
    m_backdropWidget->setGeometry(0, -offset, width(), offset);
    // Fade backdrop with progress
    const qreal progress = 1.0 - static_cast<qreal>(offset) / m_sheetWidget->height();
    QColor backdropColor(0, 0, 0, static_cast<int>(76 * progress)); // ~0.3 * 255
    m_backdropWidget->setStyleSheet(
        QStringLiteral("background: rgba(0, 0, 0, %1);").arg(progress * 0.3));
}

QString StatusMessageEditor::statusText() const
{
    return m_statusEdit->text().trimmed();
}

QString StatusMessageEditor::presence() const
{
    return m_presencePicker->currentData().toString();
}

bool StatusMessageEditor::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_backdropWidget && event->type() == QEvent::MouseButtonPress) {
        animateOut();
        return true;
    }
    return QWidget::eventFilter(obj, event);
}
