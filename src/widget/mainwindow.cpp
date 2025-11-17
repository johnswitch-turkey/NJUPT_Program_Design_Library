#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QToolBar>
#include <QScrollArea>
#include <QDockWidget>
#include <QHeaderView>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QIcon>
#include <QInputDialog>
#include <QMessageBox>

// --- 占位函数 ---
// 为了让代码能够编译，这里提供了一些空的槽函数实现。
// 在实际项目中，这些函数应该包含真正的业务逻辑。
void MainWindow::onAdd() { QMessageBox::information(this, "功能", "新增图书功能"); }
void MainWindow::onEdit() { QMessageBox::information(this, "功能", "编辑图书功能"); }
void MainWindow::onRemove() { QMessageBox::information(this, "功能", "删除图书功能"); }
void MainWindow::onBorrow() { QMessageBox::information(this, "功能", "借书功能"); }
void MainWindow::onReturn() { QMessageBox::information(this, "功能", "还书功能"); }
void MainWindow::onShowDue() { QMessageBox::information(this, "功能", "显示到期图书功能"); }
void MainWindow::onSortByBorrow() { QMessageBox::information(this, "功能", "按借阅次数排序功能"); }
void MainWindow::onOpen() { QMessageBox::information(this, "功能", "打开文件功能"); }
void MainWindow::onSave() { QMessageBox::information(this, "功能", "保存文件功能"); }
void MainWindow::onShowAll() { QMessageBox::information(this, "功能", "显示全部图书功能"); }
void MainWindow::onSwitchMode() { QMessageBox::information(this, "功能", "切换模式功能"); }
void MainWindow::onFilterByCategory() { QMessageBox::information(this, "功能", "按分类筛选功能"); }
void MainWindow::onFilterByLocation() { QMessageBox::information(this, "功能", "按位置筛选功能"); }
void MainWindow::onShowAvailable() { QMessageBox::information(this, "功能", "显示可借图书功能"); }
void MainWindow::onShowBorrowed() { QMessageBox::information(this, "功能", "显示已借图书功能"); }
void MainWindow::onShowTopBorrowed() { QMessageBox::information(this, "功能", "显示热门图书功能"); }
void MainWindow::onShowRecentlyAdded() { QMessageBox::information(this, "功能", "显示最新图书功能"); }
void MainWindow::onShowExpensiveBooks() { QMessageBox::information(this, "功能", "显示高价图书功能"); }
void MainWindow::onShowCheapBooks() { QMessageBox::information(this, "功能", "显示低价图书功能"); }
void MainWindow::onShowStatistics() { QMessageBox::information(this, "功能", "显示统计信息功能"); }
void MainWindow::onSortByName() { QMessageBox::information(this, "功能", "按名称排序功能"); }
void MainWindow::onSortByCategory() { QMessageBox::information(this, "功能", "按分类排序功能"); }
void MainWindow::onSortByLocation() { QMessageBox::information(this, "功能", "按位置排序功能"); }
void MainWindow::onSortByPrice() { QMessageBox::information(this, "功能", "按价格排序功能"); }
void MainWindow::onSortByDate() { QMessageBox::information(this, "功能", "按日期排序功能"); }
void MainWindow::onSortByBorrowCount() { QMessageBox::information(this, "功能", "按借阅排序功能"); }
void MainWindow::onAdvancedSearch() { QMessageBox::information(this, "功能", "高级搜索功能"); }
void MainWindow::onExportData() { QMessageBox::information(this, "功能", "导出数据功能"); }
void MainWindow::onImportData() { QMessageBox::information(this, "功能", "导入数据功能"); }
void MainWindow::onBackupData() { QMessageBox::information(this, "功能", "备份数据功能"); }
void MainWindow::onRestoreData() { QMessageBox::information(this, "功能", "恢复数据功能"); }
// --- 占位函数结束 ---


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , model_(nullptr)
    , tableView_(nullptr)
    , searchEdit_(nullptr)              // 初始化搜索输入框指针
    , searchButton_(nullptr)            // 初始化搜索按钮指针
    , isDarkMode_(false)                // 初始化为浅色主题
    , themeToggleButton_(nullptr)       // 初始化主题切换按钮指针
{
    ui->setupUi(this);       // 设置UI
    setupTable();                       // 初始化表格
    setupMenuBar();                     // 初始化菜单栏
    setupActions();                     // 初始化工具栏动作
    setupSearchBar();                   // 初始化搜索栏
    setupThemeToggle();                 // 初始化主题切换
    setupStyles();                      // 应用样式表
}


// ============================================================================
// 析构函数
// ============================================================================
/**
 * @brief MainWindow 析构函数
 *
 * 清理分配的UI资源
 */
MainWindow::~MainWindow()
{
    delete ui;                          // 释放UI指针
}

