#include "features/home/poll_widget.hpp"

#include <QProgressBar>
#include <QScrollArea>
#include <QStyle>
#include <QtMath>

PollData PollData::fromJson(const QJsonObject &json)
{
    PollData data;
    data.question = json.value(QStringLiteral("question")).toString();
    data.pollEventId = json.value(QStringLiteral("poll_event_id")).toString();
    data.roomId = json.value(QStringLiteral("room_id")).toString();
    data.creatorId = json.value(QStringLiteral("creator_id")).toString();
    data.isClosed = json.value(QStringLiteral("is_closed")).toBool(false);

    QString typeStr = json.value(QStringLiteral("poll_type")).toString(
        QStringLiteral("single"));
    data.type = (typeStr == QStringLiteral("multi"))
                    ? PollData::MultipleChoice
                    : PollData::SingleChoice;

    QJsonArray opts = json.value(QStringLiteral("options")).toArray();
    for (const auto &o : opts)
        data.options.append(o.toString());

    QJsonArray vArr = json.value(QStringLiteral("votes")).toArray();
    for (const auto &v : vArr)
        data.votes.append(v.toInt(0));

    // Ensure votes vector matches options count
    while (data.votes.size() < data.options.size())
        data.votes.append(0);

    return data;
}

QJsonObject PollData::toJson() const
{
    QJsonObject json;
    json[QStringLiteral("question")] = question;
    json[QStringLiteral("poll_event_id")] = pollEventId;
    json[QStringLiteral("room_id")] = roomId;
    json[QStringLiteral("creator_id")] = creatorId;
    json[QStringLiteral("is_closed")] = isClosed;
    json[QStringLiteral("poll_type")] =
        (type == MultipleChoice) ? QStringLiteral("multi")
                                 : QStringLiteral("single");

    QJsonArray opts;
    for (const auto &o : options)
        opts.append(o);
    json[QStringLiteral("options")] = opts;

    QJsonArray vArr;
    for (int v : votes)
        vArr.append(v);
    json[QStringLiteral("votes")] = vArr;

    return json;
}

// ---------------------------------------------------------------------------
// PollWidget
// ---------------------------------------------------------------------------

PollWidget::PollWidget(Mode mode, QWidget *parent)
    : QWidget(parent)
    , m_mode(mode)
{
    if (mode == Creator) {
        m_pollData.options.append(QString());
        m_pollData.options.append(QString());
        m_pollData.votes.append(0);
        m_pollData.votes.append(0);
        setupCreatorMode();
    } else {
        setupViewerMode();
    }
}

PollData PollWidget::pollData() const
{
    if (m_mode == Creator) {
        PollData data = m_pollData;
        data.question = m_questionInput->text().trimmed();
        data.options.clear();
        for (auto *input : m_optionInputs)
            data.options.append(input->text().trimmed());
        data.type = (m_pollTypeCombo->currentIndex() == 0)
                        ? PollData::SingleChoice
                        : PollData::MultipleChoice;
        data.votes.clear();
        for (int i = 0; i < data.options.size(); ++i)
            data.votes.append(0);
        return data;
    }
    return m_pollData;
}

void PollWidget::setPollData(const PollData &data)
{
    m_pollData = data;
    if (m_mode == Viewer)
        rebuildViewerOptions();
}

void PollWidget::setCurrentUserId(const QString &userId)
{
    m_currentUserId = userId;
    if (m_mode == Viewer)
        rebuildViewerOptions();
}

void PollWidget::setUserVotes(const QVector<int> &votedOptions)
{
    m_userVotes = votedOptions;
    if (m_mode == Viewer)
        rebuildViewerOptions();
}

void PollWidget::setPollClosed(bool closed)
{
    m_pollData.isClosed = closed;
    if (m_mode == Viewer)
        rebuildViewerOptions();
}

void PollWidget::setEditable(bool editable)
{
    m_editable = editable;
}

bool PollWidget::isEditable() const
{
    return m_editable;
}

// ---------------------------------------------------------------------------
// Creator mode
// ---------------------------------------------------------------------------

