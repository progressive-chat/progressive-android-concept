#include "theme_manager.hpp"

#include <QAbstractButton>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPalette>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollBar>
#include <QSplitter>
#include <QStatusBar>
#include <QTabBar>
#include <QTabWidget>
#include <QTextEdit>
#include <QToolBar>
#include <QToolTip>
#include <QWidget>
#include <QStyle>
#include <QStyleFactory>
#include <QFile>

#ifdef Q_OS_LINUX
    #include <QProcessEnvironment>
#endif

#ifdef Q_OS_WIN
    #include <QSettings>
#endif

#ifdef Q_OS_MACOS
    #include <QOperatingSystemVersion>
#endif

ThemeManager& ThemeManager::instance()
{
    static ThemeManager s_instance;
    return s_instance;
}

ThemeManager::ThemeManager()
    : QObject(nullptr)
    , m_currentTheme(ThemeType::System)
{
}

ThemeManager::ThemeType ThemeManager::currentTheme() const
{
    return m_currentTheme;
}

void ThemeManager::toggleTheme()
{
    switch (m_currentTheme) {
    case ThemeType::Dark:
        m_currentTheme = ThemeType::Light;
        break;
    case ThemeType::Light:
        m_currentTheme = ThemeType::Dark;
        break;
    case ThemeType::System:
        m_currentTheme = isSystemInDarkMode() ? ThemeType::Light : ThemeType::Dark;
        break;
    }
    emit themeChanged(m_currentTheme);
}

void ThemeManager::applyTheme(ThemeType type, QApplication* app)
{
    if (!app) return;

    m_currentTheme = type;
    ThemeType resolved = resolveSystemTheme();

    app->setStyle(QStyleFactory::create("Fusion"));

    QPalette palette = buildPalette(resolved);
    app->setPalette(palette);

    QString stylesheet = themeStylesheet(resolved);
    app->setStyleSheet(stylesheet);

    emit themeChanged(type);
}

void ThemeManager::applyTheme(ThemeType type, QWidget* widget)
{
    if (!widget) return;

    m_currentTheme = type;
    ThemeType resolved = resolveSystemTheme();

    QPalette palette = buildPalette(resolved);
    widget->setPalette(palette);
    widget->setStyleSheet(themeStylesheet(resolved));

    for (QObject* child : widget->children()) {
        if (auto* childWidget = qobject_cast<QWidget*>(child)) {
            childWidget->setPalette(palette);
        }
    }

    emit themeChanged(type);
}

ThemeManager::ThemeType ThemeManager::resolveSystemTheme() const
{
    if (m_currentTheme != ThemeType::System) {
        return m_currentTheme;
    }
    return isSystemInDarkMode() ? ThemeType::Dark : ThemeType::Light;
}

bool ThemeManager::isSystemInDarkMode() const
{
#ifdef Q_OS_LINUX
    {
        const QString gtkTheme = QProcessEnvironment::systemEnvironment()
                                     .value("GTK_THEME", QString());
        if (gtkTheme.contains("dark", Qt::CaseInsensitive)) {
            return true;
        }
    }

    {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        const QString colorScheme = env.value("COLORFGBG", QString());
        if (colorScheme.contains(":") && !colorScheme.startsWith("15;")) {
            return true;
        }
    }

    return false;
#endif

#ifdef Q_OS_WIN
    QSettings settings(
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        QSettings::NativeFormat);
    return settings.value("AppsUseLightTheme", 1).toInt() == 0;
#endif

#ifdef Q_OS_MACOS
    auto version = QOperatingSystemVersion::current();
    if (version >= QOperatingSystemVersion::MacOSMojave) {
        // macOS 10.14+ provides "AppleInterfaceStyle" user default
        // fallback: treat as light
        return false;
    }
    return false;
#endif

    return false;
}

