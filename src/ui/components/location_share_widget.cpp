#include "ui/components/location_share_widget.hpp"

#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPainter>
#include <QPaintEvent>
#include <QPainterPath>
#include <QClipboard>
#include <QGuiApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QFrame>
#include <QStandardItemModel>

// ---------------------------------------------------------------------------
// MapPlaceholder – draws a coloured grid with a pin and coordinate text
// ---------------------------------------------------------------------------

class LocationShareWidget::MapPlaceholder : public QWidget
{
public:
    explicit MapPlaceholder(QWidget *parent = nullptr)
        : QWidget(parent)
        , m_lat(0.0)
        , m_lng(0.0)
    {
        setMinimumSize(280, 200);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    void setCoordinates(double lat, double lng)
    {
        m_lat = lat;
        m_lng = lng;
        update();
    }

    QSize sizeHint() const override { return QSize(320, 220); }

protected:
    void paintEvent(QPaintEvent * /*event*/) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        const int w = width();
        const int h = height();

        // Background
        painter.fillRect(rect(), QColor(0xE8, 0xED, 0xF2));

        // Grid lines
        const int gridStep = 24;
        QPen gridPen(QColor(0xD0, 0xD7, 0xDE), 1);
        painter.setPen(gridPen);
        for (int x = gridStep; x < w; x += gridStep) {
            painter.drawLine(x, 0, x, h);
        }
        for (int y = gridStep; y < h; y += gridStep) {
            painter.drawLine(0, y, w, y);
        }

        // Pin at centre
        const int cx = w / 2;
        const int cy = h / 2 - 6;

        QPainterPath pinPath;
        const int pinR  = 10;
        const int pinH  = 18;   // bottom point offset

        // Bottom point
        pinPath.moveTo(cx, cy + pinH);
        // Right edge of triangle + right half of circle
        pinPath.lineTo(cx + pinR, cy);
        pinPath.arcTo(QRect(cx - pinR, cy - pinR * 2, pinR * 2, pinR * 2),
                      0, 180);
        // Left edge of triangle back to bottom
        pinPath.lineTo(cx, cy + pinH);
        pinPath.closeSubpath();

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0xE5, 0x39, 0x35));  // red pin
        painter.drawPath(pinPath);

        // Inner white circle
        painter.setBrush(Qt::white);
        painter.drawEllipse(QPoint(cx, cy), pinR / 2, pinR / 2);

        // Coordinate label at bottom
        const QString coordText = QStringLiteral("%1, %2")
                                      .arg(m_lat, 0, 'f', 6)
                                      .arg(m_lng, 0, 'f', 6);
        QFont font;
        font.setPixelSize(13);
        font.setBold(true);
        painter.setFont(font);
        painter.setPen(QColor(0x55, 0x55, 0x55));
        painter.drawText(QRect(8, h - 28, w - 16, 24),
                         Qt::AlignCenter, coordText);
    }

private:
    double m_lat;
    double m_lng;
};

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

LocationShareWidget::LocationShareWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    setPickerMode();
}

// ---------------------------------------------------------------------------
// setupUi
// ---------------------------------------------------------------------------

void LocationShareWidget::setupUi()
{
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    m_stack = new QStackedWidget(this);
    outerLayout->addWidget(m_stack);

    // Construction order matters: picker = index 0, viewer = index 1
    setupPickerPage();
    setupViewerPage();
}

// ---------------------------------------------------------------------------
// Picker page
// ---------------------------------------------------------------------------

