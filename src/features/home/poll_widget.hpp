#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QScrollArea>
#include <QFrame>
#include <QRadioButton>
#include <QCheckBox>
#include <QButtonGroup>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QVector>
#include <QStringList>

struct PollData
{
    QString question;
    QStringList options;
    QVector<int> votes;
    bool isClosed = false;

    enum PollType { SingleChoice, MultipleChoice };
    PollType type = SingleChoice;

    QString pollEventId;
    QString roomId;
    QString creatorId;

    QJsonObject toJson() const;
    static PollData fromJson(const QJsonObject &json);
};

class PollWidget : public QWidget
{
    Q_OBJECT

public:
    enum Mode { Creator, Viewer };

    explicit PollWidget(Mode mode, QWidget *parent = nullptr);

    PollData pollData() const;
    void setPollData(const PollData &data);

    void setCurrentUserId(const QString &userId);
    void setUserVotes(const QVector<int> &votedOptions);
    void setPollClosed(bool closed);

    void setEditable(bool editable);
    bool isEditable() const;

signals:
    void pollCreated(const PollData &pollData);
    void voteSubmitted(int optionIndex);
    void pollEnded();

private slots:
    void addOption();
    void removeOption(int index);
    void onCreatePoll();
    void onSubmitVote(int optionIndex);
    void onEndPoll();

private:
    void setupCreatorMode();
    void setupViewerMode();
    void rebuildViewerOptions();
    void updateOptionRow(int index);
    QWidget *buildViewerOptionRow(int index);
    QWidget *buildCreatorOptionRow(int index);

    Mode m_mode;
    PollData m_pollData;
    QString m_currentUserId;
    QVector<int> m_userVotes;
    bool m_editable = true;

    // Creator UI
    QLineEdit *m_questionInput = nullptr;
    QWidget *m_creatorOptionsContainer = nullptr;
    QVBoxLayout *m_creatorOptionsLayout = nullptr;
    QPushButton *m_addOptionButton = nullptr;
    QComboBox *m_pollTypeCombo = nullptr;
    QPushButton *m_createPollButton = nullptr;
    QVector<QLineEdit *> m_optionInputs;
    QVector<QPushButton *> m_removeButtons;

    // Viewer UI
    QLabel *m_questionLabel = nullptr;
    QWidget *m_viewerOptionsContainer = nullptr;
    QVBoxLayout *m_viewerOptionsLayout = nullptr;
    QLabel *m_totalVotesLabel = nullptr;
    QPushButton *m_endPollButton = nullptr;
    QLabel *m_closedLabel = nullptr;
    QVector<QWidget *> m_viewerOptionRows;

    static constexpr int kMaxOptions = 20;
    static constexpr int kMinOptions = 2;
};
