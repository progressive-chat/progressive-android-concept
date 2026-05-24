#include "message_edit_history_dialog.hpp"

#include <QApplication>
#include <QDateTime>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QPainter>
#include <QScreen>
#include <QScrollBar>
#include <QStyle>
#include <QtMath>

#include "matrix/matrix_session.hpp"

MessageEditHistoryDialog::MessageEditHistoryDialog(const QString& messageId,
                                                     const QString& roomId,
                                                     MatrixSession* session,
                                                     QWidget* parent)
    : QDialog(parent)
    , m_messageId(messageId)
    , m_roomId(roomId)
    , m_session(session)
    , m_originalLabel(nullptr)
    , m_originalTimestamp(nullptr)
    , m_editsLayout(nullptr)
    , m_scrollArea(nullptr)
    , m_viewInChatButton(nullptr)
{
    setupUi();
    loadEditHistory();
    applyStyleSheet();
}

void MessageEditHistoryDialog::setupUi()
{
    setWindowTitle(tr("Message Edit History"));
    setMinimumSize(420, 520);

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ---------- Header ----------
    auto* headerWidget = new QWidget;
    headerWidget->setObjectName(QStringLiteral("historyHeader"));
    auto* headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(16, 12, 16, 12);

    auto* titleLabel = new QLabel(tr("Edit History"));
    titleLabel->setObjectName(QStringLiteral("historyTitle"));
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    headerLayout->addWidget(titleLabel);

    headerLayout->addStretch();

    m_viewInChatButton = new QPushButton(tr("View in Chat"));
    m_viewInChatButton->setObjectName(QStringLiteral("viewInChatButton"));
    m_viewInChatButton->setCursor(Qt::PointingHandCursor);
    connect(m_viewInChatButton, &QPushButton::clicked,
            this, &MessageEditHistoryDialog::onViewInChatClicked);
    headerLayout->addWidget(m_viewInChatButton);

    rootLayout->addWidget(headerWidget);

    // ---------- Separator ----------
    auto* separator = new QFrame;
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    separator->setObjectName(QStringLiteral("historySeparator"));
    rootLayout->addWidget(separator);

    // ---------- Scroll area ----------
    m_scrollArea = new QScrollArea;
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setObjectName(QStringLiteral("historyScrollArea"));

    auto* scrollContent = new QWidget;
    scrollContent->setObjectName(QStringLiteral("scrollContent"));
    auto* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(16, 16, 16, 16);
    scrollLayout->setSpacing(12);

    // ---------- Original message section ----------
    auto* origSectionLabel = new QLabel(tr("Original"));
    origSectionLabel->setObjectName(QStringLiteral("originalSectionLabel"));
    QFont sectionFont = origSectionLabel->font();
    sectionFont.setBold(true);
    sectionFont.setPointSize(11);
    origSectionLabel->setFont(sectionFont);
    scrollLayout->addWidget(origSectionLabel);

    m_originalTimestamp = new QLabel;
    m_originalTimestamp->setObjectName(QStringLiteral("originalTimestamp"));
    m_originalTimestamp->setStyleSheet(QStringLiteral("color: #888; font-size: 11px;"));
    scrollLayout->addWidget(m_originalTimestamp);

    m_originalLabel = new QLabel;
    m_originalLabel->setObjectName(QStringLiteral("originalContent"));
    m_originalLabel->setWordWrap(true);
    m_originalLabel->setTextFormat(Qt::RichText);
    m_originalLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_originalLabel->setStyleSheet(QStringLiteral(
        "background: #f5f5f5; padding: 10px; border-radius: 6px; border: 1px solid #ddd;"));
    scrollLayout->addWidget(m_originalLabel);

    // ---------- Edits section ----------
    auto* editsSectionLabel = new QLabel(tr("Edits"));
    editsSectionLabel->setObjectName(QStringLiteral("editsSectionLabel"));
    editsSectionLabel->setFont(sectionFont);
    scrollLayout->addWidget(editsSectionLabel);

    m_editsLayout = new QVBoxLayout;
    m_editsLayout->setSpacing(10);
    scrollLayout->addLayout(m_editsLayout);

    scrollLayout->addStretch();

    m_scrollArea->setWidget(scrollContent);
    rootLayout->addWidget(m_scrollArea, 1);

    // ---------- Bottom close bar ----------
    auto* bottomBar = new QWidget;
    bottomBar->setObjectName(QStringLiteral("bottomBar"));
    auto* bottomLayout = new QHBoxLayout(bottomBar);
    bottomLayout->setContentsMargins(16, 10, 16, 10);

    auto* closeButton = new QPushButton(tr("Close"));
    closeButton->setObjectName(QStringLiteral("closeButton"));
    closeButton->setCursor(Qt::PointingHandCursor);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);
    bottomLayout->addStretch();
    bottomLayout->addWidget(closeButton);

    rootLayout->addWidget(bottomBar);
}