QWidget *PollWidget::buildCreatorOptionRow(int index)
{
    auto *row = new QWidget(m_creatorOptionsContainer);
    auto *rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(6);

    auto *input = new QLineEdit(row);
    input->setPlaceholderText(QStringLiteral("Option %1").arg(index + 1));
    input->setStyleSheet(QStringLiteral(
        "QLineEdit { border: 1px solid #ccc; border-radius: 4px; "
        "padding: 6px 8px; font-size: 13px; }"
        "QLineEdit:focus { border-color: #0084FF; }"));
    m_optionInputs.append(input);

    auto *removeBtn = new QPushButton(QStringLiteral("\u2715"), row);
    removeBtn->setFixedSize(28, 28);
    removeBtn->setStyleSheet(QStringLiteral(
        "QPushButton { color: #E74C3C; border: none; font-size: 14px; "
        "border-radius: 4px; }"
        "QPushButton:hover { background: #FDE8E8; }"));
    removeBtn->setToolTip(QStringLiteral("Remove option"));

    int idx = index;
    connect(removeBtn, &QPushButton::clicked, this, [this, idx]() {
        removeOption(idx);
    });
    m_removeButtons.append(removeBtn);

    rowLayout->addWidget(input, 1);
    rowLayout->addWidget(removeBtn);

    return row;
}

void PollWidget::setupCreatorMode()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(10);

    // Title
    auto *titleLabel = new QLabel(QStringLiteral("Create Poll"), this);
    titleLabel->setStyleSheet(QStringLiteral(
        "QLabel { font-size: 16px; font-weight: bold; color: #333; }"));
    mainLayout->addWidget(titleLabel);

    // Question input
    m_questionInput = new QLineEdit(this);
    m_questionInput->setPlaceholderText(QStringLiteral("Ask a question..."));
    m_questionInput->setStyleSheet(QStringLiteral(
        "QLineEdit { border: 1px solid #ccc; border-radius: 6px; "
        "padding: 8px 10px; font-size: 14px; }"
        "QLineEdit:focus { border-color: #0084FF; }"));
    mainLayout->addWidget(m_questionInput);

    // Options label
    auto *optionsLabel = new QLabel(QStringLiteral("Options"), this);
    optionsLabel->setStyleSheet(QStringLiteral(
        "QLabel { font-size: 12px; color: #888; font-weight: bold; "
        "margin-top: 4px; }"));
    mainLayout->addWidget(optionsLabel);

    // Options container (scrollable if many)
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setMaximumHeight(220);
    scroll->setStyleSheet(QStringLiteral("QScrollArea { border: none; }"));

    m_creatorOptionsContainer = new QWidget(scroll);
    m_creatorOptionsLayout = new QVBoxLayout(m_creatorOptionsContainer);
    m_creatorOptionsLayout->setContentsMargins(0, 0, 0, 0);
    m_creatorOptionsLayout->setSpacing(6);

    for (int i = 0; i < 2; ++i) {
        m_creatorOptionsLayout->addWidget(buildCreatorOptionRow(i));
    }

    scroll->setWidget(m_creatorOptionsContainer);
    mainLayout->addWidget(scroll);

    // Add option button
    m_addOptionButton = new QPushButton(QStringLiteral("+ Add Option"), this);
    m_addOptionButton->setFlat(true);
    m_addOptionButton->setStyleSheet(QStringLiteral(
        "QPushButton { color: #0084FF; font-size: 13px; border: 1px dashed #0084FF; "
        "border-radius: 4px; padding: 5px 12px; }"
        "QPushButton:hover { background: #E8F0FE; }"));
    connect(m_addOptionButton, &QPushButton::clicked,
            this, &PollWidget::addOption);
    mainLayout->addWidget(m_addOptionButton);

    // Poll type selector
    auto *typeRow = new QWidget(this);
    auto *typeLayout = new QHBoxLayout(typeRow);
    typeLayout->setContentsMargins(0, 0, 0, 0);
    typeLayout->setSpacing(8);

    auto *typeLabel = new QLabel(QStringLiteral("Poll type:"), typeRow);
    typeLabel->setStyleSheet(QStringLiteral(
        "QLabel { font-size: 12px; color: #555; }"));

    m_pollTypeCombo = new QComboBox(typeRow);
    m_pollTypeCombo->addItem(QStringLiteral("Single choice (radio)"));
    m_pollTypeCombo->addItem(QStringLiteral("Multiple choice (checkbox)"));
    m_pollTypeCombo->setStyleSheet(QStringLiteral(
        "QComboBox { border: 1px solid #ccc; border-radius: 4px; "
        "padding: 5px 8px; font-size: 12px; }"));

    typeLayout->addWidget(typeLabel);
    typeLayout->addWidget(m_pollTypeCombo, 1);
    mainLayout->addWidget(typeRow);

    mainLayout->addStretch();

    // Create button
    m_createPollButton = new QPushButton(QStringLiteral("Create Poll"), this);
    m_createPollButton->setStyleSheet(QStringLiteral(
        "QPushButton { background: #0084FF; color: white; border: none; "
        "border-radius: 6px; padding: 10px 24px; font-size: 14px; font-weight: bold; }"
        "QPushButton:hover { background: #0073E6; }"
        "QPushButton:disabled { background: #B0D0F0; }"));
    connect(m_createPollButton, &QPushButton::clicked,
            this, &PollWidget::onCreatePoll);
    mainLayout->addWidget(m_createPollButton);
}

