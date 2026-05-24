#pragma once

#include <QDialog>
#include <QString>
#include <QStringList>

class QTcpSocket;
class QSslSocket;
class QTextBrowser;
class QLabel;
class QPushButton;
class QTimer;
class QProgressBar;

class IrcMotdDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IrcMotdDialog(const QString& serverHost, quint16 port, bool useTls,
                           QWidget* parent = nullptr);
    ~IrcMotdDialog() override;

private slots:
    void onConnected();
    void onReadyRead();
    void onErrorOccurred();
    void onTimeout();
    void onCopyClicked();

private:
    void sendLine(const QString& line);
    void parseLine(const QString& line);
    void handleMotdStart(const QString& line);
    void handleMotdLine(const QString& line);
    void handleMotdEnd();
    void handleNoMotd();
    void finalizeMotd();
    void showError(const QString& message);
    QString ansiToHtml(const QString& text) const;

    QString m_serverHost;
    quint16 m_port;
    bool m_useTls;

    QTcpSocket* m_socket;
    QTextBrowser* m_motdView;
    QLabel* m_statusLabel;
    QPushButton* m_copyButton;
    QPushButton* m_closeButton;
    QTimer* m_timeoutTimer;
    QProgressBar* m_progressBar;

    bool m_motdStarted;
    bool m_motdEnded;
    QStringList m_motdLines;
};
