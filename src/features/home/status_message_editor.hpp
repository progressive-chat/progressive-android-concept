#pragma once

#include <QWidget>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QScrollArea>
#include <QString>
#include <QStringList>
#include <QVBoxLayout>

class MatrixSession;

class StatusMessageEditor : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int slideOffset READ slideOffset WRITE setSlideOffset)

public:
    explicit StatusMessageEditor(MatrixSession* session,
                                 QWidget* parent = nullptr);

    QString statusText() const;
    QString presence() const;

    int slideOffset() const { return m_slideOffset; }
    void setSlideOffset(int offset);

signals:
    void statusUpdated(const QString& status, const QString& presence);
    void dismissed();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onSetStatusClicked();
    void onClearStatusClicked();
    void onStatusTextChanged(const QString& text);
    void onRecentStatusClicked(const QString& status);

private:
    void setupUi();
    void animateIn();
    void animateOut();
    void updateCharCounter();
    void loadRecentStatuses();
    void saveRecentStatus(const QString& status);
    QLabel* createPresenceDot(const QString& color) const;
    QPixmap createPresencePixmap(const QString& color) const;

    MatrixSession* m_session;

    QLineEdit* m_statusEdit;
    QLabel* m_charCounter;
    QComboBox* m_presencePicker;
    QPushButton* m_setStatusButton;
    QPushButton* m_clearStatusButton;
    QHBoxLayout* m_recentLayout;
    QWidget* m_recentContainer;
    QWidget* m_backdropWidget;
    QWidget* m_sheetWidget;
    QPropertyAnimation* m_animation;
    int m_slideOffset;

    static constexpr int kMaxStatusLength = 100;
    static constexpr int kMaxRecentStatuses = 5;
};
