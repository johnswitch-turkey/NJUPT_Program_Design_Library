// mainwindow.cpp
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../utils/bookdisplay.h"
#include "../utils/librarymanager.h"

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
    // 1. 从数据管理器获取所有图书
    const QVector<Book> &books = library_.getAll();

    // 2. 清除模型中的旧数据
    model_->removeRows(0, model_->rowCount());

    // 3. 遍历图书数据，填充到模型中
    for (int row = 0; row < books.size(); ++row)
    {
        const Book &b = books[row];

        // 应用筛选条件
        if (!categoryFilter_.isEmpty() && b.category != categoryFilter_)
        {
            continue;
        }
        if (!locationFilter_.isEmpty() && b.location != locationFilter_)
        {
            continue;
        }
        if (statusFilter_ == "available" && !b.available)
        {
            continue;
        }
        if (statusFilter_ == "borrowed" && b.available)
        {
            continue;
        }

        QList<QStandardItem *> rowItems;
        rowItems << new QStandardItem(b.indexId);
        rowItems << new QStandardItem(b.name);
        rowItems << new QStandardItem(b.author);
        rowItems << new QStandardItem(b.publisher);
        rowItems << new QStandardItem(b.location);
        rowItems << new QStandardItem(b.category);
        rowItems << new QStandardItem(QString::number(b.quantity));
        rowItems << new QStandardItem(QString::number(b.price, 'f', 2));
        rowItems << new QStandardItem(b.inDate.toString("yyyy-MM-dd"));
        rowItems << new QStandardItem(b.returnDate.isValid() ? b.returnDate.toString("yyyy-MM-dd") : "");
        rowItems << new QStandardItem(QString::number(b.borrowCount));

        // 状态列：根据剩余数量/可借状态显示
        QString statusText = (b.available && b.quantity > 0)
                                 ? QStringLiteral("可借")
                                 : QStringLiteral("不可借");
        rowItems << new QStandardItem(statusText);

        model_->appendRow(rowItems);
    }

    // 4. 更新状态栏
    updateStatusBar();
    updateHeaderLabels();
}

// ============================================================================
// 核心业务逻辑槽函数
// ============================================================================
void MainWindow::onBorrow()
{
    if (currentUsername_.isEmpty() || isAdminMode_)
    {
        QMessageBox::warning(this, "借书失败", "只有学生用户可以借书，请使用学生账号登录。");
        return;
    }
    // 获取当前选中的行
    QModelIndexList selectedIndexes = tableView_->selectionModel()->selectedRows();
    if (selectedIndexes.isEmpty())
    {
        QMessageBox::information(this, "提示", "请先选择要借阅的图书！");
        return;
    }

    int row = selectedIndexes.first().row();
    QString indexId = model_->item(row, 0)->text();
    QString bookName = model_->item(row, 1)->text();

    // 检查该学生是否已借过此书
    if (currentUserHasBorrowed(indexId))
    {
        QMessageBox::warning(this, "借书失败", "你已经借过该图书，请先归还再借。");
        return;
    }

    // 检查图书是否可借
    const Book *bookPtr = library_.findByIndexId(indexId);
    if (!bookPtr || !bookPtr->available || bookPtr->quantity <= 0)
    {
        QMessageBox::warning(this, "借书失败", "该图书已无剩余可借数量！");
        return;
    }

    // 弹出输入框，让用户输入归还日期
    bool ok;
    QDate dueDate = QDate::fromString(
        QInputDialog::getText(this, "借书", "请输入归还日期 (yyyy-MM-dd):", QLineEdit::Normal, QDate::currentDate().addDays(30).toString("yyyy-MM-dd"), &ok),
        "yyyy-MM-dd");

    if (!ok || !dueDate.isValid())
    {
        return; // 用户取消或输入无效
    }

    // 更新图书数据（借出一本，数量减一）
    Book updatedBook = *bookPtr;
    if (updatedBook.quantity > 0)
    {
        updatedBook.quantity -= 1;
    }
    // 当数量减为 0 时，不可再借
    updatedBook.available = (updatedBook.quantity > 0);
    updatedBook.returnDate = dueDate;
    updatedBook.borrowCount++;

    QString error;
    if (library_.updateBook(indexId, updatedBook, &error))
    {
        // 记录到当前学生的借阅信息中
        addBorrowRecordForCurrentUser(updatedBook, QDate::currentDate(), dueDate);

        refreshTable();
        QMessageBox::information(this, "成功",
                                 QStringLiteral("成功借阅图书《%1》，归还日期为 %2").arg(bookName, dueDate.toString("yyyy-MM-dd")));
    }
    else
    {
        QMessageBox::warning(this, "失败", "借阅失败：" + error);
    }
}

void MainWindow::onReturn()
{
    if (currentUsername_.isEmpty() || isAdminMode_)
    {
        QMessageBox::warning(this, "还书失败", "只有学生用户可以还书，请使用学生账号登录。");
        return;
    }
    // 获取当前选中的行
    QModelIndexList selectedIndexes = tableView_->selectionModel()->selectedRows();
    if (selectedIndexes.isEmpty())
    {
        QMessageBox::information(this, "提示", "请先选择要归还的图书！");
        return;
    }

    int row = selectedIndexes.first().row();
    QString indexId = model_->item(row, 0)->text();
    QString bookName = model_->item(row, 1)->text();

    // 检查图书是否已借出
    const Book *bookPtr = library_.findByIndexId(indexId);
    if (!bookPtr)
    {
        QMessageBox::warning(this, "还书失败", "未找到该图书信息！");
        return;
    }

    if (!currentUserHasBorrowed(indexId))
    {
        QMessageBox::warning(this, "还书失败", "你当前没有借阅该图书，无法归还。");
        return;
    }

    // 确认归还
    auto reply = QMessageBox::question(this, "确认还书",
                                       QStringLiteral("确定要归还图书《%1》吗？").arg(bookName),
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No)
    {
        return;
    }

    // 更新图书数据（归还一本，数量加一）
    Book updatedBook = *bookPtr;
    updatedBook.quantity += 1;
    updatedBook.available = true;
    updatedBook.returnDate = QDate(); // 清空归还日期

    QString error;
    if (library_.updateBook(indexId, updatedBook, &error))
    {
        // 更新当前学生的借阅记录
        markBorrowRecordReturnedForCurrentUser(indexId, QDate::currentDate());

        refreshTable();
        QMessageBox::information(this, "成功",
                                 QStringLiteral("成功归还图书《%1》").arg(bookName));
    }
    else
    {
        QMessageBox::warning(this, "失败", "归还失败：" + error);
    }
}

