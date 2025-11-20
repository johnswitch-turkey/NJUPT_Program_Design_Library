// mainwindow.cpp
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../utils/bookdisplay.h"
#include "../utils/librarymanager.h"
#include "copymanagementdialog.h"

#include <QMenu>
#include <QAction>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QIcon>

#include <QDate>
#include <QSet>
#include <QActionGroup>
#include <algorithm>
#include <QHeaderView>

#include <QToolBar>
#include <QScrollArea>
#include <QDockWidget>
#include <QVBoxLayout>

#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

// ============================================================================
// 构造函数
// ============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), model_(nullptr), tableView_(nullptr), searchEdit_(nullptr), searchButton_(nullptr), themeToggleButton_(nullptr), categoryFilterMenu_(nullptr), statusFilterMenu_(nullptr), locationFilterMenu_(nullptr), sortMenu_(nullptr), categoryActionGroup_(nullptr), statusActionGroup_(nullptr), locationActionGroup_(nullptr), sortActionGroup_(nullptr), categoryFilter_(), statusFilter_(), locationFilter_(), currentSortType_("default"), isDarkMode_(false), isEditMode_(false)
{
    ui->setupUi(this);

    // 1. 搭建视图
    setupTable();

    // 2. 准备数据
    loadData();

    // 2.5 构建筛选菜单
    rebuildFilterMenus();

    // 3. 填充数据
    refreshTable();

    // 设置UI其他部分
    setupMenuBar();
    setupActions();
    setupSearchBar();
    setupThemeToggle();
    setupStyles();
}

// ============================================================================
// 析构函数
// ============================================================================
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadData()
{
    // 数据已通过LibraryManager自动从数据库加载
    // 如果数据库为空，会自动导入示例数据
    updateStatusBar();
}

// ============================================================================
// 视图搭建
// ============================================================================
void MainWindow::setupTable()
{
    // 创建数据模型（增加作者和出版社列）
    model_ = new QStandardItemModel(this);
    model_->setHorizontalHeaderLabels({QStringLiteral("索引号"),
                                       QStringLiteral("名称"),
                                       QStringLiteral("作者"),
                                       QStringLiteral("出版社"),
                                       QStringLiteral("馆藏地址"),
                                       QStringLiteral("类别"),
                                       QStringLiteral("数量"),
                                       QStringLiteral("价格"),
                                       QStringLiteral("入库日期"),
                                       QStringLiteral("归还日期"),
                                       QStringLiteral("借阅次数"),
                                       QStringLiteral("状态")});

    // 创建表格视图并关联模型
    tableView_ = new QTableView(this);
    tableView_->setModel(model_);
    tableView_->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView_->setAlternatingRowColors(true);
    tableView_->verticalHeader()->setDefaultSectionSize(50);
    // 1. 先设置所有列为根据内容自动调整大小
    tableView_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // 2. 再单独设置需要拉伸的列
    tableView_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    tableView_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    tableView_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    tableView_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    tableView_->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    tableView_->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    tableView_->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Stretch);
    tableView_->horizontalHeader()->setSectionResizeMode(7, QHeaderView::Stretch);
    tableView_->horizontalHeader()->setSectionResizeMode(8, QHeaderView::Stretch);
    tableView_->horizontalHeader()->setSectionResizeMode(9, QHeaderView::Stretch);
    tableView_->horizontalHeader()->setSectionResizeMode(10, QHeaderView::Stretch);

    // 设置类别、状态和馆藏地址列的最小宽度，确保有足够空间显示换行内容
    tableView_->horizontalHeader()->setMinimumSectionSize(120);

    tableView_->horizontalHeader()->setSectionsClickable(true);
    connect(tableView_->horizontalHeader(), &QHeaderView::sectionClicked,
            this, &MainWindow::onHeaderSectionClicked);

    // 将表格添加到中央布局
    ui->centralLayout->addWidget(tableView_);
}

// ============================================================================
// 数据填充
// ============================================================================
void MainWindow::refreshTable()
{
    const QVector<Book> &books = library_.getAll();
    model_->removeRows(0, model_->rowCount());

    for (int row = 0; row < books.size(); ++row) {
        const Book &b = books[row];

        // 应用筛选条件
        if (!categoryFilter_.isEmpty() && b.category != categoryFilter_) {
            continue;
        }
        if (!locationFilter_.isEmpty() && b.location != locationFilter_) {
            continue;
        }

        int totalCopies = library_.getTotalCopyCount(b.indexId);
        int availableCopies = library_.getAvailableCopyCount(b.indexId);

        if (statusFilter_ == "available" && availableCopies <= 0) {
            continue;
        }
        if (statusFilter_ == "borrowed" && availableCopies >= totalCopies) {
            continue;
        }

        QList<QStandardItem *> rowItems;
        rowItems << new QStandardItem(b.indexId);
        rowItems << new QStandardItem(b.name);
        rowItems << new QStandardItem(b.author);
        rowItems << new QStandardItem(b.publisher);
        rowItems << new QStandardItem(b.location);
        rowItems << new QStandardItem(b.category);
        rowItems << new QStandardItem(QString::number(totalCopies));  // 显示总副本数
        rowItems << new QStandardItem(QString::number(b.price, 'f', 2));
        rowItems << new QStandardItem(b.inDate.toString("yyyy-MM-dd"));

        // 归还日期：根据当前用户显示
        QString returnDateStr = "";
        if (!currentUsername_.isEmpty() && !isAdminMode_) {
            QVector<BookCopy> borrowedCopies = library_.getUserBorrowedCopies(currentUsername_);
            for (const BookCopy &copy : borrowedCopies) {
                if (copy.indexId == b.indexId) {
                    returnDateStr = copy.dueDate.toString("yyyy-MM-dd");
                    break;
                }
            }
        }
        rowItems << new QStandardItem(returnDateStr);

        rowItems << new QStandardItem(QString::number(b.borrowCount));

        // 状态列：根据可用副本数显示
        QString statusText = (availableCopies > 0) ? QStringLiteral("可借") : QStringLiteral("不可借");
        rowItems << new QStandardItem(statusText);

        model_->appendRow(rowItems);
    }

    updateStatusBar();
    updateHeaderLabels();
}

// ============================================================================
// 核心业务逻辑槽函数
// ============================================================================
void MainWindow::onBorrow()
{
    if (currentUsername_.isEmpty() || isAdminMode_) {
        QMessageBox::warning(this, "借书失败", "只有学生用户可以借书，请使用学生账号登录。");
        return;
    }

    QModelIndexList selectedIndexes = tableView_->selectionModel()->selectedRows();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择要借阅的图书！");
        return;
    }

    int row = selectedIndexes.first().row();
    QString indexId = model_->item(row, 0)->text();
    QString bookName = model_->item(row, 1)->text();

    // 检查该学生是否已借过此书
    QVector<BookCopy> borrowedCopies = library_.getUserBorrowedCopies(currentUsername_);
    for (const BookCopy &copy : borrowedCopies) {
        if (copy.indexId == indexId) {
            QMessageBox::warning(this, "借书失败", "你已经借过该图书，请先归还再借。");
            return;
        }
    }

    // 检查是否有可用副本
    if (library_.getAvailableCopyCount(indexId) <= 0) {
        QMessageBox::warning(this, "借书失败", "该图书已无剩余可借数量！");
        return;
    }

    // 弹出输入框，让用户输入归还日期
    bool ok;
    QDate dueDate = QDate::fromString(
        QInputDialog::getText(this, "借书", "请输入归还日期 (yyyy-MM-dd):",
                             QLineEdit::Normal, QDate::currentDate().addDays(30).toString("yyyy-MM-dd"), &ok),
        "yyyy-MM-dd");

    if (!ok || !dueDate.isValid()) {
        return;
    }

    QString error;
    if (library_.borrowBook(indexId, currentUsername_, dueDate, &error)) {
        refreshTable();
        QMessageBox::information(this, "成功",
                                 QStringLiteral("成功借阅图书《%1》，归还日期为 %2").arg(bookName, dueDate.toString("yyyy-MM-dd")));
    } else {
        QMessageBox::warning(this, "失败", "借阅失败：" + error);
    }
}

