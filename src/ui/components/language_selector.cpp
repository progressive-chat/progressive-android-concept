#include "language_selector.hpp"

#include <QVBoxLayout>
#include <QLabel>
#include <QCoreApplication>
#include <QLocale>

LanguageSelector::LanguageSelector(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *label = new QLabel(tr("Language"), this);
    layout->addWidget(label);

    m_combo = new QComboBox(this);
    layout->addWidget(m_combo);

    populateCombo();
    loadTranslationFiles();

    connect(m_combo, &QComboBox::currentIndexChanged, this, [this](int index) {
        emit languageChanged(localeForIndex(index));
    });
}

void LanguageSelector::populateCombo()
{
    m_languages.clear();
    m_combo->clear();

    auto addLang = [this](const QString &locale, const QString &label) {
        m_languages.append({locale, label});
        m_combo->addItem(label);
    };

    addLang(QString(),                                      QChar(0x1F310) + QStringLiteral(" ") + tr("System (auto-detect)"));
    addLang(QStringLiteral("en"),                           QStringLiteral("\xF0\x9F\x87\xAC\xF0\x9F\x87\xA7 English"));
    addLang(QStringLiteral("ru"),                           QStringLiteral("\xF0\x9F\x87\xB7\xF0\x9F\x87\xBA \xD0\xA0\xD1\x83\xD1\x81\xD1\x81\xD0\xBA\xD0\xB8\xD0\xB9"));
    addLang(QStringLiteral("de"),                           QStringLiteral("\xF0\x9F\x87\xA9\xF0\x9F\x87\xAA Deutsch"));
    addLang(QStringLiteral("fr"),                           QStringLiteral("\xF0\x9F\x87\xAB\xF0\x9F\x87\xB7 Fran\xC3\xA7\x61is"));
    addLang(QStringLiteral("es"),                           QStringLiteral("\xF0\x9F\x87\xAA\xF0\x9F\x87\xB8 Espa\xC3\xB1ol"));
    addLang(QStringLiteral("zh"),                           QStringLiteral("\xF0\x9F\x87\xA8\xF0\x9F\x87\xB3 \xE4\xB8\xAD\xE6\x96\x87"));
    addLang(QStringLiteral("ja"),                           QStringLiteral("\xF0\x9F\x87\xAF\xF0\x9F\x87\xB5 \xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E"));
    addLang(QStringLiteral("ko"),                           QStringLiteral("\xF0\x9F\x87\xB0\xF0\x9F\x87\xB7 \xED\x95\x9C\xEA\xB5\xAD\xEC\x96\xB4"));
    addLang(QStringLiteral("ar"),                           QStringLiteral("\xF0\x9F\x87\xB8\xF0\x9F\x87\xA6 \xD8\xA7\xD9\x84\xD8\xB9\xD8\xB1\xD8\xA8\xD9\x8A\xD8\xA9"));
    addLang(QStringLiteral("pt"),                           QStringLiteral("\xF0\x9F\x87\xB5\xF0\x9F\x87\xB9 Portugu\xC3\xAA\x73"));
    addLang(QStringLiteral("it"),                           QStringLiteral("\xF0\x9F\x87\xAE\xF0\x9F\x87\xB9 Italiano"));
    addLang(QStringLiteral("nl"),                           QStringLiteral("\xF0\x9F\x87\xB3\xF0\x9F\x87\xB1 Nederlands"));
    addLang(QStringLiteral("pl"),                           QStringLiteral("\xF0\x9F\x87\xB5\xF0\x9F\x87\xB1 Polski"));
    addLang(QStringLiteral("tr"),                           QStringLiteral("\xF0\x9F\x87\xB9\xF0\x9F\x87\xB7 T\xC3\xBCrk\xC3\xA7\x65"));
    addLang(QStringLiteral("uk"),                           QStringLiteral("\xF0\x9F\x87\xBA\xF0\x9F\x87\xA6 \xD0\xA3\xD0\xBA\xD1\x80\xD0\xB0\xD1\x97\xD0\xBD\xD1\x81\xD1\x8C\xD0\xBA\xD0\xB0"));
}

QString LanguageSelector::localeForIndex(int index) const
{
    if (index >= 0 && index < m_languages.size())
        return m_languages[index].locale;
    return {};
}

void LanguageSelector::setCurrentLanguage(const QString &locale)
{
    for (int i = 0; i < m_languages.size(); ++i) {
        if (m_languages[i].locale == locale) {
            m_combo->blockSignals(true);
            m_combo->setCurrentIndex(i);
            m_combo->blockSignals(false);
            return;
        }
    }
    m_combo->setCurrentIndex(0);
}

QString LanguageSelector::currentLanguage() const
{
    return localeForIndex(m_combo->currentIndex());
}

void LanguageSelector::applyLanguage(const QString &locale)
{
    if (m_translator) {
        QCoreApplication::removeTranslator(m_translator.get());
        m_translator.reset();
    }

    if (locale.isEmpty() || locale == QStringLiteral("en")) {
        setCurrentLanguage(locale.isEmpty() ? QString() : locale);
        return;
    }

    auto translator = std::make_unique<QTranslator>();
    QString path = QStringLiteral(":/translations/") + locale + QStringLiteral(".qm");

    if (translator->load(path)) {
        QCoreApplication::installTranslator(translator.get());
        m_translator = std::move(translator);
    }

    setCurrentLanguage(locale);
}

QStringList LanguageSelector::availableTranslations() const
{
    QStringList list;

    for (const auto &lang : m_languages) {
        if (lang.locale.isEmpty())
            continue;
        if (lang.locale == QStringLiteral("en"))
            continue;

        QString path = QStringLiteral(":/translations/") + lang.locale + QStringLiteral(".qm");
        if (QFile::exists(path))
            list.append(lang.locale);
    }

    return list;
}

void LanguageSelector::loadTranslationFiles()
{
    availableTranslations();
}
