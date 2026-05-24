#pragma once

#include <QDateTime>
#include <QDialog>
#include <QString>
#include <QList>

#include "log_collector.hpp"

class QPlainTextEdit;
class QLineEdit;
class QComboBox;
class QCheckBox;
class QPushButton;
class QLabel;

// ---------------------------------------------------------------------------
// LogViewerDialog
// ---------------------------------------------------------------------------

class LogViewerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LogViewerDialog(QWidget *parent = nullptr);
    ~LogViewerDialog() override;

private slots:
    void onFilterTextChanged(const QString &text);
    void onLevelFilterChanged(int index);
    void onCopyAll();
    void onSaveToFile();
    void onClearLog();
    void onNewLogEntry(const LogCollector::LogEntry &entry);
    void onAutoScrollToggled(bool enabled);
    void onAutoRefreshToggled(bool enabled);

private:
    void setupUi();
    void connectSignals();
    void loadExistingLogs();
    void applyFilters();
    void rebuildDisplay();
    void appendLogEntry(const LogCollector::LogEntry &entry, int displayIndex);
    void scrollToBottom();

    static QColor colorForLevel(QtMsgType level);
    static bool entryMatchesFilter(const LogCollector::LogEntry &entry,
                                   const QString &textFilter,
                                   int levelFilter);

    QPlainTextEdit *m_logView = nullptr;
    QLineEdit *m_filterEdit = nullptr;
    QComboBox *m_levelFilter = nullptr;
    QCheckBox *m_autoScroll = nullptr;
    QCheckBox *m_autoRefresh = nullptr;
    QLabel *m_matchCount = nullptr;
    QPushButton *m_copyAllBtn = nullptr;
    QPushButton *m_saveBtn = nullptr;
    QPushButton *m_clearBtn = nullptr;

    QList<LogCollector::LogEntry> m_allEntries;
    QList<LogCollector::LogEntry> m_filteredEntries;
    bool m_autoScrollEnabled = true;
    bool m_autoRefreshEnabled = true;
};