void MainWindow::onReturn()
{
    if (currentUsername_.isEmpty() || isAdminMode_) {
        QMessageBox::warning(this, "还书失败", "只有学生用户可以还书，请使用学生账号登录。");
        return;
    }

    // 获取当前用户借阅的副本
    QVector<BookCopy> borrowedCopies = library_.getUserBorrowedCopies(currentUsername_);
    if (borrowedCopies.isEmpty()) {
        QMessageBox::information(this, "提示", "你当前没有借阅任何图书！");
        return;
    }

    // 创建选择对话框
    QStringList copyNames;
    for (const BookCopy &copy : borrowedCopies) {
        const Book *book = library_.findByIndexId(copy.indexId);
        if (book) {
            copyNames.append(QStringLiteral("《%1》 - 副本%2 (应还: %3)")
                            .arg(book->name)
                            .arg(copy.copyNumber)
                            .arg(copy.dueDate.toString("yyyy-MM-dd")));
        }
    }

    bool ok;
    QString selectedCopy = QInputDialog::getItem(this, "还书", "请选择要归还的图书:",
                                               copyNames, 0, false, &ok);

    if (!ok || selectedCopy.isEmpty()) {
        return;
    }

    int selectedIndex = copyNames.indexOf(selectedCopy);
    if (selectedIndex < 0) return;

    const BookCopy &selectedCopyObj = borrowedCopies[selectedIndex];
    const Book *book = library_.findByIndexId(selectedCopyObj.indexId);

    auto reply = QMessageBox::question(this, "确认还书",
                                       QStringLiteral("确定要归还图书《%1》的副本%2吗？")
                                       .arg(book->name).arg(selectedCopyObj.copyNumber),
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No) {
        return;
    }

    QString error;
    if (library_.returnBook(selectedCopyObj.copyId, currentUsername_, &error)) {
        refreshTable();
        QMessageBox::information(this, "成功",
                                 QStringLiteral("成功归还图书《%1》的副本%2")
                                 .arg(book->name).arg(selectedCopyObj.copyNumber));
    } else {
        QMessageBox::warning(this, "失败", "归还失败：" + error);
    }
}

void MainWindow::onWarn()
{
    isWarn = !isWarn; // 切换状态

    if (isWarn) {
        // --- 模式激活：显示即将到期的图书 ---
        QVector<Book> dueSoonBooks = library_.getWarn(3);
        displayBooks(dueSoonBooks);

        if (dueSoonBooks.isEmpty()) {
            statusBar()->showMessage("✅ 暂无即将到期的图书。", 5000);
        } else {
            QString message = QStringLiteral("⚠️ 找到 %1 本即将到期的图书。").arg(dueSoonBooks.size());
            statusBar()->showMessage(message, 5000);
        }

        auto *warnButton = qobject_cast<QAction *>(sender());
        if (warnButton) {
            warnButton->setText(QStringLiteral("🔙 显示全部"));
        }
    } else {
        // --- 模式取消：显示所有图书 ---
        onShowAll();
        statusBar()->showMessage("已显示所有图书", 3000);

        auto *warnButton = qobject_cast<QAction *>(sender());
        if (warnButton) {
            warnButton->setText(QStringLiteral("⏰ 到期提醒"));
        }
    }
}

void MainWindow::onAddBook()
{
    if (!isAdminMode_) {
        QMessageBox::warning(this, "权限不足", "只有管理员可以添加图书，请以管理员模式登录。");
        return;
    }
    showBookDialog(Book(), false);
}

void MainWindow::onEditBook()
{
    if (!isAdminMode_) {
        QMessageBox::warning(this, "权限不足", "只有管理员可以编辑图书，请以管理员模式登录。");
        return;
    }

    QModelIndexList selectedIndexes = tableView_->selectionModel()->selectedRows();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择要编辑的图书！");
        return;
    }

    int row = selectedIndexes.first().row();
    QString indexId = model_->item(row, 0)->text();

    const Book *bookPtr = library_.findByIndexId(indexId);
    if (bookPtr) {
        showBookDialog(*bookPtr, true);
    }
}

void MainWindow::onDeleteBook()
{
    if (!isAdminMode_) {
        QMessageBox::warning(this, "权限不足", "只有管理员可以删除图书，请以管理员模式登录。");
        return;
    }

    QModelIndexList selectedIndexes = tableView_->selectionModel()->selectedRows();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择要删除的图书！");
        return;
    }

    int row = selectedIndexes.first().row();
    QString indexId = model_->item(row, 0)->text();
    QString bookName = model_->item(row, 1)->text();

    // 确认删除
    auto reply = QMessageBox::question(this, "确认删除",
                                       QStringLiteral("确定要删除图书《%1》吗？此操作不可恢复！").arg(bookName),
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No) {
        return;
    }

    if (library_.removeBookByIndexId(indexId)) {
        refreshTable();
        QMessageBox::information(this, "成功", QStringLiteral("成功删除图书《%1》").arg(bookName));
    } else {
        QMessageBox::warning(this, "失败", "删除失败！该图书可能有副本正在被借阅。");
    }
}

void MainWindow::onManageCopies()
{
    if (!isAdminMode_) {
        QMessageBox::warning(this, "权限不足", "只有管理员可以管理副本。");
        return;
    }

    QModelIndexList selectedIndexes = tableView_->selectionModel()->selectedRows();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择要管理的图书！");
        return;
    }

    int row = selectedIndexes.first().row();
    QString indexId = model_->item(row, 0)->text();

    CopyManagementDialog dialog(indexId, this);
    dialog.exec();

    refreshTable();
}

void MainWindow::onShowAll()
{
    categoryFilter_.clear();
    statusFilter_.clear();
    locationFilter_.clear();
    refreshTable();
}

void MainWindow::onSortByBorrowCount()
{
    currentSortType_ = "borrowCount";
    library_.sortByBorrowCount();
    refreshTable();
    updateHeaderLabels();
    statusBar()->showMessage("已按借阅次数排序（从高到低）", 3000);
}

void MainWindow::onSortDefault()
{
    currentSortType_ = "default";
    library_.loadFromDatabase();
    refreshTable();
    updateHeaderLabels();
    statusBar()->showMessage("已恢复默认排序", 3000);
}

void MainWindow::onSortChanged(QAction *action)
{
    if (!action)
        return;
    QString sortType = action->data().toString();
    currentSortType_ = sortType;

    if (sortType == "borrowCount") {
        library_.sortByBorrowCount();
    } else if (sortType == "default") {
        library_.loadFromDatabase();
    }

    refreshTable();
    updateHeaderLabels();
}

void MainWindow::onSwitchMode()
{
    isEditMode_ = !isEditMode_;
    setWindowTitle(isEditMode_ ? QStringLiteral("图书管理系统 (编辑模式)") : QStringLiteral("图书管理系统 (只读模式)"));
    QMessageBox::information(this, "模式切换",
                             isEditMode_ ? "已切换到编辑模式" : "已切换到只读模式");
}

void MainWindow::onSearch()
{
    if (!searchEdit_)
        return;
    QString name = searchEdit_->text().trimmed();
    if (name.isEmpty()) {
        onShowAll();
        return;
    }

    const Book *b = library_.findByName(name);
    if (b) {
        model_->removeRows(0, model_->rowCount());
        QList<QStandardItem *> rowItems;
        rowItems << new QStandardItem(b->indexId);
        rowItems << new QStandardItem(b->name);
        rowItems << new QStandardItem(b->author);
        rowItems << new QStandardItem(b->publisher);
        rowItems << new QStandardItem(b->location);
        rowItems << new QStandardItem(b->category);

        int totalCopies = library_.getTotalCopyCount(b->indexId);
        rowItems << new QStandardItem(QString::number(totalCopies));

        rowItems << new QStandardItem(QString::number(b->price, 'f', 2));
        rowItems << new QStandardItem(b->inDate.toString("yyyy-MM-dd"));

        // 归还日期：根据当前用户显示
        QString returnDateStr = "";
        if (!currentUsername_.isEmpty() && !isAdminMode_) {
            QVector<BookCopy> borrowedCopies = library_.getUserBorrowedCopies(currentUsername_);
            for (const BookCopy &copy : borrowedCopies) {
                if (copy.indexId == b->indexId) {
                    returnDateStr = copy.dueDate.toString("yyyy-MM-dd");
                    break;
                }
            }
        }
        rowItems << new QStandardItem(returnDateStr);

        rowItems << new QStandardItem(QString::number(b->borrowCount));

        int availableCopies = library_.getAvailableCopyCount(b->indexId);
        QString statusText = (availableCopies > 0) ? QStringLiteral("可借") : QStringLiteral("不可借");
        rowItems << new QStandardItem(statusText);

        model_->appendRow(rowItems);
    } else {
        QMessageBox::information(this, "未找到", QStringLiteral("没有找到名称为 \"%1\" 的图书").arg(name));
    }
}

