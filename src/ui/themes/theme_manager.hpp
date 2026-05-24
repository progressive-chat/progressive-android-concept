#pragma once

#include <QApplication>
#include <QObject>
#include <QString>
#include <QWidget>

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    enum class ThemeType {
        Light,
        Dark,
        System
    };
    Q_ENUM(ThemeType)

    static ThemeManager& instance();

    void applyTheme(ThemeType type, QApplication* app);
    void applyTheme(ThemeType type, QWidget* widget);
    ThemeType currentTheme() const;
    void toggleTheme();
    QString themeStylesheet(ThemeType type) const;

signals:
    void themeChanged(ThemeType newTheme);

private:
    ThemeManager();
    ~ThemeManager() override = default;
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    QPalette buildPalette(ThemeType type) const;
    ThemeType resolveSystemTheme() const;
    bool isSystemInDarkMode() const;

    ThemeType m_currentTheme;
};
