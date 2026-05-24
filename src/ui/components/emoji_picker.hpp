#pragma once

#include <QWidget>
#include <QTabWidget>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QLabel>
#include <QSettings>
#include <QMap>
#include <QStringList>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QTimer>

class EmojiPicker : public QWidget
{
    Q_OBJECT

public:
    explicit EmojiPicker(QWidget *parent = nullptr);
    ~EmojiPicker();

    void showAbove(QWidget *anchor);
    void showBelow(QWidget *anchor);

signals:
    void emojiSelected(const QString &emoji);

protected:
    void showEvent(QShowEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private slots:
    void onEmojiClicked();
    void onSearchTextChanged(const QString &text);
    void onCategoryChanged(int index);

private:
    void buildUi();
    void buildCategoryTabs();
    void populateTab(const QString &category, const QStringList &emojis);
    QWidget *createEmojiGrid(const QStringList &emojis, const QString &category);
    void rebuildFilteredTabs(const QString &filter);
    void addToRecent(const QString &emoji);
    void loadRecent();
    void saveRecent();
    void removeOldRecent();

    QLineEdit *m_searchBar;
    QTabWidget *m_tabWidget;
    QWidget *m_recentWidget;
    QGridLayout *m_recentGrid;

    QList<QPushButton *> m_allButtons;
    QStringList m_recent;
    QSettings m_settings;
    bool m_animated;

    static QMap<QString, QStringList> s_emojis;
};