void MainWindow::onOpen()
{
    QString path = QFileDialog::getOpenFileName(this, "导入图书数据", "", "JSON Files (*.json)");
    if (!path.isEmpty()) {
        if (library_.importFromJson(path)) {
            rebuildFilterMenus();
            refreshTable();
            QMessageBox::information(this, "成功", "数据导入成功！");
        } else {
            QMessageBox::warning(this, "失败", "文件导入失败！");
        }
    }
}

void MainWindow::onSave()
{
    QString path = QFileDialog::getSaveFileName(this, "导出图书数据", "library_export.json", "JSON Files (*.json)");
    if (!path.isEmpty()) {
        if (library_.exportToJson(path)) {
            QMessageBox::information(this, "成功", "数据导出成功！");
        } else {
            QMessageBox::warning(this, "失败", "文件导出失败！");
        }
    }
}

void MainWindow::onImport()
{
    onOpen();
}

void MainWindow::onExport()
{
    onSave();
}

void MainWindow::onRefresh()
{
    library_.loadFromDatabase();
    rebuildFilterMenus();
    refreshTable();
    QMessageBox::information(this, "成功", "数据已刷新！");
}

void MainWindow::onCategoryFilterChanged(QAction *action)
{
    if (action) {
        categoryFilter_ = action->data().toString();
        refreshTable();
    }
}

void MainWindow::onStatusFilterChanged(QAction *action)
{
    if (action) {
        statusFilter_ = action->data().toString();
        refreshTable();
    }
}

void MainWindow::onLocationFilterChanged(QAction *action)
{
    if (action) {
        locationFilter_ = action->data().toString();
        refreshTable();
    }
}

// ============================================================================
// UI设置和其他辅助函数
// ============================================================================
void MainWindow::setupActions()
{
    // 1. 创建一个纯粹的 QToolBar
    actionToolBar_ = new QToolBar(QStringLiteral("操作"), this);

    // 2. 设置其自身属性
    actionToolBar_->setMovable(false);
    actionToolBar_->setFloatable(false);
    actionToolBar_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    actionToolBar_->setOrientation(Qt::Vertical);

    // 3. 将工具栏放入一个容器
    toolBarScrollArea_ = new QScrollArea();
    toolBarScrollArea_->setWidgetResizable(false);
    toolBarScrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    toolBarScrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QWidget *toolContainer = new QWidget();
    QVBoxLayout *toolContainerLayout = new QVBoxLayout(toolContainer);
    toolContainerLayout->setContentsMargins(6, 6, 6, 6);
    toolContainerLayout->setSpacing(8);
    toolContainerLayout->addWidget(actionToolBar_);
    toolContainerLayout->addStretch();

    toolContainer->setFixedWidth(150);
    toolContainer->setMinimumHeight(800);
    toolBarScrollArea_->setWidget(toolContainer);
    toolBarScrollArea_->setFixedWidth(150);
    toolBarScrollArea_->setMaximumHeight(QWIDGETSIZE_MAX);

    toolContainer->setStyleSheet(QString("background-color: %1;").arg(isDarkMode_ ? "#22333B" : "#FEEFF1"));

    // 4. 创建 DockWidget 并将包含工具栏的 ScrollArea 放入其中
    toolBarDockWidget_ = createDockWidgetFromScrollArea(toolBarScrollArea_);

    // 5. 将整个 DockWidget 添加到主窗口
    addDockWidget(Qt::LeftDockWidgetArea, toolBarDockWidget_);

    // --- 创建所有按钮 ---
    borrowAct_ = actionToolBar_->addAction(QStringLiteral("📖 借书"));
    returnAct_ = actionToolBar_->addAction(QStringLiteral("📤 还书"));
    warnAct_ = actionToolBar_->addAction(QStringLiteral("⏰ 到期提醒"));
    myBorrowAct_ = actionToolBar_->addAction(QStringLiteral("📚 我的借阅"));
    allAct_ = actionToolBar_->addAction(QStringLiteral("📋 显示全部"));
    actionToolBar_->addSeparator();
    addBookAct_ = actionToolBar_->addAction(QStringLiteral("➕ 添加图书"));
    editBookAct_ = actionToolBar_->addAction(QStringLiteral("✏️ 编辑图书"));
    deleteBookAct_ = actionToolBar_->addAction(QStringLiteral("🗑️ 删除图书"));
    manageCopiesAct_ = actionToolBar_->addAction(QStringLiteral("📋 管理副本"));
    bookHistoryAct_ = actionToolBar_->addAction(QStringLiteral("📑 借阅记录"));
    actionToolBar_->addSeparator();
    importBookAct_ = actionToolBar_->addAction(QStringLiteral("📥 导入图书数据"));
    exportBookAct_ = actionToolBar_->addAction(QStringLiteral("📤 导出图书数据"));
    importUsersAct_ = actionToolBar_->addAction(QStringLiteral("📥 导入学生数据"));
    exportUsersAct_ = actionToolBar_->addAction(QStringLiteral("📤 导出学生数据"));
    actionToolBar_->addSeparator();
    toggleOrientationAct_ = actionToolBar_->addAction(QStringLiteral("🔄 切换布局"));

    // --- 连接信号 ---
    connect(borrowAct_, &QAction::triggered, this, &MainWindow::onBorrow);
    connect(returnAct_, &QAction::triggered, this, &MainWindow::onReturn);
    connect(warnAct_, &QAction::triggered, this, &MainWindow::onWarn);
    connect(myBorrowAct_, &QAction::triggered, this, &MainWindow::onShowMyBorrows);
    connect(allAct_, &QAction::triggered, this, &MainWindow::onShowAll);

    connect(addBookAct_, &QAction::triggered, this, &MainWindow::onAddBook);
    connect(editBookAct_, &QAction::triggered, this, &MainWindow::onEditBook);
    connect(deleteBookAct_, &QAction::triggered, this, &MainWindow::onDeleteBook);
    connect(manageCopiesAct_, &QAction::triggered, this, &MainWindow::onManageCopies);
    connect(bookHistoryAct_, &QAction::triggered, this, &MainWindow::onShowBookBorrowHistory);
    connect(importBookAct_, &QAction::triggered, this, &MainWindow::onImport);
    connect(exportBookAct_, &QAction::triggered, this, &MainWindow::onExport);
    connect(importUsersAct_, &QAction::triggered, this, &MainWindow::onImportUsers);
    connect(exportUsersAct_, &QAction::triggered, this, &MainWindow::onExportUsers);
    connect(toggleOrientationAct_, &QAction::triggered, this, &MainWindow::toggleToolBarOrientation);

    // --- 初始状态 ---
    updateActionsVisibility();
}

void MainWindow::setupMenuBar()
{
    // 菜单栏可以暂时留空，或者添加一些与工具栏重复的功能
}

void MainWindow::setupSearchBar()
{
    auto *searchWidget = new QWidget();
    auto *searchLayout = new QHBoxLayout(searchWidget);
    searchLayout->setContentsMargins(16, 8, 16, 8);
    searchLayout->setSpacing(8);

    searchEdit_ = new QLineEdit();
    searchEdit_->setPlaceholderText("🔍 搜索图书名称...");

    searchButton_ = new QPushButton("搜索");

    themeToggleButton_ = new QPushButton("🌙");
    themeToggleButton_->setToolTip("切换深浅色模式");

    searchLayout->addWidget(searchEdit_);
    searchLayout->addWidget(searchButton_);
    searchLayout->addWidget(themeToggleButton_);

    addToolBarBreak(Qt::TopToolBarArea);

    QToolBar *searchToolBar = addToolBar("搜索");
    searchToolBar->setMovable(false);
    searchToolBar->setFloatable(false);
    searchToolBar->addWidget(searchWidget);
    searchToolBar->setAllowedAreas(Qt::TopToolBarArea);

    connect(searchButton_, &QPushButton::clicked, this, &MainWindow::onSearch);
    connect(searchEdit_, &QLineEdit::returnPressed, this, &MainWindow::onSearch);
    connect(themeToggleButton_, &QPushButton::clicked, this, &MainWindow::toggleTheme);
}