void PollWidget::addOption()
{
    if (m_optionInputs.size() >= kMaxOptions)
        return;

    int index = m_optionInputs.size();
    m_creatorOptionsLayout->addWidget(buildCreatorOptionRow(index));
    m_addOptionButton->setEnabled(m_optionInputs.size() < kMaxOptions);
}

void PollWidget::removeOption(int index)
{
    if (m_optionInputs.size() <= kMinOptions)
        return;

    if (index < 0 || index >= m_optionInputs.size())
        return;

    QWidget *row = m_optionInputs[index]->parentWidget();
    m_creatorOptionsLayout->removeWidget(row);
    delete row;

    m_optionInputs.removeAt(index);
    m_removeButtons.removeAt(index);

    // Reconnect remaining remove buttons with correct indices
    for (int i = 0; i < m_removeButtons.size(); ++i) {
        m_removeButtons[i]->disconnect();
        int idx = i;
        connect(m_removeButtons[i], &QPushButton::clicked, this, [this, idx]() {
            removeOption(idx);
        });
    }

    m_addOptionButton->setEnabled(m_optionInputs.size() < kMaxOptions);
}

void PollWidget::onCreatePoll()
{
    QString question = m_questionInput->text().trimmed();
    if (question.isEmpty())
        return;

    QStringList opts;
    for (auto *input : m_optionInputs) {
        QString text = input->text().trimmed();
        if (!text.isEmpty())
            opts.append(text);
    }

    if (opts.size() < kMinOptions)
        return;

    PollData data;
    data.question = question;
    data.options = opts;
    data.type = (m_pollTypeCombo->currentIndex() == 0)
                    ? PollData::SingleChoice
                    : PollData::MultipleChoice;
    data.isClosed = false;
    data.votes = QVector<int>(opts.size(), 0);

    emit pollCreated(data);
}

// ---------------------------------------------------------------------------
// Viewer mode
// ---------------------------------------------------------------------------