QPalette ThemeManager::buildPalette(ThemeType type) const
{
    QPalette palette;

    if (type == ThemeType::Light) {
        palette.setColor(QPalette::Window,          QColor("#FFFFFF"));
        palette.setColor(QPalette::WindowText,      QColor("#1C1E21"));
        palette.setColor(QPalette::Base,            QColor("#F5F6F7"));
        palette.setColor(QPalette::AlternateBase,   QColor("#FFFFFF"));
        palette.setColor(QPalette::ToolTipBase,     QColor("#1C1E21"));
        palette.setColor(QPalette::ToolTipText,     QColor("#FFFFFF"));
        palette.setColor(QPalette::Text,            QColor("#1C1E21"));
        palette.setColor(QPalette::Button,          QColor("#E4E6EB"));
        palette.setColor(QPalette::ButtonText,      QColor("#1C1E21"));
        palette.setColor(QPalette::BrightText,      QColor("#FF0000"));
        palette.setColor(QPalette::Link,            QColor("#0084FF"));
        palette.setColor(QPalette::Highlight,       QColor("#0084FF"));
        palette.setColor(QPalette::HighlightedText, QColor("#FFFFFF"));

        palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor("#A0A4A8"));
        palette.setColor(QPalette::Disabled, QPalette::Text,       QColor("#A0A4A8"));
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#A0A4A8"));
        palette.setColor(QPalette::Disabled, QPalette::Base,       QColor("#E8E9EB"));
        palette.setColor(QPalette::Disabled, QPalette::Button,     QColor("#E8E9EB"));
    } else {
        palette.setColor(QPalette::Window,          QColor("#1E1E2E"));
        palette.setColor(QPalette::WindowText,      QColor("#CDD6F4"));
        palette.setColor(QPalette::Base,            QColor("#181825"));
        palette.setColor(QPalette::AlternateBase,   QColor("#313244"));
        palette.setColor(QPalette::ToolTipBase,     QColor("#313244"));
        palette.setColor(QPalette::ToolTipText,     QColor("#CDD6F4"));
        palette.setColor(QPalette::Text,            QColor("#CDD6F4"));
        palette.setColor(QPalette::Button,          QColor("#45475A"));
        palette.setColor(QPalette::ButtonText,      QColor("#CDD6F4"));
        palette.setColor(QPalette::BrightText,      QColor("#FF0000"));
        palette.setColor(QPalette::Link,            QColor("#89B4FA"));
        palette.setColor(QPalette::Highlight,       QColor("#89B4FA"));
        palette.setColor(QPalette::HighlightedText, QColor("#1E1E2E"));

        palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor("#6C7086"));
        palette.setColor(QPalette::Disabled, QPalette::Text,       QColor("#6C7086"));
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#6C7086"));
        palette.setColor(QPalette::Disabled, QPalette::Base,       QColor("#11111B"));
        palette.setColor(QPalette::Disabled, QPalette::Button,     QColor("#11111B"));
    }

    return palette;
}

