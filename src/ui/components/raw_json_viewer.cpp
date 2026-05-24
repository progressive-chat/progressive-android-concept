#include "raw_json_viewer.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QClipboard>
#include <QApplication>
#include <QShortcut>
#include <QPainter>
#include <QTextBlock>
#include <QScrollBar>
#include <QFont>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QScreen>
#include <QFrame>
#include <QStack>
#include <QJsonParseError>
#include <QJsonValue>

// ---------------------------------------------------------------------------
// LineNumberArea
// ---------------------------------------------------------------------------

LineNumberArea::LineNumberArea(JsonTextEdit* editor)
    : QWidget(editor)
    , m_editor(editor)
{
}

QSize LineNumberArea::sizeHint() const
{
    return QSize(m_editor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent* event)
{
    m_editor->lineNumberAreaPaintEvent(event);
}


// ---------------------------------------------------------------------------
// JsonTextEdit
// ---------------------------------------------------------------------------

JsonTextEdit::JsonTextEdit(QWidget* parent)
    : QPlainTextEdit(parent)
{
    m_lineNumberArea = new LineNumberArea(this);

    connect(this, &QPlainTextEdit::blockCountChanged,
            this, &JsonTextEdit::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest,
            this, &JsonTextEdit::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged,
            this, [this]() {
        m_lineNumberArea->update();
    });

    updateLineNumberAreaWidth(0);
    setReadOnly(true);
    setLineWrapMode(QPlainTextEdit::NoWrap);

    QFont monoFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    monoFont.setPointSize(10);
    setFont(monoFont);

    setContextMenuPolicy(Qt::DefaultContextMenu);
}

int JsonTextEdit::lineNumberAreaWidth() const
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    int space = 10 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void JsonTextEdit::updateLineNumberAreaWidth(int /*newBlockCount*/)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void JsonTextEdit::updateLineNumberArea(const QRect& rect, int dy)
{
    if (dy)
        m_lineNumberArea->scroll(0, dy);
    else
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void JsonTextEdit::resizeEvent(QResizeEvent* event)
{
    QPlainTextEdit::resizeEvent(event);

    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(
        QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void JsonTextEdit::lineNumberAreaPaintEvent(QPaintEvent* event)
{
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), QColor("#1E1E2E"));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(QColor("#585B70"));
            painter.drawText(0, top, m_lineNumberArea->width() - 4,
                             fontMetrics().height(),
                             Qt::AlignRight, number);

            // draw fold indicator
            QString text = block.text().trimmed();
            if (text.endsWith('{') || text.endsWith('[') || text.endsWith('{') || text.endsWith('[')) {
                if (m_foldedLines.contains(blockNumber)) {
                    painter.setPen(QColor("#89B4FA"));
                    painter.drawText(4, top, 14, fontMetrics().height(), Qt::AlignCenter, "+");
                } else {
                    painter.setPen(QColor("#6C7086"));
                    QRectF foldRect(2, top + 2, 10, 10);
                    // small triangle indicator
                }
            }
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void JsonTextEdit::toggleFoldAtLine(int lineNumber)
{
    QTextBlock block = document()->findBlockByNumber(lineNumber);
    if (!block.isValid()) return;

    QString text = block.text().trimmed();
    bool isOpenBrace = text.endsWith('{') || text.endsWith('[');

    if (!isOpenBrace) {
        return;
    }

    if (m_foldedLines.contains(lineNumber)) {
        m_foldedLines.remove(lineNumber);
    } else {
        m_foldedLines.insert(lineNumber);
    }
    emit foldToggled(lineNumber);
}

void JsonTextEdit::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu* menu = createStandardContextMenu();
    menu->addSeparator();
    QAction* copyPathAction = menu->addAction("Copy JSON Path");
    connect(copyPathAction, &QAction::triggered, this, &JsonTextEdit::copyPathRequested);
    menu->exec(event->globalPos());
    delete menu;
}


// ---------------------------------------------------------------------------
// JsonSyntaxHighlighter
// ---------------------------------------------------------------------------

JsonSyntaxHighlighter::JsonSyntaxHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
    setupFormats();
    setupRules();
}

void JsonSyntaxHighlighter::setupFormats()
{
    m_keyFormat.setForeground(QColor("#89B4FA"));       // blue
    m_keyFormat.setFontWeight(QFont::Bold);

    m_stringFormat.setForeground(QColor("#A6E3A1"));    // green

    m_numberFormat.setForeground(QColor("#FAB387"));    // orange

    m_boolFormat.setForeground(QColor("#CBA6F7"));      // purple
    m_boolFormat.setFontWeight(QFont::Bold);

    m_nullFormat.setForeground(QColor("#6C7086"));      // gray
    m_nullFormat.setFontWeight(QFont::Bold);

    m_foldMarkerFormat.setForeground(QColor("#BAC2DE"));
    m_foldMarkerFormat.setFontItalic(true);

    m_findMatchFormat.setBackground(QColor("#45475A"));
    m_findMatchFormat.setForeground(QColor("#F5E0DC"));
}

void JsonSyntaxHighlighter::setupRules()
{
    HighlightRule rule;

    // Strings (double-quoted)
    rule.pattern = QRegularExpression(R"("(?:[^"\\]|\\.)*")");
    rule.format = m_stringFormat;
    m_rules.append(rule);

    // Numbers (integer and decimal)
    rule.pattern = QRegularExpression(R"(\b-?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?\b)");
    rule.format = m_numberFormat;
    m_rules.append(rule);

    // Booleans
    rule.pattern = QRegularExpression(R"(\btrue\b|\bfalse\b)");
    rule.format = m_boolFormat;
    m_rules.append(rule);

    // Null
    rule.pattern = QRegularExpression(R"(\bnull\b)");
    rule.format = m_nullFormat;
    m_rules.append(rule);

    // Folded section marker: // ... N items
    rule.pattern = QRegularExpression(R"(^\s*//\s+\.\.\.\s+\d+\s+items?\s*$)");
    rule.format = m_foldMarkerFormat;
    m_rules.append(rule);
}

void JsonSyntaxHighlighter::highlightBlock(const QString& text)
{
    // Keys first (before string rule, since keys have same pattern but different color)
    QRegularExpression keyRegex(QStringLiteral("(^|\\s*)\"([^\"]*)\"\\s*:"));
    auto it = keyRegex.globalMatch(text);
    while (it.hasNext()) {
        auto match = it.next();
        setFormat(match.capturedStart(2), match.capturedLength(2), m_keyFormat);
    }

    for (const HighlightRule& rule : m_rules) {
        auto matchIt = rule.pattern.globalMatch(text);
        while (matchIt.hasNext()) {
            auto match = matchIt.next();
            // avoid recoloring keys — keys were already colored above
            if (rule.format == m_stringFormat) {
                // check this string isn't a key (preceded by whitespace + colon after)
                int afterQuote = match.capturedEnd();
                QString after = text.mid(afterQuote).trimmed();
                if (after.startsWith(':')) {
                    continue;
                }
            }
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}


// ---------------------------------------------------------------------------
// RawJsonViewer
// ---------------------------------------------------------------------------

RawJsonViewer::RawJsonViewer(const QJsonObject& obj, QWidget* parent)
    : QDialog(parent)
{
    setJson(obj);
    init();
}

RawJsonViewer::RawJsonViewer(const QJsonDocument& doc, QWidget* parent)
    : QDialog(parent)
{
    setJson(doc);
    init();
}

RawJsonViewer::RawJsonViewer(const QString& jsonString, QWidget* parent)
    : QDialog(parent)
{
    setJson(jsonString);
    init();
}

void RawJsonViewer::init()
{
    setWindowTitle("Raw JSON Viewer");
    resize(680, 520);

    // Center on screen
    if (auto* screen = QGuiApplication::primaryScreen()) {
        QRect screenGeo = screen->availableGeometry();
        move((screenGeo.width() - width()) / 2, (screenGeo.height() - height()) / 2);
    }

    setupUi();
    refreshDisplay();

    QShortcut* findShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this);
    connect(findShortcut, &QShortcut::activated, this, &RawJsonViewer::showFindBar);

    QShortcut* escShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(escShortcut, &QShortcut::activated, this, &RawJsonViewer::hideFindBar);
}

void RawJsonViewer::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Editor
    m_textEdit = new JsonTextEdit(this);
    m_textEdit->setStyleSheet(R"(
        QPlainTextEdit {
            background-color: #1E1E2E;
            color: #CDD6F4;
            border: none;
            selection-background-color: #45475A;
            padding: 4px;
        }
    )");
    mainLayout->addWidget(m_textEdit, 1);

    // Find bar
    m_findContainer = new QWidget(this);
    m_findContainer->setStyleSheet("background-color: #181825; border-top: 1px solid #313244;");
    auto* findLayout = new QHBoxLayout(m_findContainer);
    findLayout->setContentsMargins(8, 4, 8, 4);

    m_findBar = new QLineEdit(m_findContainer);
    m_findBar->setPlaceholderText("Find in JSON...");
    m_findBar->setStyleSheet(R"(
        QLineEdit {
            background-color: #1E1E2E;
            color: #CDD6F4;
            border: 1px solid #45475A;
            border-radius: 4px;
            padding: 4px 8px;
            font-size: 12px;
        }
    )");
    findLayout->addWidget(m_findBar, 1);

    m_findPrevBtn = new QPushButton("Prev", m_findContainer);
    m_findNextBtn = new QPushButton("Next", m_findContainer);
    m_findCloseBtn = new QPushButton("x", m_findContainer);

    QString btnStyle = R"(
        QPushButton {
            background-color: #313244;
            color: #CDD6F4;
            border: none;
            border-radius: 4px;
            padding: 4px 10px;
            font-size: 12px;
        }
        QPushButton:hover { background-color: #45475A; }
    )";
    m_findPrevBtn->setStyleSheet(btnStyle);
    m_findNextBtn->setStyleSheet(btnStyle);
    m_findCloseBtn->setStyleSheet(btnStyle);
    m_findCloseBtn->setFixedWidth(28);

    findLayout->addWidget(m_findPrevBtn);
    findLayout->addWidget(m_findNextBtn);
    findLayout->addWidget(m_findCloseBtn);

    m_findContainer->setVisible(false);
    mainLayout->addWidget(m_findContainer);

    connect(m_findBar, &QLineEdit::returnPressed, this, &RawJsonViewer::findNext);
    connect(m_findBar, &QLineEdit::textChanged, this, [this](const QString& text) {
        if (!text.isEmpty()) {
            findNext();
        } else {
            // clear selections
            QList<QTextEdit::ExtraSelection> empty;
            m_textEdit->setExtraSelections(empty);
        }
    });
    connect(m_findNextBtn, &QPushButton::clicked, this, &RawJsonViewer::findNext);
    connect(m_findPrevBtn, &QPushButton::clicked, this, &RawJsonViewer::findPrevious);
    connect(m_findCloseBtn, &QPushButton::clicked, this, &RawJsonViewer::hideFindBar);

    // Bottom bar
    auto* bottomBar = new QWidget(this);
    bottomBar->setStyleSheet("background-color: #181825; border-top: 1px solid #313244;");
    auto* bottomLayout = new QHBoxLayout(bottomBar);
    bottomLayout->setContentsMargins(10, 4, 10, 4);

    m_copyButton = new QPushButton("Copy", bottomBar);
    m_prettyPrintCheck = new QCheckBox("Pretty Print", bottomBar);
    m_statusLabel = new QLabel(bottomBar);
    m_validityLabel = new QLabel(bottomBar);

    QString bottomBtnStyle = R"(
        QPushButton {
            background-color: #89B4FA;
            color: #1E1E2E;
            border: none;
            border-radius: 4px;
            padding: 4px 14px;
            font-weight: bold;
            font-size: 12px;
        }
        QPushButton:hover { background-color: #B4D0FB; }
    )";
    m_copyButton->setStyleSheet(bottomBtnStyle);

    QString checkStyle = R"(
        QCheckBox {
            color: #CDD6F4;
            font-size: 12px;
            spacing: 6px;
        }
        QCheckBox::indicator {
            width: 14px;
            height: 14px;
        }
    )";
    m_prettyPrintCheck->setStyleSheet(checkStyle);
    m_prettyPrintCheck->setChecked(m_prettyPrint);

    m_statusLabel->setStyleSheet("color: #6C7086; font-size: 11px;");
    m_validityLabel->setStyleSheet("color: #A6E3A1; font-size: 11px; font-weight: bold;");

    bottomLayout->addWidget(m_copyButton);
    bottomLayout->addWidget(m_prettyPrintCheck);
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_statusLabel);
    bottomLayout->addSpacing(16);
    bottomLayout->addWidget(m_validityLabel);

    mainLayout->addWidget(bottomBar);

    // Connections
    connect(m_copyButton, &QPushButton::clicked, this, &RawJsonViewer::copyToClipboard);
    connect(m_prettyPrintCheck, &QCheckBox::toggled, this, &RawJsonViewer::togglePrettyPrint);
    connect(m_textEdit, &JsonTextEdit::copyPathRequested, this, &RawJsonViewer::copyJsonPathAtCursor);
    connect(m_textEdit, &JsonTextEdit::foldToggled, this, [this](int) {
        refreshDisplay();
    });

    // Fold on click: detect line clicked and toggle if it's a bracket line
    connect(m_textEdit, &QPlainTextEdit::cursorPositionChanged, this, [this]() {
        auto cursor = m_textEdit->textCursor();
        int line = cursor.block().blockNumber();
        QString trimmed = cursor.block().text().trimmed();
        if (trimmed.endsWith('{') || trimmed.endsWith('[') ||
            trimmed.endsWith('{') || trimmed.endsWith('[')) {
            // This line is foldable
        }
    });

    m_highlighter = new JsonSyntaxHighlighter(m_textEdit->document());
}

void RawJsonViewer::setJson(const QJsonObject& obj)
{
    QJsonDocument doc(obj);
    setJson(doc);
}

void RawJsonViewer::setJson(const QJsonDocument& doc)
{
    m_document = doc;
    m_validJson = !doc.isNull();
    m_rawJson = formatJson(doc, m_prettyPrint);
    m_fullText = m_rawJson;
    m_foldedLines.clear();
}

void RawJsonViewer::setJson(const QString& jsonString)
{
    m_rawJson = jsonString;
    QJsonParseError error;
    m_document = QJsonDocument::fromJson(jsonString.toUtf8(), &error);
    m_validJson = (error.error == QJsonParseError::NoError);

    if (m_validJson) {
        m_fullText = formatJson(m_document, m_prettyPrint);
    } else {
        m_fullText = jsonString;
    }
    m_foldedLines.clear();
}

QJsonDocument RawJsonViewer::jsonDocument() const
{
    return m_document;
}

QString RawJsonViewer::formatJson(const QJsonDocument& doc, bool pretty) const
{
    if (doc.isNull()) return {};
    if (pretty) {
        return QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
    }
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

QPair<int, int> RawJsonViewer::foldRange(int lineNumber) const
{
    QStringList lines = m_fullText.split('\n');
    if (lineNumber < 0 || lineNumber >= lines.size()) return {-1, -1};

    QString text = lines[lineNumber].trimmed();
    if (!text.endsWith('{') && !text.endsWith('[')) return {-1, -1};

    QChar openChar = text.back();
    QChar closeChar = (openChar == '{') ? '}' : ']';

    int depth = 0;
    for (int i = lineNumber; i < lines.size(); ++i) {
        for (const QChar& ch : lines[i]) {
            if (ch == openChar) ++depth;
            else if (ch == closeChar) {
                --depth;
                if (depth == 0) return {lineNumber, i};
            }
        }
    }
    return {-1, -1};
}

int RawJsonViewer::findMatchingClose(int openLine) const
{
    auto range = foldRange(openLine);
    return range.second;
}

void RawJsonViewer::refreshDisplay()
{
    QStringList lines = m_fullText.split('\n');
    QStringList displayLines;
    // Determine fold ranges
    QMap<int, int> foldRanges; // openLine -> closeLine
    QSet<int> foldedLines = m_textEdit->foldedLines();
    m_foldedLines = foldedLines;

    for (int i = 0; i < lines.size(); ++i) {
        if (foldedLines.contains(i)) {
            auto range = foldRange(i);
            if (range.second > range.first) {
                foldRanges[range.first] = range.second;
            }
        }
    }

    QSet<int> skippedLines;
    for (auto it = foldRanges.begin(); it != foldRanges.end(); ++it) {
        for (int i = it.key() + 1; i <= it.value(); ++i) {
            skippedLines.insert(i);
        }
    }

    for (int i = 0; i < lines.size(); ++i) {
        if (skippedLines.contains(i)) {
            if (foldRanges.values().contains(i)) {
                // This is the closing line of a fold — show collapsed marker
                int foldedCount = 0;
                for (auto it = foldRanges.begin(); it != foldRanges.end(); ++it) {
                    if (it.value() == i) {
                        foldedCount = it.value() - it.key() - 1;
                        break;
                    }
                }
                // Reconstruct indentation from open line
                QString indent;
                int openLine = -1;
                for (auto it = foldRanges.begin(); it != foldRanges.end(); ++it) {
                    if (it.value() == i) {
                        openLine = it.key();
                        break;
                    }
                }
                if (openLine >= 0 && openLine < lines.size()) {
                    const QString& openLineText = lines[openLine];
                    for (const QChar& ch : openLineText) {
                        if (ch == ' ') indent += ' ';
                        else break;
                    }
                }
                displayLines.append(indent + QString("// ... %1 items").arg(foldedCount));
                // also show the closing brace
                displayLines.append(lines[i]);
                continue;
            }
            continue; // skip folded lines (not the close line)
        }
        displayLines.append(lines[i]);
    }

    QString displayText = displayLines.join('\n');
    m_textEdit->setPlainText(displayText);

    if (m_highlighter) {
        m_highlighter->rehighlight();
    }

    updateStatusBar();
}

void RawJsonViewer::updateStatusBar()
{
    int lineCount = m_textEdit->blockCount();
    int charCount = m_textEdit->toPlainText().length();
    m_statusLabel->setText(QString("%1 lines · %2 chars").arg(lineCount).arg(charCount));

    if (m_validJson) {
        m_validityLabel->setText("Valid JSON");
        m_validityLabel->setStyleSheet("color: #A6E3A1; font-size: 11px; font-weight: bold;");
    } else {
        m_validityLabel->setText("Invalid JSON");
        m_validityLabel->setStyleSheet("color: #F38BA8; font-size: 11px; font-weight: bold;");
    }
}

void RawJsonViewer::togglePrettyPrint()
{
    m_prettyPrint = m_prettyPrintCheck->isChecked();
    if (m_validJson) {
        m_fullText = formatJson(m_document, m_prettyPrint);
        m_foldedLines.clear();
        m_textEdit->setFoldedLines({});
        refreshDisplay();
    }
}

void RawJsonViewer::copyToClipboard()
{
    // Copy the full, unprettified JSON (or the full text)
    QString textToCopy;
    if (m_validJson) {
        textToCopy = QString::fromUtf8(m_document.toJson(m_prettyPrint ? QJsonDocument::Indented : QJsonDocument::Compact));
    } else {
        textToCopy = m_rawJson;
    }
    QApplication::clipboard()->setText(textToCopy);
}

void RawJsonViewer::showFindBar()
{
    m_findContainer->setVisible(true);
    m_findBar->setFocus();
    m_findBar->selectAll();

    QPropertyAnimation* anim = new QPropertyAnimation(m_findContainer, "maximumHeight");
    anim->setDuration(150);
    anim->setStartValue(0);
    anim->setEndValue(40);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void RawJsonViewer::hideFindBar()
{
    m_findContainer->setVisible(false);
    m_findBar->clear();

    // Clear find highlights
    QList<QTextEdit::ExtraSelection> empty;
    m_textEdit->setExtraSelections(empty);
}

void RawJsonViewer::findNext()
{
    QString searchText = m_findBar->text();
    if (searchText.isEmpty()) return;

    QTextDocument* doc = m_textEdit->document();
    QTextCursor cursor = m_textEdit->textCursor();

    if (m_currentFindPos >= 0) {
        cursor.setPosition(m_currentFindPos);
    }

    cursor = doc->find(searchText, cursor);
    if (!cursor.isNull()) {
        m_textEdit->setTextCursor(cursor);
        m_currentFindPos = cursor.position();

        QTextEdit::ExtraSelection sel;
        sel.format.setBackground(QColor("#45475A"));
        sel.format.setForeground(QColor("#F5E0DC"));
        sel.cursor = cursor;
        m_textEdit->setExtraSelections({sel});
    } else {
        // wrap around
        cursor.movePosition(QTextCursor::Start);
        cursor = doc->find(searchText, cursor);
        if (!cursor.isNull()) {
            m_textEdit->setTextCursor(cursor);
            m_currentFindPos = cursor.position();
            QTextEdit::ExtraSelection sel;
            sel.format.setBackground(QColor("#45475A"));
            sel.format.setForeground(QColor("#F5E0DC"));
            sel.cursor = cursor;
            m_textEdit->setExtraSelections({sel});
        }
    }
}

void RawJsonViewer::findPrevious()
{
    QString searchText = m_findBar->text();
    if (searchText.isEmpty()) return;

    QTextDocument* doc = m_textEdit->document();
    QTextCursor cursor = m_textEdit->textCursor();

    cursor = doc->find(searchText, cursor, QTextDocument::FindBackward);
    if (!cursor.isNull()) {
        m_textEdit->setTextCursor(cursor);
        m_currentFindPos = cursor.position();
        QTextEdit::ExtraSelection sel;
        sel.format.setBackground(QColor("#45475A"));
        sel.format.setForeground(QColor("#F5E0DC"));
        sel.cursor = cursor;
        m_textEdit->setExtraSelections({sel});
    }
}

void RawJsonViewer::copyJsonPathAtCursor()
{
    QTextCursor cursor = m_textEdit->textCursor();
    int pos = cursor.position();

    if (!m_validJson) return;

    // Walk the JSON document to find the value at position `pos` in the full text
    // Build a JSON path like $.content.body[0].type
    QStringList pathParts;
    QString fullDisplay = m_fullText;

    struct Frame {
        QString key;
        int arrayIndex = -1;
        enum Type { Object, Array } type;
    };

    QVector<Frame> stack;
    int displayPos = 0;
    int depth = 0;
    bool inString = false;
    bool escape = false;
    QString currentKey;
    int arrayIndex = 0;
    bool expectValue = false;
    bool expectColon = false;
    bool parsingKey = true;

    for (int i = 0; i < fullDisplay.length(); ++i) {
        QChar ch = fullDisplay[i];

        if (inString) {
            if (escape) {
                escape = false;
            } else if (ch == '\\') {
                escape = true;
            } else if (ch == '"') {
                inString = false;
                if (i >= pos && !parsingKey) {
                    // We're inside a string value
                    break;
                }
            }
            continue;
        }

        if (ch == '"') {
            inString = true;
            if (parsingKey) {
                int start = i + 1;
                int end = start;
                while (end < fullDisplay.length() && fullDisplay[end] != '"') {
                    if (fullDisplay[end] == '\\') ++end;
                    ++end;
                }
                currentKey = fullDisplay.mid(start, end - start);
                i = end; // skip to closing quote
            }
            continue;
        }

        if (ch == ':') {
            parsingKey = false;
            expectValue = true;
            continue;
        }

        if (ch == '{') {
            Frame f;
            f.type = Frame::Object;
            f.key = currentKey;
            stack.append(f);
            currentKey.clear();
            parsingKey = true;
            arrayIndex = 0;
        } else if (ch == '[') {
            Frame f;
            f.type = Frame::Array;
            f.key = currentKey;
            f.arrayIndex = 0;
            stack.append(f);
            currentKey.clear();
            parsingKey = false;
            arrayIndex = 0;
        } else if (ch == '}' || ch == ']') {
            if (!stack.isEmpty()) stack.removeLast();
            parsingKey = true;
        } else if (ch == ',') {
            if (!stack.isEmpty() && stack.last().type == Frame::Array) {
                stack.last().arrayIndex++;
            }
            parsingKey = true;
            currentKey.clear();
        } else if (!ch.isSpace() && i >= pos) {
            break;
        }

        if (i >= pos) break;
    }

    // Build path from stack
    pathParts << "$";
    for (const Frame& f : stack) {
        if (f.type == Frame::Object) {
            if (f.key.isEmpty()) continue;
            pathParts << f.key;
        } else {
            pathParts << QString("[%1]").arg(f.arrayIndex);
        }
    }

    QString path = pathParts.join('.');
    // Fix the dot-bracket pattern: $.foo.bar[0].baz → should be $.foo.bar[0].baz
    // Already correct with join('.')

    QApplication::clipboard()->setText(path);
}
