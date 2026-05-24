#pragma once

#include <QString>
#include <QDateTime>
#include <QDate>
#include <QTime>

namespace DateUtil {

inline QString relativeTime(const QDateTime &dt)
{
    QDateTime now = QDateTime::currentDateTimeUtc();
    qint64 secs = dt.secsTo(now);

    if (secs < 0)
        return "in the future";
    if (secs < 60)
        return "just now";
    if (secs < 3600) {
        int mins = static_cast<int>(secs / 60);
        return QString::number(mins) + "m ago";
    }
    if (secs < 86400) {
        int hours = static_cast<int>(secs / 3600);
        return QString::number(hours) + "h ago";
    }
    if (secs < 172800) {
        return "yesterday";
    }
    if (secs < 604800) {
        int days = static_cast<int>(secs / 86400);
        return QString::number(days) + "d ago";
    }
    return dt.toLocalTime().toString("MMM d");
}

inline QString formatTimestamp(qint64 ms)
{
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(ms, Qt::UTC);
    return dt.toLocalTime().toString("MMM d, yyyy  hh:mm AP");
}

inline QString formatDate(const QDate &d)
{
    return d.toString("MMM d, yyyy");
}

inline QString formatTime(const QTime &t)
{
    return t.toString("hh:mm");
}

inline bool isToday(const QDate &d)
{
    return d == QDate::currentDate();
}

inline bool isYesterday(const QDate &d)
{
    return d == QDate::currentDate().addDays(-1);
}

inline bool isThisWeek(const QDate &d)
{
    QDate today = QDate::currentDate();
    int dayOfWeek = today.dayOfWeek();
    QDate startOfWeek = today.addDays(-(dayOfWeek - 1));
    return d >= startOfWeek && d <= today;
}

} // namespace DateUtil
