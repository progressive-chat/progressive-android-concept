#include "ui/components/markdown_renderer.hpp"

#include <QTextBrowser>
#include <QVBoxLayout>
#include <QRegularExpression>
#include <QUrl>

// =========================================================================
// Construction
// =========================================================================

MarkdownRenderer::MarkdownRenderer(QWidget* parent)
    : QWidget(parent)
    , m_textBrowser(nullptr)
    , m_layout(nullptr)
{
    setupUi();
}

// =========================================================================
// Setup
// =========================================================================

void MarkdownRenderer::setupUi()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    m_textBrowser = new QTextBrowser(this);
    m_textBrowser->setReadOnly(true);
    m_textBrowser->setOpenLinks(false);
    m_textBrowser->setOpenExternalLinks(false);
    m_textBrowser->document()->setDefaultStyleSheet(QStringLiteral(
        "body { font-family: sans-serif; }"
        "code { background-color: #f0f0f0; font-family: monospace; "
        "padding: 1px 3px; border-radius: 3px; font-size: 0.92em; }"
        "pre { background-color: #f0f0f0; padding: 10px; border-radius: 4px; "
        "white-space: pre-wrap; }"
        "pre code { background: none; padding: 0; font-size: 0.9em; }"
        "blockquote { border-left: 3px solid #ccc; margin: 5px 0 5px 0; "
        "padding: 5px 10px; color: #666; }"
        "h1 { font-size: 1.5em; margin: 10px 0 5px 0; }"
        "h2 { font-size: 1.3em; margin: 8px 0 4px 0; }"
        "h3 { font-size: 1.1em; margin: 6px 0 3px 0; }"
        "hr { border: none; border-top: 1px solid #ddd; margin: 10px 0; }"
        "p { margin: 5px 0; }"
        "ul, ol { margin: 5px 0; padding-left: 24px; }"
        "li { margin: 2px 0; }"
    ));

    m_layout->addWidget(m_textBrowser);

    connect(m_textBrowser, &QTextBrowser::anchorClicked,
            this, [this](const QUrl& url) {
                emit linkClicked(url.toString());
            });
}

// =========================================================================
// setMarkdown
// =========================================================================

void MarkdownRenderer::setMarkdown(const QString& md)
{
    m_textBrowser->setHtml(markdownToHtml(md));
}

// =========================================================================
// markdownToHtml  (block-level parser)
// =========================================================================