void LocationShareWidget::setupPickerPage()
{
    auto *page   = new QWidget();
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(12, 10, 12, 12);
    layout->setSpacing(10);

    // --- Header ---
    auto *headerLabel = new QLabel(QStringLiteral("Share Location"));
    QFont headerFont;
    headerFont.setPixelSize(16);
    headerFont.setBold(true);
    headerLabel->setFont(headerFont);
    layout->addWidget(headerLabel);

    // --- Coordinate inputs ---
    auto *coordForm = new QFormLayout();
    coordForm->setSpacing(6);

    m_latSpinBox = new QDoubleSpinBox();
    m_latSpinBox->setRange(-90.0, 90.0);
    m_latSpinBox->setDecimals(6);
    m_latSpinBox->setSingleStep(0.0001);
    m_latSpinBox->setValue(0.0);
    m_latSpinBox->setToolTip(QStringLiteral("Latitude (-90 to 90)"));
    coordForm->addRow(QStringLiteral("Latitude:"), m_latSpinBox);

    m_lngSpinBox = new QDoubleSpinBox();
    m_lngSpinBox->setRange(-180.0, 180.0);
    m_lngSpinBox->setDecimals(6);
    m_lngSpinBox->setSingleStep(0.0001);
    m_lngSpinBox->setValue(0.0);
    m_lngSpinBox->setToolTip(QStringLiteral("Longitude (-180 to 180)"));
    coordForm->addRow(QStringLiteral("Longitude:"), m_lngSpinBox);

    layout->addLayout(coordForm);

    // --- Preset picker ---
    m_presetCombo = new QComboBox();
    m_presetCombo->setToolTip(QStringLiteral("Choose a preset or recent location"));
    layout->addWidget(m_presetCombo);

    updatePresetCombo();

    // --- Coordinate preview ---
    m_coordPreview = new QLabel();
    m_coordPreview->setAlignment(Qt::AlignCenter);
    m_coordPreview->setStyleSheet(QStringLiteral(
        "QLabel { color: #555; font-size: 12px; padding: 4px 8px;"
        " background: #f0f0f0; border-radius: 4px; }"));
    layout->addWidget(m_coordPreview);

    // --- Description field ---
    auto *descLayout = new QHBoxLayout();
    auto *descLabel  = new QLabel(QStringLiteral("Label:"));
    descLabel->setStyleSheet(QStringLiteral("QLabel { font-size: 13px; }"));
    m_descriptionEdit = new QLineEdit();
    m_descriptionEdit->setPlaceholderText(
        QStringLiteral("Optional description (e.g. \"Home\", \"Café\")"));
    m_descriptionEdit->setClearButtonEnabled(true);
    descLayout->addWidget(descLabel);
    descLayout->addWidget(m_descriptionEdit, 1);
    layout->addLayout(descLayout);

    // --- Share button ---
    m_shareButton = new QPushButton(QStringLiteral("Share Location"));
    m_shareButton->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: #0084FF; color: white;"
        " border: none; border-radius: 6px; padding: 8px 16px;"
        " font-size: 14px; font-weight: bold; }"
        "QPushButton:hover { background-color: #0073E6; }"
        "QPushButton:pressed { background-color: #0062CC; }"));
    layout->addWidget(m_shareButton);

    layout->addStretch();

    m_stack->insertWidget(0, page);  // index 0 = picker

    // --- Connections ---

    QObject::connect(m_latSpinBox,
                     QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                     this, &LocationShareWidget::onCoordinateChanged);
    QObject::connect(m_lngSpinBox,
                     QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                     this, &LocationShareWidget::onCoordinateChanged);
    QObject::connect(m_presetCombo,
                     QOverload<int>::of(&QComboBox::currentIndexChanged),
                     this, &LocationShareWidget::onPresetChanged);
    QObject::connect(m_shareButton, &QPushButton::clicked,
                     this, &LocationShareWidget::onShareClicked);

    updateCoordinatePreview();
}

// ---------------------------------------------------------------------------
// Viewer page
// ---------------------------------------------------------------------------

