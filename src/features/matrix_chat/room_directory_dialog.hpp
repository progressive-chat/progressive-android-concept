#pragma once

#include <QDialog>

class QLineEdit;
class QPushButton;
class QListWidget;
class QListWidgetItem;

class RoomDirectoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RoomDirectoryDialog(QWidget *parent = nullptr);
    ~RoomDirectoryDialog() override = default;

private slots:
    void onSearchClicked();
    void onItemDoubleClicked(QListWidgetItem *item);

private:
    void setupUi();

    QLineEdit *m_searchEdit;
    QPushButton *m_searchButton;
    QListWidget *m_resultsList;
};
