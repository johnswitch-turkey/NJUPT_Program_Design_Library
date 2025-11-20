// copymanagementdialog.cpp
#include "copymanagementdialog.h"
#include "../utils/librarymanager.h"
#include <QApplication>
#include <QFont>

CopyManagementDialog::CopyManagementDialog(const QString &indexId, QWidget *parent)
    : QDialog(parent), indexId_(indexId)
{
    setWindowTitle(QStringLiteral("副本管理 - %1").arg(indexId));
    setModal(true);
    resize(600, 400);

    setupUI();
    refreshTable();
}

void CopyManagementDialog::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);

    // 统计信息
    statsLabel_ = new QLabel(this);
    QFont font = statsLabel_->font();
    font.setBold(true);
    statsLabel_->setFont(font);
    mainLayout->addWidget(statsLabel_);

    // 表格
    tableWidget_ = new QTableWidget(this);
    tableWidget_->setColumnCount(6);
    tableWidget_->setHorizontalHeaderLabels({
        QStringLiteral("副本编号"),
        QStringLiteral("副本ID"),
        QStringLiteral("状态"),
        QStringLiteral("借阅者"),
        QStringLiteral("借阅日期"),
        QStringLiteral("归还日期")
    });

    tableWidget_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tableWidget_->horizontalHeader()->setStretchLastSection(true);
    tableWidget_->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget_->setEditTriggers(QAbstractItemView::NoEditTriggers);

    mainLayout->addWidget(tableWidget_);

    // 按钮布局
    auto *buttonLayout = new QHBoxLayout();

    addButton_ = new QPushButton(QStringLiteral("添加副本"), this);
    removeButton_ = new QPushButton(QStringLiteral("删除副本"), this);
    closeButton_ = new QPushButton(QStringLiteral("关闭"), this);

    buttonLayout->addWidget(addButton_);
    buttonLayout->addWidget(removeButton_);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton_);

    mainLayout->addLayout(buttonLayout);

    connect(addButton_, &QPushButton::clicked, this, &CopyManagementDialog::onAddCopies);
    connect(removeButton_, &QPushButton::clicked, this, &CopyManagementDialog::onRemoveCopy);
    connect(closeButton_, &QPushButton::clicked, this, &QDialog::reject);
}

void CopyManagementDialog::refreshTable()
{
    // 获取LibraryManager实例
    LibraryManager *libManager = &LibraryManager::instance();
    QVector<BookCopy> copies = libManager->getBookCopies(indexId_);

    tableWidget_->setRowCount(copies.size());

    for (int i = 0; i < copies.size(); ++i) {
        const BookCopy &copy = copies[i];

        tableWidget_->setItem(i, 0, new QTableWidgetItem(QString::number(copy.copyNumber)));
        tableWidget_->setItem(i, 1, new QTableWidgetItem(copy.copyId));

        QString status = copy.isAvailable() ? QStringLiteral("可借") : QStringLiteral("已借出");
        QTableWidgetItem *statusItem = new QTableWidgetItem(status);
        statusItem->setForeground(copy.isAvailable() ? QColor("green") : QColor("red"));
        tableWidget_->setItem(i, 2, statusItem);

        tableWidget_->setItem(i, 3, new QTableWidgetItem(copy.borrowedBy));
        tableWidget_->setItem(i, 4, new QTableWidgetItem(
            copy.borrowDate.isValid() ? copy.borrowDate.toString("yyyy-MM-dd") : ""));
        tableWidget_->setItem(i, 5, new QTableWidgetItem(
            copy.dueDate.isValid() ? copy.dueDate.toString("yyyy-MM-dd") : ""));
    }

    updateStats();
}

void CopyManagementDialog::updateStats()
{
    // 获取LibraryManager实例
    LibraryManager *libManager = &LibraryManager::instance();
    int total = libManager->getTotalCopyCount(indexId_);
    int available = libManager->getAvailableCopyCount(indexId_);
    int borrowed = total - available;

    statsLabel_->setText(QStringLiteral("总计: %1 | 可借: %2 | 已借: %3")
                        .arg(total).arg(available).arg(borrowed));
}

void CopyManagementDialog::onAddCopies()
{
    bool ok;
    int count = QInputDialog::getInt(this, QStringLiteral("添加副本"),
                                     QStringLiteral("请输入要添加的副本数量:"), 1, 1, 100, 1, &ok);

    if (!ok) return;

    // 获取LibraryManager实例
    LibraryManager *libManager = &LibraryManager::instance();
    QString error;
    if (libManager->addBookCopies(indexId_, count, &error)) {
        QMessageBox::information(this, QStringLiteral("成功"),
                                QStringLiteral("成功添加 %1 个副本").arg(count));
        refreshTable();
    } else {
        QMessageBox::warning(this, QStringLiteral("失败"), error);
    }
}

void CopyManagementDialog::onRemoveCopy()
{
    int currentRow = tableWidget_->currentRow();
    if (currentRow < 0) {
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请先选择要删除的副本"));
        return;
    }

    QString copyId = tableWidget_->item(currentRow, 1)->text();
    int copyNumber = tableWidget_->item(currentRow, 0)->text().toInt();

    auto reply = QMessageBox::question(this, QStringLiteral("确认删除"),
                                       QStringLiteral("确定要删除副本 %1 吗？").arg(copyNumber),
                                       QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) return;

    // 获取LibraryManager实例
    LibraryManager *libManager = &LibraryManager::instance();
    QString error;
    if (libManager->removeBookCopy(copyId, &error)) {
        QMessageBox::information(this, QStringLiteral("成功"), QStringLiteral("删除成功"));
        refreshTable();
    } else {
        QMessageBox::warning(this, QStringLiteral("失败"), error);
    }
}