void MainWindow::setupTable()
{
    // 创建标准项目模型并设置水平表头标签
    model_ = new QStandardItemModel(this);
    model_->setHorizontalHeaderLabels({
        QStringLiteral("索引号"),
        QStringLiteral("名称"),
        QStringLiteral("馆藏地址"),
        QStringLiteral("类别"),
        QStringLiteral("数量"),
        QStringLiteral("价格"),
        QStringLiteral("入库日期"),
        QStringLiteral("归还日期"),
        QStringLiteral("借阅次数"),
        QStringLiteral("状态")            //图书是否可借
    });

    // 创建表格视图并设置数据模型
    tableView_ = new QTableView(this);
    tableView_->setModel(model_);

    // 设置表格选择行为：选择整行
    tableView_->setSelectionBehavior(QAbstractItemView::SelectRows);

    // 设置表格编辑触发：禁用编辑（只读模式）
    tableView_->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 设置表格自适应列宽
    tableView_->horizontalHeader()->setStretchLastSection(true);                                    // 最后一列填充剩余空间
    tableView_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);            // 其他列根据内容自动调整
    tableView_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);  // 名称列拉伸
    tableView_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);  // 馆藏地址列拉伸

    tableView_->verticalHeader()->setDefaultSectionSize(40);
    tableView_->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    tableView_->setAlternatingRowColors(true);

    ui->centralLayout->addWidget(tableView_);
}

void MainWindow::setupActions()
{
    auto *bar = addToolBar(QStringLiteral("操作"));
    bar->setMovable(false);
    bar->setFloatable(false);
    bar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    bar->setOrientation(Qt::Vertical);
    bar->setAllowedAreas(Qt::LeftToolBarArea);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidget(bar);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setMaximumWidth(180);
    scrollArea->setMinimumWidth(130);

    addDockWidget(Qt::LeftDockWidgetArea, createDockWidgetFromScrollArea(scrollArea));

    // 连接所有动作的信号到对应的占位槽函数
    connect(bar->addAction(QStringLiteral("📚 新增")), &QAction::triggered, this, &MainWindow::onAdd);
    connect(bar->addAction(QStringLiteral("✏️ 编辑")), &QAction::triggered, this, &MainWindow::onEdit);
    connect(bar->addAction(QStringLiteral("🗑️ 删除")), &QAction::triggered, this, &MainWindow::onRemove);
    bar->addSeparator();
    connect(bar->addAction(QStringLiteral("📖 借书")), &QAction::triggered, this, &MainWindow::onBorrow);
    connect(bar->addAction(QStringLiteral("📤 还书")), &QAction::triggered, this, &MainWindow::onReturn);
    bar->addSeparator();
    connect(bar->addAction(QStringLiteral("⏰ 到期(3天内)")), &QAction::triggered, this, &MainWindow::onShowDue);
    connect(bar->addAction(QStringLiteral("📊 按借阅次数排序")), &QAction::triggered, this, &MainWindow::onSortByBorrow);
    bar->addSeparator();
    connect(bar->addAction(QStringLiteral("📂 打开")), &QAction::triggered, this, &MainWindow::onOpen);
    connect(bar->addAction(QStringLiteral("💾 保存")), &QAction::triggered, this, &MainWindow::onSave);
    connect(bar->addAction(QStringLiteral("📋 显示全部")), &QAction::triggered, this, &MainWindow::onShowAll);
    bar->addSeparator();
    connect(bar->addAction(QStringLiteral("🔄 切换模式")), &QAction::triggered, this, &MainWindow::onSwitchMode);

    bar->addSeparator();
    connect(bar->addAction(QStringLiteral("📂 按分类筛选")), &QAction::triggered, this, &MainWindow::onFilterByCategory);
    connect(bar->addAction(QStringLiteral("📍 按位置筛选")), &QAction::triggered, this, &MainWindow::onFilterByLocation);
    connect(bar->addAction(QStringLiteral("✅ 可借图书")), &QAction::triggered, this, &MainWindow::onShowAvailable);
    connect(bar->addAction(QStringLiteral("📖 已借图书")), &QAction::triggered, this, &MainWindow::onShowBorrowed);
    connect(bar->addAction(QStringLiteral("🔥 热门图书")), &QAction::triggered, this, &MainWindow::onShowTopBorrowed);
    connect(bar->addAction(QStringLiteral("🆕 最新图书")), &QAction::triggered, this, &MainWindow::onShowRecentlyAdded);
    connect(bar->addAction(QStringLiteral("💰 高价图书")), &QAction::triggered, this, &MainWindow::onShowExpensiveBooks);
    connect(bar->addAction(QStringLiteral("💸 低价图书")), &QAction::triggered, this, &MainWindow::onShowCheapBooks);
    connect(bar->addAction(QStringLiteral("📊 统计信息")), &QAction::triggered, this, &MainWindow::onShowStatistics);

    bar->addSeparator();
    connect(bar->addAction(QStringLiteral("🔤 按名称排序")), &QAction::triggered, this, &MainWindow::onSortByName);
    connect(bar->addAction(QStringLiteral("📚 按分类排序")), &QAction::triggered, this, &MainWindow::onSortByCategory);
    connect(bar->addAction(QStringLiteral("📍 按位置排序")), &QAction::triggered, this, &MainWindow::onSortByLocation);
    connect(bar->addAction(QStringLiteral("💵 按价格排序")), &QAction::triggered, this, &MainWindow::onSortByPrice);
    connect(bar->addAction(QStringLiteral("📅 按日期排序")), &QAction::triggered, this, &MainWindow::onSortByDate);
    connect(bar->addAction(QStringLiteral("📈 按借阅排序")), &QAction::triggered, this, &MainWindow::onSortByBorrowCount);

    bar->addSeparator();
    connect(bar->addAction(QStringLiteral("🔍 高级搜索")), &QAction::triggered, this, &MainWindow::onAdvancedSearch);
    connect(bar->addAction(QStringLiteral("📤 导出数据")), &QAction::triggered, this, &MainWindow::onExportData);
    connect(bar->addAction(QStringLiteral("📥 导入数据")), &QAction::triggered, this, &MainWindow::onImportData);
    connect(bar->addAction(QStringLiteral("💾 备份数据")), &QAction::triggered, this, &MainWindow::onBackupData);
    connect(bar->addAction(QStringLiteral("🔄 恢复数据")), &QAction::triggered, this, &MainWindow::onRestoreData);
}

