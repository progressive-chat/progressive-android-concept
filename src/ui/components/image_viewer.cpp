#include "image_viewer.hpp"

#include <QFileInfo>
#include <QFileDialog>
#include <QClipboard>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QTemporaryFile>
#include <QPainter>
#include <QPaintEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QShowEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QGuiApplication>
#include <QScreen>
#include <QTimer>

namespace {

QString formatFileSize(qint64 bytes) {
    if (bytes < 0) return QStringLiteral("N/A");
    if (bytes < 1024) return QStringLiteral("%1 B").arg(bytes);
    if (bytes < 1024 * 1024) return QStringLiteral("%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
    if (bytes < 1024LL * 1024 * 1024) return QStringLiteral("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 1);
    return QStringLiteral("%1 GB").arg(bytes / (1024.0 * 1024.0 * 1024.0), 0, 'f', 2);
}

const QString kBtnStyle = QStringLiteral(
    "QPushButton {"
    "  background-color: rgba(255,255,255,25);"
    "  color: white;"
    "  border: none;"
    "  padding: 4px 10px;"
    "  border-radius: 3px;"
    "  font-size: 13px;"
    "}"
    "QPushButton:hover {"
    "  background-color: rgba(255,255,255,55);"
    "}"
    "QPushButton:pressed {"
    "  background-color: rgba(255,255,255,90);"
    "}"
);

const QString kCloseBtnStyle = QStringLiteral(
    "QPushButton {"
    "  background-color: rgba(200,50,50,170);"
    "  color: white;"
    "  border: none;"
    "  border-radius: 3px;"
    "  font-size: 14px;"
    "  font-weight: bold;"
    "  padding: 5px 12px;"
    "}"
    "QPushButton:hover {"
    "  background-color: rgba(220,70,70,200);"
    "}"
);

const QString kNavBtnStyle = QStringLiteral(
    "QPushButton {"
    "  background-color: rgba(26,26,26,140);"
    "  color: white;"
    "  border: none;"
    "  border-radius: 4px;"
    "  font-size: 22px;"
    "}"
    "QPushButton:hover {"
    "  background-color: rgba(255,255,255,50);"
    "}"
);

} // anonymous namespace

// ---------------------------------------------------------------------------
// Constructors
// ---------------------------------------------------------------------------

ImageViewer::ImageViewer(const QString& filePath, QWidget* parent)
    : QDialog(parent)
{
    init();
    addImage(filePath);
    loadImage(0);
}

ImageViewer::ImageViewer(const QImage& image, QWidget* parent)
    : QDialog(parent)
{
    init();
    addImage(image);
    loadImage(0);
}

ImageViewer::ImageViewer(const QStringList& filePaths, int startIndex, QWidget* parent)
    : QDialog(parent)
{
    init();
    for (const auto& fp : filePaths) {
        addImage(fp);
    }
    if (startIndex >= 0 && startIndex < m_entries.size()) {
        loadImage(startIndex);
    } else if (!m_entries.isEmpty()) {
        loadImage(0);
    }
}

ImageViewer::ImageViewer(const QList<QImage>& images, int startIndex, QWidget* parent)
    : QDialog(parent)
{
    init();
    for (const auto& img : images) {
        addImage(img);
    }
    if (startIndex >= 0 && startIndex < m_entries.size()) {
        loadImage(startIndex);
    } else if (!m_entries.isEmpty()) {
        loadImage(0);
    }
}

// ---------------------------------------------------------------------------
// Public helpers
// ---------------------------------------------------------------------------

void ImageViewer::addImage(const QString& filePath) {
    ImageEntry entry;
    QFileInfo fi(filePath);
    entry.sourceName = fi.fileName();
    entry.fileSize = fi.size();
    entry.hasFilePath = true;
    entry.filePath = fi.absoluteFilePath();
    entry.image = QImage(fi.absoluteFilePath());
    if (!entry.image.isNull()) {
        m_entries.append(entry);
    }
}

void ImageViewer::addImage(const QImage& image) {
    ImageEntry entry;
    entry.image = image;
    entry.sourceName = QStringLiteral("Untitled");
    entry.fileSize = -1;
    entry.hasFilePath = false;
    if (!entry.image.isNull()) {
        m_entries.append(entry);
    }
}

void ImageViewer::setCurrentIndex(int index) {
    if (index >= 0 && index < m_entries.size()) {
        loadImage(index);
    }
}

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------

void ImageViewer::init() {
    setupUi();
}

void ImageViewer::setupUi() {
    setWindowTitle(QStringLiteral("Image Viewer"));
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, false);
    setAutoFillBackground(false);
    setMinimumSize(400, 300);

    QScreen* screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect sg = screen->availableGeometry();
        resize(static_cast<int>(sg.width() * 0.82), static_cast<int>(sg.height() * 0.82));
    }

    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    // ---- main layout --------------------------------------------------------
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addStretch(1);

