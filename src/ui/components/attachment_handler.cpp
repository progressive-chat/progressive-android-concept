#include "attachment_handler.hpp"

#include <QFileDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>
#include <QImage>
#include <QImageReader>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QUuid>
#include <QUrl>

AttachmentHandler* AttachmentHandler::s_instance = nullptr;

AttachmentHandler* AttachmentHandler::instance()
{
    if (!s_instance)
        s_instance = new AttachmentHandler();
    return s_instance;
}

AttachmentHandler::AttachmentHandler(QObject* parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
}

void AttachmentHandler::setHomeserverUrl(const QString& url)
{
    m_homeserverUrl = url;
    while (m_homeserverUrl.endsWith(QLatin1Char('/')))
        m_homeserverUrl.chop(1);
}

void AttachmentHandler::setAccessToken(const QString& token)
{
    m_accessToken = token;
}

void AttachmentHandler::pickImage(QWidget* parent)
{
    const QString filePath = QFileDialog::getOpenFileName(
        parent,
        tr("Select Image"),
        QString(),
        tr("Images (*.png *.jpg *.jpeg *.gif *.bmp *.webp);;All Files (*)"));

    if (filePath.isEmpty())
        return;

    QImage image(filePath);
    if (image.isNull())
        return;

    emit imageSelected(filePath, image);
}

void AttachmentHandler::pickFile(QWidget* parent)
{
    const QString filePath = QFileDialog::getOpenFileName(parent, tr("Select File"));

    if (filePath.isEmpty())
        return;

    const QFileInfo info(filePath);
    emit fileSelected(filePath, info.size());
}

void AttachmentHandler::pickAudio(QWidget* parent)
{
    const QString filePath = QFileDialog::getOpenFileName(
        parent,
        tr("Select Audio"),
        QString(),
        tr("Audio (*.mp3 *.wav *.ogg *.flac *.aac *.m4a *.wma);;All Files (*)"));

    if (filePath.isEmpty())
        return;

    const QFileInfo info(filePath);
    emit fileSelected(filePath, info.size());
}

void AttachmentHandler::pickVideo(QWidget* parent)
{
    const QString filePath = QFileDialog::getOpenFileName(
        parent,
        tr("Select Video"),
        QString(),
        tr("Video (*.mp4 *.webm *.avi *.mov *.mkv *.wmv *.flv);;All Files (*)"));

    if (filePath.isEmpty())
        return;

    const QFileInfo info(filePath);
    emit fileSelected(filePath, info.size());
}

void AttachmentHandler::pickLocation(QWidget* parent)
{
    QDialog dialog(parent);
    dialog.setWindowTitle(tr("Pick Location"));
    dialog.setMinimumWidth(320);

    auto* layout = new QFormLayout(&dialog);

    auto* latSpin = new QDoubleSpinBox(&dialog);
    latSpin->setRange(-90.0, 90.0);
    latSpin->setDecimals(6);
    latSpin->setValue(0.0);
    latSpin->setPrefix(QStringLiteral("Lat: "));
    layout->addRow(tr("Latitude:"), latSpin);

    auto* lngSpin = new QDoubleSpinBox(&dialog);
    lngSpin->setRange(-180.0, 180.0);
    lngSpin->setDecimals(6);
    lngSpin->setValue(0.0);
    lngSpin->setPrefix(QStringLiteral("Lng: "));
    layout->addRow(tr("Longitude:"), lngSpin);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addRow(buttons);

    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    dialog.exec();
}

void AttachmentHandler::sendImage(const QString& roomId, const QString& filePath)
{
    if (!QFile::exists(filePath)) {
        emit uploadFailed(tr("File not found: %1").arg(filePath));
        return;
    }

    uploadFile(filePath, roomId, QStringLiteral("m.image"));
}

void AttachmentHandler::sendFile(const QString& roomId, const QString& filePath)
{
    if (!QFile::exists(filePath)) {
        emit uploadFailed(tr("File not found: %1").arg(filePath));
        return;
    }

    uploadFile(filePath, roomId, QStringLiteral("m.file"));
}

void AttachmentHandler::sendAudio(const QString& roomId, const QString& filePath)
{
    if (!QFile::exists(filePath)) {
        emit uploadFailed(tr("File not found: %1").arg(filePath));
        return;
    }

    uploadFile(filePath, roomId, QStringLiteral("m.audio"));
}

void AttachmentHandler::sendVideo(const QString& roomId, const QString& filePath)
{
    if (!QFile::exists(filePath)) {
        emit uploadFailed(tr("File not found: %1").arg(filePath));
        return;
    }

    uploadFile(filePath, roomId, QStringLiteral("m.video"));
}

QImage AttachmentHandler::generateThumbnail(const QString& imagePath, int maxWidth, int maxHeight)
{
    QImageReader reader(imagePath);
    reader.setAutoTransform(true);

    const QSize originalSize = reader.size();
    if (!originalSize.isValid()) {
        return {};
    }

    const QSize scaledSize = originalSize.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio);
    reader.setScaledSize(scaledSize);

    QImage thumb = reader.read();
    if (thumb.isNull()) {
        return {};
    }

    emit thumbnailReady(imagePath, thumb);
    return thumb;
}

