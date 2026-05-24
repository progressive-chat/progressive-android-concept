#include "features/matrix_chat/command_palette.hpp"

#include <QApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPainter>
#include <QPropertyAnimation>
#include <QShortcut>
#include <QShowEvent>
#include <QVBoxLayout>

namespace {

constexpr int kPaletteWidth = 600;
constexpr int kPaletteMaxHeight = 400;
constexpr int kFadeDurationMs = 150;

QListWidgetItem *createCommandItem(const Command &cmd, QListWidget *parent)
{
    auto *item = new QListWidgetItem();
    item->setData(Qt::UserRole, cmd.name);
    item->setData(Qt::UserRole + 1, cmd.category);
    item->setSizeHint(QSize(0, 40));

    auto *rowWidget = new QWidget(parent);
    auto *rowLayout = new QHBoxLayout(rowWidget);
    rowLayout->setContentsMargins(12, 4, 12, 4);
    rowLayout->setSpacing(10);

    auto *iconLabel = new QLabel(cmd.icon, rowWidget);
    iconLabel->setFixedWidth(28);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet(QStringLiteral("font-size: 16px;"));
    rowLayout->addWidget(iconLabel);

    auto *nameLabel = new QLabel(cmd.name, rowWidget);
    nameLabel->setStyleSheet(QStringLiteral("font-size: 13px;"));
    rowLayout->addWidget(nameLabel, 1);

    if (!cmd.shortcut.isEmpty()) {
        auto *shortcutLabel = new QLabel(cmd.shortcut, rowWidget);
        shortcutLabel->setStyleSheet(QStringLiteral(
            "color: #888; font-size: 11px; background: #f3f4f6; "
            "border-radius: 3px; padding: 2px 6px;"));
        rowLayout->addWidget(shortcutLabel);
    }

    parent->setItemWidget(item, rowWidget);
    return item;
}

} // anonymous namespace

CommandPalette::CommandPalette(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::Dialog)
    , m_searchEdit(nullptr)
    , m_commandList(nullptr)
    , m_categoryLabel(nullptr)
    , m_countLabel(nullptr)
    , m_fadeAnimation(nullptr)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);
    setupUi();
    populateCommands();

    m_fadeAnimation = new QPropertyAnimation(this, "windowOpacity", this);
    m_fadeAnimation->setDuration(kFadeDurationMs);
    m_fadeAnimation->setStartValue(0.0);
    m_fadeAnimation->setEndValue(1.0);
    m_fadeAnimation->setEasingCurve(QEasingCurve::OutCubic);
}

void CommandPalette::installGlobalShortcut(QWidget *parent, CommandPalette *palette)
{
    auto *shortcut = new QShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+P")), parent);
    QObject::connect(shortcut, &QShortcut::activated, palette, &CommandPalette::open);
}

void CommandPalette::setupUi()
{
    setFixedSize(kPaletteWidth, kPaletteMaxHeight);

    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    auto *container = new QWidget(this);
    container->setObjectName(QStringLiteral("paletteContainer"));
    container->setStyleSheet(QStringLiteral(
        "#paletteContainer {"
        "  background: #ffffff;"
        "  border: 1px solid #d1d5db;"
        "  border-radius: 12px;"
        "}"));

    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_searchEdit = new QLineEdit(container);
    m_searchEdit->setPlaceholderText(tr("Type a command..."));
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setStyleSheet(QStringLiteral(
        "QLineEdit {"
        "  border: none;"
        "  border-bottom: 1px solid #e5e7eb;"
        "  border-radius: 12px 12px 0 0;"
        "  padding: 14px 16px;"
        "  font-size: 15px;"
        "  background: transparent;"
        "}"
        "QLineEdit:focus {"
        "  border-bottom: 2px solid #6366f1;"
        "}"));
    m_searchEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    layout->addWidget(m_searchEdit);

    auto *infoRow = new QHBoxLayout();
    infoRow->setContentsMargins(16, 6, 16, 6);

    m_categoryLabel = new QLabel(container);
    m_categoryLabel->setStyleSheet(QStringLiteral(
        "color: #6b7280; font-size: 11px; font-weight: bold;"));
    infoRow->addWidget(m_categoryLabel);

    infoRow->addStretch();

    m_countLabel = new QLabel(container);
    m_countLabel->setStyleSheet(QStringLiteral(
        "color: #9ca3af; font-size: 11px;"));
    infoRow->addWidget(m_countLabel);

    layout->addLayout(infoRow);

    m_commandList = new QListWidget(container);
    m_commandList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_commandList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_commandList->setStyleSheet(QStringLiteral(
        "QListWidget {"
        "  border: none;"
        "  border-radius: 0 0 12px 12px;"
        "  background: transparent;"
        "  outline: none;"
        "}"
        "QListWidget::item {"
        "  border-bottom: 1px solid #f3f4f6;"
        "}"
        "QListWidget::item:selected {"
        "  background: #eef2ff;"
        "  color: #3730a3;"
        "}"));
    layout->addWidget(m_commandList, 1);

    outerLayout->addWidget(container);

    connect(m_searchEdit, &QLineEdit::textChanged, this, &CommandPalette::onSearchTextChanged);
    connect(m_commandList, &QListWidget::itemActivated, this, &CommandPalette::onCommandActivated);
    connect(m_commandList, &QListWidget::currentItemChanged, this, [this](QListWidgetItem *current, QListWidgetItem *) {
        if (current) {
            m_categoryLabel->setText(current->data(Qt::UserRole + 1).toString());
        }
    });
}