QWidget *PollWidget::buildViewerOptionRow(int index)
{
    if (index < 0 || index >= m_pollData.options.size())
        return new QWidget(this);

    const QString &optionText = m_pollData.options.at(index);
    int voteCount = (index < m_pollData.votes.size())
                        ? m_pollData.votes.at(index)
                        : 0;

    int totalVotes = 0;
    for (int v : m_pollData.votes)
        totalVotes += v;

    int percentage = (totalVotes > 0)
                         ? qRound(static_cast<double>(voteCount) * 100.0 / totalVotes)
                         : 0;

    bool userVotedForThis = m_userVotes.contains(index);
    bool hasUserVoted = !m_userVotes.isEmpty();

    auto *row = new QWidget(m_viewerOptionsContainer);
    row->setStyleSheet(userVotedForThis
                           ? QStringLiteral("QWidget { background: #E8F0FE; border-radius: 6px; }")
                           : QString());

    auto *rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(8, 6, 8, 6);
    rowLayout->setSpacing(8);

    // Vote button / indicator
    if (!m_pollData.isClosed && m_editable) {
        auto *voteBtn = new QPushButton(row);
        voteBtn->setFixedSize(32, 32);
        if (m_pollData.type == PollData::SingleChoice) {
            voteBtn->setCheckable(true);
            voteBtn->setChecked(userVotedForThis);
        }
        voteBtn->setText(userVotedForThis ? QStringLiteral("\u2713") : QString());
        voteBtn->setStyleSheet(QStringLiteral(
            "QPushButton { border: 2px solid #ccc; border-radius: 16px; "
            "font-size: 14px; color: white; background: white; }"
            "QPushButton:checked { background: #0084FF; border-color: #0084FF; }"
            "QPushButton:hover { border-color: #0084FF; }"));

        int optIndex = index;
        connect(voteBtn, &QPushButton::clicked, this, [this, optIndex]() {
            onSubmitVote(optIndex);
        });
        rowLayout->addWidget(voteBtn);
    } else if (userVotedForThis) {
        auto *checkLabel = new QLabel(QStringLiteral("\u2713"), row);
        checkLabel->setStyleSheet(QStringLiteral(
            "QLabel { color: #0084FF; font-size: 16px; font-weight: bold; }"));
        checkLabel->setFixedWidth(24);
        checkLabel->setAlignment(Qt::AlignCenter);
        rowLayout->addWidget(checkLabel);
    } else {
        auto *spacer = new QWidget(row);
        spacer->setFixedWidth(32);
        rowLayout->addWidget(spacer);
    }

    // Option text + progress bar
    auto *textAndBar = new QWidget(row);
    auto *tabLayout = new QVBoxLayout(textAndBar);
    tabLayout->setContentsMargins(0, 0, 0, 0);
    tabLayout->setSpacing(4);

    auto *optLabel = new QLabel(optionText, textAndBar);
    optLabel->setWordWrap(true);
    optLabel->setStyleSheet(QStringLiteral(
        "QLabel { font-size: 13px; color: #333; }"));
    tabLayout->addWidget(optLabel);

    // Progress bar
    auto *bar = new QProgressBar(textAndBar);
    bar->setRange(0, 100);
    bar->setValue(percentage);
    bar->setTextVisible(false);
    bar->setFixedHeight(8);
    bar->setStyleSheet(QStringLiteral(
        "QProgressBar { border: none; border-radius: 4px; background: #EEE; }"
        "QProgressBar::chunk { background: #0084FF; border-radius: 4px; }"));
    tabLayout->addWidget(bar);

    rowLayout->addWidget(textAndBar, 1);

    // Vote count + percentage
    auto *countLabel = new QLabel(
        QStringLiteral("%1 (%2%)")
            .arg(voteCount)
            .arg(percentage),
        row);
    countLabel->setStyleSheet(QStringLiteral(
        "QLabel { font-size: 11px; color: #888; min-width: 52px; }"));
    countLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    rowLayout->addWidget(countLabel);

    return row;
}

