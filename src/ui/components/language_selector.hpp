#pragma once

#include <QWidget>
#include <QComboBox>
#include <QTranslator>
#include <QDir>
#include <memory>

class LanguageSelector : public QWidget
{
    Q_OBJECT

public:
    explicit LanguageSelector(QWidget *parent = nullptr);

    void setCurrentLanguage(const QString &locale);
    QString currentLanguage() const;

    void applyLanguage(const QString &locale);

    QStringList availableTranslations() const;

signals:
    void languageChanged(const QString &locale);

private:
    void populateCombo();
    QString localeForIndex(int index) const;
    void loadTranslationFiles();

    QComboBox *m_combo;
    std::unique_ptr<QTranslator> m_translator;

    struct LanguageInfo
    {
        QString locale;
        QString label;
    };
    QVector<LanguageInfo> m_languages;
};