void MainWindow::setupMenuBar()
{
    menuBar_ = menuBar();
    // ... (菜单栏样式设置代码与原文件相同，这里省略以保持简洁)
    // 为了完整，这里保留菜单创建和连接的逻辑
    bookMenu_ = menuBar_->addMenu("📚 图书管理");
    connect(bookMenu_->addAction("📖 新增图书"), &QAction::triggered, this, &MainWindow::onAdd);
    connect(bookMenu_->addAction("✏️ 编辑图书"), &QAction::triggered, this, &MainWindow::onEdit);
    connect(bookMenu_->addAction("🗑️ 删除图书"), &QAction::triggered, this, &MainWindow::onRemove);
    // ... 其他菜单项连接 ...

    queryMenu_ = menuBar_->addMenu("🔍 查询筛选");
    // ... 其他菜单项连接 ...

    sortMenu_ = menuBar_->addMenu("📊 排序功能");
    // ... 其他菜单项连接 ...

    dataMenu_ = menuBar_->addMenu("💾 数据管理");
    // ... 其他菜单项连接 ...

    systemMenu_ = menuBar_->addMenu("⚙️ 系统设置");
    connect(systemMenu_->addAction("🔄 切换模式"), &QAction::triggered, this, &MainWindow::onSwitchMode);
    connect(systemMenu_->addAction("🌙 切换主题"), &QAction::triggered, this, &MainWindow::toggleTheme);
    connect(systemMenu_->addAction("ℹ️ 关于系统"), &QAction::triggered, [this]() {
        QMessageBox::about(this, "关于图书管理系统", "📚 图书管理系统 v2.0 (UI Only)\n\n这是一个仅包含UI界面的版本。");
    });
}

void MainWindow::setupSearchBar()
{
    QWidget *searchWidget = new QWidget();
    QHBoxLayout *searchLayout = new QHBoxLayout(searchWidget);
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

void MainWindow::onSearch()
{
    if (!searchEdit_) return;
    QString name = searchEdit_->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("ℹ️ 提示"), QStringLiteral("请输入要搜索的图书名称"));
        return;
    }
    QMessageBox::information(this, QStringLiteral("🔍 搜索"), QStringLiteral("搜索功能未实现，您输入了: %1").arg(name));
}

void MainWindow::setupStyles()
{
    applyTheme(false);
    setWindowTitle(QStringLiteral("图书管理系统 (UI Only)"));
    setWindowIcon(QIcon(":/icons/library.svg")); // 假设你有这个资源文件
    statusBar()->setMinimumHeight(28);
    updateStatusBar();
}

void MainWindow::updateUIForUserMode()
{
    // 在纯UI版本中，此函数可以简化或留空
    // 因为没有业务逻辑，权限控制没有意义
}

void MainWindow::updateStatusBar()
{
    QString modeText = "UI模式";
    QString statusText = QStringLiteral("👤 用户: UI演示 | 🔐 模式: %1").arg(modeText);
    statusBar()->showMessage(statusText);
    statusBar()->setMinimumHeight(28);
}

QDockWidget* MainWindow::createDockWidgetFromScrollArea(QScrollArea *scrollArea)
{
    QDockWidget *dockWidget = new QDockWidget("功能栏", this);
    dockWidget->setWidget(scrollArea);
    dockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
    dockWidget->setTitleBarWidget(new QWidget());
    dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    return dockWidget;
}

void MainWindow::applyTheme(bool isDark)
{
    QString styles = getThemeStyles(isDark);
    setStyleSheet(styles);
}

QString MainWindow::getThemeStyles(bool isDark)
{
    // ... (此函数内容与原文件完全相同，这里省略以节省篇幅)
    // 它包含了所有深色和浅色主题的QSS样式表
    if (isDark) {
        return QStringLiteral("/* ... 深色主题QSS ... */");
    } else {
        return QStringLiteral("/* ... 浅色主题QSS ... */");
    }
}
