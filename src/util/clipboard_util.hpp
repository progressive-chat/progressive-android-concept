#pragma once

#include <QString>
#include <QImage>
#include <QApplication>
#include <QClipboard>

namespace ClipboardUtil {

inline void copy(const QString &text)
{
    QApplication::clipboard()->setText(text);
}

inline QString paste()
{
    return QApplication::clipboard()->text();
}

inline void copyImage(const QImage &img)
{
    QApplication::clipboard()->setImage(img);
}

} // namespace ClipboardUtil
