#pragma once

#include <QWidget>
#include <QJsonArray>
#include <QJsonObject>
#include <QVector>
#include <QString>

class QLineEdit;
class QComboBox;
class QPushButton;
class QListWidget;
class QListWidgetItem;
class QProgressBar;
class QTimer;
class QLabel;

class LemmySession;

struct LemmySearchResult {
    enum Type { Post, Community, User, Comment, Unknown };
    Type type = Unknown;
    qint64 id = 0;
    QString title;
    QString subtitle;
    QString communityName;
    qint64 communityId = 0;
    QString userName;
    qint64 userId = 0;
    QString body;
    qint64 score = 0;
    qint64 commentCount = 0;
    qint64 subscriberCount = 0;
    QString avatarUrl;
    QString published;
};

class LemmySearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LemmySearchWidget(QWidget *parent = nullptr);
    ~LemmySearchWidget() override;

    void setInstanceUrl(const QString &url);

signals:
    void postSelected(qint64 postId);
    void communitySelected(qint64 communityId);
    void userSelected(qint64 userId);

private slots:
    void onSearchTriggered();
    void onSearchTextChanged(const QString &text);
    void onResultItemActivated(QListWidgetItem *item);
    void onLoadMoreClicked();

private:
    void setupUi();
    void performSearch(int page);
    void clearResults();
    void showLoading(bool loading);
    void setLoadMoreVisible(bool visible);

    QWidget *createPostResultWidget(const LemmySearchResult &res) const;
    QWidget *createCommunityResultWidget(const LemmySearchResult &res) const;
    QWidget *createUserResultWidget(const LemmySearchResult &res) const;
    QWidget *createCommentResultWidget(const LemmySearchResult &res) const;

    QVector<LemmySearchResult> parseSearchResponse(const QJsonObject &json) const;

    enum ItemDataRole {
        ResultIndexRole = Qt::UserRole + 1
    };

    QLineEdit *m_searchBar;
    QComboBox *m_typeCombo;
    QComboBox *m_sortCombo;
    QListWidget *m_resultsList;
    QPushButton *m_loadMoreButton;
    QProgressBar *m_loadingBar;
    QLabel *m_emptyLabel;
    QTimer *m_debounceTimer;

    QString m_instanceUrl;
    QString m_currentSearchQuery;
    QString m_currentSearchType;
    int m_currentPage = 1;
    bool m_hasMorePages = false;
    QVector<LemmySearchResult> m_results;
};
