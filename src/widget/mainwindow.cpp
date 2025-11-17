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
#include <QFileDialog>
#include <QDate>
#include <QSet>
#include <QActionGroup>
#include <algorithm>
#include <QHeaderView>




// ============================================================================
// 构造函数
// ============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , model_(nullptr)
    , tableView_(nullptr)
    , searchEdit_(nullptr)
    , searchButton_(nullptr)
    , themeToggleButton_(nullptr)
    , categoryFilterMenu_(nullptr)
    , statusFilterMenu_(nullptr)
    , categoryActionGroup_(nullptr)
    , statusActionGroup_(nullptr)
    , categoryFilter_()
    , statusFilter_()
    , isDarkMode_(false)
{
    ui->setupUi(this);

    // 1. 搭建视图
    setupTable();

    // 2. 准备数据
    loadSampleData();

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

// ============================================================================
// 数据准备
// ============================================================================
void MainWindow::loadSampleData()
{
    // 清空现有数据
    library_.clear();

    // 创建一些示例图书
    // 创建示例图书数据
    QVector<Book> sampleBooks = {
        // 计算机类图书
        Book{"CS001", "C++程序设计教程", "仙林图书馆", "计算机科学", 5, 45.80, QDate(2023, 1, 15), QDate(), 12, true},
        Book{"CS002", "数据结构与算法分析", "三牌楼图书馆", "计算机科学", 3, 68.50, QDate(2023, 2, 20), QDate(), 8, true},
        Book{"CS003", "操作系统概念", "仙林图书馆", "计算机科学", 4, 89.00, QDate(2023, 3, 10), QDate(), 15, true},
        Book{"CS004", "计算机网络", "三牌楼图书馆", "计算机科学", 6, 76.20, QDate(2023, 1, 25), QDate(), 9, true},
        Book{"CS005", "数据库系统概论", "仙林图书馆", "计算机科学", 2, 92.50, QDate(2023, 4, 5), QDate(), 6, true},

        // 文学类图书
        Book{"LIT001", "红楼梦", "三牌楼图书馆", "文学", 8, 35.60, QDate(2023, 1, 10), QDate(), 25, true},
        Book{"LIT002", "百年孤独", "仙林图书馆", "文学", 4, 42.80, QDate(2023, 2, 15), QDate(), 18, true},
        Book{"LIT003", "活着", "三牌楼图书馆", "文学", 6, 28.90, QDate(2023, 3, 1), QDate(), 22, true},
        Book{"LIT004", "平凡的世界", "仙林图书馆", "文学", 5, 55.00, QDate(2023, 1, 20), QDate(), 16, true},
        Book{"LIT005", "围城", "三牌楼图书馆", "文学", 3, 38.50, QDate(2023, 2, 28), QDate(), 14, true},

        // 历史类图书
        Book{"HIS001", "中国通史", "仙林图书馆", "历史", 4, 78.00, QDate(2023, 1, 5), QDate(), 11, true},
        Book{"HIS002", "世界文明史", "三牌楼图书馆", "历史", 3, 85.50, QDate(2023, 3, 15), QDate(), 7, true},
        Book{"HIS003", "明朝那些事儿", "仙林图书馆", "历史", 6, 48.80, QDate(2023, 2, 10), QDate(), 20, true},
        Book{"HIS004", "人类简史", "三牌楼图书馆", "历史", 5, 65.20, QDate(2023, 4, 1), QDate(), 13, true},

        // 科学类图书
        Book{"SCI001", "时间简史", "仙林图书馆", "科学", 3, 52.00, QDate(2023, 1, 30), QDate(), 9, true},
        Book{"SCI002", "物种起源", "三牌楼图书馆", "科学", 2, 68.80, QDate(2023, 3, 20), QDate(), 5, true},
        Book{"SCI003", "相对论", "仙林图书馆", "科学", 1, 75.50, QDate(2023, 2, 25), QDate(), 3, true},
        Book{"SCI004", "量子力学原理", "三牌楼图书馆", "科学", 2, 88.00, QDate(2023, 4, 10), QDate(), 4, true},

        // 外语类图书
        Book{"ENG001", "新概念英语", "仙林图书馆", "外语", 10, 32.50, QDate(2023, 1, 12), QDate(), 35, true},
        Book{"ENG002", "托福词汇精选", "三牌楼图书馆", "外语", 8, 45.80, QDate(2023, 2, 18), QDate(), 28, true},
        Book{"ENG003", "雅思考试指南", "仙林图书馆", "外语", 6, 58.20, QDate(2023, 3, 8), QDate(), 19, true},
        Book{"ENG004", "商务英语", "三牌楼图书馆", "外语", 4, 42.00, QDate(2023, 1, 28), QDate(), 12, true},

        // 艺术类图书
        Book{"ART001", "西方美术史", "仙林图书馆", "艺术", 3, 72.50, QDate(2023, 2, 5), QDate(), 8, true},
        Book{"ART002", "中国书法艺术", "三牌楼图书馆", "艺术", 2, 55.80, QDate(2023, 3, 12), QDate(), 6, true},
        Book{"ART003", "音乐理论基础", "仙林图书馆", "艺术", 4, 48.00, QDate(2023, 1, 18), QDate(), 10, true},

        // 哲学类图书
        Book{"PHI001", "论语", "三牌楼图书馆", "哲学", 5, 25.80, QDate(2023, 1, 8), QDate(), 17, true},
        Book{"PHI002", "道德经", "仙林图书馆", "哲学", 4, 22.50, QDate(2023, 2, 22), QDate(), 14, true},
        Book{"PHI003", "苏菲的世界", "三牌楼图书馆", "哲学", 3, 38.80, QDate(2023, 3, 25), QDate(), 11, true},

        // 一些已借出的图书
        Book{"CS006", "人工智能导论", "仙林图书馆", "计算机科学", 2, 95.00, QDate(2023, 4, 15), QDate(2024, 1, 15), 3, false},
        Book{"LIT006", "1984", "三牌楼图书馆", "文学", 3, 36.50, QDate(2023, 2, 8), QDate(2024, 1, 20), 7, false},
        Book{"ENG005", "英语语法大全", "仙林图书馆", "外语", 5, 52.80, QDate(2023, 3, 18), QDate(2024, 1, 25), 9, false},
        Book{"SCI005", "宇宙的奥秘", "三牌楼图书馆", "科学", 2, 68.00, QDate(2023, 1, 22), QDate(2024, 1, 30), 5, false}};

    // 将示例图书添加到数据管理器
    for (const auto &book : sampleBooks) {
        library_.addBook(book);
    }
}

// ============================================================================
// 视图搭建
// ============================================================================
void MainWindow::setupTable()
{
    // 创建数据模型
    model_ = new QStandardItemModel(this);
    model_->setHorizontalHeaderLabels({
        QStringLiteral("索引号"), QStringLiteral("名称"), QStringLiteral("馆藏地址"),
        QStringLiteral("类别"), QStringLiteral("数量"), QStringLiteral("价格"),
        QStringLiteral("入库日期"), QStringLiteral("归还日期"), QStringLiteral("借阅次数"),
        QStringLiteral("状态")
    });

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
    tableView_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch); // 名称列拉伸
    tableView_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch); // 名称列拉伸
    tableView_->horizontalHeader()->setSectionResizeMode(8, QHeaderView::Stretch); // 馆藏地址列拉伸

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
    for (int row = 0; row < books.size(); ++row) {
        const Book &b = books[row];

        if (!categoryFilter_.isEmpty() && b.category != categoryFilter_) {
            continue;
        }
        if (statusFilter_ == "available" && !b.available) {
            continue;
        }
        if (statusFilter_ == "borrowed" && b.available) {
            continue;
        }
        QList<QStandardItem*> rowItems;
        rowItems << new QStandardItem(b.indexId);
        rowItems << new QStandardItem(b.name);
        rowItems << new QStandardItem(b.location);
        rowItems << new QStandardItem(b.category);
        rowItems << new QStandardItem(QString::number(b.quantity));
        rowItems << new QStandardItem(QString::number(b.price, 'f', 2));
        rowItems << new QStandardItem(b.inDate.toString("yyyy-MM-dd"));
        rowItems << new QStandardItem(b.returnDate.isValid() ? b.returnDate.toString("yyyy-MM-dd") : "");
        rowItems << new QStandardItem(QString::number(b.borrowCount));
        rowItems << new QStandardItem(b.available ? "✅ 可借" : "❌ 已借出");

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
    // 获取当前选中的行
    QModelIndexList selectedIndexes = tableView_->selectionModel()->selectedRows();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择要借阅的图书！");
        return;
    }

    int row = selectedIndexes.first().row();
    QString indexId = model_->item(row, 0)->text();
    QString bookName = model_->item(row, 1)->text();

    // 检查图书是否可借
    const Book* bookPtr = library_.findByIndexId(indexId);
    if (!bookPtr || !bookPtr->available) {
        QMessageBox::warning(this, "借书失败", "该图书已被借出，无法再次借阅！");
        return;
    }

    // 弹出输入框，让用户输入归还日期
    bool ok;
    QDate dueDate = QDate::fromString(
        QInputDialog::getText(this, "借书", "请输入归还日期 (yyyy-MM-dd):", QLineEdit::Normal, QDate::currentDate().addDays(30).toString("yyyy-MM-dd"), &ok),
        "yyyy-MM-dd"
    );

    if (!ok || !dueDate.isValid()) {
        return; // 用户取消或输入无效
    }

    // 更新数据
    Book updatedBook = *bookPtr;
    updatedBook.available = false;
    updatedBook.returnDate = dueDate;
    updatedBook.borrowCount++;

    QString error;
    if (library_.updateBook(indexId, updatedBook, &error)) {
        refreshTable();
        QMessageBox::information(this, "成功", QStringLiteral("成功借阅图书《%1》，归还日期为 %2").arg(bookName, dueDate.toString("yyyy-MM-dd")));
    } else {
        QMessageBox::warning(this, "失败", "借阅失败：" + error);
    }
}