    // ---- toolbar ------------------------------------------------------------
    m_toolbar = new QWidget(this);
    m_toolbar->setAutoFillBackground(true);
    m_toolbar->setStyleSheet(QStringLiteral(
        "background-color: rgba(26,26,26,210);"
    ));

    auto* tbLayout = new QHBoxLayout(m_toolbar);
    tbLayout->setContentsMargins(10, 6, 10, 6);
    tbLayout->setSpacing(6);

    auto makeBtn = [&](const QString& text, const QString& tooltip) -> QPushButton* {
        auto* btn = new QPushButton(text, m_toolbar);
        btn->setStyleSheet(kBtnStyle);
        btn->setToolTip(tooltip);
        btn->setFocusPolicy(Qt::NoFocus);
        return btn;
    };

    // Navigation group
    m_prevBtn = makeBtn(QStringLiteral("\u25C0"), QStringLiteral("Previous (Left Arrow)"));
    m_nextBtn = makeBtn(QStringLiteral("\u25B6"), QStringLiteral("Next (Right Arrow)"));
    tbLayout->addWidget(m_prevBtn);
    tbLayout->addWidget(m_nextBtn);
    tbLayout->addSpacing(12);

    // View group
    m_zoomOutBtn = makeBtn(QStringLiteral("\u2212"), QStringLiteral("Zoom Out"));
    m_zoomInBtn  = makeBtn(QStringLiteral("+"), QStringLiteral("Zoom In"));
    m_fitBtn     = makeBtn(QStringLiteral("Fit"), QStringLiteral("Fit to Window"));
    tbLayout->addWidget(m_zoomOutBtn);
    tbLayout->addWidget(m_zoomInBtn);
    tbLayout->addWidget(m_fitBtn);
    tbLayout->addSpacing(12);

    // Rotate group
    m_rotateLeftBtn  = makeBtn(QStringLiteral("\u21B6"), QStringLiteral("Rotate Left 90\u00B0"));
    m_rotateRightBtn = makeBtn(QStringLiteral("\u21B7"), QStringLiteral("Rotate Right 90\u00B0"));
    tbLayout->addWidget(m_rotateLeftBtn);
    tbLayout->addWidget(m_rotateRightBtn);
    tbLayout->addSpacing(12);

    // Status label (center part)
    m_statusLabel = new QLabel(m_toolbar);
    m_statusLabel->setStyleSheet(QStringLiteral("color: #cccccc; font-size: 12px; background: transparent;"));
    tbLayout->addWidget(m_statusLabel);
    tbLayout->addStretch(1);

    // Action group
    m_saveAsBtn        = makeBtn(QStringLiteral("Save As\u2026"), QStringLiteral("Save image to file"));
    m_copyBtn          = makeBtn(QStringLiteral("Copy"), QStringLiteral("Copy to clipboard"));
    m_openExternalBtn  = makeBtn(QStringLiteral("Open Externally"), QStringLiteral("Open in default application"));
    tbLayout->addWidget(m_saveAsBtn);
    tbLayout->addWidget(m_copyBtn);
    tbLayout->addWidget(m_openExternalBtn);

    m_mainLayout->addWidget(m_toolbar);

    // ---- overlay navigation arrows ------------------------------------------
    m_prevBtn = new QPushButton(QStringLiteral("\u25C0"), this);
    m_prevBtn->setStyleSheet(kNavBtnStyle);
    m_prevBtn->setFixedSize(44, 80);
    m_prevBtn->setFocusPolicy(Qt::NoFocus);
    m_prevBtn->setToolTip(QStringLiteral("Previous (Left Arrow)"));
    m_prevBtn->raise();