void LocationShareWidget::setupViewerPage()
{
    auto *page   = new QWidget();
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // --- Map placeholder ---
    m_mapPlaceholder = new MapPlaceholder();
    m_mapPlaceholder->setMinimumHeight(180);
    layout->addWidget(m_mapPlaceholder);

    // --- Bottom info card ---
    auto *card = new QFrame();
    card->setStyleSheet(QStringLiteral(
        "QFrame { background: #fff; border-top: 1px solid #ddd; }"));
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(14, 10, 14, 14);
    cardLayout->setSpacing(8);

    // Coordinates
    m_viewerCoordLabel = new QLabel();
    m_viewerCoordLabel->setTextFormat(Qt::RichText);
    m_viewerCoordLabel->setOpenExternalLinks(false);
    m_viewerCoordLabel->setTextInteractionFlags(
        Qt::TextBrowserInteraction | Qt::LinksAccessibleByMouse);
    QObject::connect(m_viewerCoordLabel, &QLabel::linkActivated,
                     this, &LocationShareWidget::onOpenInBrowser);
    cardLayout->addWidget(m_viewerCoordLabel);

    // Description
    m_viewerDescLabel = new QLabel();
    m_viewerDescLabel->setStyleSheet(QStringLiteral(
        "QLabel { color: #333; font-size: 13px; }"));
    m_viewerDescLabel->setWordWrap(true);
    cardLayout->addWidget(m_viewerDescLabel);

    // Button row
    auto *btnRow = new QHBoxLayout();
    btnRow->setSpacing(8);

    m_openBrowserButton = new QPushButton(QStringLiteral("Open in Browser"));
    m_openBrowserButton->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: #f0f0f0; color: #333;"
        " border: 1px solid #ccc; border-radius: 6px; padding: 6px 14px;"
        " font-size: 13px; }"
        "QPushButton:hover { background-color: #e4e4e4; }"));
    btnRow->addWidget(m_openBrowserButton);

    m_copyButton = new QPushButton(QStringLiteral("Copy Coordinates"));
    m_copyButton->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: #f0f0f0; color: #333;"
        " border: 1px solid #ccc; border-radius: 6px; padding: 6px 14px;"
        " font-size: 13px; }"
        "QPushButton:hover { background-color: #e4e4e4; }"));
    btnRow->addWidget(m_copyButton);
    btnRow->addStretch();

    cardLayout->addLayout(btnRow);

    layout->addWidget(card);

    m_stack->insertWidget(1, page);  // index 1 = viewer

    // --- Connections ---
    QObject::connect(m_openBrowserButton, &QPushButton::clicked,
                     this, &LocationShareWidget::onOpenInBrowser);
    QObject::connect(m_copyButton, &QPushButton::clicked,
                     this, &LocationShareWidget::onCopyCoordinates);
}

// ---------------------------------------------------------------------------
// Mode switching
// ---------------------------------------------------------------------------

void LocationShareWidget::setViewerMode(double lat, double lng,
                                         const QString &description)
{
    m_viewerLat  = lat;
    m_viewerLng  = lng;
    m_viewerDesc = description;

    m_mapPlaceholder->setCoordinates(lat, lng);

    // Build rich-text coordinate label with geo: link
    const QString coordStr =
        QStringLiteral("%1, %2")
            .arg(lat, 0, 'f', 6)
            .arg(lng, 0, 'f', 6);
    const QString geoUri =
        QStringLiteral("geo:%1,%2").arg(lat, 0, 'f', 6).arg(lng, 0, 'f', 6);
    m_viewerCoordLabel->setText(
        QStringLiteral(
            "<span style='font-size:14px; font-weight:bold; color:#333;'>%1</span>"
            " &nbsp; <a href=\"%2\" style='color:#0084FF; "
            "text-decoration:none;'>geo: link</a>")
            .arg(coordStr.toHtmlEscaped(), geoUri));

    // Description
    if (description.isEmpty()) {
        m_viewerDescLabel->hide();
    } else {
        m_viewerDescLabel->setText(description);
        m_viewerDescLabel->show();
    }

    m_stack->setCurrentIndex(1);
}

void LocationShareWidget::setPickerMode()
{
    m_stack->setCurrentIndex(0);
    updatePresetCombo();
    updateCoordinatePreview();
}

// ---------------------------------------------------------------------------
// Slots – picker
// ---------------------------------------------------------------------------

void LocationShareWidget::onShareClicked()
{
    const double lat = m_latSpinBox->value();
    const double lng = m_lngSpinBox->value();
    const QString desc = m_descriptionEdit->text().trimmed();

    addRecentLocation(lat, lng, desc);

    emit locationShared(lat, lng, desc);
}

void LocationShareWidget::onPresetChanged(int index)
{
    if (m_updatingPreset) {
        return;
    }

    if (index < 0) {
        return;
    }

    // Index 0 = custom (spinbox values)
    // Index 1 = current location placeholder
    // Separator after index 1
    // Index 3+ = recent locations

    if (index == 0) {
        // Custom – keep current spinbox values, nothing to do
        return;
    }

    if (index == 1) {
        // Current Location placeholder – set to a fixed placeholder value
        m_updatingPreset = true;
        m_latSpinBox->setValue(51.5074);
        m_lngSpinBox->setValue(-0.1278);
        m_updatingPreset = false;
        updateCoordinatePreview();
        return;
    }

    // Recent location
    const int recentIdx = index - 3;  // skip custom, current, separator
    if (recentIdx >= 0 && recentIdx < m_recentLocations.size()) {
        const auto &loc = m_recentLocations.at(recentIdx);
        m_updatingPreset = true;
        m_latSpinBox->setValue(loc.latitude);
        m_lngSpinBox->setValue(loc.longitude);
        m_updatingPreset = false;
        updateCoordinatePreview();
    }
}