QString ThemeManager::themeStylesheet(ThemeType type) const
{
    bool dark = (type == ThemeType::Dark);

    const QString accent       = dark ? "#89B4FA" : "#0084FF";
    const QString accentHover  = dark ? "#B4D0FB" : "#0073E6";
    const QString accentPress  = dark ? "#74A8F9" : "#0066CC";
    const QString bgPrimary    = dark ? "#1E1E2E" : "#FFFFFF";
    const QString bgSecondary  = dark ? "#181825" : "#F5F6F7";
    const QString bgTertiary   = dark ? "#313244" : "#E4E6EB";
    const QString bgElevated   = dark ? "#313244" : "#FFFFFF";
    const QString textPrimary  = dark ? "#CDD6F4" : "#1C1E21";
    const QString textSecondary = dark ? "#A6ADC8" : "#606770";
    const QString textDisabled = dark ? "#6C7086" : "#A0A4A8";
    const QString borderColor  = dark ? "#45475A" : "#DADDE1";
    const QString scrollBg     = dark ? "#11111B" : "#E8E9EB";
    const QString scrollHandle = dark ? "#585B70" : "#BEC3C9";
    const QString hoverOverlay = dark ? "rgba(205, 214, 244, 0.06)" : "rgba(0, 0, 0, 0.04)";

    return QStringLiteral(R"(
    /* ── Global ─────────────────────────────────────────────── */
    * {
        font-family: "Segoe UI", "Noto Sans", "Helvetica Neue", Arial, sans-serif;
        font-size: 13px;
    }

    /* ── QPushButton ────────────────────────────────────────── */
    QPushButton {
        background-color: %1;
        color: %3;
        border: 1px solid %2;
        border-radius: 6px;
        padding: 7px 18px;
        min-width: 80px;
        min-height: 30px;
    }
    QPushButton:hover {
        background-color: %21;
        border-color: %5;
    }
    QPushButton:pressed {
        background-color: %22;
        border-color: %5;
    }
    QPushButton:disabled {
        background-color: %4;
        color: %11;
        border-color: %2;
    }
    QPushButton:default {
        border-width: 2px;
    }

    /* ── QLineEdit ──────────────────────────────────────────── */
    QLineEdit {
        background-color: %2;
        color: %3;
        border: 1px solid %2;
        border-radius: 6px;
        padding: 6px 10px;
        selection-background-color: %5;
        selection-color: %10;
    }
    QLineEdit:focus {
        border-color: %5;
        background-color: %4;
    }
    QLineEdit:disabled {
        background-color: %2;
        color: %11;
    }

    /* ── QTextEdit / QPlainTextEdit ─────────────────────────── */
    QTextEdit, QPlainTextEdit {
        background-color: %2;
        color: %3;
        border: 1px solid %2;
        border-radius: 6px;
        padding: 6px;
        selection-background-color: %5;
        selection-color: %10;
    }
    QTextEdit:focus, QPlainTextEdit:focus {
        border-color: %5;
        background-color: %4;
    }

    /* ── QListWidget / QListView / QTreeView / QTableView ───── */
    QListWidget, QListView, QTreeView, QTableView {
        background-color: %2;
        alternate-background-color: %4;
        color: %3;
        border: 1px solid %2;
        border-radius: 6px;
        padding: 2px;
        outline: none;
        selection-background-color: %5;
        selection-color: %10;
        gridline-color: %2;
    }
    QListWidget::item, QListView::item {
        padding: 6px 10px;
        border-radius: 4px;
    }
    QListWidget::item:hover, QListView::item:hover,
    QTreeView::item:hover, QTableView::item:hover {
        background-color: %21;
    }
    QListWidget::item:selected, QListView::item:selected,
    QTreeView::item:selected, QTableView::item:selected {
        background-color: %5;
        color: %10;
    }
    QListWidget::item:selected:!active,
    QListView::item:selected:!active {
        background-color: %4;
        color: %3;
    }

    /* ── QScrollBar ─────────────────────────────────────────── */
    QScrollBar:vertical {
        background: %12;
        width: 10px;
        margin: 0;
        border-radius: 5px;
    }
    QScrollBar::handle:vertical {
        background: %13;
        min-height: 30px;
        border-radius: 5px;
    }
    QScrollBar::handle:vertical:hover {
        background: %5;
    }
    QScrollBar::handle:vertical:pressed {
        background: %5;
    }
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
        height: 0px;
        border: none;
        background: none;
    }
    QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
        background: none;
    }

    QScrollBar:horizontal {
        background: %12;
        width: 10px;
        height: 10px;
        margin: 0;
        border-radius: 5px;
    }
    QScrollBar::handle:horizontal {
        background: %13;
        min-width: 30px;
        border-radius: 5px;
    }
    QScrollBar::handle:horizontal:hover {
        background: %5;
    }
    QScrollBar::handle:horizontal:pressed {
        background: %5;
    }
    QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
        width: 0px;
        border: none;
        background: none;
    }
    QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
        background: none;
    }

    /* ── QMenu / QMenuBar ───────────────────────────────────── */
    QMenuBar {
        background-color: %1;
        color: %3;
        border-bottom: 1px solid %2;
        padding: 2px 0px;
    }
    QMenuBar::item {
        padding: 6px 14px;
        background: transparent;
        border-radius: 4px;
    }
    QMenuBar::item:selected {
        background-color: %21;
    }
    QMenuBar::item:pressed {
        background-color: %4;
    }

    QMenu {
        background-color: %4;
        color: %3;
        border: 1px solid %2;
        border-radius: 8px;
        padding: 6px;
    }
    QMenu::item {
        padding: 8px 32px 8px 16px;
        border-radius: 4px;
        margin: 2px 4px;
    }
    QMenu::item:selected {
        background-color: %5;
        color: %10;
    }
    QMenu::item:disabled {
        color: %11;
    }
    QMenu::separator {
        height: 1px;
        background: %2;
        margin: 4px 8px;
    }
    QMenu::indicator {
        width: 16px;
        height: 16px;
        margin-left: 6px;
    }
    QMenu::indicator:non-exclusive:unchecked {
        border: 1px solid %2;
        background: %2;
    }
    QMenu::indicator:non-exclusive:checked {
        border: 1px solid %5;
        background: %5;
    }

    /* ── QTabWidget / QTabBar ────────────────────────────────── */
    QTabWidget::pane {
        border: 1px solid %2;
        border-radius: 6px;
        background-color: %2;
        top: -1px;
    }
    QTabBar::tab {
        background-color: %1;
        color: %9;
        border: 1px solid transparent;
        padding: 8px 20px;
        margin-right: 2px;
        border-top-left-radius: 6px;
        border-top-right-radius: 6px;
    }
    QTabBar::tab:hover {
        background-color: %21;
        color: %3;
    }
    QTabBar::tab:selected {
        background-color: %2;
        color: %5;
        border: 1px solid %2;
        border-bottom-color: %2;
    }
    QTabBar::tab:disabled {
        color: %11;
    }
    QTabBar::close-button {
        border-radius: 3px;
    }
    QTabBar::close-button:hover {
        background-color: %4;
    }

    /* ── QToolTip ────────────────────────────────────────────── */
    QToolTip {
        background-color: %7;
        color: %8;
        border: 1px solid %2;
        border-radius: 6px;
        padding: 4px 8px;
    }

    /* ── QComboBox ───────────────────────────────────────────── */
    QComboBox {
        background-color: %2;
        color: %3;
        border: 1px solid %2;
        border-radius: 6px;
        padding: 6px 12px 6px 12px;
        min-height: 20px;
        selection-background-color: %5;
        selection-color: %10;
    }
    QComboBox:hover {
        border-color: %5;
    }
    QComboBox:focus, QComboBox:on {
        border-color: %5;
        background-color: %4;
    }
    QComboBox:disabled {
        background-color: %2;
        color: %11;
    }
    QComboBox::drop-down {
        subcontrol-origin: padding;
        subcontrol-position: top right;
        width: 24px;
        border-left: 1px solid %2;
        border-top-right-radius: 6px;
        border-bottom-right-radius: 6px;
    }
    QComboBox::down-arrow {
        width: 10px;
        height: 10px;
    }
    QComboBox QAbstractItemView {
        background-color: %4;
        color: %3;
        border: 1px solid %2;
        border-radius: 6px;
        selection-background-color: %5;
        selection-color: %10;
        outline: none;
    }
    QComboBox QAbstractItemView::item {
        padding: 6px 12px;
        border-radius: 4px;
    }
    QComboBox QAbstractItemView::item:hover {
        background-color: %21;
    }

    /* ── QCheckBox / QRadioButton ────────────────────────────── */
    QCheckBox, QRadioButton {
        color: %3;
        spacing: 8px;
        padding: 4px 0px;
    }
    QCheckBox::indicator, QRadioButton::indicator {
        width: 18px;
        height: 18px;
        border-radius: 4px;
        border: 2px solid %2;
        background-color: %2;
    }
    QRadioButton::indicator {
        border-radius: 10px;
    }
    QCheckBox::indicator:hover, QRadioButton::indicator:hover {
        border-color: %5;
    }
    QCheckBox::indicator:checked, QRadioButton::indicator:checked {
        background-color: %5;
        border-color: %5;
    }
    QCheckBox::indicator:disabled, QRadioButton::indicator:disabled {
        background-color: %4;
        border-color: %2;
    }
    QCheckBox::indicator:checked:disabled,
    QRadioButton::indicator:checked:disabled {
        background-color: %11;
        border-color: %11;
    }

    /* ── QProgressBar ────────────────────────────────────────── */
    QProgressBar {
        background-color: %2;
        color: %10;
        border: 1px solid %2;
        border-radius: 6px;
        text-align: center;
        min-height: 20px;
        font-weight: bold;
    }
    QProgressBar::chunk {
        background-color: %5;
        border-radius: 5px;
    }
    QProgressBar:disabled {
        color: %11;
    }
    QProgressBar:disabled::chunk {
        background-color: %11;
    }

    /* ── QSplitter ───────────────────────────────────────────── */
    QSplitter::handle {
        background-color: %2;
    }
    QSplitter::handle:horizontal {
        width: 3px;
    }
    QSplitter::handle:vertical {
        height: 3px;
    }
    QSplitter::handle:hover {
        background-color: %5;
    }

    /* ── QStatusBar ──────────────────────────────────────────── */
    QStatusBar {
        background-color: %1;
        color: %3;
        border-top: 1px solid %2;
        padding: 2px 8px;
    }
    QStatusBar::item {
        border: none;
    }
    QStatusBar QLabel {
        color: %3;
        padding: 0px 4px;
    }

    /* ── QToolBar ────────────────────────────────────────────── */
    QToolBar {
        background-color: %1;
        border-bottom: 1px solid %2;
        padding: 4px;
        spacing: 4px;
    }
    QToolBar::separator {
        background-color: %2;
        width: 1px;
        margin: 4px 6px;
    }
    QToolBar QToolButton {
        background-color: transparent;
        color: %3;
        border: 1px solid transparent;
        border-radius: 6px;
        padding: 6px 10px;
    }
    QToolBar QToolButton:hover {
        background-color: %21;
        border-color: %2;
    }
    QToolBar QToolButton:pressed, QToolBar QToolButton:checked {
        background-color: %4;
        border-color: %5;
    }
    QToolBar QToolButton:disabled {
        color: %11;
    }

    /* ── QHeaderView (table/list headers) ────────────────────── */
    QHeaderView {
        background-color: %1;
        color: %3;
    }
    QHeaderView::section {
        background-color: %1;
        color: %9;
        border: none;
        border-bottom: 2px solid %2;
        border-right: 1px solid %2;
        padding: 8px 12px;
        font-weight: bold;
        text-align: left;
    }
    QHeaderView::section:hover {
        background-color: %21;
        color: %3;
    }
    QHeaderView::section:pressed {
        background-color: %4;
    }
    QHeaderView::down-arrow, QHeaderView::up-arrow {
        width: 8px;
        height: 8px;
    }

    /* ── QGroupBox ───────────────────────────────────────────── */
    QGroupBox {
        color: %3;
        border: 1px solid %2;
        border-radius: 6px;
        margin-top: 14px;
        padding: 16px 12px 12px 12px;
        font-weight: bold;
    }
    QGroupBox::title {
        subcontrol-origin: margin;
        subcontrol-position: top left;
        padding: 0px 8px;
        color: %3;
    }

    /* ── QSpinBox / QDoubleSpinBox ───────────────────────────── */
    QSpinBox, QDoubleSpinBox {
        background-color: %2;
        color: %3;
        border: 1px solid %2;
        border-radius: 6px;
        padding: 6px 10px;
        selection-background-color: %5;
        selection-color: %10;
    }
    QSpinBox:focus, QDoubleSpinBox:focus {
        border-color: %5;
    }
    QSpinBox::up-button, QDoubleSpinBox::up-button,
    QSpinBox::down-button, QDoubleSpinBox::down-button {
        background-color: %4;
        border: none;
        border-radius: 3px;
        margin: 2px;
    }
    QSpinBox::up-button:hover, QDoubleSpinBox::up-button:hover,
    QSpinBox::down-button:hover, QDoubleSpinBox::down-button:hover {
        background-color: %5;
    }

    /* ── QSlider ─────────────────────────────────────────────── */
    QSlider::groove:horizontal {
        background: %2;
        height: 4px;
        border-radius: 2px;
    }
    QSlider::handle:horizontal {
        background: %5;
        width: 16px;
        height: 16px;
        margin: -6px 0;
        border-radius: 8px;
    }
    QSlider::handle:horizontal:hover {
        background: %14;
    }
    QSlider::add-page:horizontal {
        background: %2;
    }
    QSlider::sub-page:horizontal {
        background: %5;
        border-radius: 2px;
    }
    QSlider::groove:vertical {
        background: %2;
        width: 4px;
        border-radius: 2px;
    }
    QSlider::handle:vertical {
        background: %5;
        width: 16px;
        height: 16px;
        margin: 0 -6px;
        border-radius: 8px;
    }

    /* ── QDockWidget ──────────────────────────────────────────── */
    QDockWidget {
        color: %3;
        titlebar-close-icon: none;
        titlebar-normal-icon: none;
    }
    QDockWidget::title {
        background-color: %1;
        padding: 8px 12px;
        border-bottom: 1px solid %2;
        text-align: left;
    }

    /* ── QFrame[frameShape="4"] ── (HLine) ───────────────────── */
    QFrame[frameShape="4"], QFrame[frameShape="5"] {
        color: %2;
    }
    )")
    .arg(bgPrimary)      // %1
    .arg(bgSecondary)    // %2
    .arg(textPrimary)    // %3
    .arg(bgElevated)     // %4
    .arg(accent)         // %5
    .arg(bgTertiary)     // %6
    // %7, %8 reserved for tooltip colors — used as literal in the sheet
    .arg(dark ? "#313244" : "#1C1E21",      // %7  ToolTipBase
         dark ? "#CDD6F4" : "#FFFFFF",      // %8  ToolTipText
         textSecondary,                      // %9
         dark ? "#1E1E2E" : "#FFFFFF",      // %10 HighlightedText
         textDisabled,                       // %11
         scrollBg,                           // %12
         scrollHandle,                       // %13
         accentHover,                        // %14
         QString(),                          // %15 unused
         QString(),                          // %16 unused
         QString(),                          // %17 unused
         QString(),                          // %18 unused
         QString(),                          // %19 unused
         QString(),                          // %20 unused
         hoverOverlay,                       // %21
         accentPress);                       // %22
}
