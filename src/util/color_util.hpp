#pragma once

#include <QString>
#include <QColor>
#include <QList>
#include <QCryptographicHash>
#include <cmath>

namespace ColorUtil {

inline QColor fromHash(const QString &input, float saturation = 0.7f, float lightness = 0.5f)
{
    QByteArray hash = QCryptographicHash::hash(input.toUtf8(), QCryptographicHash::Sha256);
    int hue = (static_cast<unsigned char>(hash[0]) << 8) | static_cast<unsigned char>(hash[1]);
    hue = hue % 360;

    float s = qBound(0.0f, saturation, 1.0f);
    float l = qBound(0.0f, lightness, 1.0f);

    float c = (1.0f - std::abs(2.0f * l - 1.0f)) * s;
    float x = c * (1.0f - std::abs(std::fmod(hue / 60.0f, 2.0f) - 1.0f));
    float m = l - c / 2.0f;

    float r = 0, g = 0, b = 0;
    if (hue < 60)       { r = c; g = x; }
    else if (hue < 120) { r = x; g = c; }
    else if (hue < 180) { g = c; b = x; }
    else if (hue < 240) { g = x; b = c; }
    else if (hue < 300) { r = x; b = c; }
    else                { r = c; b = x; }

    return QColor(
        static_cast<int>((r + m) * 255),
        static_cast<int>((g + m) * 255),
        static_cast<int>((b + m) * 255)
    );
}

inline QColor fromHash(const QString &input, const QList<QColor> &palette)
{
    if (palette.isEmpty())
        return Qt::gray;

    QByteArray hash = QCryptographicHash::hash(input.toUtf8(), QCryptographicHash::Sha256);
    int index = static_cast<unsigned char>(hash[0]) % palette.size();
    return palette[index];
}

inline QString toHex(const QColor &c)
{
    return c.name().toUpper();
}

inline QColor withAlpha(const QColor &c, int alpha)
{
    QColor result(c);
    result.setAlpha(qBound(0, alpha, 255));
    return result;
}

inline QColor contrastText(const QColor &background)
{
    float luminance = 0.299f * background.redF()
                    + 0.587f * background.greenF()
                    + 0.114f * background.blueF();
    return luminance > 0.5f ? QColor(0, 0, 0) : QColor(255, 255, 255);
}

inline QColor lerp(const QColor &a, const QColor &b, float t)
{
    t = qBound(0.0f, t, 1.0f);
    return QColor(
        static_cast<int>(a.red()   + (b.red()   - a.red())   * t),
        static_cast<int>(a.green() + (b.green() - a.green()) * t),
        static_cast<int>(a.blue()  + (b.blue()  - a.blue())  * t),
        static_cast<int>(a.alpha() + (b.alpha() - a.alpha()) * t)
    );
}

} // namespace ColorUtil
