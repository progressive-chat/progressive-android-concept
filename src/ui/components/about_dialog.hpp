#pragma once

#include <QDialog>
#include <QLabel>
#include <QPushButton>

class AboutDialog : public QDialog {
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog() override = default;

private:
    void setupUi();
    QPixmap generateAppIcon(int size) const;

    QLabel *m_appIconLabel = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_versionLabel = nullptr;
    QLabel *m_descriptionLabel = nullptr;
    QLabel *m_builtWithLabel = nullptr;
    QLabel *m_standardBadge = nullptr;
    QLabel *m_authorsLabel = nullptr;
    QLabel *m_linkLabel = nullptr;
    QLabel *m_licenseLabel = nullptr;
    QPushButton *m_thirdPartyButton = nullptr;
    QPushButton *m_okButton = nullptr;
};
