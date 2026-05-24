#pragma once

#include <QWidget>
#include <QVector>
#include <QJsonObject>
#include <QDateTime>

class QTabBar;
class QStackedWidget;
class QScrollArea;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QLabel;
class QNetworkAccessManager;

struct MediaItem {
    QString mxcUri;
    QString fileName;
    QString senderId;
    QString senderName;
    QString mimeType;
    qint64 fileSize = 0;
    qint64 timestamp = 0;
    QJsonObject metadata;
    QByteArray thumbnailData;
};

class MediaGalleryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MediaGalleryWidget(const QString &roomId, QWidget *parent = nullptr);
    ~MediaGalleryWidget() override = default;

signals:
    void imageOpened(const QString &filePath);
    void fileOpened(const QString &filePath);

private slots:
    void onTabChanged(int index);
    void onLoadMore();
    void onImageThumbnailClicked(int mediaIndex);
    void onVideoThumbnailClicked(int mediaIndex);
    void onFileDownloadClicked(int mediaIndex);
    void onContextMenuRequested(const QPoint &pos, int mediaIndex, int tabIndex);
    void onLinksContextMenu(const QPoint &pos);

private:
    void setupUi();
    void loadMedia();
    void fetchMediaBatch(const QString &from = {});
    void processMediaEvents(const QJsonArray &events);
    void populateImagesTab();
    void populateVideosTab();
    void populateFilesTab();
    void populateLinksTab();
    void populateAudioTab();
    void downloadMedia(const QString &mxcUri, int mediaIndex, int tabIndex);
    void downloadAndShow(int mediaIndex, int tabIndex);
    QString resolveMxcToHttp(const QString &mxcUri) const;
    QWidget *createImageThumbnail(const MediaItem &item, int index);
    QWidget *createVideoThumbnail(const MediaItem &item, int index);
    void showEmptyState(QLayout *layout, QLabel *&label);
    QString formatFileSize(qint64 bytes) const;
    QString formatDate(qint64 ts) const;
    QString senderDisplayName(const MediaItem &item) const;
    QString iconForMimeType(const QString &mimeType) const;

    QString m_roomId;
    QString m_homeserverUrl;
    QString m_accessToken;
    QNetworkAccessManager *m_nam;

    QTabBar *m_tabBar;
    QStackedWidget *m_stack;

    // Tab pages
    QWidget *m_imagesPage;
    QWidget *m_videosPage;
    QWidget *m_filesPage;
    QWidget *m_linksPage;
    QWidget *m_audioPage;

    // Scroll areas
    QScrollArea *m_imagesScroll;
    QScrollArea *m_videosScroll;

    // Lists
    QListWidget *m_filesList;
    QListWidget *m_linksList;
    QListWidget *m_audioList;

    // Load more buttons
    QPushButton *m_loadMoreImages;
    QPushButton *m_loadMoreVideos;
    QPushButton *m_loadMoreFiles;
    QPushButton *m_loadMoreLinks;
    QPushButton *m_loadMoreAudio;

    // Empty state labels
    QLabel *m_imagesEmptyLabel;
    QLabel *m_videosEmptyLabel;
    QLabel *m_filesEmptyLabel;
    QLabel *m_linksEmptyLabel;
    QLabel *m_audioEmptyLabel;

    // Tab indices
    enum Tab {
        TAB_IMAGES = 0,
        TAB_VIDEOS,
        TAB_FILES,
        TAB_LINKS,
        TAB_AUDIO,
    };

    QVector<MediaItem> m_images;
    QVector<MediaItem> m_videos;
    QVector<MediaItem> m_files;
    QVector<MediaItem> m_links;
    QVector<MediaItem> m_audio;

    QString m_imagesNextBatch;
    QString m_videosNextBatch;
    QString m_filesNextBatch;
    QString m_linksNextBatch;
    QString m_audioNextBatch;

    bool m_imagesLoaded = false;
    bool m_videosLoaded = false;
    bool m_filesLoaded = false;
    bool m_linksLoaded = false;
    bool m_audioLoaded = false;

    int m_loadedCount = 0;
    static constexpr int kBatchSize = 50;
};
