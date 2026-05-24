#pragma once

#include <QByteArray>
#include <QChar>
#include <QString>
#include <QStringList>
#include <Qt>

namespace StringUtil {

bool isEmpty(const QString& str);

bool isBlank(const QString& str);

QString trim(const QString& str);

QStringList split(const QString& str, QChar delimiter);

QString join(const QStringList& list, const QString& separator);

bool contains(const QString& str, const QString& substring,
              Qt::CaseSensitivity cs = Qt::CaseInsensitive);

bool startsWith(const QString& str, const QString& prefix);

QString md5(const QString& str);

QString sha256(const QString& str);

QString base64Encode(const QByteArray& data);

QByteArray base64Decode(const QString& str);

QString urlEncode(const QString& str);

QString urlDecode(const QString& str);

QString ellipsize(const QString& str, int maxLength);

QString generateUuid();

} // namespace StringUtil
