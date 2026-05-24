#pragma once

#include <QDialog>
#include <QDateTime>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QString>
#include <QVector>
#include <QPair>

class MatrixSession;

struct EditEntry {
    QString content;
    QDateTime timestamp;
};

class MessageEditHistoryDialog : public QDialog {
    Q_OBJECT

public:
    explicit MessageEditHistoryDialog(const QString& messageId,
                                      const QString& roomId,
                                      MatrixSession* session,
                                      QWidget* parent = nullptr);

    QString messageId() const { return m_messageId; }
    QString roomId() const { return m_roomId; }

signals:
    void viewInChatRequested(const QString& roomId, const QString& eventId);

private slots:
    void onViewInChatClicked();

private:
    void setupUi();
    void loadEditHistory();
    QVector<EditEntry> fetchEditHistory() const;
    QVector<EditEntry> generateStubEdits() const;
    QWidget* createEditWidget(const EditEntry& edit, int index, int total);
    QString computeDiff(const QString& before, const QString& after) const;
    void applyStyleSheet();

    QString m_messageId;
    QString m_roomId;
    MatrixSession* m_session;

    QLabel* m_originalLabel;
    QLabel* m_originalTimestamp;
    QVBoxLayout* m_editsLayout;
    QScrollArea* m_scrollArea;
    QPushButton* m_viewInChatButton;

    QVector<EditEntry> m_edits;
};
