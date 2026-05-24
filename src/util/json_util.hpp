#pragma once

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

namespace JsonUtil {

inline QString optString(const QJsonObject &obj, const QString &key, const QString &defaultVal = {})
{
    if (obj.isEmpty())
        return defaultVal;
    const auto val = obj.value(key);
    if (val.isUndefined() || val.isNull())
        return defaultVal;
    return val.toString(defaultVal);
}

inline int optInt(const QJsonObject &obj, const QString &key, int defaultVal = 0)
{
    if (obj.isEmpty())
        return defaultVal;
    const auto val = obj.value(key);
    if (val.isUndefined() || val.isNull())
        return defaultVal;
    bool ok = false;
    int result = val.toVariant().toInt(&ok);
    return ok ? result : defaultVal;
}

inline qint64 optLong(const QJsonObject &obj, const QString &key, qint64 defaultVal = 0)
{
    if (obj.isEmpty())
        return defaultVal;
    const auto val = obj.value(key);
    if (val.isUndefined() || val.isNull())
        return defaultVal;
    bool ok = false;
    qint64 result = val.toVariant().toLongLong(&ok);
    return ok ? result : defaultVal;
}

inline bool optBool(const QJsonObject &obj, const QString &key, bool defaultVal = false)
{
    if (obj.isEmpty())
        return defaultVal;
    const auto val = obj.value(key);
    if (val.isUndefined() || val.isNull())
        return defaultVal;
    return val.toBool(defaultVal);
}

inline QJsonObject optObject(const QJsonObject &obj, const QString &key)
{
    if (obj.isEmpty())
        return {};
    const auto val = obj.value(key);
    if (val.isUndefined() || val.isNull() || !val.isObject())
        return {};
    return val.toObject();
}

inline QJsonArray optArray(const QJsonObject &obj, const QString &key)
{
    if (obj.isEmpty())
        return {};
    const auto val = obj.value(key);
    if (val.isUndefined() || val.isNull() || !val.isArray())
        return {};
    return val.toArray();
}

QJsonDocument parse(const QString &json);

QJsonObject parseObject(const QString &json);

QString stringify(const QJsonObject &obj);

QString stringify(const QJsonArray &arr);

void merge(QJsonObject &target, const QJsonObject &source);

} // namespace JsonUtil