    m_nextBtn = new QPushButton(QStringLiteral("\u25B6"), this);
    m_nextBtn->setStyleSheet(kNavBtnStyle);
    m_nextBtn->setFixedSize(44, 80);
    m_nextBtn->setFocusPolicy(Qt::NoFocus);
    m_nextBtn->setToolTip(QStringLiteral("Next (Right Arrow)"));
    m_nextBtn->raise();

    // ---- close button -------------------------------------------------------
    m_closeBtn = new QPushButton(QStringLiteral("\u2715"), this);
    m_closeBtn->setStyleSheet(kCloseBtnStyle);
    m_closeBtn->setFixedSize(38, 38);
    m_closeBtn->setFocusPolicy(Qt::NoFocus);
    m_closeBtn->setToolTip(QStringLiteral("Close (Esc)"));
    m_closeBtn->raise();

    // ---- connections --------------------------------------------------------
    connect(m_prevBtn, &QPushButton::clicked, this, &ImageViewer::navigatePrevious);
    connect(m_nextBtn, &QPushButton::clicked, this, &ImageViewer::navigateNext);
    connect(m_zoomInBtn, &QPushButton::clicked, this, &ImageViewer::zoomIn);
    connect(m_zoomOutBtn, &QPushButton::clicked, this, &ImageViewer::zoomOut);
    connect(m_fitBtn, &QPushButton::clicked, this, &ImageViewer::fitToWindow);
    connect(m_rotateLeftBtn, &QPushButton::clicked, this, &ImageViewer::rotateLeft);
    connect(m_rotateRightBtn, &QPushButton::clicked, this, &ImageViewer::rotateRight);
    connect(m_saveAsBtn, &QPushButton::clicked, this, &ImageViewer::saveAs);
    connect(m_copyBtn, &QPushButton::clicked, this, &ImageViewer::copyToClipboard);
    connect(m_openExternalBtn, &QPushButton::clicked, this, &ImageViewer::openExternally);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::close);
}

// ---------------------------------------------------------------------------
// Image management
// ---------------------------------------------------------------------------

void ImageViewer::loadImage(int index) {
    if (index < 0 || index >= m_entries.size()) return;

    m_currentIndex = index;
    m_rotation = 0.0;
    m_panOffset = QPointF(0, 0);
    fitToWindow();

    updateStatusBar();
    updateNavigationButtons();
    update();
}

void ImageViewer::updateStatusBar() {
    if (m_currentIndex < 0 || m_currentIndex >= m_entries.size()) {
        m_statusLabel->setText(QString());
        return;
    }

    const auto& entry = m_entries[m_currentIndex];
    const QImage& img = entry.image;
    if (img.isNull()) {
        m_statusLabel->setText(QStringLiteral("No image"));
        return;
    }

    QStringList parts;
    parts << QStringLiteral("%1 / %2").arg(m_currentIndex + 1).arg(m_entries.size());
    parts << entry.sourceName;
    parts << QStringLiteral("%1\u00D7%2").arg(img.width()).arg(img.height());
    parts << formatFileSize(entry.fileSize);
    parts << QStringLiteral("Zoom: %1%").arg(static_cast<int>(m_zoomLevel));

    int rot = static_cast<int>(m_rotation) % 360;
    if (rot != 0) {
        parts << QStringLiteral("Rot: %1\u00B0").arg(rot);
    }

    m_statusLabel->setText(parts.join(QStringLiteral("  |  ")));
}

void ImageViewer::updateNavigationButtons() {
    bool hasMultiple = m_entries.size() > 1;
    m_prevBtn->setVisible(hasMultiple);
    m_nextBtn->setVisible(hasMultiple);
}

void ImageViewer::positionOverlayButtons() {
    int w = width();
    int h = height();
    int toolbarH = m_toolbar->height();

    m_closeBtn->move(w - 48, 10);
    m_prevBtn->move(10, (h - toolbarH) / 2 - 40);
    m_nextBtn->move(w - 54, (h - toolbarH) / 2 - 40);
}

// ---------------------------------------------------------------------------
// Geometry helpers
// ---------------------------------------------------------------------------

