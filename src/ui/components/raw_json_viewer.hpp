#pragma once

#include <QDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QCheckBox>
#include <QSet>
#include <QMap>
#include <QPoint>
#include <QRegularExpression>
#include <QPropertyAnimation>

class JsonTextEdit;
class JsonSyntaxHighlighter;

class RawJsonViewer : public QDialog {
    Q_OBJECT

public:
    explicit RawJsonViewer(const QJsonObject& obj, QWidget* parent = nullptr);
    explicit RawJsonViewer(const QJsonDocument& doc, QWidget* parent = nullptr);
    explicit RawJsonViewer(const QString& jsonString, QWidget* parent = nullptr);

    void setJson(const QJsonObject& obj);
    void setJson(const QJsonDocument& doc);
    void setJson(const QString& jsonString);
    QJsonDocument jsonDocument() const;

private slots:
    void togglePrettyPrint();
    void copyToClipboard();
    void showFindBar();
    void hideFindBar();
    void findNext();
    void findPrevious();
    void copyJsonPathAtCursor();

private:
    void init();
    void setupUi();
    QString formatJson(const QJsonDocument& doc, bool pretty) const;
    void refreshDisplay();
    void updateStatusBar();
    QPair<int, int> foldRange(int lineNumber) const;
    int findMatchingClose(int openLine) const;

    JsonTextEdit* m_textEdit;
    JsonSyntaxHighlighter* m_highlighter;
    QPushButton* m_copyButton;
    QCheckBox* m_prettyPrintCheck;
    QLineEdit* m_findBar;
    QPushButton* m_findNextBtn;
    QPushButton* m_findPrevBtn;
    QPushButton* m_findCloseBtn;
    QLabel* m_statusLabel;
    QLabel* m_validityLabel;
    QWidget* m_findContainer;

    QString m_rawJson;
    QString m_fullText;
    QJsonDocument m_document;
    bool m_validJson;
    bool m_prettyPrint;
    QSet<int> m_foldedLines;
    int m_currentFindPos;
};


class JsonTextEdit : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit JsonTextEdit(QWidget* parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent* event);
    int lineNumberAreaWidth() const;
    void toggleFoldAtLine(int lineNumber);
    QSet<int> foldedLines() const { return m_foldedLines; }
    void setFoldedLines(const QSet<int>& lines) { m_foldedLines = lines; }

signals:
    void foldToggled(int lineNumber);
    void copyPathRequested();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect& rect, int dy);

private:
    QWidget* m_lineNumberArea;
    QSet<int> m_foldedLines;

    friend class LineNumberArea;
};


class LineNumberArea : public QWidget {
public:
    explicit LineNumberArea(JsonTextEdit* editor);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    JsonTextEdit* m_editor;
};


class JsonSyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit JsonSyntaxHighlighter(QTextDocument* parent);

protected:
    void highlightBlock(const QString& text) override;

private:
    struct HighlightRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightRule> m_rules;

    QTextCharFormat m_keyFormat;
    QTextCharFormat m_stringFormat;
    QTextCharFormat m_numberFormat;
    QTextCharFormat m_boolFormat;
    QTextCharFormat m_nullFormat;
    QTextCharFormat m_foldMarkerFormat;
    QTextCharFormat m_findMatchFormat;

    void setupFormats();
    void setupRules();
};
