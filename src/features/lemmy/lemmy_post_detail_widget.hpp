#pragma once

#include <QWidget>
#include <QMap>
#include <QVector>
#include <QString>

#include "lemmy/lemmy_message.hpp"

class QVBoxLayout;
class QHBoxLayout;
class QScrollArea;
class QLabel;
class QPushButton;
class QTextEdit;
class QFrame;

class LemmyPostDetailWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LemmyPostDetailWidget(QWidget *parent = nullptr);
    ~LemmyPostDetailWidget() override;

public slots:
    void loadPost(qint64 postId, QString instanceUrl);
    void submitComment(qint64 parentId, QString content);

signals:
    void backRequested();

private slots:
    void onUpvotePost();
    void onDownvotePost();
    void onUpvoteComment(qint64 commentId);
    void onDownvoteComment(qint64 commentId);
    void onToggleCollapse(qint64 commentId);
    void onLoadMoreReplies(qint64 parentId, int page);

private:
    void setupUi();
    void buildPostHeader();
    void buildCommentTree();
    void clearComments();
    void addCommentsRecursive(qint64 parentId, int depth, QWidget *container, QVBoxLayout *layout);
    void updateVoteDisplay(QLabel *scoreLabel, QPushButton *upBtn, QPushButton *downBtn, int vote, qint64 score);
    void onReplyClicked(qint64 parentId);
    void onSubmitInlineReply(qint64 parentId, QTextEdit *input, QWidget *replyContainer);

    QWidget *createPostBodyWidget();
    QWidget *createPostMetaWidget();
    QWidget *createCommentWidget(const LemmyComment &comment);
    QWidget *createReplyInputWidget(qint64 parentId, QWidget *parentContainer);
    QWidget *createVoteWidget(qint64 score, bool isPost, qint64 id);
    QWidget *createUrlEmbedWidget();

    QString simpleMarkdownToHtml(const QString &text) const;
    QString formatTimestamp(const QString &published) const;
    QString timeAgo(const QDateTime &dt) const;
    QString avatarColorForName(const QString &name) const;
    QString initialsFromName(const QString &name) const;

    static constexpr int kIndentPerLevel = 20;

    static const QStringList kDepthColors;

    QWidget *m_headerWidget;
    QVBoxLayout *m_headerLayout;

    QScrollArea *m_commentScrollArea;
    QWidget *m_commentContainer;
    QVBoxLayout *m_commentLayout;

    QWidget *m_replyBar;
    QTextEdit *m_replyInput;
    QPushButton *m_submitButton;

    QPushButton *m_backButton;

    qint64 m_postId = 0;
    QString m_instanceUrl;
    LemmyPost m_currentPost;

    QMap<qint64, QVector<LemmyComment>> m_children;
    QVector<LemmyComment> m_rootComments;

    QMap<qint64, bool> m_collapsed;
    QMap<qint64, int> m_commentVotes;
    QMap<qint64, QWidget *> m_commentContainers;
    int m_postVote = 0;

    QLabel *m_postScoreLabel = nullptr;
    QPushButton *m_postUpBtn = nullptr;
    QPushButton *m_postDownBtn = nullptr;

    int m_commentPage = 1;
    int m_replyPage = 1;
};