void MainWindow::setupThemeToggle()
{
    isDarkMode_ = false;
}

void MainWindow::toggleTheme()
{
    isDarkMode_ = !isDarkMode_;
    applyTheme(isDarkMode_);
    if (themeToggleButton_) {
        themeToggleButton_->setText(isDarkMode_ ? "☀️" : "🌙");
        themeToggleButton_->setToolTip(isDarkMode_ ? "切换到浅色模式" : "切换到深色模式");
    }
}

void MainWindow::setupStyles()
{
    applyTheme(false);
    setWindowTitle(QStringLiteral("图书管理系统"));
    setWindowIcon(QIcon("..//library.svg"));
    statusBar()->setMinimumHeight(28);
    updateStatusBar();
}

void MainWindow::updateStatusBar()
{
    int totalBooks = library_.getTotalBooks();
    int totalCopies = library_.getTotalCopies();
    int availableCopies = library_.getAvailableCopies();
    int borrowedCopies = totalCopies - availableCopies;

    QString statusText = QStringLiteral("📊 图书种类: %1 | 📚 总副本: %2 | ✅ 可借: %3 | ❌ 已借: %4")
                             .arg(totalBooks)
                             .arg(totalCopies)
                             .arg(availableCopies)
                             .arg(borrowedCopies);
    statusBar()->showMessage(statusText);
}

// ============================================================================
// 当前用户设置
// ============================================================================
void MainWindow::setCurrentUser(const QString &username, bool isAdminMode, const QString &usersFilePath)
{
    currentUsername_ = username;
    isAdminMode_ = isAdminMode;
    usersFilePath_ = usersFilePath;

    if (isAdminMode_) {
        setWindowTitle(QStringLiteral("图书管理系统 - 管理员模式 (%1)").arg(username));
    } else {
        setWindowTitle(QStringLiteral("图书管理系统 - 学生模式 (%1)").arg(username));
    }

    updateActionsVisibility();
    refreshTable();
}

void MainWindow::updateActionsVisibility()
{
    // 普通用户可见的按钮
    bool isStudent = !isAdminMode_;

    if (borrowAct_)
        borrowAct_->setVisible(isStudent);
    if (returnAct_)
        returnAct_->setVisible(isStudent);
    if (warnAct_)
        warnAct_->setVisible(isStudent);
    if (myBorrowAct_)
        myBorrowAct_->setVisible(isStudent);
    if (allAct_)
        allAct_->setVisible(isStudent);

    // 管理员可见的按钮
    if (addBookAct_)
        addBookAct_->setVisible(isAdminMode_);
    if (editBookAct_)
        editBookAct_->setVisible(isAdminMode_);
    if (deleteBookAct_)
        deleteBookAct_->setVisible(isAdminMode_);
    if (manageCopiesAct_)
        manageCopiesAct_->setVisible(isAdminMode_);
    if (bookHistoryAct_)
        bookHistoryAct_->setVisible(isAdminMode_);
    if (importBookAct_)
        importBookAct_->setVisible(isAdminMode_);
    if (exportBookAct_)
        exportBookAct_->setVisible(isAdminMode_);
    if (importUsersAct_)
        importUsersAct_->setVisible(isAdminMode_);
    if (exportUsersAct_)
        exportUsersAct_->setVisible(isAdminMode_);

    // 切换布局按钮对所有用户可见
    if (toggleOrientationAct_)
        toggleOrientationAct_->setVisible(true);
}

void MainWindow::toggleToolBarOrientation()
{
    if (!actionToolBar_ || !toolBarDockWidget_ || !toolBarScrollArea_) {
        return;
    }

    isToolBarVertical_ = !isToolBarVertical_;

    removeDockWidget(toolBarDockWidget_);

    if (isToolBarVertical_) {
        actionToolBar_->setOrientation(Qt::Vertical);
        actionToolBar_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        actionToolBar_->setMinimumWidth(110);
        actionToolBar_->setMinimumHeight(0);
        actionToolBar_->setMaximumHeight(QWIDGETSIZE_MAX);

        toolBarScrollArea_->setWidgetResizable(false);
        toolBarScrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        toolBarScrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        toolBarScrollArea_->setFixedWidth(130);
        toolBarScrollArea_->setMinimumHeight(100);
        toolBarScrollArea_->setMaximumHeight(QWIDGETSIZE_MAX);
        toolBarDockWidget_->setFixedWidth(130);

        toolBarDockWidget_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        addDockWidget(Qt::LeftDockWidgetArea, toolBarDockWidget_);
    } else {
        actionToolBar_->setOrientation(Qt::Horizontal);
        actionToolBar_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        actionToolBar_->setFixedHeight(45);
        actionToolBar_->setMinimumWidth(0);
        actionToolBar_->setMaximumWidth(QWIDGETSIZE_MAX);

        toolBarScrollArea_->setWidgetResizable(false);
        toolBarScrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        toolBarScrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        toolBarScrollArea_->setMinimumHeight(50);
        toolBarScrollArea_->setMaximumHeight(50);
        toolBarScrollArea_->setMinimumWidth(200);
        toolBarScrollArea_->setMaximumWidth(QWIDGETSIZE_MAX);
        toolBarDockWidget_->setMinimumWidth(0);
        toolBarDockWidget_->setMaximumWidth(QWIDGETSIZE_MAX);
        toolBarScrollArea_->setMinimumWidth(200);

        toolBarDockWidget_->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
        addDockWidget(Qt::TopDockWidgetArea, toolBarDockWidget_);
    }

    statusBar()->showMessage(
        isToolBarVertical_ ? "已切换到竖向布局（左边）" : "已切换到横向布局（顶部）",
        2000);
}

void MainWindow::onImportUsers()
{
    if (!isAdminMode_) {
        QMessageBox::warning(this, "权限不足", "只有管理员可以导入学生数据。");
        return;
    }

    QString path = QFileDialog::getOpenFileName(this, "导入学生数据", "", "JSON Files (*.json)");
    if (!path.isEmpty()) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, "失败", "无法打开文件！");
            return;
        }

        QByteArray data = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull() || !doc.isArray()) {
            QMessageBox::warning(this, "失败", "文件格式错误！");
            return;
        }

        QJsonArray importedArray = doc.array();
        QJsonArray currentArray = loadUsersJson();

        QSet<QString> existingUsernames;
        for (const QJsonValue &value : currentArray) {
            if (value.isObject()) {
                existingUsernames.insert(value.toObject().value("username").toString());
            }
        }

        int addedCount = 0;
        for (const QJsonValue &value : importedArray) {
            if (!value.isObject())
                continue;
            QJsonObject userObj = value.toObject();
            QString username = userObj.value("username").toString();
            if (!existingUsernames.contains(username)) {
                currentArray.append(userObj);
                existingUsernames.insert(username);
                addedCount++;
            }
        }

        if (saveUsersJson(currentArray)) {
            QMessageBox::information(this, "成功",
                                     QStringLiteral("成功导入 %1 条学生数据！").arg(addedCount));
        } else {
            QMessageBox::warning(this, "失败", "保存学生数据失败！");
        }
    }
}

void MainWindow::onExportUsers()
{
    if (!isAdminMode_) {
        QMessageBox::warning(this, "权限不足", "只有管理员可以导出学生数据。");
        return;
    }

    QString path = QFileDialog::getSaveFileName(this, "导出学生数据", "users_export.json", "JSON Files (*.json)");
    if (!path.isEmpty()) {
        QJsonArray usersArray = loadUsersJson();
        QJsonDocument doc(usersArray);

        QFile file(path);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(doc.toJson());
            file.close();
            QMessageBox::information(this, "成功", "学生数据导出成功！");
        } else {
            QMessageBox::warning(this, "失败", "无法保存文件！");
        }
    }
}