void MessageEditHistoryDialog::loadEditHistory()
{
    m_edits = fetchEditHistory();

    if (m_edits.isEmpty()) {
        m_originalLabel->setText(tr("No edit history available."));
        m_originalTimestamp->setText({});
        return;
    }

    // First entry is the original message
    const EditEntry& original = m_edits.first();
    m_originalLabel->setText(original.content);
    m_originalTimestamp->setText(tr("Sent at %1").arg(
        original.timestamp.toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"))));

    const int totalEdits = m_edits.size();

    // Subsequent entries are edits; display in reverse chronological order
    for (int i = totalEdits - 1; i >= 1; --i) {
        QWidget* editWidget = createEditWidget(m_edits.at(i), i, totalEdits);
        m_editsLayout->addWidget(editWidget);
    }
}

QVector<EditEntry> MessageEditHistoryDialog::fetchEditHistory() const
{
    if (m_session) {
        // Real implementation: fetch aggregated edit relations from Matrix
        // GET /_matrix/client/r0/rooms/{roomId}/relations/{eventId}?relType=m.replace
        // For now, fall through to stub.
    }
    return generateStubEdits();
}

QVector<EditEntry> MessageEditHistoryDialog::generateStubEdits() const
{
    QVector<EditEntry> edits;

    QDateTime base = QDateTime::currentDateTime().addSecs(-3600);

    {
        EditEntry orig;
        orig.content = QStringLiteral("Hello everyone! Let's meet at the cafe on Main Street.");
        orig.timestamp = base;
        edits.append(orig);
    }

    {
        EditEntry e1;
        e1.content = QStringLiteral("Hello everyone! Let's meet at the <b>new</b> cafe on Main Street.");
        e1.timestamp = base.addSecs(300);
        edits.append(e1);
    }

    {
        EditEntry e2;
        e2.content = QStringLiteral("Hello team! Let's meet at the new cafe on <b>Oak</b> Street.");
        e2.timestamp = base.addSecs(600);
        edits.append(e2);
    }

    {
        EditEntry e3;
        e3.content = QStringLiteral("Hello team! Let's meet at the new cafe on Oak Street at <b>3 PM</b>.");
        e3.timestamp = base.addSecs(900);
        edits.append(e3);
    }

    {
        EditEntry e4;
        e4.content = QStringLiteral("Hello <b>team</b>! Let's meet at the new cafe on Oak Street at 3 PM. Bring your laptops!");
        e4.timestamp = base.addSecs(1200);
        edits.append(e4);
    }

    return edits;
}

QWidget* MessageEditHistoryDialog::createEditWidget(const EditEntry& edit,
                                                     int index,
                                                     int total)
{
    auto* widget = new QWidget;
    widget->setObjectName(QStringLiteral("editEntryWidget"));

    const bool isLatest = (index == total - 1);

    auto* layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    // Timestamp header
    auto* header = new QLabel(tr("Edited at %1")
        .arg(edit.timestamp.toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"))));
    header->setObjectName(QStringLiteral("editTimestamp"));
    header->setStyleSheet(QStringLiteral("color: #666; font-size: 11px; font-style: italic;"));
    layout->addWidget(header);

    // Content area
    QString displayContent;
    if (isLatest) {
        displayContent = edit.content;
    } else {
        // Compute diff against the next (later) edit
        const EditEntry* later = nullptr;
        for (int j = index + 1; j < m_edits.size(); ++j) {
            if (j != index) {
                later = &m_edits.at(j);
                break;
            }
        }
        if (later) {
            displayContent = computeDiff(later->content, edit.content);
        } else {
            displayContent = edit.content;
        }
    }

    auto* contentLabel = new QLabel(displayContent);
    contentLabel->setObjectName(QStringLiteral("editContent"));
    contentLabel->setWordWrap(true);
    contentLabel->setTextFormat(Qt::RichText);
    contentLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    QString contentStyle = QStringLiteral(
        "padding: 10px; border-radius: 6px; background: #f9f9f9; border: 1px solid #e0e0e0;");
    if (isLatest) {
        contentStyle.prepend(QStringLiteral("border-left: 4px solid #1976D2; "));
    }
    contentLabel->setStyleSheet(contentStyle);

    layout->addWidget(contentLabel);

    return widget;
}

QString MessageEditHistoryDialog::computeDiff(const QString& before,
                                               const QString& after) const
{
    // Simple character-level LCS diff for plain-text; preserves HTML in
    // the input so rich-text messages are displayed correctly.

    QString beforePlain = before;
    beforePlain.remove(QRegularExpression(QStringLiteral("<[^>]*>")));
    QString afterPlain = after;
    afterPlain.remove(QRegularExpression(QStringLiteral("<[^>]*>")));

    if (beforePlain == afterPlain) {
        return after; // no change
    }

    // Compute longest-common-subsequence lengths
    const int m = beforePlain.size();
    const int n = afterPlain.size();

    QVector<QVector<int>> dp(m + 1, QVector<int>(n + 1, 0));
    for (int i = 1; i <= m; ++i) {
        for (int j = 1; j <= n; ++j) {
            if (beforePlain.at(i - 1) == afterPlain.at(j - 1)) {
                dp[i][j] = dp[i - 1][j - 1] + 1;
            } else {
                dp[i][j] = qMax(dp[i - 1][j], dp[i][j - 1]);
            }
        }
    }

    // Backtrack to produce diff
    QString result;
    result.reserve(n * 2);

    int i = m, j = n;
    QVector<QChar> removedBuf;
    QVector<QChar> addedBuf;

    auto flushRemoved = [&]() {
        if (!removedBuf.isEmpty()) {
            QString txt;
            for (QChar c : removedBuf) {
                txt += (c == QLatin1Char('&') ? QStringLiteral("&amp;")
                     : c == QLatin1Char('<')  ? QStringLiteral("&lt;")
                     : c == QLatin1Char('>')  ? QStringLiteral("&gt;")
                     : c == QLatin1Char('"')  ? QStringLiteral("&quot;")
                     : QString(c));
            }
            result += QStringLiteral("<span style=\"color: #d32f2f; text-decoration: line-through;\">%1</span>").arg(txt);
            removedBuf.clear();
        }
    };

    auto flushAdded = [&]() {
        if (!addedBuf.isEmpty()) {
            QString txt;
            for (QChar c : addedBuf) {
                txt += (c == QLatin1Char('&') ? QStringLiteral("&amp;")
                     : c == QLatin1Char('<')  ? QStringLiteral("&lt;")
                     : c == QLatin1Char('>')  ? QStringLiteral("&gt;")
                     : c == QLatin1Char('"')  ? QStringLiteral("&quot;")
                     : QString(c));
            }
            result += QStringLiteral("<span style=\"color: #2e7d32;\">%1</span>").arg(txt);
            addedBuf.clear();
        }
    };

    while (i > 0 || j > 0) {
        if (i > 0 && j > 0 && beforePlain.at(i - 1) == afterPlain.at(j - 1)) {
            flushRemoved();
            flushAdded();
            QChar c = beforePlain.at(i - 1);
            result.prepend(c == QLatin1Char('&') ? QStringLiteral("&amp;")
                         : c == QLatin1Char('<')  ? QStringLiteral("&lt;")
                         : c == QLatin1Char('>')  ? QStringLiteral("&gt;")
                         : c == QLatin1Char('"')  ? QStringLiteral("&quot;")
                         : QString(c));
            --i;
            --j;
        } else if (j > 0 && (i == 0 || dp[i][j - 1] >= dp[i - 1][j])) {
            addedBuf.prepend(afterPlain.at(j - 1));
            --j;
        } else {
            removedBuf.prepend(beforePlain.at(i - 1));
            --i;
        }
    }
    flushRemoved();
    flushAdded();

    return result;
}

void MessageEditHistoryDialog::onViewInChatClicked()
{
    emit viewInChatRequested(m_roomId, m_messageId);
    close();
}

void MessageEditHistoryDialog::applyStyleSheet()
{
    setStyleSheet(QStringLiteral(R"(
        #historyHeader {
            background: #ffffff;
        }
        #historyTitle {
            color: #212121;
        }
        #historySeparator {
            color: #e0e0e0;
        }
        #historyScrollArea {
            border: none;
            background: #fafafa;
        }
        #scrollContent {
            background: #fafafa;
        }
        #editEntryWidget {
            background: transparent;
        }
        #viewInChatButton {
            background: #1976D2;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 6px 14px;
            font-weight: bold;
            font-size: 12px;
        }
        #viewInChatButton:hover {
            background: #1565C0;
        }
        #closeButton {
            background: #e0e0e0;
            border: none;
            border-radius: 4px;
            padding: 6px 18px;
            font-size: 12px;
        }
        #closeButton:hover {
            background: #bdbdbd;
        }
        #bottomBar {
            background: #ffffff;
            border-top: 1px solid #e0e0e0;
        }
        #originalSectionLabel, #editsSectionLabel {
            color: #424242;
            margin-top: 4px;
        }
    )"));
}
