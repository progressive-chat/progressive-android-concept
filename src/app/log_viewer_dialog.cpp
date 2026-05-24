#include "log_viewer_dialog.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextOption>
#include <QTextStream>
#include <QVBoxLayout>

#include "log_collector.hpp"

// ===================================================================
// LogTextEdit -- QPlainTextEdit with a line-number gutter
// ===================================================================

namespace {

class LogTextEdit : public QPlainTextEdit {
public:
    explicit LogTextEdit(QWidget *parent = nullptr);

    void gutterPaintEvent(QPaintEvent *event);
    int gutterWidth() const;

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateGutterWidth();
    void updateGutterGeometry(const QRect &rect, int dy);
    void fullGutterUpdate();

    class GutterWidget : public QWidget {
    public:
        explicit GutterWidget(LogTextEdit *editor)
            : QWidget(editor), m_editor(editor) {}
        QSize sizeHint() const override {
            return QSize(m_editor->gutterWidth(), 0);
        }
    protected:
        void paintEvent(QPaintEvent *event) override {
            m_editor->gutterPaintEvent(event);
        }
    private:
        LogTextEdit *m_editor;
    };

    GutterWidget *m_gutter;
};

// -------------------------------------------------------------------

LogTextEdit::LogTextEdit(QWidget *parent)
    : QPlainTextEdit(parent)
{
    m_gutter = new GutterWidget(this);

    connect(this, &QPlainTextEdit::blockCountChanged,
            this, &LogTextEdit::updateGutterWidth);
    connect(this, &QPlainTextEdit::updateRequest,
            this, &LogTextEdit::updateGutterGeometry);
    connect(verticalScrollBar(), &QScrollBar::valueChanged,
            m_gutter, QOverload<>::of(&QWidget::update));

    updateGutterWidth();
}

int LogTextEdit::gutterWidth() const
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    int space = 8 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void LogTextEdit::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    QRect cr = contentsRect();
    m_gutter->setGeometry(
        QRect(cr.left(), cr.top(), gutterWidth(), cr.height()));
}

void LogTextEdit::updateGutterWidth()
{
    setViewportMargins(gutterWidth(), 0, 0, 0);
    // re-position the gutter after margin change
    QRect cr = contentsRect();
    m_gutter->setGeometry(
        QRect(cr.left(), cr.top(), gutterWidth(), cr.height()));
}

void LogTextEdit::updateGutterGeometry(const QRect &rect, int dy)
{
    if (dy)
        m_gutter->scroll(0, dy);
    else
        m_gutter->update(0, rect.y(), m_gutter->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateGutterWidth();
}

void LogTextEdit::gutterPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_gutter);
    painter.fillRect(event->rect(), QColor(245, 245, 245));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top =
        qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    const int gutterRight = m_gutter->width() - 4;
    const int lineHeight = fontMetrics().height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(QColor(170, 170, 170));
            painter.drawText(0, top, gutterRight, lineHeight,
                             Qt::AlignRight | Qt::AlignVCenter, number);
        }
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

} // anonymous namespace

// ===================================================================
// LogViewerDialog
// ===================================================================

LogViewerDialog::LogViewerDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
    connectSignals();
    loadExistingLogs();
}

LogViewerDialog::~LogViewerDialog() = default;

// -------------------------------------------------------------------
// UI construction
// -------------------------------------------------------------------

void LogViewerDialog::setupUi()
{
    setWindowTitle(tr("Application Log"));
    resize(960, 680);

    auto *mainLayout = new QVBoxLayout(this);

    // ---- Toolbar ----
    auto *toolbar = new QHBoxLayout;

    m_filterEdit = new QLineEdit(this);
    m_filterEdit->setPlaceholderText(tr("Filter logs..."));
    m_filterEdit->setClearButtonEnabled(true);
    toolbar->addWidget(m_filterEdit);

    m_levelFilter = new QComboBox(this);
    m_levelFilter->addItem(tr("All"),      -1);
    m_levelFilter->addItem(tr("Debug"),    static_cast<int>(QtDebugMsg));
    m_levelFilter->addItem(tr("Info"),     static_cast<int>(QtInfoMsg));
    m_levelFilter->addItem(tr("Warning"),  static_cast<int>(QtWarningMsg));
    m_levelFilter->addItem(tr("Error"),    static_cast<int>(QtCriticalMsg));
    m_levelFilter->addItem(tr("Critical"), static_cast<int>(QtFatalMsg));
    toolbar->addWidget(m_levelFilter);

    m_matchCount = new QLabel(this);
    toolbar->addWidget(m_matchCount);

    toolbar->addStretch();

    m_autoScroll = new QCheckBox(tr("Auto-scroll"), this);
    m_autoScroll->setChecked(true);
    toolbar->addWidget(m_autoScroll);

    m_autoRefresh = new QCheckBox(tr("Auto-refresh"), this);
    m_autoRefresh->setChecked(true);
    toolbar->addWidget(m_autoRefresh);

    mainLayout->addLayout(toolbar);

    // ---- Log view ----
    m_logView = new LogTextEdit(this);
    m_logView->setReadOnly(true);
    m_logView->setWordWrapMode(QTextOption::NoWrap);

    QFont monoFont(QStringLiteral("monospace"), 10);
    monoFont.setStyleHint(QFont::Monospace);
    monoFont.setFixedPitch(true);
    m_logView->setFont(monoFont);
    m_logView->document()->setDefaultFont(monoFont);

    mainLayout->addWidget(m_logView, 1);

    // ---- Bottom button row ----
    auto *btnLayout = new QHBoxLayout;

    m_copyAllBtn = new QPushButton(tr("Copy All"), this);
    btnLayout->addWidget(m_copyAllBtn);

    m_saveBtn = new QPushButton(tr("Save to File"), this);
    btnLayout->addWidget(m_saveBtn);

    m_clearBtn = new QPushButton(tr("Clear Log"), this);
    btnLayout->addWidget(m_clearBtn);

    btnLayout->addStretch();

    auto *closeBtn = new QPushButton(tr("Close"), this);
    connect(closeBtn, &QPushButton::clicked,
            this, &QDialog::close);
    btnLayout->addWidget(closeBtn);

    mainLayout->addLayout(btnLayout);
}

