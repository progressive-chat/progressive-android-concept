#pragma once

#include <QWidget>
#include <QLabel>
#include <QVector>
#include <QPixmap>
#include <QFrame>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>

enum class MessageState {
    Sent,
    Delivered,
    Read
};

struct ReadReceiptEntry {
    QString userId;
    QString displayName;
    qint64 timestamp;
};

class ReadReceiptsWidget : public QWidget {
    Q_OBJECT
public:
    explicit ReadReceiptsWidget(QWidget* parent = nullptr);

    QSize sizeHint() const override { return QSize(200, 28); }
    QSize minimumSizeHint() const override { return QSize(60, 28); }

    void setDirectMessage(bool isDm);
    void setMessageState(MessageState state);
    void addReadReceipt(const QString& userId, const QString& displayName, qint64 timestamp);
    void clearReceipts();

    bool event(QEvent *e) override;

private slots:
    void onMoreClicked();
    void onStatusTextClicked();

private:
    void setupUi();
    void applyStyle();
    void updateDisplay();
    void rebuildAvatars();
    void showFullListPopup();
    QPixmap generateAvatar(const QString& displayName, int size);
    QString formatTimestamp(qint64 timestamp) const;
    QColor colorForName(const QString& name) const;

    bool m_isDm = false;
    MessageState m_messageState = MessageState::Sent;
    QVector<ReadReceiptEntry> m_receipts;

    QLabel* m_statusIcon = nullptr;
    QLabel* m_statusText = nullptr;
    QWidget* m_avatarsRow = nullptr;
    QHBoxLayout* m_avatarsLayout = nullptr;
    QLabel* m_moreLabel = nullptr;

    QFrame* m_fullListFrame = nullptr;
    bool m_fullListVisible = false;

    static constexpr int kAvatarSize = 18;
    static constexpr int kMaxVisibleAvatars = 5;
    static constexpr int kAvatarSpacing = 2;
};
