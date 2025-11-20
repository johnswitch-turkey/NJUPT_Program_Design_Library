// copymanagementdialog.h
#ifndef COPYMANAGEMENTDIALOG_H
#define COPYMANAGEMENTDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include "../utils/librarymanager.h"
#include "../utils/bookcopy.h"

class CopyManagementDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CopyManagementDialog(const QString &indexId, QWidget *parent = nullptr);

private slots:
    void onAddCopies();
    void onRemoveCopy();
    void refreshTable();

private:
    void setupUI();
    void updateStats();

    QString indexId_;
    QTableWidget *tableWidget_;
    QLabel *statsLabel_;
    QPushButton *addButton_;
    QPushButton *removeButton_;
    QPushButton *closeButton_;
};

#endif // COPYMANAGEMENTDIALOG_H
//
// Created by 13913 on 2025/11/19.
//

#ifndef CLION_TEST_COPYMANAGEMENTDIALOG_H
#define CLION_TEST_COPYMANAGEMENTDIALOG_H


class copymanagementdialog
{
};


#endif //CLION_TEST_COPYMANAGEMENTDIALOG_H