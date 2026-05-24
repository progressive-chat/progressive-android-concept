#include "json_util.hpp"

#include <QJsonParseError>

namespace JsonUtil {

QJsonDocument parse(const QString &json)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError)
        return {};
    return doc;
}

QJsonObject parseObject(const QString &json)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject())
        return {};
    return doc.object();
}

QString stringify(const QJsonObject &obj)
{
    return QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

QString stringify(const QJsonArray &arr)
{
    return QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact));
}

void merge(QJsonObject &target, const QJsonObject &source)
{
    if (source.isEmpty())
        return;

    for (auto it = source.constBegin(); it != source.constEnd(); ++it) {
        const QString &key = it.key();
        const QJsonValue val = it.value();

        if (val.isObject() && target.contains(key) && target.value(key).isObject()) {
            QJsonObject nestedTarget = target.value(key).toObject();
            QJsonObject nestedSource = val.toObject();
            merge(nestedTarget, nestedSource);
            target[key] = nestedTarget;
        } else {
            target[key] = val;
        }
    }
}

} // namespace JsonUtil
