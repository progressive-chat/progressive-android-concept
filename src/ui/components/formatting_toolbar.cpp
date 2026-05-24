#include "formatting_toolbar.hpp"

#include <QHBoxLayout>
#include <QInputDialog>
#include <QTextEdit>
#include <QTextBlock>
#include <QTextCursor>
#include <QToolButton>

FormattingToolbar::FormattingToolbar(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(2, 0, 2, 0);
    layout->setSpacing(1);

    m_boldButton = createToggleButton(QStringLiteral("B"),
        QStringLiteral("Bold (Ctrl+B)"));
    m_italicButton = createToggleButton(QStringLiteral("I"),
        QStringLiteral("Italic (Ctrl+I)"));
    m_strikethroughButton = createToggleButton(QStringLiteral("S"),
        QStringLiteral("Strikethrough (Ctrl+Shift+S)"));
    m_codeButton = createToggleButton(QStringLiteral("</>"),
        QStringLiteral("Code (Ctrl+E)"));
    m_quoteButton = createToggleButton(QStringLiteral("\""),
        QStringLiteral("Quote (Ctrl+Shift+>)"));
    m_listButton = createToggleButton(QStringLiteral("\u2261"),
        QStringLiteral("List (Ctrl+Shift+L)"));
    m_numberedListButton = createToggleButton(QStringLiteral("1."),
        QStringLiteral("Numbered List (Ctrl+Shift+N)"));
    m_linkButton = createButton(QStringLiteral("\U0001F517"),
        QStringLiteral("Link (Ctrl+K)"));

    layout->addWidget(m_boldButton);
    layout->addWidget(m_italicButton);
    layout->addWidget(m_strikethroughButton);
    layout->addWidget(m_codeButton);
    layout->addWidget(m_quoteButton);
    layout->addWidget(m_listButton);
    layout->addWidget(m_numberedListButton);
    layout->addWidget(m_linkButton);
    layout->addStretch();

    connect(m_boldButton, &QToolButton::clicked, this, &FormattingToolbar::onBold);
    connect(m_italicButton, &QToolButton::clicked, this, &FormattingToolbar::onItalic);
    connect(m_strikethroughButton, &QToolButton::clicked, this, &FormattingToolbar::onStrikethrough);
    connect(m_codeButton, &QToolButton::clicked, this, &FormattingToolbar::onCode);
    connect(m_quoteButton, &QToolButton::clicked, this, &FormattingToolbar::onQuote);
    connect(m_listButton, &QToolButton::clicked, this, &FormattingToolbar::onList);
    connect(m_numberedListButton, &QToolButton::clicked, this, &FormattingToolbar::onNumberedList);
    connect(m_linkButton, &QToolButton::clicked, this, &FormattingToolbar::onLink);

    const auto buttonStyle = QStringLiteral(
        "QToolButton {"
        "  border: 1px solid transparent;"
        "  border-radius: 3px;"
        "  padding: 2px 6px;"
        "  font-weight: normal;"
        "  background: transparent;"
        "}"
        "QToolButton:hover {"
        "  border-color: #c0c0c0;"
        "  background: #e8e8e8;"
        "}"
        "QToolButton:checked {"
        "  border-color: #a0a0a0;"
        "  background: #d0d0d0;"
        "  font-weight: bold;"
        "}"
    );

    setStyleSheet(buttonStyle);
    setFixedHeight(kButtonHeight + layout->contentsMargins().top() + layout->contentsMargins().bottom());
}

void FormattingToolbar::setTarget(QTextEdit* editor)
{
    if (m_target) {
        disconnect(m_target, &QTextEdit::cursorPositionChanged,
                   this, &FormattingToolbar::updateButtonStates);
    }

    m_target = editor;

    if (m_target) {
        connect(m_target, &QTextEdit::cursorPositionChanged,
                this, &FormattingToolbar::updateButtonStates);
        updateButtonStates();
    }
}

void FormattingToolbar::onBold()
{
    toggleWrapping(QStringLiteral("**"), QStringLiteral("**"));
}

void FormattingToolbar::onItalic()
{
    toggleWrapping(QStringLiteral("*"), QStringLiteral("*"));
}

void FormattingToolbar::onStrikethrough()
{
    toggleWrapping(QStringLiteral("~~"), QStringLiteral("~~"));
}