void MainWindow::onReturn()
{
    // 获取当前选中的行
    QModelIndexList selectedIndexes = tableView_->selectionModel()->selectedRows();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择要归还的图书！");
        return;
    }

    int row = selectedIndexes.first().row();
    QString indexId = model_->item(row, 0)->text();
    QString bookName = model_->item(row, 1)->text();

    // 检查图书是否已借出
    const Book* bookPtr = library_.findByIndexId(indexId);
    if (!bookPtr || bookPtr->available) {
        QMessageBox::warning(this, "还书失败", "该图书未被借出，无需归还！");
        return;
    }

    // 确认归还
    auto reply = QMessageBox::question(this, "确认还书",
                                      QStringLiteral("确定要归还图书《%1》吗？").arg(bookName),
                                      QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No) {
        return;
    }

    // 更新数据
    Book updatedBook = *bookPtr;
    updatedBook.available = true;
    updatedBook.returnDate = QDate(); // 清空归还日期

    QString error;
    if (library_.updateBook(indexId, updatedBook, &error)) {
        refreshTable();
        QMessageBox::information(this, "成功", QStringLiteral("成功归还图书《%1》").arg(bookName));
    } else {
        QMessageBox::warning(this, "失败", "归还失败：" + error);
    }
}

void MainWindow::onShowAll()
{
    refreshTable();
}