QDockWidget *MainWindow::createDockWidgetFromScrollArea(QScrollArea *scrollArea)
{
    QDockWidget *dockWidget = new QDockWidget("功能栏", this);
    dockWidget->setWidget(scrollArea);
    dockWidget->setFeatures(QDockWidget::DockWidgetMovable |
                            QDockWidget::DockWidgetFloatable |
                            QDockWidget::DockWidgetClosable);
    dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea |
                                Qt::RightDockWidgetArea |
                                Qt::TopDockWidgetArea |
                                Qt::BottomDockWidgetArea);
    return dockWidget;
}

void MainWindow::applyTheme(bool isDark)
{
    QString styles = getThemeStyles(isDark);
    setStyleSheet(styles);

    QString menuStyles = getMenuStyles(isDark);
    if (categoryFilterMenu_) {
        categoryFilterMenu_->setStyleSheet(menuStyles);
    }
    if (statusFilterMenu_) {
        statusFilterMenu_->setStyleSheet(menuStyles);
    }
    if (locationFilterMenu_) {
        locationFilterMenu_->setStyleSheet(menuStyles);
    }
    if (sortMenu_) {
        sortMenu_->setStyleSheet(menuStyles);
    }
}

void MainWindow::showBookDialog(const Book &book, bool isEdit)
{
    BookDialog dialog(this);
    if (isEdit) {
        dialog.setBook(book);
        dialog.setWindowTitle("编辑图书信息");
    } else {
        dialog.setWindowTitle("添加新图书");
    }

    if (dialog.exec() == QDialog::Accepted) {
        Book newBook = dialog.getBook();

        if (newBook.indexId.isEmpty() || newBook.name.isEmpty()) {
            QMessageBox::warning(this, "错误", "索引号和名称不能为空！");
            return;
        }

        QString error;
        bool success;

        if (isEdit) {
            success = library_.updateBook(book.indexId, newBook, &error);
        } else {
            success = library_.addBook(newBook, &error);
        }

        if (success) {
            refreshTable();
            QMessageBox::information(this, "成功",
                                     isEdit ? "图书信息更新成功！" : "图书添加成功！");
        } else {
            QMessageBox::warning(this, "失败", error);
        }
    }
}

QString MainWindow::getMenuStyles(bool isDark)
{
    if (isDark) {
        return QString(
            "QMenu {"
            "    background-color: #22333B;"
            "    border: 2px solid #3A4A52;"
            "    border-radius: 8px;"
            "    padding: 8px 0px;"
            "    color: #D1E7DD;"
            "    font-size: 14px;"
            "    font-weight: 500;"
            "}"
            "QMenu::item {"
            "    background-color: transparent;"
            "    padding: 12px 24px;"
            "    border: none;"
            "    min-height: 20px;"
            "}"
            "QMenu::item:selected {"
            "    background-color: #52B788;"
            "    color: #1A252F;"
            "    border-radius: 4px;"
            "    margin: 0px 8px;"
            "}"
            "QMenu::item:checked {"
            "    background-color: #52B788;"
            "    color: #1A252F;"
            "    border-radius: 4px;"
            "    margin: 0px 8px;"
            "}"
            "QMenu::separator {"
            "    height: 1px;"
            "    background-color: #3A4A52;"
            "    margin: 8px 16px;"
            "}");
    } else {
        return QString(
            "QMenu {"
            "    background-color: #FFFFFF;"
            "    border: 2px solid #F8D7DC;"
            "    border-radius: 8px;"
            "    padding: 8px 0px;"
            "    color: #5A4B56;"
            "    font-size: 14px;"
            "    font-weight: 500;"
            "}"
            "QMenu::item {"
            "    background-color: transparent;"
            "    padding: 12px 24px;"
            "    border: none;"
            "    min-height: 20px;"
            "}"
            "QMenu::item:selected {"
            "    background-color: #F9A8D4;"
            "    color: #FFFFFF;"
            "    border-radius: 4px;"
            "    margin: 0px 8px;"
            "}"
            "QMenu::item:checked {"
            "    background-color: #F9A8D4;"
            "    color: #FFFFFF;"
            "    border-radius: 4px;"
            "    margin: 0px 8px;"
            "}"
            "QMenu::separator {"
            "    height: 1px;"
            "    background-color: #F8D7DC;"
            "    margin: 8px 16px;"
            "}");
    }
}