void FormattingToolbar::onCode()
{
    if (!m_target) return;

    QTextCursor cursor = m_target->textCursor();
    const QString text = cursor.selectedText();

    if (text.isEmpty()) {
        cursor.insertText(QStringLiteral("``"));
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);
        m_target->setTextCursor(cursor);
        return;
    }

    if (text.contains(u'\n')) {
        insertMultilineFence(QStringLiteral("```"));
    } else {
        // Toggle inline code
        const int origStart = cursor.selectionStart();
        const int origEnd = cursor.selectionEnd();

        cursor.setPosition(origStart - 1);
        cursor.setPosition(origEnd + 1, QTextCursor::KeepAnchor);
        const QString surrounding = cursor.selectedText();

        if (surrounding.startsWith(u'`') && surrounding.endsWith(u'`') && surrounding.length() >= 2) {
            cursor.setPosition(origStart - 1);
            cursor.setPosition(origEnd + 1, QTextCursor::KeepAnchor);
            cursor.insertText(text);
        } else {
            cursor.setPosition(origStart);
            cursor.setPosition(origEnd, QTextCursor::KeepAnchor);
            cursor.insertText(u'`' + text + u'`');
        }
    }

    m_target->setFocus();
}

void FormattingToolbar::onQuote()
{
    toggleLinePrefix(QStringLiteral("> "));
}

void FormattingToolbar::onList()
{
    toggleLinePrefix(QStringLiteral("- "));
}

void FormattingToolbar::onNumberedList()
{
    toggleLinePrefix(QStringLiteral("1. "));
}

void FormattingToolbar::onLink()
{
    if (!m_target) return;

    QTextCursor cursor = m_target->textCursor();
    const QString linkText = cursor.selectedText();

    bool ok = false;
    const QString url = QInputDialog::getText(this,
        QStringLiteral("Insert Link"),
        QStringLiteral("URL:"),
        QLineEdit::Normal,
        QStringLiteral("https://"),
        &ok);

    if (!ok || url.isEmpty()) return;

    cursor = m_target->textCursor();

    if (linkText.isEmpty()) {
        const QString placeholder = QStringLiteral("[text](%1)").arg(url);
        cursor.insertText(placeholder);
        // Select "text" for easy replacement
        const int bracketPos = cursor.position() - placeholder.length();
        cursor.setPosition(bracketPos + 1);
        cursor.setPosition(bracketPos + 5, QTextCursor::KeepAnchor);
    } else {
        const QString link = QStringLiteral("[%1](%2)").arg(linkText, url);
        cursor.insertText(link);
    }

    m_target->setTextCursor(cursor);
    m_target->setFocus();
}

void FormattingToolbar::updateButtonStates()
{
    if (!m_target) return;

    const bool bold = isCursorInsideDelimiter(QStringLiteral("**"));
    const bool italic = isCursorInsideDelimiter(QStringLiteral("*"));
    const bool strike = isCursorInsideDelimiter(QStringLiteral("~~"));
    const bool code = isCursorInsideDelimiter(QStringLiteral("`")) ||
                      isCursorInsideDelimiter(QStringLiteral("```"));

    m_boldButton->setChecked(bold);
    m_italicButton->setChecked(italic);
    m_strikethroughButton->setChecked(strike);
    m_codeButton->setChecked(code);
    m_quoteButton->setChecked(isCurrentLinePrefixedWith(QStringLiteral("> ")));
    m_listButton->setChecked(isCurrentLinePrefixedWith(QStringLiteral("- ")));
    m_numberedListButton->setChecked(isCurrentLinePrefixedWith(QStringLiteral("1. ")));
}

QToolButton* FormattingToolbar::createButton(const QString& text, const QString& tooltip)
{
    auto* button = new QToolButton(this);
    button->setText(text);
    button->setToolTip(tooltip);
    button->setFixedHeight(kButtonHeight);
    button->setToolButtonStyle(Qt::ToolButtonTextOnly);
    button->setAutoRaise(true);
    return button;
}

QToolButton* FormattingToolbar::createToggleButton(const QString& text, const QString& tooltip)
{
    auto* button = createButton(text, tooltip);
    button->setCheckable(true);
    return button;
}

void FormattingToolbar::toggleWrapping(const QString& left, const QString& right)
{
    if (!m_target) return;

    QTextCursor cursor = m_target->textCursor();
    const QString text = cursor.selectedText();

    if (text.isEmpty()) {
        const QString insertion = left + right;
        cursor.insertText(insertion);
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, right.length());
        m_target->setTextCursor(cursor);
        return;
    }

    const int origStart = cursor.selectionStart();
    const int origEnd = cursor.selectionEnd();
    const int leftLen = left.length();
    const int rightLen = right.length();

    cursor.setPosition(std::max(0, origStart - leftLen));
    cursor.setPosition(origEnd + rightLen, QTextCursor::KeepAnchor);
    const QString surrounding = cursor.selectedText();

    if (surrounding.startsWith(left) && surrounding.endsWith(right) && surrounding.length() >= leftLen + rightLen) {
        cursor.setPosition(origStart - leftLen);
        cursor.setPosition(origEnd + rightLen, QTextCursor::KeepAnchor);
        cursor.insertText(text);
        cursor.setPosition(origStart - leftLen);
        cursor.setPosition(origEnd - leftLen, QTextCursor::KeepAnchor);
    } else {
        cursor.setPosition(origStart);
        cursor.setPosition(origEnd, QTextCursor::KeepAnchor);
        cursor.insertText(left + text + right);
        cursor.setPosition(origStart);
        cursor.setPosition(origEnd + leftLen + rightLen, QTextCursor::KeepAnchor);
    }

    m_target->setTextCursor(cursor);
    m_target->setFocus();
}