qreal ImageViewer::computeFitZoom() const {
    if (m_currentIndex < 0 || m_currentIndex >= m_entries.size()) return 100.0;

    const QImage& img = m_entries[m_currentIndex].image;
    if (img.isNull()) return 100.0;

    int imgW = img.width();
    int imgH = img.height();

    int rot = static_cast<int>(m_rotation) % 360;
    if (rot < 0) rot += 360;
    if (rot == 90 || rot == 270) {
        std::swap(imgW, imgH);
    }

    int availableH = height() - m_toolbar->height() - 20;
    int availableW = width() - 20;
    if (availableW <= 0 || availableH <= 0) return 100.0;

    qreal fitW = static_cast<qreal>(availableW) / imgW * 100.0;
    qreal fitH = static_cast<qreal>(availableH) / imgH * 100.0;
    return qMin(fitW, fitH);
}

QImage ImageViewer::displayedImage() const {
    if (m_currentIndex < 0 || m_currentIndex >= m_entries.size()) return {};
    const QImage& src = m_entries[m_currentIndex].image;
    if (src.isNull()) return {};

    int rot = static_cast<int>(m_rotation) % 360;
    if (rot < 0) rot += 360;
    if (rot == 0) return src;

    QTransform t;
    t.rotate(rot);
    return src.transformed(t, Qt::SmoothTransformation);
}

// ---------------------------------------------------------------------------
// Paint
// ---------------------------------------------------------------------------

void ImageViewer::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), QColor(0x1a, 0x1a, 0x1a));

    if (m_currentIndex < 0 || m_currentIndex >= m_entries.size()) return;

    const QImage& img = m_entries[m_currentIndex].image;
    if (img.isNull()) return;

    QPointF widgetCenter(width() / 2.0, (height() - m_toolbar->height()) / 2.0);
    QPointF imageCenter(img.width() / 2.0, img.height() / 2.0);

    painter.save();
    painter.translate(widgetCenter);
    painter.translate(m_panOffset);
    painter.scale(m_zoomLevel / 100.0, m_zoomLevel / 100.0);
    painter.rotate(m_rotation);
    painter.translate(-imageCenter);
    painter.drawImage(0, 0, img);
    painter.restore();
}

// ---------------------------------------------------------------------------
// Events
// ---------------------------------------------------------------------------

void ImageViewer::resizeEvent(QResizeEvent* event) {
    QDialog::resizeEvent(event);
    positionOverlayButtons();
    m_toolbarHeight = m_toolbar->height();
}

void ImageViewer::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    positionOverlayButtons();
    m_toolbarHeight = m_toolbar->height();

    setWindowOpacity(0.0);
    auto* anim = new QPropertyAnimation(this, "windowOpacity", this);
    anim->setDuration(280);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void ImageViewer::wheelEvent(QWheelEvent* event) {
    if (m_currentIndex < 0 || m_currentIndex >= m_entries.size()) {
        event->ignore();
        return;
    }

    qreal oldZoom = m_zoomLevel;
    qreal newZoom;

    if (event->angleDelta().y() > 0) {
        newZoom = qMin(oldZoom + 10.0, 500.0);
    } else {
        newZoom = qMax(oldZoom - 10.0, 10.0);
    }

    if (!qFuzzyCompare(newZoom, oldZoom)) {
        QPointF cursor = event->position();
        QPointF widgetCenter(width() / 2.0, (height() - m_toolbar->height()) / 2.0);

        qreal oldScale = oldZoom / 100.0;
        qreal newScale = newZoom / 100.0;

        QPointF rel = cursor - widgetCenter - m_panOffset;
        m_panOffset += (1.0 - newScale / oldScale) * rel;

        m_zoomLevel = newZoom;
        updateStatusBar();
        update();
    }

    event->accept();
}

void ImageViewer::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_isPanning = true;
        m_lastMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    QDialog::mousePressEvent(event);
}

void ImageViewer::mouseMoveEvent(QMouseEvent* event) {
    if (m_isPanning) {
        QPoint delta = event->pos() - m_lastMousePos;
        m_lastMousePos = event->pos();
        m_panOffset += QPointF(delta);
        update();
        event->accept();
        return;
    }
    QDialog::mouseMoveEvent(event);
}