QString MainWindow::getThemeStyles(bool isDark)
{
    if (isDark) {
        return QString(
            "QMainWindow {"
            "    background-color: #1A252F;"
            "    color: #D1E7DD;"
            "    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;"
            "}"
            "QToolBar {"
            "    background-color: #22333B;"
            "    border: none;"
            "    border-right: 1px solid #3A4A52;"
            "    spacing: 8px;"
            "    padding: 12px 8px;"
            "}"
            "QToolButton {"
            "    background-color: #3A4A52;"
            "    color: #D1E7DD;"
            "    border: 1px solid #4A5A62;"
            "    border-radius: 12px;"
            "    padding: 10px 6px;"
            "    margin: 2px;"
            "    font-size: 13px;"
            "    font-weight: 600;"
            "    min-width: 110px;"
            "    min-height: 45px;"
            "    max-width: 150px;"
            "    text-align: center;"
            "}"
            "QToolButton:hover {"
            "    background-color: #4A5A62;"
            "    border-color: #52B788;"
            "    color: #52B788;"
            "}"
            "QToolButton:pressed {"
            "    background-color: #52B788;"
            "    color: #1A252F;"
            "    border-color: #40916C;"
            "}"
            "QStatusBar {"
            "    background-color: #22333B;"
            "    color: #95D5B2;"
            "    border-top: 1px solid #3A4A52;"
            "    padding: 6px 16px;"
            "    font-size: 14px;"
            "    min-height: 28px;"
            "    line-height: 1.4;"
            "}"
            "QTableView {"
            "    background-color: #1A252F;"
            "    alternate-background-color: #22333B;"
            "    selection-background-color: #52B788;"
            "    selection-color: #1A252F;"
            "    gridline-color: #3A4A52;"
            "    color: #D1E7DD;"
            "    border: 1px solid #3A4A52;"
            "    border-radius: 12px;"
            "}"
            "QTableView::item {"
            "    padding: 12px 16px;"
            "    border: none;"
            "    min-height: 44px;"
            "    font-size: 15px;"
            "    color: #D1E7DD;"
            "}"
            "QTableView::item:selected {"
            "    background-color: #52B788;"
            "    color: #1A252F;"
            "}"
            "QTableView::item:hover {"
            "    background-color: #3A4A52;"
            "}"
            "QHeaderView::section {"
            "    background-color: #22333B;"
            "    color: #F4A261;"
            "    padding: 16px 12px;"
            "    border: none;"
            "    font-weight: 600;"
            "    font-size: 15px;"
            "    min-height: 60px;"
            "    border-bottom: 2px solid #F4A261;"
            "}"
            "QHeaderView::section:hover {"
            "    background-color: #3A4A52;"
            "}"
            "QLineEdit {"
            "    background-color: #3A4A52;"
            "    border: 2px solid #4A5A62;"
            "    border-radius: 20px;"
            "    padding: 8px 16px;"
            "    font-size: 14px;"
            "    color: #D1E7DD;"
            "    min-height: 20px;"
            "}"
            "QLineEdit:focus {"
            "    border-color: #52B788;"
            "    background-color: #4A5A62;"
            "}"
            "QPushButton {"
            "    background-color: #F4A261;"
            "    color: #1A252F;"
            "    border: none;"
            "    border-radius: 20px;"
            "    padding: 8px 20px;"
            "    font-size: 14px;"
            "    font-weight: 600;"
            "    min-width: 60px;"
            "    min-height: 20px;"
            "}"
            "QPushButton:hover {"
            "    background-color: #E76F51;"
            "}"
            "QDockWidget {"
            "    background-color: #22333B;"
            "    border: none;"
            "    border-right: 1px solid #3A4A52;"
            "}"
            "QScrollArea {"
            "    background-color: #22333B;"
            "    border: none;"
            "}"
            "QScrollBar:vertical {"
            "    background-color: #3A4A52;"
            "    width: 8px;"
            "    border-radius: 4px;"
            "}"
            "QScrollBar::handle:vertical {"
            "    background-color: #4A5A62;"
            "    border-radius: 4px;"
            "    min-height: 20px;"
            "}"
            "QScrollBar::handle:vertical:hover {"
            "    background-color: #52B788;"
            "}"
            "QScrollBar:horizontal {"
            "    background-color: #22333B;"
            "    height: 12px;"
            "    border-radius: 6px;"
            "    margin: 2px;"
            "}"
            "QScrollBar::handle:horizontal {"
            "    background-color: #52B788;"
            "    border-radius: 6px;"
            "    min-width: 30px;"
            "    margin: 2px;"
            "}"
            "QScrollBar::handle:horizontal:hover {"
            "    background-color: #74C69D;"
            "}"
            "QScrollBar::handle:horizontal:pressed {"
            "    background-color: #40916C;"
            "}"
            "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {"
            "    width: 0px;"
            "    background: none;"
            "}"
            "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {"
            "    background: none;"
            "}");
    } else {
        return QString(
            "QMainWindow {"
            "    background-color: #FFF9FA;"
            "    color: #5A4B56;"
            "    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;"
            "}"
            "QToolBar {"
            "    background-color: #FEEFF1;"
            "    border: none;"
            "    border-right: 1px solid #F8D7DC;"
            "    spacing: 8px;"
            "    padding: 12px 8px;"
            "}"
            "QToolButton {"
            "    background-color: #FFFFFF;"
            "    color: #5A4B56;"
            "    border: 1px solid #F8D7DC;"
            "    border-radius: 12px;"
            "    padding: 10px 6px;"
            "    margin: 2px;"
            "    font-size: 13px;"
            "    font-weight: 600;"
            "    min-width: 110px;"
            "    min-height: 45px;"
            "    max-width: 150px;"
            "    text-align: center;"
            "}"
            "QToolButton:hover {"
            "    background-color: #FEEFF1;"
            "    border-color: #F9A8D4;"
            "    color: #E11D48;"
            "}"
            "QToolButton:pressed {"
            "    background-color: #F9A8D4;"
            "    color: #FFFFFF;"
            "    border-color: #DB7093;"
            "}"
            "QStatusBar {"
            "    background-color: #FEEFF1;"
            "    color: #E11D48;"
            "    border-top: 1px solid #F8D7DC;"
            "    padding: 6px 16px;"
            "    font-size: 14px;"
            "    min-height: 28px;"
            "    line-height: 1.4;"
            "}"
            "QTableView {"
            "    background-color: #FFFFFF;"
            "    alternate-background-color: #FEEFF1;"
            "    selection-background-color: #F9A8D4;"
            "    selection-color: #FFFFFF;"
            "    gridline-color: #F8D7DC;"
            "    color: #5A4B56;"
            "    border: 1px solid #F8D7DC;"
            "    border-radius: 12px;"
            "}"
            "QTableView::item {"
            "    padding: 12px 16px;"
            "    border: none;"
            "    min-height: 44px;"
            "    font-size: 15px;"
            "    color: #5A4B56;"
            "}"
            "QTableView::item:selected {"
            "    background-color: #F9A8D4;"
            "    color: #FFFFFF;"
            "}"
            "QTableView::item:hover {"
            "    background-color: #FEEFF1;"
            "}"
            "QHeaderView::section {"
            "    background-color: #FEEFF1;"
            "    color: #E11D48;"
            "    padding: 16px 12px;"
            "    border: none;"
            "    font-weight: 600;"
            "    font-size: 15px;"
            "    min-height: 60px;"
            "    border-bottom: 2px solid #F9A8D4;"
            "}"
            "QHeaderView::section:hover {"
            "    background-color: #FEE5E9;"
            "}"
            "QLineEdit {"
            "    background-color: #FFFFFF;"
            "    border: 2px solid #F8D7DC;"
            "    border-radius: 20px;"
            "    padding: 8px 16px;"
            "    font-size: 14px;"
            "    color: #5A4B56;"
            "    min-height: 20px;"
            "}"
            "QLineEdit:focus {"
            "    border-color: #F9A8D4;"
            "    background-color: #FFF5F7;"
            "}"
            "QPushButton {"
            "    background-color: #F9A8D4;"
            "    color: #FFFFFF;"
            "    border: none;"
            "    border-radius: 20px;"
            "    padding: 8px 20px;"
            "    font-size: 14px;"
            "    font-weight: 600;"
            "    min-width: 60px;"
            "    min-height: 20px;"
            "}"
            "QPushButton:hover {"
            "    background-color: #E11D48;"
            "}"
            "QDockWidget {"
            "    background-color: #FEEFF1;"
            "    border: none;"
            "    border-right: 1px solid #F8D7DC;"
            "}"
            "QScrollArea {"
            "    background-color: #FEEFF1;"
            "    border: none;"
            "}"
            "QScrollBar:vertical {"
            "    background-color: #F8D7DC;"
            "    width: 8px;"
            "    border-radius: 4px;"
            "}"
            "QScrollBar::handle:vertical {"
            "    background-color: #F9A8D4;"
            "    border-radius: 4px;"
            "    min-height: 20px;"
            "}"
            "QScrollBar::handle:vertical:hover {"
            "    background-color: #E11D48;"
            "}"
            "QScrollBar:horizontal {"
            "    background-color: #FEEFF1;"
            "    height: 12px;"
            "    border-radius: 6px;"
            "    margin: 2px;"
            "}"
            "QScrollBar::handle:horizontal {"
            "    background-color: #F9A8D4;"
            "    border-radius: 6px;"
            "    min-width: 30px;"
            "    margin: 2px;"
            "}"
            "QScrollBar::handle:horizontal:hover {"
            "    background-color: #F7B2D7;"
            "}"
            "QScrollBar::handle:horizontal:pressed {"
            "    background-color: #E11D48;"
            "}"
            "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {"
            "    width: 0px;"
            "    background: none;"
            "}"
            "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {"
            "    background: none;"
            "}");
    }
}

