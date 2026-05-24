#pragma once

#include <QWidget>
#include <QVector>

#include "lemmy/lemmy_message.hpp"
#include "protocol/protocol_room.hpp"

class QListWidget;
class QListWidgetItem;
class QComboBox;
class QPushButton;
class QLabel;
class QTabBar;
class QStackedWidget;
class QProgressBar;

class LemmySession;

class LemmyHomeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LemmyHomeWidget(QWidget *parent = nullptr);
    ~LemmyHomeWidget() override;

    void setActiveInstance(const QString &instanceUrl);

signals:
    void postSelected(qint64 postId);
    void communitySelected(qint64 communityId);

public slots:
    void refreshPosts();
    void refreshCommunities();

private slots:
    void onInstanceChanged(int index);
    void onSortChanged(int index);
    void onTabChanged(int index);
    void onPostItemActivated(QListWidgetItem *item);
    void onCommunityItemActivated(QListWidgetItem *item);
    void onUpvoteClicked();
    void onDownvoteClicked();
    void onCommunityClicked();
    void onRefreshClicked();

private:
    void setupUi();
    void populateInstanceSelector();
    void populatePostsList();
    void populateCommunitiesList();
    void showLoading(bool loading, const QString &message, bool isPosts);
    void updateVoteButtons(const LemmyPost &post);

    QWidget *createPostCardWidget(const LemmyPost &post, int dataIndex);
    QWidget *createCommunityCardWidget(const ProtocolRoom &room, int dataIndex);

    enum ItemDataRole {
        PostIndexRole = Qt::UserRole + 1,
        CommunityIndexRole
    };

    // Top bar
    QComboBox *m_instanceSelector;
    QComboBox *m_sortSelector;
    QPushButton *m_refreshButton;

    // Tabs
    QTabBar *m_tabBar;
    QStackedWidget *m_stack;

    // Posts tab
    QListWidget *m_postsList;
    QProgressBar *m_postsLoading;
    QLabel *m_postsEmptyLabel;

    // Communities tab
    QListWidget *m_communitiesList;
    QProgressBar *m_communitiesLoading;
    QLabel *m_communitiesEmptyLabel;

    // Data
    QVector<LemmyPost> m_posts;
    QVector<ProtocolRoom> m_communities;
    QString m_activeInstanceUrl;
    QString m_currentSort;
    qint64 m_filterCommunityId;

    // Current vote target
    qint64 m_votePostId;

    LemmySession *m_currentSession;
};