QString MarkdownRenderer::markdownToHtml(const QString& md)
{
    const QStringList lines = md.split(QChar('\n'));
    QString html;

    bool inCodeBlock = false;
    QString codeBlockContent;
    bool inUnorderedList = false;
    bool inOrderedList = false;
    bool inBlockquote = false;
    int blockquoteLevel = 0;

    // Paragraph accumulation
    QString paragraphText;
    bool inParagraph = false;
    bool lastLineHadBreak = false;

    auto flushParagraph = [&]() {
        if (inParagraph) {
            html += QStringLiteral("<p>") + paragraphText
                    + QStringLiteral("</p>\n");
            paragraphText.clear();
            inParagraph = false;
            lastLineHadBreak = false;
        }
    };

    auto closeBlocks = [&]() {
        flushParagraph();
        if (inUnorderedList) {
            html += QStringLiteral("</ul>\n");
            inUnorderedList = false;
        }
        if (inOrderedList) {
            html += QStringLiteral("</ol>\n");
            inOrderedList = false;
        }
        if (inBlockquote) {
            for (int b = 0; b < blockquoteLevel; ++b)
                html += QStringLiteral("</blockquote>\n");
            inBlockquote = false;
            blockquoteLevel = 0;
        }
    };

    for (const QString& rawLine : lines) {
        QString line = rawLine;

        // Trailing double-space → hard line break
        bool endsWithBreak = line.endsWith(QStringLiteral("  "));
        if (endsWithBreak)
            line.chop(2);

        // ---------------------------------------------------------------
        // Code block toggle
        // ---------------------------------------------------------------
        if (line.trimmed().startsWith(QStringLiteral("```"))) {
            if (!inCodeBlock) {
                closeBlocks();
                inCodeBlock = true;
                codeBlockContent.clear();
            } else {
                inCodeBlock = false;
                html += QStringLiteral("<pre><code>")
                        + codeBlockContent.toHtmlEscaped()
                        + QStringLiteral("</code></pre>\n");
            }
            continue;
        }

        // Inside a code block – accumulate raw lines
        if (inCodeBlock) {
            if (!codeBlockContent.isEmpty())
                codeBlockContent += QChar('\n');
            codeBlockContent += line;
            continue;
        }

        // ---------------------------------------------------------------
        // Blank line – separates blocks
        // ---------------------------------------------------------------
        if (line.trimmed().isEmpty()) {
            closeBlocks();
            continue;
        }

        // ---------------------------------------------------------------
        // Header  # / ## / ### ...
        // ---------------------------------------------------------------
        QRegularExpression headerRe(
            QStringLiteral(R"(^(#{1,6})\s+(.*)$)"));
        QRegularExpressionMatch hMatch = headerRe.match(line);
        if (hMatch.hasMatch()) {
            closeBlocks();
            int level = hMatch.captured(1).length();
            QString content = processInline(hMatch.captured(2));
            html += QStringLiteral("<h%1>%2</h%1>\n")
                        .arg(level).arg(content);
            continue;
        }

        // ---------------------------------------------------------------
        // Horizontal rule  --- / *** / ___
        // ---------------------------------------------------------------
        QRegularExpression hrRe(
            QStringLiteral(R"(^(-{3,}|\*{3,}|_{3,})\s*$)"));
        if (hrRe.match(line).hasMatch()) {
            closeBlocks();
            html += QStringLiteral("<hr>\n");
            continue;
        }

        // ---------------------------------------------------------------
        // Blockquote  > ...
        // ---------------------------------------------------------------
        QRegularExpression bqRe(
            QStringLiteral(R"(^\s*(>+)\s*(.*)$)"));
        QRegularExpressionMatch bqMatch = bqRe.match(line);
        if (bqMatch.hasMatch()) {
            flushParagraph();
            if (inUnorderedList) {
                html += QStringLiteral("</ul>\n");
                inUnorderedList = false;
            }
            if (inOrderedList) {
                html += QStringLiteral("</ol>\n");
                inOrderedList = false;
            }

            int bqCount = bqMatch.captured(1).length();
            QString bqContent = bqMatch.captured(2);

            if (!inBlockquote) {
                for (int b = 0; b < bqCount; ++b)
                    html += QStringLiteral("<blockquote>\n");
                inBlockquote = true;
                blockquoteLevel = bqCount;
            } else if (bqCount != blockquoteLevel) {
                for (int b = 0; b < blockquoteLevel; ++b)
                    html += QStringLiteral("</blockquote>\n");
                for (int b = 0; b < bqCount; ++b)
                    html += QStringLiteral("<blockquote>\n");
                blockquoteLevel = bqCount;
            }

            html += QStringLiteral("<p>") + processInline(bqContent)
                    + QStringLiteral("</p>\n");
            if (endsWithBreak)
                html += QStringLiteral("<br>\n");
            continue;
        }

        if (inBlockquote) {
            for (int b = 0; b < blockquoteLevel; ++b)
                html += QStringLiteral("</blockquote>\n");
            inBlockquote = false;
            blockquoteLevel = 0;
        }

        // ---------------------------------------------------------------
        // Unordered list  - / * / +
        // ---------------------------------------------------------------
        QRegularExpression ulRe(
            QStringLiteral(R"(^[-*+]\s+(.*)$)"));
        QRegularExpressionMatch ulMatch = ulRe.match(line);
        if (ulMatch.hasMatch()) {
            flushParagraph();
            if (!inUnorderedList) {
                if (inOrderedList) {
                    html += QStringLiteral("</ol>\n");
                    inOrderedList = false;
                }
                html += QStringLiteral("<ul>\n");
                inUnorderedList = true;
            }
            html += QStringLiteral("<li>")
                    + processInline(ulMatch.captured(1))
                    + QStringLiteral("</li>\n");
            if (endsWithBreak)
                html += QStringLiteral("<br>\n");
            continue;
        }

        // ---------------------------------------------------------------
        // Ordered list  1. / 2. ...
        // ---------------------------------------------------------------
        QRegularExpression olRe(
            QStringLiteral(R"(^(\d+)\.\s+(.*)$)"));
        QRegularExpressionMatch olMatch = olRe.match(line);
        if (olMatch.hasMatch()) {
            flushParagraph();
            if (!inOrderedList) {
                if (inUnorderedList) {
                    html += QStringLiteral("</ul>\n");
                    inUnorderedList = false;
                }
                html += QStringLiteral("<ol>\n");
                inOrderedList = true;
            }
            html += QStringLiteral("<li>")
                    + processInline(olMatch.captured(2))
                    + QStringLiteral("</li>\n");
            if (endsWithBreak)
                html += QStringLiteral("<br>\n");
            continue;
        }

        // ---------------------------------------------------------------
        // Not a list item – close any open list
        // ---------------------------------------------------------------
        if (inUnorderedList) {
            html += QStringLiteral("</ul>\n");
            inUnorderedList = false;
        }
        if (inOrderedList) {
            html += QStringLiteral("</ol>\n");
            inOrderedList = false;
        }

        // ---------------------------------------------------------------
        // Regular paragraph line
        // ---------------------------------------------------------------
        QString processed = processInline(line);
        if (!inParagraph) {
            inParagraph = true;
            paragraphText = processed;
            lastLineHadBreak = endsWithBreak;
        } else {
            paragraphText += lastLineHadBreak
                                 ? QStringLiteral("<br>") + processed
                                 : QChar(' ') + processed;
            lastLineHadBreak = endsWithBreak;
        }
    }

    // Flush remaining open constructs
    closeBlocks();
    if (inCodeBlock) {
        html += QStringLiteral("<pre><code>")
                + codeBlockContent.toHtmlEscaped()
                + QStringLiteral("</code></pre>\n");
    }
    if (inUnorderedList)
        html += QStringLiteral("</ul>\n");
    if (inOrderedList)
        html += QStringLiteral("</ol>\n");
    if (inBlockquote) {
        for (int b = 0; b < blockquoteLevel; ++b)
            html += QStringLiteral("</blockquote>\n");
    }

    return html.trimmed();
}