void PollWidget::setupViewerMode()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    // Question
    m_questionLabel = new QLabel(this);
    m_questionLabel->setWordWrap(true);
    m_questionLabel->setStyleSheet(QStringLiteral(
        "QLabel { font-size: 15px; font-weight: bold; color: #222; "
        "padding-bottom: 4px; }"));
    m_questionLabel->setText(m_pollData.question);
    mainLayout->addWidget(m_questionLabel);

    // Options
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setStyleSheet(QStringLiteral("QScrollArea { border: none; }"));

    m_viewerOptionsContainer = new QWidget(scroll);
    m_viewerOptionsLayout = new QVBoxLayout(m_viewerOptionsContainer);
    m_viewerOptionsLayout->setContentsMargins(0, 0, 0, 0);
    m_viewerOptionsLayout->setSpacing(6);

    for (int i = 0; i < m_pollData.options.size(); ++i) {
        auto *row = buildViewerOptionRow(i);
        m_viewerOptionsLayout->addWidget(row);
        m_viewerOptionRows.append(row);
    }

    scroll->setWidget(m_viewerOptionsContainer);
    mainLayout->addWidget(scroll, 1);

    // Total votes
    int totalVotes = 0;
    for (int v : m_pollData.votes)
        totalVotes += v;

    m_totalVotesLabel = new QLabel(this);
    m_totalVotesLabel->setStyleSheet(QStringLiteral(
        "QLabel { font-size: 11px; color: #999; padding-top: 4px; }"));
    m_totalVotesLabel->setText(
        totalVotes == 0
            ? QStringLiteral("No votes yet")
            : totalVotes == 1
                ? QStringLiteral("1 total vote")
                : QStringLiteral("%1 total votes").arg(totalVotes));
    mainLayout->addWidget(m_totalVotesLabel);

    // Closed label
    m_closedLabel = new QLabel(QStringLiteral("Poll closed"), this);
    m_closedLabel->setAlignment(Qt::AlignCenter);
    m_closedLabel->setStyleSheet(QStringLiteral(
        "QLabel { color: white; background: #E74C3C; font-size: 12px; "
        "font-weight: bold; padding: 4px 12px; border-radius: 4px; }"));
    m_closedLabel->setVisible(m_pollData.isClosed);
    mainLayout->addWidget(m_closedLabel);

    // End poll button (for creator)
    m_endPollButton = new QPushButton(QStringLiteral("End Poll"), this);
    m_endPollButton->setStyleSheet(QStringLiteral(
        "QPushButton { background: transparent; color: #E74C3C; border: 1px solid #E74C3C; "
        "border-radius: 4px; padding: 6px 16px; font-size: 12px; }"
        "QPushButton:hover { background: #FDE8E8; }"));
    m_endPollButton->setVisible(
        !m_pollData.isClosed && m_currentUserId == m_pollData.creatorId);
    connect(m_endPollButton, &QPushButton::clicked,
            this, &PollWidget::onEndPoll);
    mainLayout->addWidget(m_endPollButton);
}

void PollWidget::rebuildViewerOptions()
{
    // Clear existing option rows
    for (auto *row : m_viewerOptionRows) {
        m_viewerOptionsLayout->removeWidget(row);
        delete row;
    }
    m_viewerOptionRows.clear();

    // Rebuild
    for (int i = 0; i < m_pollData.options.size(); ++i) {
        auto *row = buildViewerOptionRow(i);
        m_viewerOptionsLayout->addWidget(row);
        m_viewerOptionRows.append(row);
    }

    // Update totals
    int totalVotes = 0;
    for (int v : m_pollData.votes)
        totalVotes += v;

    m_totalVotesLabel->setText(
        totalVotes == 0
            ? QStringLiteral("No votes yet")
            : totalVotes == 1
                ? QStringLiteral("1 total vote")
                : QStringLiteral("%1 total votes").arg(totalVotes));

    m_closedLabel->setVisible(m_pollData.isClosed);
    m_endPollButton->setVisible(
        !m_pollData.isClosed && m_currentUserId == m_pollData.creatorId);
}

void PollWidget::updateOptionRow(int index)
{
    if (index < 0 || index >= m_viewerOptionRows.size())
        return;

    m_viewerOptionsLayout->removeWidget(m_viewerOptionRows[index]);
    delete m_viewerOptionRows[index];

    auto *row = buildViewerOptionRow(index);
    m_viewerOptionRows[index] = row;
    m_viewerOptionsLayout->insertWidget(index, row);
}

void PollWidget::onSubmitVote(int optionIndex)
{
    if (m_pollData.isClosed || !m_editable)
        return;

    // Update local state
    if (m_pollData.type == PollData::SingleChoice) {
        m_userVotes = {optionIndex};
    } else {
        if (m_userVotes.contains(optionIndex))
            m_userVotes.removeAll(optionIndex);
        else
            m_userVotes.append(optionIndex);
    }

    emit voteSubmitted(optionIndex);
}

void PollWidget::onEndPoll()
{
    m_pollData.isClosed = true;
    rebuildViewerOptions();
    emit pollEnded();
}