// -------------------------------------------------------------------
// Signal / slot wiring
// -------------------------------------------------------------------

void LogViewerDialog::connectSignals()
{
    connect(m_filterEdit, &QLineEdit::textChanged,
            this, &LogViewerDialog::onFilterTextChanged);
    connect(m_levelFilter,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LogViewerDialog::onLevelFilterChanged);
    connect(m_autoScroll, &QCheckBox::toggled,
            this, &LogViewerDialog::onAutoScrollToggled);
    connect(m_autoRefresh, &QCheckBox::toggled,
            this, &LogViewerDialog::onAutoRefreshToggled);

    connect(m_copyAllBtn, &QPushButton::clicked,
            this, &LogViewerDialog::onCopyAll);
    connect(m_saveBtn, &QPushButton::clicked,
            this, &LogViewerDialog::onSaveToFile);
    connect(m_clearBtn, &QPushButton::clicked,
            this, &LogViewerDialog::onClearLog);

    auto &collector = LogCollector::instance();
    connect(&collector, &LogCollector::newLogEntry,
            this, &LogViewerDialog::onNewLogEntry);
}

// -------------------------------------------------------------------
// Load existing logs from collector
// -------------------------------------------------------------------

void LogViewerDialog::loadExistingLogs()
{
    auto &collector = LogCollector::instance();
    m_allEntries = collector.entries();
    applyFilters();
}

// -------------------------------------------------------------------
// Filter helpers
// -------------------------------------------------------------------

bool LogViewerDialog::entryMatchesFilter(const LogCollector::LogEntry &entry,
                                         const QString &textFilter,
                                         int levelFilter)
{
    // Level filter: -1 = "All"
    if (levelFilter >= 0 && static_cast<int>(entry.level) != levelFilter)
        return false;

    if (textFilter.isEmpty())
        return true;

    return entry.message.contains(textFilter, Qt::CaseInsensitive)
           || entry.timestamp.toString(Qt::ISODate)
                  .contains(textFilter, Qt::CaseInsensitive);
}

void LogViewerDialog::applyFilters()
{
    const QString text = m_filterEdit->text();
    const int level = m_levelFilter->currentData().toInt();

    m_filteredEntries.clear();
    m_filteredEntries.reserve(m_allEntries.size());

    for (const auto &entry : m_allEntries) {
        if (entryMatchesFilter(entry, text, level))
            m_filteredEntries.append(entry);
    }

    rebuildDisplay();
}

// -------------------------------------------------------------------
// Rebuild the entire display
// -------------------------------------------------------------------

void LogViewerDialog::rebuildDisplay()
{
    m_logView->clear();

    for (int i = 0; i < m_filteredEntries.size(); ++i)
        appendLogEntry(m_filteredEntries[i], i + 1);

    m_matchCount->setText(
        tr("%1/%2 matched")
            .arg(m_filteredEntries.size())
            .arg(m_allEntries.size()));
}

// -------------------------------------------------------------------
// Append a single log entry
// -------------------------------------------------------------------

QColor LogViewerDialog::colorForLevel(QtMsgType level)
{
    switch (level) {
    case QtDebugMsg:    return QColor(150, 150, 150);   // gray
    case QtWarningMsg:  return QColor(255, 165, 0);     // orange
    case QtCriticalMsg: return QColor(240, 60, 60);     // red
    case QtFatalMsg:    return QColor(220, 20, 20);     // bold red
    case QtInfoMsg:
    default:            return QColor();                // invalid = default
    }
}