// =========================================================================
// processInline  (inline-formatting parser)
// =========================================================================

QString MarkdownRenderer::processInline(QString text)
{
    // Helper: build a numeric HTML entity for an escaped character
    auto charToEntity = [](QChar c) -> QString {
        return QStringLiteral("&#%1;").arg(c.unicode());
    };

    // -------------------------------------------------------------------
    // 1. Protect inline code spans  `code`  →  placeholder
    // -------------------------------------------------------------------
    QStringList codeContents;
    {
        QRegularExpression re(QStringLiteral(R"(`([^`]+)`)"));
        int idx = 0;
        QRegularExpressionMatch m;
        while ((m = re.match(text)).hasMatch()) {
            QString ph = QStringLiteral("\x01%1\x02")
                             .arg(idx, 4, 16, QChar('0'));
            codeContents.append(m.captured(1));
            text.replace(m.capturedStart(), m.capturedLength(), ph);
            ++idx;
        }
    }

    // -------------------------------------------------------------------
    // 2. Handle backslash escapes  \*  →  placeholder
    // -------------------------------------------------------------------
    QStringList escapedChars;
    {
        QRegularExpression re(
            QStringLiteral(R"(\\([\\`*_{}\[\]()#+\-.!|~<>&]))"));
        int idx = 0;
        QRegularExpressionMatch m;
        while ((m = re.match(text)).hasMatch()) {
            QString ph = QStringLiteral("\x03%1\x04")
                             .arg(idx, 4, 16, QChar('0'));
            escapedChars.append(m.captured(1));
            text.replace(m.capturedStart(), m.capturedLength(), ph);
            ++idx;
        }
    }

    // -------------------------------------------------------------------
    // 3. HTML-escape remaining text
    // -------------------------------------------------------------------
    text = text.toHtmlEscaped();

    // -------------------------------------------------------------------
    // 4. Restore escaped characters as numeric HTML entities (so they
    //    render literally and do not participate in formatting matches)
    // -------------------------------------------------------------------
    for (int i = 0; i < escapedChars.size(); ++i) {
        QString ph = QStringLiteral("\x03%1\x04")
                         .arg(i, 4, 16, QChar('0'));
        text.replace(ph, charToEntity(escapedChars[i][0]));
    }

    // -------------------------------------------------------------------
    // 5. Images  ![alt](url)  – before links so linked images work
    // -------------------------------------------------------------------
    {
        QRegularExpression re(
            QStringLiteral(R"(!\[([^\]]*)\]\(([^\)]+)\))"));
        QRegularExpressionMatch m;
        while ((m = re.match(text)).hasMatch()) {
            text.replace(m.capturedStart(), m.capturedLength(),
                         QStringLiteral("<img src=\"%1\" alt=\"%2\" />")
                             .arg(m.captured(2), m.captured(1)));
        }
    }

    // -------------------------------------------------------------------
    // 6. Links  [text](url)
    // -------------------------------------------------------------------
    {
        QRegularExpression re(
            QStringLiteral(R"(\[([^\]]*)\]\(([^\)]+)\))"));
        QRegularExpressionMatch m;
        while ((m = re.match(text)).hasMatch()) {
            text.replace(m.capturedStart(), m.capturedLength(),
                         QStringLiteral("<a href=\"%1\">%2</a>")
                             .arg(m.captured(2), m.captured(1)));
        }
    }

    // -------------------------------------------------------------------
    // 7. Bold  **text**
    // -------------------------------------------------------------------
    {
        QRegularExpression re(QStringLiteral(R"(\*\*(.+?)\*\*)"));
        text.replace(re, QStringLiteral("<b>\\1</b>"));
    }

    // -------------------------------------------------------------------
    // 8. Italic  *text*  (but not **)
    // -------------------------------------------------------------------
    {
        QRegularExpression re(
            QStringLiteral(R"((?<!\*)\*(?!\*)(.+?)(?<!\*)\*(?!\*))"));
        text.replace(re, QStringLiteral("<i>\\1</i>"));
    }

    // -------------------------------------------------------------------
    // 9. Strikethrough  ~~text~~
    // -------------------------------------------------------------------
    {
        QRegularExpression re(QStringLiteral(R"(~~(.+?)~~)"));
        text.replace(re, QStringLiteral("<s>\\1</s>"));
    }

    // -------------------------------------------------------------------
    // 10. Matrix mentions  @user:server
    // -------------------------------------------------------------------
    {
        QRegularExpression re(
            QStringLiteral(R"((^|\s)@([\w.\-]+:[\w.\-]+))"));
        text.replace(re,
                     QStringLiteral("\\1<a href=\"matrix:u/\\2\">@\\2</a>"));
    }

    // -------------------------------------------------------------------
    // 11. Matrix room references  #room:server
    // -------------------------------------------------------------------
    {
        QRegularExpression re(
            QStringLiteral(R"((^|\s)#([\w.\-]+:[\w.\-]+))"));
        text.replace(re,
                     QStringLiteral("\\1<a href=\"matrix:r/\\2\">#\\2</a>"));
    }

    // -------------------------------------------------------------------
    // 12. Matrix commands  /command
    // -------------------------------------------------------------------
    {
        QRegularExpression re(
            QStringLiteral(R"((^|\s)(/[a-zA-Z]\w*))"));
        text.replace(re,
                     QStringLiteral(
                         "\\1<span style=\"color:#888888\">\\2</span>"));
    }

    // -------------------------------------------------------------------
    // 13. Auto-link plain URLs
    // -------------------------------------------------------------------
    {
        QRegularExpression re(
            QStringLiteral(R"((?<!["'>=])(https?://[^\s<>"')]+))"));
        text.replace(re, QStringLiteral("<a href=\"\\1\">\\1</a>"));
    }
    {
        QRegularExpression re(
            QStringLiteral(
                R"((?<!["'>=])www\.[\w\-]+(\.[\w\-]+)+[/?#]?[^\s<>"')]*))"));
        text.replace(re,
                     QStringLiteral("<a href=\"http://\\1\">\\1</a>"));
    }

    // -------------------------------------------------------------------
    // 14. Restore code spans as <code> elements
    // -------------------------------------------------------------------
    for (int i = 0; i < codeContents.size(); ++i) {
        QString ph = QStringLiteral("\x01%1\x02")
                         .arg(i, 4, 16, QChar('0'));
        text.replace(ph, QStringLiteral("<code>")
                             + codeContents[i].toHtmlEscaped()
                             + QStringLiteral("</code>"));
    }

    return text;
}