void MainWindow::rebuildFilterMenus()
{
    delete categoryFilterMenu_;
    delete statusFilterMenu_;
    delete locationFilterMenu_;
    delete sortMenu_;
    delete categoryActionGroup_;
    delete statusActionGroup_;
    delete locationActionGroup_;
    delete sortActionGroup_;

    // 类别筛选菜单
    categoryFilterMenu_ = new QMenu(this);
    categoryFilterMenu_->setMinimumWidth(200);
    categoryActionGroup_ = new QActionGroup(categoryFilterMenu_);
    categoryActionGroup_->setExclusive(true);

    auto addCategoryAction = [this](const QString &label, const QString &value, bool separator = false) {
        if (separator) {
            categoryFilterMenu_->addSeparator();
            return static_cast<QAction *>(nullptr);
        }
        QAction *action = categoryFilterMenu_->addAction(label);
        action->setCheckable(true);
        action->setData(value);
        categoryActionGroup_->addAction(action);
        if (value == categoryFilter_) {
            action->setChecked(true);
        }
        return action;
    };

    QAction *allCategoryAction = addCategoryAction(QStringLiteral("全部类别"), QString());
    if (categoryFilter_.isEmpty() && allCategoryAction) {
        allCategoryAction->setChecked(true);
    }

    QSet<QString> categorySet;
    for (const Book &book : library_.getAll()) {
        if (!book.category.isEmpty()) {
            categorySet.insert(book.category);
        }
    }
    if (!categoryFilter_.isEmpty() && !categorySet.contains(categoryFilter_)) {
        categoryFilter_.clear();
    }
    QList<QString> categories = QList<QString>(categorySet.begin(), categorySet.end());
    std::sort(categories.begin(), categories.end(), [](const QString &a, const QString &b) {
        return a.localeAwareCompare(b) < 0;
    });

    if (!categories.isEmpty()) {
        addCategoryAction(QString(), QString(), true);
    }
    for (const QString &category : categories) {
        addCategoryAction(category, category);
    }

    connect(categoryActionGroup_, &QActionGroup::triggered, this, [this](QAction *action) {
        categoryFilter_ = action->data().toString();
        refreshTable();
    });

    // 状态筛选菜单
    statusFilterMenu_ = new QMenu(this);
    statusFilterMenu_->setMinimumWidth(200);
    statusActionGroup_ = new QActionGroup(statusFilterMenu_);
    statusActionGroup_->setExclusive(true);

    auto addStatusAction = [this](const QString &label, const QString &value) {
        QAction *action = statusFilterMenu_->addAction(label);
        action->setCheckable(true);
        action->setData(value);
        statusActionGroup_->addAction(action);
        if (statusFilter_ == value) {
            action->setChecked(true);
        }
        return action;
    };

    QAction *allStatusAction = addStatusAction(QStringLiteral("全部状态"), QString());
    QAction *availableAction = addStatusAction(QStringLiteral("仅可借"), QStringLiteral("available"));
    QAction *borrowedAction = addStatusAction(QStringLiteral("仅已借出"), QStringLiteral("borrowed"));

    if (statusFilter_.isEmpty() && allStatusAction) {
        allStatusAction->setChecked(true);
    } else if (statusFilter_ == "available" && availableAction) {
        availableAction->setChecked(true);
    } else if (statusFilter_ == "borrowed" && borrowedAction) {
        borrowedAction->setChecked(true);
    }

    connect(statusActionGroup_, &QActionGroup::triggered, this, [this](QAction *action) {
        statusFilter_ = action->data().toString();
        refreshTable();
    });

    // 馆藏地址筛选菜单
    locationFilterMenu_ = new QMenu(this);
    locationFilterMenu_->setMinimumWidth(200);
    locationActionGroup_ = new QActionGroup(locationFilterMenu_);
    locationActionGroup_->setExclusive(true);

    auto addLocationAction = [this](const QString &label, const QString &value, bool separator = false) {
        if (separator) {
            locationFilterMenu_->addSeparator();
            return static_cast<QAction *>(nullptr);
        }
        QAction *action = locationFilterMenu_->addAction(label);
        action->setCheckable(true);
        action->setData(value);
        locationActionGroup_->addAction(action);
        if (value == locationFilter_) {
            action->setChecked(true);
        }
        return action;
    };

    QAction *allLocationAction = addLocationAction(QStringLiteral("全部校区"), QString());
    if (locationFilter_.isEmpty() && allLocationAction) {
        allLocationAction->setChecked(true);
    }

    addLocationAction(QString(), QString(), true);
    addLocationAction(QStringLiteral("仙林图书馆"), QStringLiteral("仙林图书馆"));
    addLocationAction(QStringLiteral("三牌楼图书馆"), QStringLiteral("三牌楼图书馆"));

    connect(locationActionGroup_, &QActionGroup::triggered, this, [this](QAction *action) {
        locationFilter_ = action->data().toString();
        refreshTable();
    });

    // 排序菜单
    sortMenu_ = new QMenu(this);
    sortMenu_->setMinimumWidth(200);
    sortActionGroup_ = new QActionGroup(sortMenu_);
    sortActionGroup_->setExclusive(true);

    auto addSortAction = [this](const QString &label, const QString &value) {
        QAction *action = sortMenu_->addAction(label);
        action->setCheckable(true);
        action->setData(value);
        sortActionGroup_->addAction(action);
        if (value == currentSortType_) {
            action->setChecked(true);
        }
        return action;
    };

    QAction *defaultSortAction = addSortAction(QStringLiteral("默认排序"), QStringLiteral("default"));
    QAction *borrowCountSortAction = addSortAction(QStringLiteral("热门排序"), QStringLiteral("borrowCount"));

    if (currentSortType_.isEmpty() || currentSortType_ == "default") {
        if (defaultSortAction)
            defaultSortAction->setChecked(true);
    } else if (currentSortType_ == "borrowCount" && borrowCountSortAction) {
        borrowCountSortAction->setChecked(true);
    }

    connect(sortActionGroup_, &QActionGroup::triggered, this, &MainWindow::onSortChanged);

    applyTheme(isDarkMode_);
}

void MainWindow::updateHeaderLabels()
{
    if (!model_)
        return;

    QString categoryLabel = QStringLiteral("类别 ▼");
    if (!categoryFilter_.isEmpty()) {
        categoryLabel = QStringLiteral("类别 ▼\n%1").arg(categoryFilter_);
    }
    model_->setHeaderData(5, Qt::Horizontal, categoryLabel);

    QString locationLabel = QStringLiteral("馆藏地址 ▼");
    if (!locationFilter_.isEmpty()) {
        locationLabel = QStringLiteral("馆藏地址 ▼\n%1").arg(locationFilter_);
    }
    model_->setHeaderData(4, Qt::Horizontal, locationLabel);

    QString statusLabel = QStringLiteral("状态 ▼");
    if (statusFilter_ == "available") {
        statusLabel = QStringLiteral("状态 ▼\n可借");
    } else if (statusFilter_ == "borrowed") {
        statusLabel = QStringLiteral("状态 ▼\n已借出");
    }
    model_->setHeaderData(11, Qt::Horizontal, statusLabel);

    QString borrowCountLabel = QStringLiteral("借阅次数 ▼");
    if (currentSortType_ == "borrowCount") {
        borrowCountLabel = QStringLiteral("借阅次数 ▼\n热门排序");
    } else if (currentSortType_ == "default") {
        borrowCountLabel = QStringLiteral("借阅次数 ▼\n默认排序");
    }
    model_->setHeaderData(10, Qt::Horizontal, borrowCountLabel);
}

void MainWindow::onHeaderSectionClicked(int section)
{
    if (section == 4) {
        showFilterMenu(locationFilterMenu_, section);
    } else if (section == 5) {
        showFilterMenu(categoryFilterMenu_, section);
    } else if (section == 10) {
        showFilterMenu(sortMenu_, section);
    } else if (section == 11) {
        showFilterMenu(statusFilterMenu_, section);
    }
}

void MainWindow::showFilterMenu(QMenu *menu, int section)
{
    if (!menu || !tableView_)
        return;
    QHeaderView *header = tableView_->horizontalHeader();

    int x = header->sectionViewportPosition(section);
    int width = header->sectionSize(section);
    int height = header->height();

    QRect sectionRect(x, 0, width, height);
    QPoint globalPos = header->viewport()->mapToGlobal(sectionRect.bottomLeft());

    menu->popup(globalPos);
}

void MainWindow::displayBooks(const QVector<Book> &booksToShow)
{
    model_->removeRows(0, model_->rowCount());

    for (int row = 0; row < booksToShow.size(); ++row) {
        const Book &b = booksToShow[row];
        QList<QStandardItem *> rowItems;
        rowItems << new QStandardItem(b.indexId);
        rowItems << new QStandardItem(b.name);
        rowItems << new QStandardItem(b.author);
        rowItems << new QStandardItem(b.publisher);
        rowItems << new QStandardItem(b.location);
        rowItems << new QStandardItem(b.category);

        int totalCopies = library_.getTotalCopyCount(b.indexId);
        rowItems << new QStandardItem(QString::number(totalCopies));

        rowItems << new QStandardItem(QString::number(b.price, 'f', 2));
        rowItems << new QStandardItem(b.inDate.toString("yyyy-MM-dd"));

        // 归还日期：根据当前用户显示
        QString returnDateStr = "";
        if (!currentUsername_.isEmpty() && !isAdminMode_) {
            QVector<BookCopy> borrowedCopies = library_.getUserBorrowedCopies(currentUsername_);
            for (const BookCopy &copy : borrowedCopies) {
                if (copy.indexId == b.indexId) {
                    returnDateStr = copy.dueDate.toString("yyyy-MM-dd");
                    break;
                }
            }
        }
        rowItems << new QStandardItem(returnDateStr);

        rowItems << new QStandardItem(QString::number(b.borrowCount));

        int availableCopies = library_.getAvailableCopyCount(b.indexId);
        QString statusText = (availableCopies > 0) ? QStringLiteral("可借") : QStringLiteral("不可借");
        rowItems << new QStandardItem(statusText);

        model_->appendRow(rowItems);
    }

    updateStatusBar();
}

