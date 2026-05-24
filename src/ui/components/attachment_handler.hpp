#pragma once

#include <QObject>
#include <QImage>
#include <QString>
#include <QJsonObject>

#include <utility>

class QWidget;
class QNetworkAccessManager;

class AttachmentHandler : public QObject
{
    Q_OBJECT

public:
    static AttachmentHandler* instance();

    void setHomeserverUrl(const QString& url);
    void setAccessToken(const QString& token);

    void pickImage(QWidget* parent = nullptr);
    void pickFile(QWidget* parent = nullptr);
    void pickAudio(QWidget* parent = nullptr);
    void pickVideo(QWidget* parent = nullptr);
    void pickLocation(QWidget* parent = nullptr);

    void sendImage(const QString& roomId, const QString& filePath);
    void sendFile(const QString& roomId, const QString& filePath);
    void sendAudio(const QString& roomId, const QString& filePath);
    void sendVideo(const QString& roomId, const QString& filePath);

    QImage generateThumbnail(const QString& imagePath, int maxWidth = 400, int maxHeight = 300);
    static QString formatFileSize(qint64 bytes);
    static QString mimeTypeFromPath(const QString& path);

signals:
    void imageSelected(const QString& path, const QImage& image);
    void fileSelected(const QString& path, qint64 size);
    void uploadComplete(const QString& contentUri);
    void uploadFailed(const QString& error);
    void thumbnailReady(const QString& path, const QImage& thumb);

private:
    explicit AttachmentHandler(QObject* parent = nullptr);
    ~AttachmentHandler() override = default;
    AttachmentHandler(const AttachmentHandler&) = delete;
    AttachmentHandler& operator=(const AttachmentHandler&) = delete;

    void uploadFile(const QString& filePath,
                    const QString& roomId = {},
                    const QString& msgType = {});

    void sendMediaMessage(const QString& roomId,
                          const QString& contentUri,
                          const QString& msgType,
                          const QString& filename,
                          qint64 fileSize,
                          const QString& mimeType,
                          const QString& filePath);

    std::pair<int, int> imageDimensions(const QString& imagePath) const;
    QString generateTxnId() const;

    static AttachmentHandler* s_instance;

    QString m_homeserverUrl;
    QString m_accessToken;
    QNetworkAccessManager* m_networkManager = nullptr;
};