void MainWindow::onSwitchMode()
{
    // 恢复一个简单的实现，避免链接错误
    QMessageBox::information(this, "功能", "切换模式功能 (待实现)");
}

void MainWindow::onSearch()
{
    if (!searchEdit_) return;
    QString name = searchEdit_->text().trimmed();
    if (name.isEmpty()) {
        onShowAll(); // 如果搜索框为空，显示所有
        return;
    }

    const Book *b = library_.findByName(name);
    if (b) {
        // 找到了，只显示这一本
        model_->removeRows(0, model_->rowCount());
        QList<QStandardItem*> rowItems;
        rowItems << new QStandardItem(b->indexId);
        rowItems << new QStandardItem(b->name);
        rowItems << new QStandardItem(b->location);
        rowItems << new QStandardItem(b->category);
        rowItems << new QStandardItem(QString::number(b->quantity));
        rowItems << new QStandardItem(QString::number(b->price, 'f', 2));
        rowItems << new QStandardItem(b->inDate.toString("yyyy-MM-dd"));
        rowItems << new QStandardItem(b->returnDate.isValid() ? b->returnDate.toString("yyyy-MM-dd") : "");
        rowItems << new QStandardItem(QString::number(b->borrowCount));
        rowItems << new QStandardItem(b->available ? "✅ 可借" : "❌ 已借出");
        model_->appendRow(rowItems);
    } else {
        QMessageBox::information(this, "未找到", QStringLiteral("没有找到名称为 \"%1\" 的图书").arg(name));
    }
}

