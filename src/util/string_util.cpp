#include "string_util.hpp"

#include <QCryptographicHash>
#include <QUrl>
#include <QUuid>

namespace StringUtil {

bool isEmpty(const QString& str)
{
    return str.isEmpty();
}

bool isBlank(const QString& str)
{
    return str.trimmed().isEmpty();
}

QString trim(const QString& str)
{
    return str.trimmed();
}

QStringList split(const QString& str, QChar delimiter)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return str.split(delimiter, Qt::SkipEmptyParts);
#else
    return str.split(delimiter, QString::SkipEmptyParts);
#endif
}

QString join(const QStringList& list, const QString& separator)
{
    return list.join(separator);
}

bool contains(const QString& str, const QString& substring,
              Qt::CaseSensitivity cs)
{
    return str.contains(substring, cs);
}

bool startsWith(const QString& str, const QString& prefix)
{
    return str.startsWith(prefix);
}

QString md5(const QString& str)
{
    return QString::fromLatin1(
        QCryptographicHash::hash(str.toUtf8(), QCryptographicHash::Md5).toHex());
}

QString sha256(const QString& str)
{
    return QString::fromLatin1(
        QCryptographicHash::hash(str.toUtf8(), QCryptographicHash::Sha256).toHex());
}

QString base64Encode(const QByteArray& data)
{
    return QString::fromLatin1(data.toBase64());
}

QByteArray base64Decode(const QString& str)
{
    return QByteArray::fromBase64(str.toUtf8());
}

QString urlEncode(const QString& str)
{
    return QUrl::toPercentEncoding(str);
}

QString urlDecode(const QString& str)
{
    return QUrl::fromPercentEncoding(str.toUtf8());
}

QString ellipsize(const QString& str, int maxLength)
{
    if (maxLength <= 0 || str.length() <= maxLength)
        return str;

    if (maxLength <= 3)
        return QString(maxLength, QChar('.'));

    return str.left(maxLength - 3) + QStringLiteral("...");
}

QString generateUuid()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

} // namespace StringUtil
