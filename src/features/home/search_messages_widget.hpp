#pragma once

#include <QWidget>
#include <QVector>
#include <QString>
#include <optional>

#include "protocol/protocol_message.hpp"
#include "protocol/protocol_room.hpp"
#include "protocol/protocol_type.hpp"

class QLineEdit;
class QComboBox;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QLabel;
class QTimer;
class QShortcut;
class LoadingSpinner;

class SearchMessagesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SearchMessagesWidget(QWidget *parent = nullptr);
    ~SearchMessagesWidget() override;

signals:
    void resultSelected(const QString &roomId, const QString &messageId);

public slots:
    void openAndFocus();

private slots:
    void onSearchTextChanged();
    void onSearchReturnPressed();
    void onDebounceTimeout();
    void onProtocolFilterChanged(int index);
    void onRoomFilterChanged(int index);
    void onResultClicked(QListWidgetItem *item);
    void onLoadMoreClicked();
    void onRecentSearchClicked(QListWidgetItem *item);

private:
    struct SearchResult {
        ProtocolMessage message;
        QString roomName;
    };

    void setupUi();
    void triggerSearch();
    void performSearch();
    void displayResultsPage();
    void clearResults();
    void updateRoomFilter();
    void showNoResultsPlaceholder(bool show);
    void showSearchingIndicator(bool show);
    void showRecentSearchesPanel(bool show);
    void refreshRecentSearchesList();
    void saveRecentSearch(const QString &query);
    QStringList loadRecentSearches() const;
    QWidget *createResultItemWidget(const SearchResult &result);
    QString highlightMatch(const QString &text) const;
    static QColor protocolColor(ProtocolType type);
    static QString protocolName(ProtocolType type);
    static QString formatTimestamp(qint64 timestampMs);
    static QPixmap makeAvatarPixmap(const QString &name, int size, const QColor &bgColor);
    static QString initialsFromName(const QString &name);

    enum ItemDataRole {
        RoomIdRole = Qt::UserRole + 1,
        MessageIdRole
    };

    static constexpr int kPageSize = 20;
    static constexpr int kDebounceMs = 500;
    static constexpr int kMaxRecentSearches = 10;
    static constexpr int kMessagesPerRoom = 200;

    QLineEdit *m_searchEdit = nullptr;
    QComboBox *m_protocolFilter = nullptr;
    QComboBox *m_roomFilter = nullptr;
    QListWidget *m_resultsList = nullptr;
    QPushButton *m_loadMoreButton = nullptr;
    QLabel *m_noResultsLabel = nullptr;
    QWidget *m_searchingContainer = nullptr;
    LoadingSpinner *m_spinner = nullptr;
    QLabel *m_searchingLabel = nullptr;
    QLabel *m_recentLabel = nullptr;
    QListWidget *m_recentList = nullptr;
    QTimer *m_debounceTimer = nullptr;
    QShortcut *m_shortcut = nullptr;

    QVector<SearchResult> m_allResults;
    QVector<QString> m_roomFilterIds;
    int m_displayedCount = 0;
    std::optional<ProtocolType> m_currentProtocolFilter;
    QString m_currentRoomFilter;
    QString m_currentQuery;
};
