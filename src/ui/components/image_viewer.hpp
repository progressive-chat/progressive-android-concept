#pragma once

#include <QDialog>
#include <QImage>
#include <QStringList>
#include <QPoint>
#include <QPointF>
#include <QList>

class QLabel;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;

class ImageViewer : public QDialog {
    Q_OBJECT

public:
    explicit ImageViewer(const QString& filePath, QWidget* parent = nullptr);
    explicit ImageViewer(const QImage& image, QWidget* parent = nullptr);
    ImageViewer(const QStringList& filePaths, int startIndex, QWidget* parent = nullptr);
    ImageViewer(const QList<QImage>& images, int startIndex, QWidget* parent = nullptr);

    ~ImageViewer() override = default;

    void addImage(const QString& filePath);
    void addImage(const QImage& image);
    void setCurrentIndex(int index);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void showEvent(QShowEvent* event) override;

private slots:
    void zoomIn();
    void zoomOut();
    void fitToWindow();
    void rotateLeft();
    void rotateRight();
    void navigatePrevious();
    void navigateNext();
    void saveAs();
    void copyToClipboard();
    void openExternally();

private:
    void init();
    void setupUi();
    void loadImage(int index);
    void updateStatusBar();
    void updateNavigationButtons();
    void positionOverlayButtons();
    qreal computeFitZoom() const;

    QImage displayedImage() const;

    struct ImageEntry {
        QImage image;
        QString sourceName;
        qint64 fileSize = -1;
        bool hasFilePath = false;
        QString filePath;
    };

    QList<ImageEntry> m_entries;
    int m_currentIndex = -1;

    qreal m_zoomLevel = 100.0;
    qreal m_rotation = 0.0;
    QPointF m_panOffset;
    bool m_isPanning = false;
    QPoint m_lastMousePos;

    int m_toolbarHeight = 0;

    QLabel* m_statusLabel = nullptr;
    QPushButton* m_prevBtn = nullptr;
    QPushButton* m_nextBtn = nullptr;
    QPushButton* m_zoomInBtn = nullptr;
    QPushButton* m_zoomOutBtn = nullptr;
    QPushButton* m_fitBtn = nullptr;
    QPushButton* m_rotateLeftBtn = nullptr;
    QPushButton* m_rotateRightBtn = nullptr;
    QPushButton* m_saveAsBtn = nullptr;
    QPushButton* m_copyBtn = nullptr;
    QPushButton* m_openExternalBtn = nullptr;
    QPushButton* m_closeBtn = nullptr;
    QWidget* m_toolbar = nullptr;
    QVBoxLayout* m_mainLayout = nullptr;
};