void CommandPalette::populateCommands()
{
    m_allCommands = {
        {tr("Go to Room"),             tr("Navigation"), QStringLiteral("Ctrl+G"),   QStringLiteral("\U0001F4AC"), [this]() { emit goToRoomRequested(); }},
        {tr("Open Settings"),          tr("Navigation"), QStringLiteral("Ctrl+,"),   QStringLiteral("\u2699\ufe0f"), [this]() { emit openSettingsRequested(); }},
        {tr("Open Profile"),           tr("Navigation"), QString(),                  QStringLiteral("\U0001F464"), [this]() { emit openProfileRequested(); }},

        {tr("Start Direct Chat"),      tr("Chat"),       QStringLiteral("Ctrl+D"),   QStringLiteral("\u2709\ufe0f"), [this]() { emit startDirectChatRequested(); }},
        {tr("Create Room"),            tr("Chat"),       QString(),                  QStringLiteral("\U0001F3E0"), [this]() { emit createRoomRequested(); }},
        {tr("Join Room"),              tr("Chat"),       QStringLiteral("Ctrl+J"),   QStringLiteral("\U0001F50D"), [this]() { emit joinRoomRequested(); }},
        {tr("Mark All Read"),          tr("Chat"),       QStringLiteral("Ctrl+Shift+A"), QStringLiteral("\u2705"), [this]() { emit markAllReadRequested(); }},

        {tr("Connect Matrix"),         tr("Protocol"),   QString(),                  QStringLiteral("\U0001F310"), [this]() { emit connectMatrixRequested(); }},
        {tr("Connect IRC"),            tr("Protocol"),   QString(),                  QStringLiteral("\U0001F4E1"), [this]() { emit connectIrcRequested(); }},
        {tr("Add Lemmy Instance"),     tr("Protocol"),   QString(),                  QStringLiteral("\U0001F5DE"), [this]() { emit addLemmyInstanceRequested(); }},

        {tr("Toggle Theme"),           tr("View"),       QString(),                  QStringLiteral("\U0001F3A8"), [this]() { emit toggleThemeRequested(); }},
        {tr("Toggle Compact Mode"),    tr("View"),       QString(),                  QStringLiteral("\U0001F4D0"), [this]() { emit toggleCompactModeRequested(); }},
        {tr("Toggle Sidebar"),         tr("View"),       QStringLiteral("Ctrl+B"),   QStringLiteral("\u2630\ufe0f"), [this]() { emit toggleSidebarRequested(); }},

        {tr("Export Chat"),            tr("Tools"),      QString(),                  QStringLiteral("\U0001F4E4"), [this]() { emit exportChatRequested(); }},
        {tr("Import Keys"),            tr("Tools"),      QString(),                  QStringLiteral("\U0001F511"), [this]() { emit importKeysRequested(); }},
        {tr("Clear Cache"),            tr("Tools"),      QString(),                  QStringLiteral("\U0001F5D1"), [this]() { emit clearCacheRequested(); }},
        {tr("View Logs"),              tr("Tools"),      QString(),                  QStringLiteral("\U0001F4C4"), [this]() { emit viewLogsRequested(); }},
    };

    filterCommands(QString());
}