void LocationShareWidget::onCoordinateChanged()
{
    if (m_updatingPreset) {
        return;
    }

    // User manually changed coordinates → revert preset to "Custom"
    m_presetCombo->blockSignals(true);
    m_presetCombo->setCurrentIndex(0);
    m_presetCombo->blockSignals(false);

    updateCoordinatePreview();
}

void LocationShareWidget::updateCoordinatePreview()
{
    const double lat = m_latSpinBox->value();
    const double lng = m_lngSpinBox->value();
    m_coordPreview->setText(
        QStringLiteral("%1, %2")
            .arg(lat, 0, 'f', 6)
            .arg(lng, 0, 'f', 6));
}

// ---------------------------------------------------------------------------
// Slots – viewer
// ---------------------------------------------------------------------------

void LocationShareWidget::onOpenInBrowser()
{
    const QString url = QStringLiteral(
        "https://www.openstreetmap.org/?mlat=%1&mlon=%2#map=15/%1/%2")
                            .arg(m_viewerLat, 0, 'f', 6)
                            .arg(m_viewerLng, 0, 'f', 6);
    QDesktopServices::openUrl(QUrl(url));
}

void LocationShareWidget::onCopyCoordinates()
{
    const QString text = QStringLiteral("%1, %2")
                             .arg(m_viewerLat, 0, 'f', 6)
                             .arg(m_viewerLng, 0, 'f', 6);
    QGuiApplication::clipboard()->setText(text);
}

// ---------------------------------------------------------------------------
// Recent locations management
// ---------------------------------------------------------------------------

void LocationShareWidget::addRecentLocation(double lat, double lng,
                                             const QString &description)
{
    const PresetLocation loc(lat, lng, description);

    // Avoid duplicates – remove if already exists
    for (int i = 0; i < m_recentLocations.size(); ++i) {
        const auto &existing = m_recentLocations.at(i);
        if (qFuzzyCompare(existing.latitude, lat) &&
            qFuzzyCompare(existing.longitude, lng)) {
            m_recentLocations.removeAt(i);
            break;
        }
    }

    m_recentLocations.prepend(loc);

    // Keep at most 8 recent entries
    while (m_recentLocations.size() > 8) {
        m_recentLocations.removeLast();
    }
}

void LocationShareWidget::updatePresetCombo()
{
    m_presetCombo->blockSignals(true);

    const int oldIdx = m_presetCombo->currentIndex();
    m_presetCombo->clear();

    m_presetCombo->addItem(QStringLiteral("Custom coordinates"));
    m_presetCombo->addItem(
        QStringLiteral("\xF0\x9F\x93\x8D  Current Location"),
        QStringLiteral("placeholder"));

    m_presetCombo->insertSeparator(2);

    for (const auto &loc : m_recentLocations) {
        QString text = QStringLiteral("%1, %2")
                           .arg(loc.latitude, 0, 'f', 4)
                           .arg(loc.longitude, 0, 'f', 4);
        if (!loc.description.isEmpty()) {
            text += QStringLiteral("  — %1").arg(loc.description);
        }
        m_presetCombo->addItem(text);
    }

    if (m_recentLocations.isEmpty()) {
        m_presetCombo->addItem(
            QStringLiteral("(no recent locations)"));
        QStandardItemModel *model =
            qobject_cast<QStandardItemModel *>(m_presetCombo->model());
        if (model) {
            model->item(m_presetCombo->count() - 1)
                ->setEnabled(false);
        }
    }

    m_presetCombo->blockSignals(false);

    if (oldIdx >= 0 && oldIdx < m_presetCombo->count()) {
        m_presetCombo->setCurrentIndex(oldIdx);
    } else {
        m_presetCombo->setCurrentIndex(0);
    }
}