void MainWindow::onOpen()
{
    QString path = QFileDialog::getOpenFileName(this, "打开图书数据", "", "JSON Files (*.json)");
    if (!path.isEmpty()) {
        QString error;
        if (library_.loadFromFile(path, &error)) {
            rebuildFilterMenus();
            refreshTable();
            QMessageBox::information(this, "成功", "数据加载成功！");
        } else {
            QMessageBox::warning(this, "失败", "文件加载失败：" + error);
        }
    }
}

void MainWindow::onSave()
{
    QString path = QFileDialog::getSaveFileName(this, "保存图书数据", "library.json", "JSON Files (*.json)");
    if (!path.isEmpty()) {
        QString error;
        if (library_.saveToFile(path, &error)) {
            QMessageBox::information(this, "成功", "数据保存成功！");
        } else {
            QMessageBox::warning(this, "失败", "文件保存失败：" + error);
        }
    }
}


// ============================================================================
// UI设置和其他辅助函数
// ============================================================================
void MainWindow::setupActions()
{
    auto *bar = addToolBar(QStringLiteral("操作"));
    bar->setMovable(false);
    bar->setFloatable(false);
    bar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    bar->setOrientation(Qt::Vertical);
    bar->setAllowedAreas(Qt::LeftToolBarArea);

    auto *scrollArea = new QScrollArea();
    scrollArea->setWidget(bar);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setMaximumWidth(180);
    scrollArea->setMinimumWidth(130);

    addDockWidget(Qt::LeftDockWidgetArea, createDockWidgetFromScrollArea(scrollArea));

    // 只添加需要的动作
    auto borrowAct = bar->addAction(QStringLiteral("📖 借书"));
    auto returnAct = bar->addAction(QStringLiteral("📤 还书"));
    bar->addSeparator();
    auto openAct = bar->addAction(QStringLiteral("📂 打开"));
    auto saveAct = bar->addAction(QStringLiteral("💾 保存"));
    auto allAct = bar->addAction(QStringLiteral("📋 显示全部"));

    connect(borrowAct, &QAction::triggered, this, &MainWindow::onBorrow);
    connect(returnAct, &QAction::triggered, this, &MainWindow::onReturn);
    connect(openAct, &QAction::triggered, this, &MainWindow::onOpen);
    connect(saveAct, &QAction::triggered, this, &MainWindow::onSave);
    connect(allAct, &QAction::triggered, this, &MainWindow::onShowAll);
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
    setWindowTitle(QStringLiteral("图书管理系统 (只读模式)"));
    setWindowIcon(QIcon(":/icons/library.svg"));
    statusBar()->setMinimumHeight(28);
    updateStatusBar();
}

