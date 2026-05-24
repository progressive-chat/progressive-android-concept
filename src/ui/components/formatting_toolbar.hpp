#pragma once

#include <QWidget>

class QTextEdit;
class QToolButton;

class FormattingToolbar : public QWidget
{
    Q_OBJECT

public:
    explicit FormattingToolbar(QWidget* parent = nullptr);
    ~FormattingToolbar() override = default;

    void setTarget(QTextEdit* editor);
    QTextEdit* target() const { return m_target; }

private slots:
    void onBold();
    void onItalic();
    void onStrikethrough();
    void onCode();
    void onQuote();
    void onList();
    void onNumberedList();
    void onLink();
    void updateButtonStates();

private:
    static constexpr int kButtonHeight = 28;
    static constexpr int kContextSearchRadius = 2000;

    QToolButton* createButton(const QString& text, const QString& tooltip);
    QToolButton* createToggleButton(const QString& text, const QString& tooltip);

    void toggleWrapping(const QString& left, const QString& right);
    void toggleLinePrefix(const QString& prefix);
    void insertMultilineFence(const QString& fence);

    [[nodiscard]] bool isCursorInsideDelimiter(const QString& delim) const;
    [[nodiscard]] bool isCurrentLinePrefixedWith(const QString& prefix) const;
    [[nodiscard]] QString selectedText() const;
    [[nodiscard]] int cursorPosition() const;

    QTextEdit* m_target = nullptr;

    QToolButton* m_boldButton = nullptr;
    QToolButton* m_italicButton = nullptr;
    QToolButton* m_strikethroughButton = nullptr;
    QToolButton* m_codeButton = nullptr;
    QToolButton* m_quoteButton = nullptr;
    QToolButton* m_listButton = nullptr;
    QToolButton* m_numberedListButton = nullptr;
    QToolButton* m_linkButton = nullptr;
};
