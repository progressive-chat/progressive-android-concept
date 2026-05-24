#pragma once

#include <QWidget>
#include <QString>

class QTextBrowser;
class QVBoxLayout;

class MarkdownRenderer : public QWidget
{
    Q_OBJECT

public:
    explicit MarkdownRenderer(QWidget* parent = nullptr);

    void setMarkdown(const QString& md);
    static QString markdownToHtml(const QString& md);

signals:
    void linkClicked(const QString& url);

private:
    void setupUi();

    static QString processInline(QString text);

    QTextBrowser* m_textBrowser = nullptr;
    QVBoxLayout*   m_layout      = nullptr;
};
