#pragma once

#include <QDialog>
#include <QVector>
#include <functional>

class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QLabel;
class QShortcut;
class QPropertyAnimation;

struct Command
{
    QString name;
    QString category;
    QString shortcut;
    QString icon;
    std::function<void()> action;
};

class CommandPalette : public QDialog
{
    Q_OBJECT

public:
    explicit CommandPalette(QWidget *parent = nullptr);
    ~CommandPalette() override = default;

    static void installGlobalShortcut(QWidget *parent, CommandPalette *palette);

signals:
    void goToRoomRequested();
    void openSettingsRequested();
    void openProfileRequested();
    void startDirectChatRequested();
    void createRoomRequested();
    void joinRoomRequested();
    void markAllReadRequested();
    void connectMatrixRequested();
    void connectIrcRequested();
    void addLemmyInstanceRequested();
    void toggleThemeRequested();
    void toggleCompactModeRequested();
    void toggleSidebarRequested();
    void exportChatRequested();
    void importKeysRequested();
    void clearCacheRequested();
    void viewLogsRequested();

public slots:
    void open();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void onSearchTextChanged(const QString &text);
    void onCommandActivated(QListWidgetItem *item);

private:
    void setupUi();
    void populateCommands();
    void filterCommands(const QString &query);
    void moveSelection(int delta);
    void executeCurrentCommand();
    void animateFadeIn();

    QLineEdit *m_searchEdit;
    QListWidget *m_commandList;
    QLabel *m_categoryLabel;
    QLabel *m_countLabel;
    QPropertyAnimation *m_fadeAnimation;

    QVector<Command> m_allCommands;
    QVector<int> m_filteredIndices;
};
