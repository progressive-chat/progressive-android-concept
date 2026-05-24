#pragma once

#include <QWidget>
#include <QString>

class AvatarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AvatarWidget(const QString &name = QString(),
                          const QString &avatarUrl = QString(),
                          int size = 40,
                          QWidget *parent = nullptr);

    void setName(const QString &name);
    void setSize(int size);
    void setImageUrl(const QString &avatarUrl);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    static QColor colorFromName(const QString &name);
    static QString initialsFromName(const QString &name);

    QString m_name;
    QString m_avatarUrl;
    int m_size;
};