void CommandPalette::filterCommands(const QString &query)
{
    m_commandList->blockSignals(true);
    m_commandList->clear();
    m_filteredIndices.clear();

    const QString lowerQuery = query.toLower();

    for (int i = 0; i < m_allCommands.size(); ++i) {
        const Command &cmd = m_allCommands.at(i);
        if (query.isEmpty()
            || cmd.name.toLower().contains(lowerQuery)
            || cmd.category.toLower().contains(lowerQuery)
            || cmd.shortcut.toLower().contains(lowerQuery)) {
            m_filteredIndices.append(i);
        }
    }

    QString currentCategory;
    for (int idx : std::as_const(m_filteredIndices)) {
        const Command &cmd = m_allCommands.at(idx);

        if (cmd.category != currentCategory) {
            currentCategory = cmd.category;

            auto *headerItem = new QListWidgetItem();
            headerItem->setFlags(Qt::NoItemFlags);
            headerItem->setData(Qt::UserRole + 1, cmd.category);
            headerItem->setSizeHint(QSize(0, 28));

            auto *headerWidget = new QWidget(m_commandList);
            auto *headerLayout = new QHBoxLayout(headerWidget);
            headerLayout->setContentsMargins(16, 2, 16, 2);

            auto *label = new QLabel(cmd.category.toUpper(), headerWidget);
            label->setStyleSheet(QStringLiteral(
                "color: #6b7280; font-size: 11px; font-weight: bold;"));
            headerLayout->addWidget(label);

            m_commandList->addItem(headerItem);
            m_commandList->setItemWidget(headerItem, headerWidget);
        }

        QListWidgetItem *item = createCommandItem(cmd, m_commandList);
        m_commandList->addItem(item);
    }

    m_commandList->blockSignals(false);

    m_countLabel->setText(tr("%n command(s)", "", static_cast<int>(m_filteredIndices.size())));

    if (m_commandList->count() > 0) {
        for (int i = 0; i < m_commandList->count(); ++i) {
            auto *it = m_commandList->item(i);
            if (it && (it->flags() & Qt::ItemIsSelectable)) {
                m_commandList->setCurrentRow(i);
                m_categoryLabel->setText(it->data(Qt::UserRole + 1).toString());
                break;
            }
        }
    }
}

void CommandPalette::onSearchTextChanged(const QString &text)
{
    filterCommands(text.trimmed());
}

void CommandPalette::onCommandActivated(QListWidgetItem *item)
{
    if (!item)
        return;

    const QString name = item->data(Qt::UserRole).toString();
    for (const auto &cmd : std::as_const(m_allCommands)) {
        if (cmd.name == name) {
            if (cmd.action)
                cmd.action();
            close();
            return;
        }
    }
}

void CommandPalette::open()
{
    m_searchEdit->clear();
    filterCommands(QString());
    m_searchEdit->setFocus();
    QDialog::open();
    animateFadeIn();
}

void CommandPalette::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Escape:
        close();
        break;
    case Qt::Key_Down:
        moveSelection(1);
        break;
    case Qt::Key_Up:
        moveSelection(-1);
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        executeCurrentCommand();
        break;
    default:
        QDialog::keyPressEvent(event);
        break;
    }
}

void CommandPalette::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);

    const QWidget *pw = parentWidget();
    if (pw) {
        const QRect parentRect = pw->geometry();
        const int x = parentRect.x() + (parentRect.width() - width()) / 2;
        const int y = parentRect.y() + parentRect.height() / 5;
        move(x, y);
    }
}

void CommandPalette::moveSelection(int delta)
{
    const int count = m_commandList->count();
    if (count == 0)
        return;

    int currentRow = m_commandList->currentRow();
    int newRow = currentRow;

    do {
        newRow = (newRow + delta + count) % count;
        if (newRow == currentRow)
            break;

        auto *item = m_commandList->item(newRow);
        if (item && (item->flags() & Qt::ItemIsSelectable)) {
            m_commandList->setCurrentRow(newRow);
            m_commandList->scrollToItem(item, QAbstractItemView::EnsureVisible);
            break;
        }
    } while (true);
}

void CommandPalette::executeCurrentCommand()
{
    auto *item = m_commandList->currentItem();
    if (item && (item->flags() & Qt::ItemIsSelectable)) {
        onCommandActivated(item);
    }
}

void CommandPalette::animateFadeIn()
{
    if (m_fadeAnimation->state() == QAbstractAnimation::Running)
        m_fadeAnimation->stop();

    setWindowOpacity(0.0);
    m_fadeAnimation->start();
}

#include "command_palette.moc"