void MainWindow::updateStatusBar()
{
    int total = library_.getTotalBooks();
    int available = library_.getAvailableBooks();
    int borrowed = library_.getBorrowedBooks();

    QString statusText = QStringLiteral("📊 总计: %1 | ✅ 可借: %2 | ❌ 已借: %3")
                             .arg(total).arg(available).arg(borrowed);
    statusBar()->showMessage(statusText);
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

// 这里 getThemeStyles 函数保持不变，直接使用你之前的完整版本
QString MainWindow::getThemeStyles(bool isDark)
{
    if (isDark) {
        // ... 你的深色主题QSS ...
        return QStringLiteral("QMainWindow {"
        "    background-color: #1A252F;" /* 墨绿背景 */
        "    color: #D1E7DD;" /* 柔和的白色 */
        "    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;"
        "}"
        "QToolBar {"
        "    background-color: #22333B;" /* 稍亮的绿 */
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
        "    border-color: #52B788;" /* 薄荷绿边框 */
        "    color: #52B788;" /* 薄荷绿文字 */
        "}"
        "QToolButton:pressed {"
        "    background-color: #52B788;"
        "    color: #1A252F;"
        "    border-color: #40916C;"
        "}"
        "QStatusBar {"
        "    background-color: #22333B;"
        "    color: #95D5B2;" /* 更柔和的绿色文字 */
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
        "    color: #F4A261;" /* 琥珀橙表头 */
        "    padding: 16px 12px;"
        "    border: none;"
        "    font-weight: 600;"
        "    font-size: 15px;"
        "    min-height: 44px;"
        "    border-bottom: 2px solid #F4A261;" /* 琥珀橙下边框 */
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
        "    border-color: #52B788;" /* 聚焦时薄荷绿边框 */
        "    background-color: #4A5A62;"
        "}"
        "QPushButton {"
        "    background-color: #F4A261;" /* 琥珀橙按钮 */
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
        "    background-color: #E76F51;" /* 悬停时变为珊瑚红 */
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
        "    background-color: #52B788;" /* 滚动条悬停时薄荷绿 */
        "}"
        );
    } else {
        return QStringLiteral(
            "QMainWindow {"
        "    background-color: #FFF9FA;" /* 浅粉色背景基调 */
        "    color: #5A4B56;" /* 深粉灰色文字 */
        "    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;"
        "}"
        "QToolBar {"
        "    background-color: #FEEFF1;" /* 浅粉色工具栏 */
        "    border: none;"
        "    border-right: 1px solid #F8D7DC;" /* 浅粉边框 */
        "    spacing: 8px;"
        "    padding: 12px 8px;"
        "}"
        "QToolButton {"
        "    background-color: #FFFFFF;"
        "    color: #5A4B56;"
        "    border: 1px solid #F8D7DC;" /* 浅粉边框 */
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
        "    background-color: #FEEFF1;" /* 浅粉悬停背景 */
        "    border-color: #F9A8D4;" /* 亮粉色边框 */
        "    color: #E11D48;" /* 深粉色文字 */
        "}"
        "QToolButton:pressed {"
        "    background-color: #F9A8D4;" /* 亮粉色点击背景 */
        "    color: #FFFFFF;"
        "    border-color: #DB7093;" /* 深粉边框 */
        "}"
        "QStatusBar {"
        "    background-color: #FEEFF1;" /* 浅粉色状态栏 */
        "    color: #E11D48;" /* 深粉色文字 */
        "    border-top: 1px solid #F8D7DC;" /* 浅粉边框 */
        "    padding: 6px 16px;"
        "    font-size: 14px;"
        "    min-height: 28px;"
        "    line-height: 1.4;"
        "}"
        "QTableView {"
        "    background-color: #FFFFFF;"
        "    alternate-background-color: #FEEFF1;" /* 浅粉色交替行 */
        "    selection-background-color: #F9A8D4;" /* 亮粉色选中背景 */
        "    selection-color: #FFFFFF;"
        "    gridline-color: #F8D7DC;" /* 浅粉网格线 */
        "    color: #5A4B56;" /* 深粉灰色文字 */
        "    border: 1px solid #F8D7DC;" /* 浅粉边框 */
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
        "    background-color: #FEEFF1;" /* 浅粉悬停背景 */
        "}"
        "QHeaderView::section {"
        "    background-color: #FEEFF1;" /* 浅粉色表头 */
        "    color: #E11D48;" /* 深粉色表头文字 */
        "    padding: 16px 12px;"
        "    border: none;"
        "    font-weight: 600;"
        "    font-size: 15px;"
        "    min-height: 44px;"
        "    border-bottom: 2px solid #F9A8D4;" /* 亮粉色下边框 */
        "}"
        "QHeaderView::section:hover {"
        "    background-color: #FEE5E9;" /* 稍深的浅粉色悬停 */
        "}"
        "QLineEdit {"
        "    background-color: #FFFFFF;"
        "    border: 2px solid #F8D7DC;" /* 浅粉边框 */
        "    border-radius: 20px;"
        "    padding: 8px 16px;"
        "    font-size: 14px;"
        "    color: #5A4B56;"
        "    min-height: 20px;"
        "}"
        "QLineEdit:focus {"
        "    border-color: #F9A8D4;" /* 亮粉色聚焦边框 */
        "    background-color: #FFF5F7;" /* 极浅粉色聚焦背景 */
        "}"
        "QPushButton {"
        "    background-color: #F9A8D4;" /* 亮粉色按钮 */
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
        "    background-color: #E11D48;" /* 深粉色悬停 */
        "}"
        "QDockWidget {"
        "    background-color: #FEEFF1;" /* 浅粉色停靠窗 */
        "    border: none;"
        "    border-right: 1px solid #F8D7DC;" /* 浅粉边框 */
        "}"
        "QScrollArea {"
        "    background-color: #FEEFF1;"
        "    border: none;"
        "}"
        "QScrollBar:vertical {"
        "    background-color: #F8D7DC;" /* 浅粉色滚动条背景 */
        "    width: 8px;"
        "    border-radius: 4px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background-color: #F9A8D4;" /* 亮粉色滚动条 */
        "    border-radius: 4px;"
        "    min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background-color: #E11D48;" /* 深粉色悬停 */
        "}"

        );
    }
}

void MainWindow::rebuildFilterMenus()
{
    delete categoryFilterMenu_;
    delete statusFilterMenu_;
    delete categoryActionGroup_;
    delete statusActionGroup_;

    categoryFilterMenu_ = new QMenu(this);
    categoryFilterMenu_->setMinimumWidth(200);
    categoryActionGroup_ = new QActionGroup(categoryFilterMenu_);
    categoryActionGroup_->setExclusive(true);

    auto addCategoryAction = [this](const QString &label, const QString &value, bool separator = false) {
        if (separator) {
            categoryFilterMenu_->addSeparator();
            return static_cast<QAction*>(nullptr);
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
}

void MainWindow::updateHeaderLabels()
{
    if (!model_) return;

    QString categoryLabel = QStringLiteral("类别");
    if (!categoryFilter_.isEmpty()) {
        categoryLabel += QStringLiteral(" · %1").arg(categoryFilter_);
    }
    model_->setHeaderData(3, Qt::Horizontal, categoryLabel);

    QString statusLabel = QStringLiteral("状态");
    if (statusFilter_ == "available") {
        statusLabel += QStringLiteral(" · 可借");
    } else if (statusFilter_ == "borrowed") {
        statusLabel += QStringLiteral(" · 已借出");
    }
    model_->setHeaderData(9, Qt::Horizontal, statusLabel);
}

void MainWindow::onHeaderSectionClicked(int section)
{
    if (section == 3) {
        showFilterMenu(categoryFilterMenu_, section);
    } else if (section == 9) {
        showFilterMenu(statusFilterMenu_, section);
    }
}

void MainWindow::showFilterMenu(QMenu *menu, int section)
{
    if (!menu || !tableView_) return;
    QHeaderView *header = tableView_->horizontalHeader();

    // 修复：使用 sectionPosition() 和 sectionSize() 替代不存在的 sectionRect()
    int x = header->sectionPosition(section);
    int width = header->sectionSize(section);
    int height = header->height();

    QRect sectionRect(x, 0, width, height);
    QPoint globalPos = header->mapToGlobal(sectionRect.bottomLeft());
    menu->popup(globalPos);
}