void ImageViewer::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && m_isPanning) {
        m_isPanning = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }
    QDialog::mouseReleaseEvent(event);
}

void ImageViewer::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
    case Qt::Key_Escape:
        close();
        break;
    case Qt::Key_Left:
    case Qt::Key_A:
        navigatePrevious();
        break;
    case Qt::Key_Right:
    case Qt::Key_D:
        navigateNext();
        break;
    case Qt::Key_Plus:
    case Qt::Key_Equal:
        zoomIn();
        break;
    case Qt::Key_Minus:
        zoomOut();
        break;
    case Qt::Key_0:
    case Qt::Key_F:
        fitToWindow();
        break;
    case Qt::Key_R:
        rotateRight();
        break;
    case Qt::Key_L:
        rotateLeft();
        break;
    case Qt::Key_S:
        if (event->modifiers() & Qt::ControlModifier) {
            saveAs();
        }
        break;
    case Qt::Key_C:
        if (event->modifiers() & Qt::ControlModifier) {
            copyToClipboard();
        }
        break;
    default:
        QDialog::keyPressEvent(event);
        break;
    }
}

// ---------------------------------------------------------------------------
// Slots
// ---------------------------------------------------------------------------

void ImageViewer::zoomIn() {
    m_zoomLevel = qMin(m_zoomLevel + 10.0, 500.0);
    updateStatusBar();
    update();
}

void ImageViewer::zoomOut() {
    m_zoomLevel = qMax(m_zoomLevel - 10.0, 10.0);
    updateStatusBar();
    update();
}

void ImageViewer::fitToWindow() {
    m_zoomLevel = computeFitZoom();
    m_panOffset = QPointF(0, 0);
    updateStatusBar();
    update();
}

void ImageViewer::rotateLeft() {
    m_rotation = std::fmod(m_rotation - 90.0, 360.0);
    m_panOffset = QPointF(0, 0);
    m_zoomLevel = computeFitZoom();
    updateStatusBar();
    update();
}

void ImageViewer::rotateRight() {
    m_rotation = std::fmod(m_rotation + 90.0, 360.0);
    m_panOffset = QPointF(0, 0);
    m_zoomLevel = computeFitZoom();
    updateStatusBar();
    update();
}

void ImageViewer::navigatePrevious() {
    if (m_entries.isEmpty()) return;
    int idx = m_currentIndex - 1;
    if (idx < 0) idx = m_entries.size() - 1;
    loadImage(idx);
}

void ImageViewer::navigateNext() {
    if (m_entries.isEmpty()) return;
    int idx = m_currentIndex + 1;
    if (idx >= m_entries.size()) idx = 0;
    loadImage(idx);
}

void ImageViewer::saveAs() {
    if (m_currentIndex < 0 || m_currentIndex >= m_entries.size()) return;

    QString defaultName = m_entries[m_currentIndex].sourceName;
    if (defaultName.isEmpty()) defaultName = QStringLiteral("image.png");

    QString filter = QStringLiteral(
        "PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp);;All Files (*)"
    );

    QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Save Image As"),
                                                defaultName, filter);
    if (path.isEmpty()) return;

    QImage toSave = displayedImage();
    toSave.save(path);
}

void ImageViewer::copyToClipboard() {
    if (m_currentIndex < 0 || m_currentIndex >= m_entries.size()) return;

    QImage img = displayedImage();
    if (!img.isNull()) {
        QApplication::clipboard()->setImage(img);
    }
}

void ImageViewer::openExternally() {
    if (m_currentIndex < 0 || m_currentIndex >= m_entries.size()) return;

    const auto& entry = m_entries[m_currentIndex];

    if (entry.hasFilePath && QFileInfo::exists(entry.filePath)) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(entry.filePath));
        return;
    }

    QTemporaryFile* tmpFile = QTemporaryFile::createNativeFile(
        QDir::tempPath() + QStringLiteral("/imageviewer_XXXXXX.png"));
    if (!tmpFile) return;

    tmpFile->setAutoRemove(true);
    if (displayedImage().save(tmpFile, "PNG")) {
        tmpFile->flush();
        QDesktopServices::openUrl(QUrl::fromLocalFile(tmpFile->fileName()));
    }
    // The file will be removed when tmpFile is destroyed
    delete tmpFile;
}
