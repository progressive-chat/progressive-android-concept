#pragma once

#include <QWidget>
#include <QString>
#include <QStringList>
#include <QList>
#include <QPoint>

class QPushButton;
class QHBoxLayout;

class ReactionPicker : public QWidget
{
    Q_OBJECT

public:
    explicit ReactionPicker(QWidget* parent = nullptr);
    ~ReactionPicker() override;

    void showAt(const QPoint& globalPos);
    void setSelectedEmoji(const QString& emoji);
    void clearSelection();
    QString selectedEmoji() const;

signals:
    void reactionSelected(const QString& emoji);
    void moreRequested();

protected:
    void focusOutEvent(QFocusEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    void setupUi();
    QPushButton* createEmojiButton(const QString& emoji);
    QPushButton* createMoreButton();
    void onEmojiClicked(const QString& emoji);
    void updateButtonStyles();

    QHBoxLayout* m_layout = nullptr;
    QList<QPushButton*> m_emojiButtons;
    QPushButton* m_moreButton = nullptr;
    QString m_selectedEmoji;
    QStringList m_defaultEmojis;
};