void MainWindow::onWarn()
{
    isWarn = !isWarn; // 切换状态

    if (isWarn)
    {
        // --- 模式激活：显示即将到期的图书 ---

        // 1. 从数据管理器获取三天内到期的图书
        QVector<Book> dueSoonBooks = library_.getWarn(3);

        // 2. 使用辅助函数显示这些图书
        displayBooks(dueSoonBooks);

        // 3. (可选) 在状态栏显示提示信息
        if (dueSoonBooks.isEmpty())
        {
            statusBar()->showMessage("✅ 暂无即将到期的图书。", 5000);
        }
        else
        {
            QString message = QStringLiteral("⚠️ 找到 %1 本即将到期的图书。").arg(dueSoonBooks.size());
            statusBar()->showMessage(message, 5000);
        }

        // 4. (可选) 改变按钮样式，提供视觉反馈
        auto *warnButton = qobject_cast<QAction *>(sender());
        if (warnButton)
        {
            warnButton->setText(QStringLiteral("🔙 显示全部")); // 改变按钮文本
        }
    }
    else
    {
        // --- 模式取消：显示所有图书 ---

        // 1. 调用 onShowAll() 来刷新并显示所有图书
        onShowAll();

        // 2. (可选) 在状态栏显示提示信息
        statusBar()->showMessage("已显示所有图书", 3000);

        // 3. (可选) 恢复按钮原始文本
        auto *warnButton = qobject_cast<QAction *>(sender());
        if (warnButton)
        {
            warnButton->setText(QStringLiteral("⏰ 到期提醒")); // 恢复原始文本
        }
    }
}

void MainWindow::onAddBook()
{
    if (!isAdminMode_)
    {
        QMessageBox::warning(this, "权限不足", "只有管理员可以添加图书，请以管理员模式登录。");
        return;
    }
    showBookDialog(Book(), false);
}

void MainWindow::onEditBook()
{
    if (!isAdminMode_)
    {
        QMessageBox::warning(this, "权限不足", "只有管理员可以编辑图书，请以管理员模式登录。");
        return;
    }
    // 获取当前选中的行
    QModelIndexList selectedIndexes = tableView_->selectionModel()->selectedRows();
    if (selectedIndexes.isEmpty())
    {
        QMessageBox::information(this, "提示", "请先选择要编辑的图书！");
        return;
    }

    int row = selectedIndexes.first().row();
    QString indexId = model_->item(row, 0)->text();

    const Book *bookPtr = library_.findByIndexId(indexId);
    if (bookPtr)
    {
        showBookDialog(*bookPtr, true);
    }
}