QJsonArray MainWindow::loadUsersJson() const
{
    QJsonArray array;
    if (usersFilePath_.isEmpty())
        return array;

    QFile file(usersFilePath_);
    if (!file.open(QIODevice::ReadOnly)) {
        return array;
    }
    const QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isArray()) {
        array = doc.array();
    }
    return array;
}

bool MainWindow::saveUsersJson(const QJsonArray &array) const
{
    if (usersFilePath_.isEmpty())
        return false;
    QFile file(usersFilePath_);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    QJsonDocument doc(array);
    file.write(doc.toJson());
    file.close();
    return true;
}

QStringList MainWindow::getCurrentUserAllowedCategories() const
{
    QStringList result;
    if (currentUsername_.isEmpty())
        return result;

    QJsonArray array = loadUsersJson();
    for (const QJsonValue &value : array) {
        if (!value.isObject())
            continue;
        QJsonObject obj = value.toObject();
        if (obj.value("username").toString() == currentUsername_) {
            QJsonArray cats = obj.value("allowedCategories").toArray();
            for (const QJsonValue &v : cats) {
                result << v.toString();
            }
            break;
        }
    }
    return result;
}

bool MainWindow::currentUserHasBorrowed(const QString &indexId) const
{
    if (currentUsername_.isEmpty())
        return false;

    QJsonArray array = loadUsersJson();
    for (const QJsonValue &value : array) {
        if (!value.isObject())
            continue;
        QJsonObject obj = value.toObject();
        if (obj.value("username").toString() != currentUsername_)
            continue;
        QJsonArray borrows = obj.value("borrows").toArray();
        for (const QJsonValue &bVal : borrows) {
            if (!bVal.isObject())
                continue;
            QJsonObject bObj = bVal.toObject();
            if (bObj.value("indexId").toString() == indexId &&
                !bObj.value("returned").toBool(false)) {
                return true;
            }
        }
        break;
    }
    return false;
}

void MainWindow::addBorrowRecordForCurrentUser(const Book &book, const QDate &borrowDate, const QDate &dueDate)
{
    if (currentUsername_.isEmpty())
        return;

    QJsonArray array = loadUsersJson();
    for (int i = 0; i < array.size(); ++i) {
        if (!array.at(i).isObject())
            continue;
        QJsonObject obj = array.at(i).toObject();
        if (obj.value("username").toString() != currentUsername_)
            continue;

        QJsonArray borrows = obj.value("borrows").toArray();
        QJsonObject rec;
        rec["indexId"] = book.indexId;
        rec["bookName"] = book.name;
        rec["borrowDate"] = borrowDate.toString(Qt::ISODate);
        rec["dueDate"] = dueDate.toString(Qt::ISODate);
        rec["returnDate"] = QString();
        rec["returned"] = false;
        borrows.append(rec);
        obj["borrows"] = borrows;
        array[i] = obj;
        break;
    }
    saveUsersJson(array);
}

void MainWindow::markBorrowRecordReturnedForCurrentUser(const QString &indexId, const QDate &returnDate)
{
    if (currentUsername_.isEmpty())
        return;

    QJsonArray array = loadUsersJson();
    for (int i = 0; i < array.size(); ++i) {
        if (!array.at(i).isObject())
            continue;
        QJsonObject obj = array.at(i).toObject();
        if (obj.value("username").toString() != currentUsername_)
            continue;

        QJsonArray borrows = obj.value("borrows").toArray();
        bool changed = false;
        for (int j = 0; j < borrows.size(); ++j) {
            if (!borrows.at(j).isObject())
                continue;
            QJsonObject bObj = borrows.at(j).toObject();
            if (bObj.value("indexId").toString() == indexId &&
                !bObj.value("returned").toBool(false)) {
                bObj["returned"] = true;
                bObj["returnDate"] = returnDate.toString(Qt::ISODate);
                borrows[j] = bObj;
                changed = true;
                break;
            }
        }
        if (changed) {
            obj["borrows"] = borrows;
            array[i] = obj;
            break;
        }
    }
    saveUsersJson(array);
}

QString MainWindow::borrowRecordsForCurrentUserText() const
{
    if (currentUsername_.isEmpty()) {
        return QStringLiteral("当前未登录学生用户。");
    }

    QJsonArray array = loadUsersJson();
    for (const QJsonValue &value : array) {
        if (!value.isObject())
            continue;
        QJsonObject obj = value.toObject();
        if (obj.value("username").toString() != currentUsername_)
            continue;

        QJsonArray borrows = obj.value("borrows").toArray();
        if (borrows.isEmpty()) {
            return QStringLiteral("你还没有任何借阅记录。");
        }

        QStringList lines;
        for (const QJsonValue &bVal : borrows) {
            if (!bVal.isObject())
                continue;
            QJsonObject bObj = bVal.toObject();
            const QString bookName = bObj.value("bookName").toString();
            const QString indexId = bObj.value("indexId").toString();
            const QString borrowDate = bObj.value("borrowDate").toString();
            const QString dueDate = bObj.value("dueDate").toString();
            const QString returnDate = bObj.value("returnDate").toString();
            const bool returned = bObj.value("returned").toBool(false);

            QString line = QStringLiteral("《%1》(索引:%2)\n  借出: %3 | 应还: %4")
                               .arg(bookName, indexId, borrowDate, dueDate);
            if (returned) {
                line += QStringLiteral(" | 实还: %1").arg(returnDate);
            } else {
                line += QStringLiteral(" | 状态: 未还");
            }
            lines << line;
        }
        return lines.join("\n\n");
    }

    return QStringLiteral("未找到当前用户的借阅记录。");
}

QString MainWindow::borrowHistoryForBookText(const QString &indexId) const
{
    if (indexId.isEmpty()) {
        return QStringLiteral("未选择图书。");
    }

    QJsonArray array = loadUsersJson();
    QStringList lines;

    for (const QJsonValue &value : array) {
        if (!value.isObject())
            continue;
        QJsonObject obj = value.toObject();
        const QString username = obj.value("username").toString();
        QJsonArray borrows = obj.value("borrows").toArray();

        for (const QJsonValue &bVal : borrows) {
            if (!bVal.isObject())
                continue;
            QJsonObject bObj = bVal.toObject();
            if (bObj.value("indexId").toString() != indexId)
                continue;

            const QString bookName = bObj.value("bookName").toString();
            const QString borrowDate = bObj.value("borrowDate").toString();
            const QString dueDate = bObj.value("dueDate").toString();
            const QString returnDate = bObj.value("returnDate").toString();
            const bool returned = bObj.value("returned").toBool(false);

            QString line = QStringLiteral("用户: %1\n《%2》(索引:%3)\n  借出: %4 | 应还: %5")
                               .arg(username, bookName, indexId, borrowDate, dueDate);
            if (returned) {
                line += QStringLiteral(" | 实还: %1").arg(returnDate);
            } else {
                line += QStringLiteral(" | 状态: 未还");
            }
            lines << line;
        }
    }

    if (lines.isEmpty()) {
        return QStringLiteral("该图书暂无任何借阅记录。");
    }
    return lines.join("\n\n");
}

void MainWindow::onShowMyBorrows()
{
    if (currentUsername_.isEmpty() || isAdminMode_) {
        QMessageBox::information(this, "提示", "请以学生账号登录后查看自己的借阅信息。");
        return;
    }
    QMessageBox::information(this, "我的借阅", borrowRecordsForCurrentUserText());
}

void MainWindow::onShowBookBorrowHistory()
{
    if (!isAdminMode_) {
        QMessageBox::warning(this, "权限不足", "只有管理员可以查看图书借阅记录。");
        return;
    }

    QModelIndexList selectedIndexes = tableView_->selectionModel()->selectedRows();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择一条图书记录。");
        return;
    }

    int row = selectedIndexes.first().row();
    QString indexId = model_->item(row, 0)->text();
    QString bookName = model_->item(row, 1)->text();

    QString text = borrowHistoryForBookText(indexId);
    QMessageBox::information(this,
                             QStringLiteral("《%1》的借阅记录").arg(bookName),
                             text);
}