QString AttachmentHandler::formatFileSize(qint64 bytes)
{
    if (bytes < 0)
        return QStringLiteral("0 B");

    constexpr double kb = 1024.0;
    constexpr double mb = kb * 1024.0;
    constexpr double gb = mb * 1024.0;

    if (bytes < static_cast<qint64>(kb))
        return QStringLiteral("%1 B").arg(bytes);
    if (bytes < static_cast<qint64>(mb))
        return QStringLiteral("%1 KB").arg(bytes / kb, 0, 'f', 1);
    if (bytes < static_cast<qint64>(gb))
        return QStringLiteral("%1 MB").arg(bytes / mb, 0, 'f', 1);
    return QStringLiteral("%1 GB").arg(bytes / gb, 0, 'f', 2);
}

QString AttachmentHandler::mimeTypeFromPath(const QString& path)
{
    QMimeDatabase db;
    const QMimeType mime = db.mimeTypeForFile(path);
    return mime.name();
}

void AttachmentHandler::uploadFile(const QString& filePath,
                                    const QString& roomId,
                                    const QString& msgType)
{
    if (m_homeserverUrl.isEmpty() || m_accessToken.isEmpty()) {
        emit uploadFailed(tr("Homeserver URL or access token not configured"));
        return;
    }

    const QFileInfo fileInfo(filePath);
    const QString mimeType = mimeTypeFromPath(filePath);

    auto* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(mimeType));
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
        QVariant(QStringLiteral("form-data; name=\"file\"; filename=\"%1\"")
                     .arg(fileInfo.fileName())));

    auto* file = new QFile(filePath);
    if (!file->open(QIODevice::ReadOnly)) {
        emit uploadFailed(tr("Cannot open file: %1").arg(filePath));
        delete file;
        delete multiPart;
        return;
    }

    filePart.setBodyDevice(file);
    file->setParent(multiPart);
    multiPart->append(filePart);

    const QUrl url(m_homeserverUrl + QStringLiteral("/_matrix/media/v3/upload"));
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", ("Bearer " + m_accessToken).toUtf8());

    QNetworkReply* reply = m_networkManager->post(request, multiPart);
    multiPart->setParent(reply);

    reply->setProperty("roomId", roomId);
    reply->setProperty("msgType", msgType);
    reply->setProperty("filename", fileInfo.fileName());
    reply->setProperty("fileSize", fileInfo.size());
    reply->setProperty("mimeType", mimeType);
    reply->setProperty("filePath", filePath);

    QObject::connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            emit uploadFailed(reply->errorString());
            return;
        }

        const QByteArray data = reply->readAll();
        const QJsonDocument doc = QJsonDocument::fromJson(data);
        const QJsonObject obj = doc.object();

        const QString contentUri = obj.value(QStringLiteral("content_uri")).toString();
        if (contentUri.isEmpty()) {
            emit uploadFailed(tr("Invalid response from server: no content_uri"));
            return;
        }

        emit uploadComplete(contentUri);

        const QString targetRoomId = reply->property("roomId").toString();
        if (!targetRoomId.isEmpty()) {
            const QString targetMsgType = reply->property("msgType").toString();
            const QString filename     = reply->property("filename").toString();
            const qint64 fileSize      = reply->property("fileSize").toLongLong();
            const QString mime         = reply->property("mimeType").toString();
            const QString sourcePath   = reply->property("filePath").toString();

            sendMediaMessage(targetRoomId, contentUri,
                             targetMsgType, filename,
                             fileSize, mime, sourcePath);
        }
    });
}

void AttachmentHandler::sendMediaMessage(const QString& roomId,
                                          const QString& contentUri,
                                          const QString& msgType,
                                          const QString& filename,
                                          qint64 fileSize,
                                          const QString& mimeType,
                                          const QString& filePath)
{
    const QString txnId = generateTxnId();

    QJsonObject info;
    info[QStringLiteral("size")]     = fileSize;
    info[QStringLiteral("mimetype")] = mimeType;

    if (msgType == QLatin1String("m.image") && !filePath.isEmpty()) {
        const auto [w, h] = imageDimensions(filePath);
        if (w > 0 && h > 0) {
            info[QStringLiteral("w")] = w;
            info[QStringLiteral("h")] = h;
        }

        const QImage thumb = generateThumbnail(filePath);
        if (!thumb.isNull()) {
            info[QStringLiteral("thumbnail_info")] = QJsonObject{
                {QStringLiteral("w"),        thumb.width()},
                {QStringLiteral("h"),        thumb.height()},
                {QStringLiteral("mimetype"), QStringLiteral("image/jpeg")},
                {QStringLiteral("size"),     static_cast<qint64>(0)}
            };
        }
    }

    QJsonObject content;
    content[QStringLiteral("msgtype")] = msgType;
    content[QStringLiteral("body")]    = filename;
    content[QStringLiteral("url")]     = contentUri;
    content[QStringLiteral("info")]    = info;

    const QJsonDocument doc(content);
    const QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    const QUrl url(m_homeserverUrl
                   + QStringLiteral("/_matrix/client/v3/rooms/%1/send/m.room.message/%2")
                         .arg(roomId, txnId));

    QNetworkRequest request(url);
    request.setRawHeader("Authorization", ("Bearer " + m_accessToken).toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("application/json"));

    QNetworkReply* reply = m_networkManager->put(request, jsonData);

    QObject::connect(reply, &QNetworkReply::finished, this, [reply]() {
        reply->deleteLater();
    });
}

std::pair<int, int> AttachmentHandler::imageDimensions(const QString& imagePath) const
{
    QImageReader reader(imagePath);
    reader.setAutoTransform(true);
    const QSize size = reader.size();
    if (size.isValid())
        return {size.width(), size.height()};
    return {0, 0};
}

QString AttachmentHandler::generateTxnId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}