void MainWindow::onDeleteBook()
{
    if (!isAdminMode_)
    {
        QMessageBox::warning(this, "权限不足", "只有管理员可以删除图书，请以管理员模式登录。");
        return;
    }
    // 获取当前选中的行
    QModelIndexList selectedIndexes = tableView_->selectionModel()->selectedRows();
    if (selectedIndexes.isEmpty())
    {
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
    if (reply == QMessageBox::No)
    {
        return;
    }

    if (library_.removeBookByIndexId(indexId))
    {
        refreshTable();
        QMessageBox::information(this, "成功", QStringLiteral("成功删除图书《%1》").arg(bookName));
    }
    else
    {
        QMessageBox::warning(this, "失败", "删除失败！");
    }
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
    // 按借阅次数排序（从高到低）
    currentSortType_ = "borrowCount";
    library_.sortByBorrowCount();
    refreshTable();
    updateHeaderLabels();
    statusBar()->showMessage("已按借阅次数排序（从高到低）", 3000);
}

void MainWindow::onSortDefault()
{
    // 恢复默认排序（按索引号排序）
    currentSortType_ = "default";
    library_.loadFromDatabase(); // 重新加载数据以恢复原始顺序
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

    if (sortType == "borrowCount")
    {
        library_.sortByBorrowCount();
    }
    else if (sortType == "default")
    {
        library_.loadFromDatabase(); // 重新加载数据以恢复原始顺序
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
    if (name.isEmpty())
    {
        onShowAll(); // 如果搜索框为空，显示所有
        return;
    }

    const Book *b = library_.findByName(name);
    if (b)
    {
        model_->removeRows(0, model_->rowCount());
        QList<QStandardItem *> rowItems;
        rowItems << new QStandardItem(b->indexId);
        rowItems << new QStandardItem(b->name);
        rowItems << new QStandardItem(b->author);
        rowItems << new QStandardItem(b->publisher);
        rowItems << new QStandardItem(b->location);
        rowItems << new QStandardItem(b->category);
        rowItems << new QStandardItem(QString::number(b->quantity));
        rowItems << new QStandardItem(QString::number(b->price, 'f', 2));
        rowItems << new QStandardItem(b->inDate.toString("yyyy-MM-dd"));
        rowItems << new QStandardItem(b->returnDate.isValid() ? b->returnDate.toString("yyyy-MM-dd") : "");
        rowItems << new QStandardItem(QString::number(b->borrowCount));
        QString statusText = (b->available && b->quantity > 0)
                                 ? QStringLiteral("可借")
                                 : QStringLiteral("不可借");
        rowItems << new QStandardItem(statusText);
        model_->appendRow(rowItems);
    }
    else
    {
        QMessageBox::information(this, "未找到", QStringLiteral("没有找到名称为 \"%1\" 的图书").arg(name));
    }
}

void MainWindow::onOpen()
{
    QString path = QFileDialog::getOpenFileName(this, "导入图书数据", "", "JSON Files (*.json)");
    if (!path.isEmpty())
    {
        if (library_.importFromJson(path))
        {
            rebuildFilterMenus();
            refreshTable();
            QMessageBox::information(this, "成功", "数据导入成功！");
        }
        else
        {
            QMessageBox::warning(this, "失败", "文件导入失败！");
        }
    }
}

void MainWindow::onSave()
{
    QString path = QFileDialog::getSaveFileName(this, "导出图书数据", "library_export.json", "JSON Files (*.json)");
    if (!path.isEmpty())
    {
        if (library_.exportToJson(path))
        {
            QMessageBox::information(this, "成功", "数据导出成功！");
        }
        else
        {
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
    if (action)
    {
        categoryFilter_ = action->data().toString();
        refreshTable();
    }
}

void MainWindow::onStatusFilterChanged(QAction *action)
{
    if (action)
    {
        statusFilter_ = action->data().toString();
        refreshTable();
    }
}

void MainWindow::onLocationFilterChanged(QAction *action)
{
    if (action)
    {
        locationFilter_ = action->data().toString();
        refreshTable();
    }
}

// ============================================================================
// UI设置和其他辅助函数
// ============================================================================
void MainWindow::setupActions()
{
    // 1. 创建一个纯粹的 QToolBar，不使用 addToolBar()
    actionToolBar_ = new QToolBar(QStringLiteral("操作"), this);

    // 2. 设置其自身属性
    actionToolBar_->setMovable(false);
    actionToolBar_->setFloatable(false);
    actionToolBar_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    actionToolBar_->setOrientation(Qt::Vertical);

    // 3. 将工具栏放入一个容器，再把容器设置为滚动区的 widget（关键改动）
    toolBarScrollArea_ = new QScrollArea();
    // 不让 scroll area 自动调整内部 widget 大小，保证当内容超出时出现竖向滚动条
    toolBarScrollArea_->setWidgetResizable(false);
    toolBarScrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    toolBarScrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // 容器，包含工具栏和一个伸展，保证工具栏靠上，底部出现可滚动空间
    QWidget *toolContainer = new QWidget();
    QVBoxLayout *toolContainerLayout = new QVBoxLayout(toolContainer);
    toolContainerLayout->setContentsMargins(6, 6, 6, 6);
    toolContainerLayout->setSpacing(8);
    toolContainerLayout->addWidget(actionToolBar_);
    toolContainerLayout->addStretch();

    // 不使用滑条，改为依赖 QScrollArea 的竖向滚动条在内容超出时出现。

    toolContainer->setFixedWidth(150);
    toolContainer->setMinimumHeight(800);
    toolBarScrollArea_->setWidget(toolContainer);
    // 固定功能栏宽度，使用竖向滚动条进行上下翻页
    toolBarScrollArea_->setFixedWidth(150);
    toolBarScrollArea_->setMaximumHeight(QWIDGETSIZE_MAX);

    toolContainer->setStyleSheet(QString("background-color: %1;").arg(isDarkMode_ ? "#22333B" : "#FEEFF1"));

    // 4. 创建 DockWidget 并将包含工具栏的 ScrollArea 放入其中
    toolBarDockWidget_ = createDockWidgetFromScrollArea(toolBarScrollArea_);

    // 5. 将整个 DockWidget 添加到主窗口
    addDockWidget(Qt::LeftDockWidgetArea, toolBarDockWidget_);

    // --- 创建所有按钮（只执行一次） ---
    borrowAct_ = actionToolBar_->addAction(QStringLiteral("📖 借书"));
    returnAct_ = actionToolBar_->addAction(QStringLiteral("📤 还书"));
    warnAct_ = actionToolBar_->addAction(QStringLiteral("⏰ 到期提醒"));
    myBorrowAct_ = actionToolBar_->addAction(QStringLiteral("📚 我的借阅"));
    allAct_ = actionToolBar_->addAction(QStringLiteral("📋 显示全部"));
    actionToolBar_->addSeparator();
    addBookAct_ = actionToolBar_->addAction(QStringLiteral("➕ 添加图书"));
    editBookAct_ = actionToolBar_->addAction(QStringLiteral("✏️ 编辑图书"));
    deleteBookAct_ = actionToolBar_->addAction(QStringLiteral("🗑️ 删除图书"));
    bookHistoryAct_ = actionToolBar_->addAction(QStringLiteral("📑 借阅记录"));
    actionToolBar_->addSeparator();
    importBookAct_ = actionToolBar_->addAction(QStringLiteral("📥 导入图书数据"));
    exportBookAct_ = actionToolBar_->addAction(QStringLiteral("📤 导出图书数据"));
    importUsersAct_ = actionToolBar_->addAction(QStringLiteral("📥 导入学生数据"));
    exportUsersAct_ = actionToolBar_->addAction(QStringLiteral("📤 导出学生数据"));
    actionToolBar_->addSeparator();
    toggleOrientationAct_ = actionToolBar_->addAction(QStringLiteral("🔄 切换布局"));

    // --- 连接信号（只执行一次） ---
    connect(borrowAct_, &QAction::triggered, this, &MainWindow::onBorrow);
    connect(returnAct_, &QAction::triggered, this, &MainWindow::onReturn);
    connect(warnAct_, &QAction::triggered, this, &MainWindow::onWarn);
    connect(myBorrowAct_, &QAction::triggered, this, &MainWindow::onShowMyBorrows);
    connect(allAct_, &QAction::triggered, this, &MainWindow::onShowAll);

    connect(addBookAct_, &QAction::triggered, this, &MainWindow::onAddBook);
    connect(editBookAct_, &QAction::triggered, this, &MainWindow::onEditBook);
    connect(deleteBookAct_, &QAction::triggered, this, &MainWindow::onDeleteBook);
    connect(bookHistoryAct_, &QAction::triggered, this, &MainWindow::onShowBookBorrowHistory);
    connect(importBookAct_, &QAction::triggered, this, &MainWindow::onImport);
    connect(exportBookAct_, &QAction::triggered, this, &MainWindow::onExport);
    connect(importUsersAct_, &QAction::triggered, this, &MainWindow::onImportUsers);
    connect(exportUsersAct_, &QAction::triggered, this, &MainWindow::onExportUsers);
    connect(toggleOrientationAct_, &QAction::triggered, this, &MainWindow::toggleToolBarOrientation);

    // --- 初始状态：在所有按钮创建完毕后，统一设置可见性 ---
    updateActionsVisibility();
}

void MainWindow::setupMenuBar()
{
    // 菜单栏可以暂时留空，或者添加一些与工具栏重复的功能
    // ...
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

    // --- 修改：使用 addToolBarBreak 确保它在最顶部 ---
    // addToolBarBreak 会在指定区域插入一个分隔符，使得后续的 addToolBar
    // 调用会创建一个新的工具栏行。这能确保搜索栏始终显示在所有其他顶部控件之上。
    addToolBarBreak(Qt::TopToolBarArea);

    QToolBar *searchToolBar = addToolBar("搜索");
    searchToolBar->setMovable(false);
    searchToolBar->setFloatable(false);
    searchToolBar->addWidget(searchWidget);
    // 限制搜索栏只能在顶部，避免用户意外拖动。
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
    if (themeToggleButton_)
    {
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
    int total = library_.getTotalBooks();
    int available = library_.getAvailableBooks();
    int borrowed = library_.getBorrowedBooks();

    QString statusText = QStringLiteral("📊 总计: %1 | ✅ 可借: %2 | ❌ 已借: %3")
                             .arg(total)
                             .arg(available)
                             .arg(borrowed);
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

    if (isAdminMode_)
    {
        setWindowTitle(QStringLiteral("图书管理系统 - 管理员模式 (%1)").arg(username));
    }
    else
    {
        setWindowTitle(QStringLiteral("图书管理系统 - 学生模式 (%1)").arg(username));
    }

    // 根据角色更新按钮显示
    updateActionsVisibility();

    // 重新刷新表格，使不同学生看到不同的可借信息
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
        warnAct_->setVisible(isStudent); // 所有用户都可以看到到期提醒
    if (myBorrowAct_)
        myBorrowAct_->setVisible(isStudent);
    if (allAct_)
        allAct_->setVisible(isStudent); // 所有用户都可以显示全部

    // 管理员可见的按钮
    if (addBookAct_)
        addBookAct_->setVisible(isAdminMode_);
    if (editBookAct_)
        editBookAct_->setVisible(isAdminMode_);
    if (deleteBookAct_)
        deleteBookAct_->setVisible(isAdminMode_);
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

/**
 * @brief 切换工具栏的方向（竖向/横向）
 *
 * 此函数通过改变 QDockWidget 的停靠区域来实现工具栏方向的切换。
 * 核心是精确控制 QToolBar 和 QScrollArea 的尺寸策略，以确保在内容超出可视范围时，
 * 滚动条能正确显示，并且在横向模式下能正确停靠在顶部。
 */
void MainWindow::toggleToolBarOrientation()
{
    // 安全检查：确保要操作的控件指针有效，防止程序崩溃
    if (!actionToolBar_ || !toolBarDockWidget_ || !toolBarScrollArea_)
    {
        return;
    }

    // 1. 切换内部状态标志
    isToolBarVertical_ = !isToolBarVertical_;

    // 2. 在修改停靠属性前，必须先将 QDockWidget 从主窗口中移除
    removeDockWidget(toolBarDockWidget_);

    if (isToolBarVertical_)
    {
        // ========================================================================
        // 分支：切换到竖向布局（停靠在左侧）
        // ========================================================================

        // --- 步骤1：设置 QToolBar 本身的方向和尺寸策略 ---
        actionToolBar_->setOrientation(Qt::Vertical);
        actionToolBar_->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        // 关键：设置最小宽度而不是固定宽度，让工具栏根据内容自动调整
        actionToolBar_->setMinimumWidth(110);
        actionToolBar_->setMinimumHeight(0); // 高度不设限，允许增长
        // 新增：移除最大高度限制
        actionToolBar_->setMaximumHeight(QWIDGETSIZE_MAX);

        // --- 步骤2：设置 QScrollArea 的滚动条和尺寸策略 ---
        // 关键：设置 QScrollArea 不自动调整其内部控件的大小
        toolBarScrollArea_->setWidgetResizable(false);
        toolBarScrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        toolBarScrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        toolBarScrollArea_->setFixedWidth(130);
        toolBarScrollArea_->setMinimumHeight(100);
        // 新增：确保滚动区域没有高度限制
        toolBarScrollArea_->setMaximumHeight(QWIDGETSIZE_MAX);
        // 竖向模式下固定停靠窗口宽度
        toolBarDockWidget_->setFixedWidth(130);

        // --- 步骤3：设置 QDockWidget 的停靠区域并重新添加 ---
        // 修正：使用 DockWidgetArea 枚举
        toolBarDockWidget_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        addDockWidget(Qt::LeftDockWidgetArea, toolBarDockWidget_);
    }
    else
    {
        // ========================================================================
        // 分支：切换到横向布局（停靠在顶部）
        // ========================================================================

        // --- 步骤1：设置 QToolBar 本身的方向和尺寸策略 ---
        actionToolBar_->setOrientation(Qt::Horizontal);
        actionToolBar_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        actionToolBar_->setFixedHeight(45);
        actionToolBar_->setMinimumWidth(0);
        // 新增：移除宽度限制
        actionToolBar_->setMaximumWidth(QWIDGETSIZE_MAX);

        // --- 步骤2：设置 QScrollArea 的滚动条和尺寸策略 ---
        toolBarScrollArea_->setWidgetResizable(false);
        toolBarScrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        toolBarScrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        toolBarScrollArea_->setMinimumHeight(50);
        toolBarScrollArea_->setMaximumHeight(50);
        toolBarScrollArea_->setMinimumWidth(200);
        // 新增：确保滚动区域没有宽度限制
        toolBarScrollArea_->setMaximumWidth(QWIDGETSIZE_MAX);
        // 取消竖向模式的固定宽度限制，使横向工具栏能扩展
        toolBarDockWidget_->setMinimumWidth(0);
        toolBarDockWidget_->setMaximumWidth(QWIDGETSIZE_MAX);
        toolBarScrollArea_->setMinimumWidth(200);

        // --- 步骤3：设置 QDockWidget 的停靠区域并重新添加 ---
        // 修正：使用 DockWidgetArea 枚举，TopToolBarArea 改为 TopDockWidgetArea
        toolBarDockWidget_->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
        addDockWidget(Qt::TopDockWidgetArea, toolBarDockWidget_);
    }

    // 3. 在状态栏显示提示信息
    statusBar()->showMessage(
        isToolBarVertical_ ? "已切换到竖向布局（左边）" : "已切换到横向布局（顶部）",
        2000);
}

void MainWindow::onImportUsers()
{
    if (!isAdminMode_)
    {
        QMessageBox::warning(this, "权限不足", "只有管理员可以导入学生数据。");
        return;
    }

    QString path = QFileDialog::getOpenFileName(this, "导入学生数据", "", "JSON Files (*.json)");
    if (!path.isEmpty())
    {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::warning(this, "失败", "无法打开文件！");
            return;
        }

        QByteArray data = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull() || !doc.isArray())
        {
            QMessageBox::warning(this, "失败", "文件格式错误！");
            return;
        }

        QJsonArray importedArray = doc.array();
        QJsonArray currentArray = loadUsersJson();

        // 合并数据（避免重复用户名）
        QSet<QString> existingUsernames;
        for (const QJsonValue &value : currentArray)
        {
            if (value.isObject())
            {
                existingUsernames.insert(value.toObject().value("username").toString());
            }
        }

        int addedCount = 0;
        for (const QJsonValue &value : importedArray)
        {
            if (!value.isObject())
                continue;
            QJsonObject userObj = value.toObject();
            QString username = userObj.value("username").toString();
            if (!existingUsernames.contains(username))
            {
                currentArray.append(userObj);
                existingUsernames.insert(username);
                addedCount++;
            }
        }

        if (saveUsersJson(currentArray))
        {
            QMessageBox::information(this, "成功",
                                     QStringLiteral("成功导入 %1 条学生数据！").arg(addedCount));
        }
        else
        {
            QMessageBox::warning(this, "失败", "保存学生数据失败！");
        }
    }
}

void MainWindow::onExportUsers()
{
    if (!isAdminMode_)
    {
        QMessageBox::warning(this, "权限不足", "只有管理员可以导出学生数据。");
        return;
    }

    QString path = QFileDialog::getSaveFileName(this, "导出学生数据", "users_export.json", "JSON Files (*.json)");
    if (!path.isEmpty())
    {
        QJsonArray usersArray = loadUsersJson();
        QJsonDocument doc(usersArray);

        QFile file(path);
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(doc.toJson());
            file.close();
            QMessageBox::information(this, "成功", "学生数据导出成功！");
        }
        else
        {
            QMessageBox::warning(this, "失败", "无法保存文件！");
        }
    }
}

/**
 * @brief 创建一个包含 QScrollArea 的 QDockWidget
 *
 * 此函数封装了创建停靠窗口的通用设置，使其可以被拖动、浮动和关闭。
 * 最重要的是，它正确设置了允许停靠的区域。
 *
 * @param scrollArea 指向要被嵌入的 QScrollArea 对象的指针
 * @return 返回一个配置好的 QDockWidget 指针
 */
QDockWidget *MainWindow::createDockWidgetFromScrollArea(QScrollArea *scrollArea)
{
    // 创建一个新的 QDockWidget 实例，并设置标题
    QDockWidget *dockWidget = new QDockWidget("功能栏", this);

    // 将传入的 scrollArea 设置为 dockWidget 的核心控件
    dockWidget->setWidget(scrollArea);

    // 配置 QDockWidget 的特性：
    // DockWidgetMovable: 允许用户拖动停靠窗口到不同的停靠区域。
    // DockWidgetFloatable: 允许用户将停靠窗口从主窗口中分离出来，成为一个独立的浮动窗口。
    // DockWidgetClosable: 允许用户关闭停靠窗口。
    dockWidget->setFeatures(QDockWidget::DockWidgetMovable |
                            QDockWidget::DockWidgetFloatable |
                            QDockWidget::DockWidgetClosable);

    // 设置 QDockWidget 允许被停靠的区域（关键修正点）
    // 原始代码错误地混合了 DockWidgetArea 和 ToolBarArea。
    // QDockWidget 的 setAllowedAreas 函数只接受 Qt::DockWidgetArea 枚举类型的组合。
    // 这里修正为只使用 DockWidgetArea，允许停靠在所有四个边缘。
    dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea |
                                Qt::RightDockWidgetArea |
                                Qt::TopDockWidgetArea |
                                Qt::BottomDockWidgetArea);

    // 返回创建并配置好的 QDockWidget 对象
    return dockWidget;
}

void MainWindow::applyTheme(bool isDark)
{
    QString styles = getThemeStyles(isDark);
    setStyleSheet(styles);

    // 同时应用菜单样式
    QString menuStyles = getMenuStyles(isDark);
    if (categoryFilterMenu_)
    {
        categoryFilterMenu_->setStyleSheet(menuStyles);
    }
    if (statusFilterMenu_)
    {
        statusFilterMenu_->setStyleSheet(menuStyles);
    }
    if (locationFilterMenu_)
    {
        locationFilterMenu_->setStyleSheet(menuStyles);
    }
    if (sortMenu_)
    {
        sortMenu_->setStyleSheet(menuStyles);
    }
}

void MainWindow::showBookDialog(const Book &book, bool isEdit)
{
    BookDialog dialog(this);
    if (isEdit)
    {
        dialog.setBook(book);
        dialog.setWindowTitle("编辑图书信息");
    }
    else
    {
        dialog.setWindowTitle("添加新图书");
    }

    if (dialog.exec() == QDialog::Accepted)
    {
        Book newBook = dialog.getBook();

        // 验证必填字段
        if (newBook.indexId.isEmpty() || newBook.name.isEmpty())
        {
            QMessageBox::warning(this, "错误", "索引号和名称不能为空！");
            return;
        }

        QString error;
        bool success;

        if (isEdit)
        {
            success = library_.updateBook(book.indexId, newBook, &error);
        }
        else
        {
            success = library_.addBook(newBook, &error);
        }

        if (success)
        {
            refreshTable();
            QMessageBox::information(this, "成功",
                                     isEdit ? "图书信息更新成功！" : "图书添加成功！");
        }
        else
        {
            QMessageBox::warning(this, "失败", error);
        }
    }
}

// 新增菜单样式函数
QString MainWindow::getMenuStyles(bool isDark)
{
    if (isDark)
    {
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
    }
    else
    {
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
    if (isDark)
    {
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
            // 保持竖向滚动条不变，只修改横向滚动条
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
            // 修改横向滚动条样式
            "QScrollBar:horizontal {"
            "    background-color: #22333B;" // 与背景色一致
            "    height: 12px;"              // 稍微加高
            "    border-radius: 6px;"        // 圆角
            "    margin: 2px;"
            "}"
            "QScrollBar::handle:horizontal {"
            "    background-color: #52B788;" // 薄荷绿主色调
            "    border-radius: 6px;"
            "    min-width: 30px;" // 最小宽度
            "    margin: 2px;"
            "}"
            "QScrollBar::handle:horizontal:hover {"
            "    background-color: #74C69D;" // 悬停时更亮的绿色
            "}"
            "QScrollBar::handle:horizontal:pressed {"
            "    background-color: #40916C;" // 按下时更深的绿色
            "}"
            "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {"
            "    width: 0px;" // 隐藏左右箭头
            "    background: none;"
            "}"
            "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {"
            "    background: none;" // 透明背景
            "}");
    }
    else
    {
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
            // 保持竖向滚动条不变，只修改横向滚动条
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
            // 修改横向滚动条样式
            "QScrollBar:horizontal {"
            "    background-color: #FEEFF1;" // 与背景色一致
            "    height: 12px;"              // 稍微加高
            "    border-radius: 6px;"        // 圆角
            "    margin: 2px;"
            "}"
            "QScrollBar::handle:horizontal {"
            "    background-color: #F9A8D4;" // 粉色主色调
            "    border-radius: 6px;"
            "    min-width: 30px;" // 最小宽度
            "    margin: 2px;"
            "}"
            "QScrollBar::handle:horizontal:hover {"
            "    background-color: #F7B2D7;" // 悬停时更亮的粉色
            "}"
            "QScrollBar::handle:horizontal:pressed {"
            "    background-color: #E11D48;" // 按下时更深的粉色
            "}"
            "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {"
            "    width: 0px;" // 隐藏左右箭头
            "    background: none;"
            "}"
            "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {"
            "    background: none;" // 透明背景
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

    auto addCategoryAction = [this](const QString &label, const QString &value, bool separator = false)
    {
        if (separator)
        {
            categoryFilterMenu_->addSeparator();
            return static_cast<QAction *>(nullptr);
        }
        QAction *action = categoryFilterMenu_->addAction(label);
        action->setCheckable(true);
        action->setData(value);
        categoryActionGroup_->addAction(action);
        if (value == categoryFilter_)
        {
            action->setChecked(true);
        }
        return action;
    };

    QAction *allCategoryAction = addCategoryAction(QStringLiteral("全部类别"), QString());
    if (categoryFilter_.isEmpty() && allCategoryAction)
    {
        allCategoryAction->setChecked(true);
    }

    QSet<QString> categorySet;
    for (const Book &book : library_.getAll())
    {
        if (!book.category.isEmpty())
        {
            categorySet.insert(book.category);
        }
    }
    if (!categoryFilter_.isEmpty() && !categorySet.contains(categoryFilter_))
    {
        categoryFilter_.clear();
    }
    QList<QString> categories = QList<QString>(categorySet.begin(), categorySet.end());
    std::sort(categories.begin(), categories.end(), [](const QString &a, const QString &b)
              { return a.localeAwareCompare(b) < 0; });

    if (!categories.isEmpty())
    {
        addCategoryAction(QString(), QString(), true);
    }
    for (const QString &category : categories)
    {
        addCategoryAction(category, category);
    }

    connect(categoryActionGroup_, &QActionGroup::triggered, this, [this](QAction *action)
            {
        categoryFilter_ = action->data().toString();
        refreshTable(); });

    // 状态筛选菜单
    statusFilterMenu_ = new QMenu(this);
    statusFilterMenu_->setMinimumWidth(200);
    statusActionGroup_ = new QActionGroup(statusFilterMenu_);
    statusActionGroup_->setExclusive(true);

    auto addStatusAction = [this](const QString &label, const QString &value)
    {
        QAction *action = statusFilterMenu_->addAction(label);
        action->setCheckable(true);
        action->setData(value);
        statusActionGroup_->addAction(action);
        if (statusFilter_ == value)
        {
            action->setChecked(true);
        }
        return action;
    };

    QAction *allStatusAction = addStatusAction(QStringLiteral("全部状态"), QString());
    QAction *availableAction = addStatusAction(QStringLiteral("仅可借"), QStringLiteral("available"));
    QAction *borrowedAction = addStatusAction(QStringLiteral("仅已借出"), QStringLiteral("borrowed"));

    if (statusFilter_.isEmpty() && allStatusAction)
    {
        allStatusAction->setChecked(true);
    }
    else if (statusFilter_ == "available" && availableAction)
    {
        availableAction->setChecked(true);
    }
    else if (statusFilter_ == "borrowed" && borrowedAction)
    {
        borrowedAction->setChecked(true);
    }

    connect(statusActionGroup_, &QActionGroup::triggered, this, [this](QAction *action)
            {
        statusFilter_ = action->data().toString();
        refreshTable(); });

    // 馆藏地址筛选菜单
    locationFilterMenu_ = new QMenu(this);
    locationFilterMenu_->setMinimumWidth(200);
    locationActionGroup_ = new QActionGroup(locationFilterMenu_);
    locationActionGroup_->setExclusive(true);

    auto addLocationAction = [this](const QString &label, const QString &value, bool separator = false)
    {
        if (separator)
        {
            locationFilterMenu_->addSeparator();
            return static_cast<QAction *>(nullptr);
        }
        QAction *action = locationFilterMenu_->addAction(label);
        action->setCheckable(true);
        action->setData(value);
        locationActionGroup_->addAction(action);
        if (value == locationFilter_)
        {
            action->setChecked(true);
        }
        return action;
    };

    QAction *allLocationAction = addLocationAction(QStringLiteral("全部校区"), QString());
    if (locationFilter_.isEmpty() && allLocationAction)
    {
        allLocationAction->setChecked(true);
    }

    // 添加两个固定校区选项
    addLocationAction(QString(), QString(), true);
    addLocationAction(QStringLiteral("仙林图书馆"), QStringLiteral("仙林图书馆"));
    addLocationAction(QStringLiteral("三牌楼图书馆"), QStringLiteral("三牌楼图书馆"));

    connect(locationActionGroup_, &QActionGroup::triggered, this, [this](QAction *action)
            {
        locationFilter_ = action->data().toString();
        refreshTable(); });

    // 排序菜单
    sortMenu_ = new QMenu(this);
    sortMenu_->setMinimumWidth(200);
    sortActionGroup_ = new QActionGroup(sortMenu_);
    sortActionGroup_->setExclusive(true);

    auto addSortAction = [this](const QString &label, const QString &value)
    {
        QAction *action = sortMenu_->addAction(label);
        action->setCheckable(true);
        action->setData(value);
        sortActionGroup_->addAction(action);
        if (value == currentSortType_)
        {
            action->setChecked(true);
        }
        return action;
    };

    QAction *defaultSortAction = addSortAction(QStringLiteral("默认排序"), QStringLiteral("default"));
    QAction *borrowCountSortAction = addSortAction(QStringLiteral("热门排序"), QStringLiteral("borrowCount"));

    if (currentSortType_.isEmpty() || currentSortType_ == "default")
    {
        if (defaultSortAction)
            defaultSortAction->setChecked(true);
    }
    else if (currentSortType_ == "borrowCount" && borrowCountSortAction)
    {
        borrowCountSortAction->setChecked(true);
    }

    connect(sortActionGroup_, &QActionGroup::triggered, this, &MainWindow::onSortChanged);

    // 应用菜单样式
    applyTheme(isDarkMode_);
}

void MainWindow::updateHeaderLabels()
{
    if (!model_)
        return;

    // 修改：使用换行显示筛选信息，并添加倒三角符号
    QString categoryLabel = QStringLiteral("类别 ▼");
    if (!categoryFilter_.isEmpty())
    {
        categoryLabel = QStringLiteral("类别 ▼\n%1").arg(categoryFilter_);
    }
    model_->setHeaderData(5, Qt::Horizontal, categoryLabel);

    // 新增：馆藏地址表头（列索引调整）
    QString locationLabel = QStringLiteral("馆藏地址 ▼");
    if (!locationFilter_.isEmpty())
    {
        locationLabel = QStringLiteral("馆藏地址 ▼\n%1").arg(locationFilter_);
    }
    model_->setHeaderData(4, Qt::Horizontal, locationLabel);

    QString statusLabel = QStringLiteral("状态 ▼");
    if (statusFilter_ == "available")
    {
        statusLabel = QStringLiteral("状态 ▼\n可借");
    }
    else if (statusFilter_ == "borrowed")
    {
        statusLabel = QStringLiteral("状态 ▼\n已借出");
    }
    model_->setHeaderData(11, Qt::Horizontal, statusLabel);

    // 借阅次数列显示排序信息
    QString borrowCountLabel = QStringLiteral("借阅次数 ▼");
    if (currentSortType_ == "borrowCount")
    {
        borrowCountLabel = QStringLiteral("借阅次数 ▼\n热门排序");
    }
    else if (currentSortType_ == "default")
    {
        borrowCountLabel = QStringLiteral("借阅次数 ▼\n默认排序");
    }
    model_->setHeaderData(10, Qt::Horizontal, borrowCountLabel);
}

void MainWindow::onHeaderSectionClicked(int section)
{
    if (section == 4)
    { // 馆藏地址列
        showFilterMenu(locationFilterMenu_, section);
    }
    else if (section == 5)
    { // 类别列
        showFilterMenu(categoryFilterMenu_, section);
    }
    else if (section == 10)
    { // 借阅次数列
        showFilterMenu(sortMenu_, section);
    }
    else if (section == 11)
    { // 状态列
        showFilterMenu(statusFilterMenu_, section);
    }
}

void MainWindow::showFilterMenu(QMenu *menu, int section)
{
    if (!menu || !tableView_)
        return;
    QHeaderView *header = tableView_->horizontalHeader();

    // 获取表头部分在视口中的位置和大小
    int x = header->sectionViewportPosition(section);
    int width = header->sectionSize(section);
    int height = header->height();

    // 构建一个准确的矩形
    QRect sectionRect(x, 0, width, height);

    // 将视口坐标映射到全局屏幕坐标
    QPoint globalPos = header->viewport()->mapToGlobal(sectionRect.bottomLeft());

    // 弹出菜单
    menu->popup(globalPos);
}

void MainWindow::displayBooks(const QVector<Book> &booksToShow)
{
    // 1. 清除模型中的旧数据
    model_->removeRows(0, model_->rowCount());

    // 2. 遍历传入的图书列表，填充到模型中
    for (int row = 0; row < booksToShow.size(); ++row)
    {
        const Book &b = booksToShow[row];
        QList<QStandardItem *> rowItems;
        rowItems << new QStandardItem(b.indexId);
        rowItems << new QStandardItem(b.name);
        rowItems << new QStandardItem(b.author);
        rowItems << new QStandardItem(b.publisher);
        rowItems << new QStandardItem(b.location);
        rowItems << new QStandardItem(b.category);
        rowItems << new QStandardItem(QString::number(b.quantity));
        rowItems << new QStandardItem(QString::number(b.price, 'f', 2));
        rowItems << new QStandardItem(b.inDate.toString("yyyy-MM-dd"));
        rowItems << new QStandardItem(b.returnDate.isValid() ? b.returnDate.toString("yyyy-MM-dd") : "");
        rowItems << new QStandardItem(QString::number(b.borrowCount));

        QString statusText = (b.available && b.quantity > 0)
                                 ? QStringLiteral("可借")
                                 : QStringLiteral("不可借");
        rowItems << new QStandardItem(statusText);

        model_->appendRow(rowItems);
    }

    // 3. 更新状态栏
    updateStatusBar();
}

// ============================================================================
// 用户与借阅信息相关辅助函数实现
// ============================================================================

QJsonArray MainWindow::loadUsersJson() const
{
    QJsonArray array;
    if (usersFilePath_.isEmpty())
        return array;

    QFile file(usersFilePath_);
    if (!file.open(QIODevice::ReadOnly))
    {
        return array;
    }
    const QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isArray())
    {
        array = doc.array();
    }
    return array;
}

bool MainWindow::saveUsersJson(const QJsonArray &array) const
{
    if (usersFilePath_.isEmpty())
        return false;
    QFile file(usersFilePath_);
    if (!file.open(QIODevice::WriteOnly))
    {
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
    for (const QJsonValue &value : array)
    {
        if (!value.isObject())
            continue;
        QJsonObject obj = value.toObject();
        if (obj.value("username").toString() == currentUsername_)
        {
            QJsonArray cats = obj.value("allowedCategories").toArray();
            for (const QJsonValue &v : cats)
            {
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
    for (const QJsonValue &value : array)
    {
        if (!value.isObject())
            continue;
        QJsonObject obj = value.toObject();
        if (obj.value("username").toString() != currentUsername_)
            continue;
        QJsonArray borrows = obj.value("borrows").toArray();
        for (const QJsonValue &bVal : borrows)
        {
            if (!bVal.isObject())
                continue;
            QJsonObject bObj = bVal.toObject();
            if (bObj.value("indexId").toString() == indexId &&
                !bObj.value("returned").toBool(false))
            {
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
    for (int i = 0; i < array.size(); ++i)
    {
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
    for (int i = 0; i < array.size(); ++i)
    {
        if (!array.at(i).isObject())
            continue;
        QJsonObject obj = array.at(i).toObject();
        if (obj.value("username").toString() != currentUsername_)
            continue;

        QJsonArray borrows = obj.value("borrows").toArray();
        bool changed = false;
        for (int j = 0; j < borrows.size(); ++j)
        {
            if (!borrows.at(j).isObject())
                continue;
            QJsonObject bObj = borrows.at(j).toObject();
            if (bObj.value("indexId").toString() == indexId &&
                !bObj.value("returned").toBool(false))
            {
                bObj["returned"] = true;
                bObj["returnDate"] = returnDate.toString(Qt::ISODate);
                borrows[j] = bObj;
                changed = true;
                break;
            }
        }
        if (changed)
        {
            obj["borrows"] = borrows;
            array[i] = obj;
            break;
        }
    }
    saveUsersJson(array);
}

QString MainWindow::borrowRecordsForCurrentUserText() const
{
    if (currentUsername_.isEmpty())
    {
        return QStringLiteral("当前未登录学生用户。");
    }

    QJsonArray array = loadUsersJson();
    for (const QJsonValue &value : array)
    {
        if (!value.isObject())
            continue;
        QJsonObject obj = value.toObject();
        if (obj.value("username").toString() != currentUsername_)
            continue;

        QJsonArray borrows = obj.value("borrows").toArray();
        if (borrows.isEmpty())
        {
            return QStringLiteral("你还没有任何借阅记录。");
        }

        QStringList lines;
        for (const QJsonValue &bVal : borrows)
        {
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
            if (returned)
            {
                line += QStringLiteral(" | 实还: %1").arg(returnDate);
            }
            else
            {
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
    if (indexId.isEmpty())
    {
        return QStringLiteral("未选择图书。");
    }

    QJsonArray array = loadUsersJson();
    QStringList lines;

    for (const QJsonValue &value : array)
    {
        if (!value.isObject())
            continue;
        QJsonObject obj = value.toObject();
        const QString username = obj.value("username").toString();
        QJsonArray borrows = obj.value("borrows").toArray();

        for (const QJsonValue &bVal : borrows)
        {
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
            if (returned)
            {
                line += QStringLiteral(" | 实还: %1").arg(returnDate);
            }
            else
            {
                line += QStringLiteral(" | 状态: 未还");
            }
            lines << line;
        }
    }

    if (lines.isEmpty())
    {
        return QStringLiteral("该图书暂无任何借阅记录。");
    }
    return lines.join("\n\n");
}

void MainWindow::onShowMyBorrows()
{
    if (currentUsername_.isEmpty() || isAdminMode_)
    {
        QMessageBox::information(this, "提示", "请以学生账号登录后查看自己的借阅信息。");
        return;
    }
    QMessageBox::information(this, "我的借阅", borrowRecordsForCurrentUserText());
}

void MainWindow::onShowBookBorrowHistory()
{
    if (!isAdminMode_)
    {
        QMessageBox::warning(this, "权限不足", "只有管理员可以查看图书借阅记录。");
        return;
    }

    QModelIndexList selectedIndexes = tableView_->selectionModel()->selectedRows();
    if (selectedIndexes.isEmpty())
    {
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
