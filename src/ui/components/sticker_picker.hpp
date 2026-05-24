#pragma once

#include <QWidget>
#include <QTabWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QVector>
#include <QPair>
#include <QMap>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>

struct StickerPack {
    QString name;
    QVector<QPair<QString, QString>> stickers; // emoji, name
};

class StickerPicker : public QWidget {
    Q_OBJECT
public:
    explicit StickerPicker(QWidget* parent = nullptr);

    QSize sizeHint() const override { return QSize(360, 480); }

signals:
    void stickerSelected(const QString& stickerEmoji);

private slots:
    void onStickerClicked();
    void onSearchTextChanged(const QString& text);
    void onAddStickerPack();
    void onStickerContextMenu(const QPoint& pos);

private:
    void setupUi();
    void applyStyle();
    void loadFavorites();
    void saveFavorites();
    void populateTabs();
    void populateTab(int index, const QVector<QPair<QString, QString>>& stickers);
    QPushButton* createStickerButton(const QString& emoji, const QString& name);
    bool isFavorite(const QString& emoji) const;
    void toggleFavorite(const QString& emoji);
    void updateFavoriteTab();
    void filterStickers(const QString& filter);

    static QPixmap renderEmojiPixmap(const QString& emoji, int size);

    QTabWidget* m_tabWidget = nullptr;
    QLineEdit* m_searchBar = nullptr;
    QPushButton* m_addPackButton = nullptr;
    QSettings m_settings;
    QVector<QString> m_favorites;
    QVector<StickerPack> m_packs;

    struct TabContent {
        QGridLayout* gridLayout = nullptr;
        QWidget* container = nullptr;
        QVector<QPushButton*> buttons;
    };
    QMap<int, TabContent> m_tabContents;

    static constexpr int kGridColumns = 4;
    static constexpr int kStickerButtonSize = 72;
};