void FormattingToolbar::toggleLinePrefix(const QString& prefix)
{
    if (!m_target) return;

    QTextCursor cursor = m_target->textCursor();
    cursor.beginEditBlock();

    const int selStart = cursor.selectionStart();
    const int selEnd = cursor.selectionEnd();
    const int prefixLen = prefix.length();

    if (selStart == selEnd) {
        QTextBlock block = cursor.block();
        const QString line = block.text();

        cursor.movePosition(QTextCursor::StartOfBlock);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

        if (line.startsWith(prefix)) {
            cursor.insertText(line.mid(prefixLen));
        } else {
            cursor.insertText(prefix + line);
        }
    } else {
        cursor.setPosition(selStart);
        const int startBlockNum = cursor.block().blockNumber();
        cursor.setPosition(selEnd);
        const int endBlockNum = cursor.block().blockNumber();

        bool allPrefixed = true;
        for (int bn = startBlockNum; bn <= endBlockNum && allPrefixed; ++bn) {
            const QTextBlock block = m_target->document()->findBlockByNumber(bn);
            if (!block.isValid() || !block.text().startsWith(prefix)) {
                allPrefixed = false;
            }
        }

        for (int bn = startBlockNum; bn <= endBlockNum; ++bn) {
            QTextBlock block = m_target->document()->findBlockByNumber(bn);
            if (!block.isValid()) continue;

            cursor.setPosition(block.position());
            cursor.movePosition(QTextCursor::StartOfBlock);
            cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

            if (allPrefixed) {
                cursor.insertText(block.text().mid(prefixLen));
            } else {
                cursor.insertText(prefix + block.text());
            }
        }
    }

    cursor.endEditBlock();
    m_target->setTextCursor(cursor);
    m_target->setFocus();
}

void FormattingToolbar::insertMultilineFence(const QString& fence)
{
    if (!m_target) return;

    QTextCursor cursor = m_target->textCursor();
    const QString text = cursor.selectedText();
    const int origStart = cursor.selectionStart();
    const int origEnd = cursor.selectionEnd();

    // Check if already fenced (look for fence before and after selection)
    const QString before = m_target->toPlainText().mid(
        std::max(0, origStart - static_cast<int>(fence.length()) - 1),
        fence.length() + 1);
    const QString after = m_target->toPlainText().mid(
        origEnd,
        fence.length() + 1);

    const bool hasOpenFence = before.contains(fence + u'\n');
    const bool hasCloseFence = after.startsWith(u'\n' + fence);

    if (hasOpenFence && hasCloseFence) {
        // Remove fences
        const int openStart = origStart - static_cast<int>(fence.length()) - 1;
        cursor.setPosition(openStart);
        cursor.setPosition(origEnd + 1 + static_cast<int>(fence.length()), QTextCursor::KeepAnchor);
        cursor.insertText(text);
    } else {
        cursor.setPosition(origStart);
        cursor.setPosition(origEnd, QTextCursor::KeepAnchor);
        cursor.insertText(fence + u'\n' + text + u'\n' + fence);
    }

    m_target->setTextCursor(cursor);
    m_target->setFocus();
}

bool FormattingToolbar::isCursorInsideDelimiter(const QString& delim) const
{
    if (!m_target || delim.isEmpty()) return false;

    const QString fullText = m_target->toPlainText();
    const int pos = m_target->textCursor().position();
    const int delimLen = delim.length();
    const int searchStart = std::max(0, pos - kContextSearchRadius);
    const int searchEnd = std::min(static_cast<int>(fullText.length()), pos + kContextSearchRadius);

    const QString contextBefore = fullText.mid(searchStart, pos - searchStart);
    const QString contextAfter = fullText.mid(pos, searchEnd - pos);

    int count = 0;
    int idx = 0;
    while (idx <= contextBefore.length() - delimLen) {
        if (QStringView(contextBefore).mid(idx, delimLen) == delim) {
            ++count;
            idx += delimLen;
        } else {
            ++idx;
        }
    }

    return (count % 2) == 1;
}

bool FormattingToolbar::isCurrentLinePrefixedWith(const QString& prefix) const
{
    if (!m_target) return false;

    QTextCursor cursor = m_target->textCursor();
    cursor.movePosition(QTextCursor::StartOfBlock);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    return cursor.selectedText().startsWith(prefix);
}

QString FormattingToolbar::selectedText() const
{
    if (!m_target) return {};
    return m_target->textCursor().selectedText();
}

int FormattingToolbar::cursorPosition() const
{
    if (!m_target) return 0;
    return m_target->textCursor().position();
}
