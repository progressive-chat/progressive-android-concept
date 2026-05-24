#include "about_dialog.hpp"

#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QFont>
#include <QFile>

namespace progressive_chat {

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("About Progressive Chat");
    setMinimumSize(500, 400);
    setupUi();
}

void AboutDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);

    m_tabWidget = new QTabWidget();
    setupAboutTab();
    setupCreditsTab();
    setupLicensesTab();
    mainLayout->addWidget(m_tabWidget);

    auto *closeBtn = new QPushButton("Close");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(closeBtn, 0, Qt::AlignRight);
}

void AboutDialog::setupAboutTab()
{
    auto *tab = new QWidget();
    auto *layout = new QVBoxLayout(tab);

    // Logo
    m_logoLabel = new QLabel("Progressive Chat");
    QFont logoFont;
    logoFont.setPixelSize(32);
    logoFont.setBold(true);
    m_logoLabel->setFont(logoFont);
    m_logoLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_logoLabel);

    // Version
    m_versionLabel = new QLabel("Version " + QApplication::applicationVersion());
    m_versionLabel->setAlignment(Qt::AlignCenter);
    m_versionLabel->setStyleSheet("color: #888; font-size: 14px;");
    layout->addWidget(m_versionLabel);

    layout->addSpacing(16);

    // Description
    auto *descLabel = new QLabel(
        "Progressive Chat is a 100% C++ desktop Matrix client.
"
        "Built with Qt6 and modern C++23.

"
        "Features:
"
        "• Matrix, IRC, and Lemmy protocol support
"
        "• End-to-end encryption via OLM/Megolm
"
        "• Spaces, threads, polls, and reactions
"
        "• Voice/video calling support
"
        "• Cross-platform (Linux, macOS, Windows)

"
        "https://progressive.chat");
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("font-size: 12px;");
    layout->addWidget(descLabel);

    layout->addSpacing(16);

    // Links
    auto *websiteBtn = new QPushButton("Website");
    connect(websiteBtn, &QPushButton::clicked, []() {
        QDesktopServices::openUrl(QUrl("https://progressive.chat"));
    });

    auto *sourceBtn = new QPushButton("Source Code");
    connect(sourceBtn, &QPushButton::clicked, []() {
        QDesktopServices::openUrl(QUrl("https://github.com/progressive-chat/progressive-android-concept"));
    });

    auto *linkLayout = new QHBoxLayout();
    linkLayout->addStretch();
    linkLayout->addWidget(websiteBtn);
    linkLayout->addWidget(sourceBtn);
    linkLayout->addStretch();
    layout->addLayout(linkLayout);

    layout->addStretch();
    m_tabWidget->addTab(tab, "About");
}

void AboutDialog::setupCreditsTab()
{
    auto *tab = new QWidget();
    auto *layout = new QVBoxLayout(tab);

    m_creditsView = new QTextBrowser();
    m_creditsView->setHtml(
        "<h2>Credits</h2>"
        "<p><b>Lead Developer:</b> MaurerAnton</p>"
        "<p><b>Organization:</b> Progressive Chat</p>"
        "<br><h3>Libraries</h3>"
        "<ul>"
        "<li><b>Qt6</b> — Cross-platform application framework</li>"
        "<li><b>libolm</b> — Double Ratchet encryption library</li>"
        "<li><b>Matrix SDK</b> — Core Matrix protocol implementation</li>"
        "</ul>"
        "<br><h3>Contributors</h3>"
        "<p>See the GitHub repository for a full list of contributors.</p>"
    );
    layout->addWidget(m_creditsView);
    m_tabWidget->addTab(tab, "Credits");
}

void AboutDialog::setupLicensesTab()
{
    auto *tab = new QWidget();
    auto *layout = new QVBoxLayout(tab);

    m_licenseView = new QTextBrowser();
    m_licenseView->setPlainText(
        "Progressive Chat
"
        "Copyright (c) 2024-2026 Progressive Chat

"
        "Licensed under the Apache License, Version 2.0

"
        "https://www.apache.org/licenses/LICENSE-2.0

"
        "---

"
        "Included third-party libraries:

"
        "libolm — Apache 2.0
"
        "Qt6 — LGPL v3 / GPL v2 / Commercial
"
    );
    layout->addWidget(m_licenseView);
    m_tabWidget->addTab(tab, "License");
}

} // namespace progressive_chat
