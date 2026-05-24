#pragma once

#include <QWidget>
#include <QString>

class QDoubleSpinBox;
class QLineEdit;
class QPushButton;
class QLabel;
class QComboBox;
class QStackedWidget;

class LocationShareWidget : public QWidget
{
    Q_OBJECT

public:
    struct PresetLocation {
        double latitude = 0.0;
        double longitude = 0.0;
        QString description;
        PresetLocation() = default;
        PresetLocation(double lat, double lng, const QString &desc = {})
            : latitude(lat), longitude(lng), description(desc) {}
    };

    explicit LocationShareWidget(QWidget *parent = nullptr);

    void setViewerMode(double lat, double lng,
                       const QString &description = QString());
    void setPickerMode();

signals:
    void locationShared(double lat, double lng, const QString &description);

private slots:
    void onShareClicked();
    void onPresetChanged(int index);
    void onCoordinateChanged();
    void onOpenInBrowser();
    void onCopyCoordinates();

private:
    void setupUi();
    void setupPickerPage();
    void setupViewerPage();
    void updateCoordinatePreview();
    void updatePresetCombo();
    void addRecentLocation(double lat, double lng,
                           const QString &description);

    class MapPlaceholder;

    // Picker widgets
    QDoubleSpinBox *m_latSpinBox   = nullptr;
    QDoubleSpinBox *m_lngSpinBox   = nullptr;
    QComboBox      *m_presetCombo  = nullptr;
    QLineEdit      *m_descriptionEdit = nullptr;
    QLabel         *m_coordPreview = nullptr;
    QPushButton    *m_shareButton  = nullptr;

    // Viewer widgets
    MapPlaceholder *m_mapPlaceholder     = nullptr;
    QLabel         *m_viewerCoordLabel   = nullptr;
    QLabel         *m_viewerDescLabel    = nullptr;
    QPushButton    *m_openBrowserButton  = nullptr;
    QPushButton    *m_copyButton         = nullptr;

    QStackedWidget *m_stack = nullptr;

    // Viewer state
    double  m_viewerLat  = 0.0;
    double  m_viewerLng  = 0.0;
    QString m_viewerDesc;

    // Recent locations
    QList<PresetLocation> m_recentLocations;
    bool m_updatingPreset = false;
};