void LogViewerDialog::appendLogEntry(const LogCollector::LogEntry &entry, int displayIndex)
{
    QTextCursor cursor(m_logView->document());
    cursor.movePosition(QTextCursor::End);

    // -- line number --
    {
        QTextCharFormat fmt;
        fmt.setForeground(QColor(170, 170, 170));
        cursor.insertText(
            QStringLiteral("%1  ").arg(displayIndex, 4), fmt);
    }

    // -- timestamp --
    {
        QTextCharFormat fmt;
        fmt.setForeground(QColor(100, 149, 237));
        cursor.insertText(
            entry.timestamp.toString(QStringLiteral("[yyyy-MM-dd hh:mm:ss.zzz] ")),
            fmt);
    }

    // -- log level tag --
    const char *levelTag = "[Debug]    ";
    QtMsgType lvl = entry.level;
    switch (lvl) {
    case QtDebugMsg:    levelTag = "[Debug]    "; break;
    case QtInfoMsg:     levelTag = "[Info]     "; break;
    case QtWarningMsg:  levelTag = "[Warning]  "; break;
    case QtCriticalMsg: levelTag = "[Error]    "; break;
    case QtFatalMsg:    levelTag = "[Critical] "; break;
    }

    QColor lvlColor = colorForLevel(lvl);
    {
        QTextCharFormat fmt;
        if (lvlColor.isValid())
            fmt.setForeground(lvlColor);
        if (lvl == QtFatalMsg)
            fmt.setFontWeight(QFont::Bold);
        cursor.insertText(QString::fromLatin1(levelTag), fmt);
    }

    // -- message body --
    {
        QTextCharFormat fmt;
        if (lvlColor.isValid() && lvl != QtDebugMsg)
            fmt.setForeground(lvlColor);
        if (lvl == QtFatalMsg)
            fmt.setFontWeight(QFont::Bold);
        cursor.insertText(entry.message, fmt);
    }

    cursor.insertBlock();
}

// -------------------------------------------------------------------
// Slots
// -------------------------------------------------------------------

void LogViewerDialog::onFilterTextChanged(const QString & /*text*/)
{
    applyFilters();
}

void LogViewerDialog::onLevelFilterChanged(int /*index*/)
{
    applyFilters();
}

void LogViewerDialog::onCopyAll()
{
    QClipboard *cb = QApplication::clipboard();
    cb->setText(m_logView->toPlainText());
}

void LogViewerDialog::onSaveToFile()
{
    const QString defaultName =
        QDir::homePath()
        + QStringLiteral("/application_log_%1.txt")
              .arg(QDateTime::currentDateTime().toString(
                  QStringLiteral("yyyyMMdd_HHmmss")));

    const QString filePath = QFileDialog::getSaveFileName(
        this, tr("Save Log"), defaultName,
        tr("Text files (*.txt);;All files (*)"));

    if (filePath.isEmpty())
        return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(
            this, tr("Error"),
            tr("Could not save file: %1").arg(file.errorString()));
        return;
    }

    QTextStream stream(&file);
    stream << m_logView->toPlainText();
}

void LogViewerDialog::onClearLog()
{
    auto answer = QMessageBox::question(
        this, tr("Clear Log"),
        tr("Are you sure you want to clear the entire log?"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (answer != QMessageBox::Yes)
        return;

    m_allEntries.clear();
    m_filteredEntries.clear();
    m_logView->clear();
    m_matchCount->setText(tr("0/0 matched"));
}

void LogViewerDialog::onNewLogEntry(const LogCollector::LogEntry &entry)
{
    m_allEntries.append(entry);

    if (!m_autoRefreshEnabled)
        return;

    const QString text = m_filterEdit->text();
    const int level = m_levelFilter->currentData().toInt();

    if (entryMatchesFilter(entry, text, level)) {
        m_filteredEntries.append(entry);
        appendLogEntry(entry, m_filteredEntries.size());
    }

    m_matchCount->setText(
        tr("%1/%2 matched")
            .arg(m_filteredEntries.size())
            .arg(m_allEntries.size()));

    if (m_autoScrollEnabled)
        scrollToBottom();
}

void LogViewerDialog::onAutoScrollToggled(bool enabled)
{
    m_autoScrollEnabled = enabled;
    if (enabled)
        scrollToBottom();
}

void LogViewerDialog::onAutoRefreshToggled(bool enabled)
{
    m_autoRefreshEnabled = enabled;
    if (enabled) {
        // re-sync from collector and re-apply filters
        auto &collector = LogCollector::instance();
        m_allEntries = collector.entries();
        applyFilters();
    }
}

void LogViewerDialog::scrollToBottom()
{
    QScrollBar *sb = m_logView->verticalScrollBar();
    sb->setValue(sb->maximum());
}
