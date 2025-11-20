// mainwindow.cpp
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../utils/bookdisplay.h"
#include "../utils/librarymanager.h"
#include "copymanagementdialog.h"
#include "bookdetaildialog.h"
#include "borrowdialog.h"

#include <QMenu>
#include <QAction>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QIcon>
#include <QComboBox>
#include <QTextCharFormat>
#include <QFont>

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
/**
 * @brief MainWindow构造函数
 *
 * 初始化主窗口的所有组件和功能
 *
 * 初始化顺序说明：
 * 1. UI基础设置 - 加载Qt Designer设计的界面
 * 2. 数据视图搭建 - 创建表格视图和数据模型
 * 3. 数据加载 - 从数据库加载图书数据
 * 4. 筛选菜单构建 - 根据数据动态生成筛选选项
 * 5. 表格数据填充 - 将数据显示在表格中
 * 6. UI组件初始化 - 设置菜单栏、工具栏、搜索栏等
 * 7. 样式应用 - 应用主题样式和UI美化
 *
 * @param parent 父窗口指针，默认为nullptr
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      model_(nullptr),
      tableView_(nullptr),
      searchEdit_(nullptr),
      searchButton_(nullptr),
      themeToggleButton_(nullptr),
      searchModeComboBox_(nullptr),
      categoryFilterMenu_(nullptr),
      statusFilterMenu_(nullptr),
      locationFilterMenu_(nullptr),
      sortMenu_(nullptr),
      categoryActionGroup_(nullptr),
      statusActionGroup_(nullptr),
      locationActionGroup_(nullptr),
      sortActionGroup_(nullptr),
      categoryFilter_(),
      statusFilter_(),
      locationFilter_(),
      currentSortType_("default"),
      currentSearchKeyword_(),
      currentSearchMode_(),
      isSearchActive_(false),
      isDarkMode_(false),
      isEditMode_(false)
{
    // UI基础设置：加载Qt Designer设计的界面布局
    ui->setupUi(this);

    // 1. 数据视图搭建：创建表格视图和数据模型，定义表格结构
    setupTable();

    // 2. 数据准备：从数据库加载图书数据，如果为空则自动导入示例数据
    loadData();

    // 2.5 筛选菜单构建：根据实际数据动态生成筛选和排序菜单
    rebuildFilterMenus();

    // 3. 表格数据填充：将数据加载到表格视图中显示
    refreshTable();

    // UI组件初始化：设置各个功能区域
    setupMenuBar();      // 菜单栏设置
    setupActions();      // 工具栏和功能按钮
    setupSearchBar();    // 搜索区域
    setupThemeToggle();  // 主题切换按钮
    setupStyles();       // 应用样式主题
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
// 数据视图搭建
// ============================================================================
/**
 * @brief 设置表格视图和数据模型
 *
 * 功能说明：
 * 1. 创建QStandardItemModel作为数据模型，管理表格数据
 * 2. 设置表格列标题，包含图书的所有重要信息字段
 * 3. 创建QTableView作为视图组件，用于显示数据
 * 4. 配置表格的各种显示属性和行为设置
 * 5. 设置列宽和排序功能
 * 6. 关联信号槽，处理用户交互事件
 *
 * 表格结构说明：
 * - 索引号：图书的唯一标识符
 * - 名称：图书标题
 * - 作者：图书作者信息
 * - 出版社：图书出版社
 * - 馆藏地址：图书存放位置（三牌楼/仙林）
 * - 类别：图书分类（人文/科技/外语等）
 * - 数量：该图书的副本总数
 * - 价格：图书价格
 * - 入库日期：图书录入系统的日期
 * - 归还日期：当前状态的归还日期（主要用于搜索）
 * - 借阅次数：图书被借阅的总次数
 * - 状态：图书的当前状态（可借/不可借）
 */
void MainWindow::setupTable()
{
    // 创建数据模型：使用QStandardItemModel管理表格数据
    model_ = new QStandardItemModel(this);

    // 设置表格列标题：定义12个信息列，覆盖图书的完整信息
    model_->setHorizontalHeaderLabels({
        QStringLiteral("索引号"),     // 0: 唯一标识
        QStringLiteral("名称"),       // 1: 图书标题
        QStringLiteral("作者"),       // 2: 作者信息
        QStringLiteral("出版社"),     // 3: 出版社
        QStringLiteral("馆藏地址"),   // 4: 存放位置
        QStringLiteral("类别"),       // 5: 图书分类
        QStringLiteral("数量"),       // 6: 副本总数
        QStringLiteral("价格"),       // 7: 图书价格
        QStringLiteral("入库日期"),   // 8: 录入日期
        QStringLiteral("归还日期"),   // 9: 当前状态归还日期
        QStringLiteral("借阅次数"),   // 10: 借阅统计
        QStringLiteral("状态")        // 11: 可用状态
    });

    // 创建表格视图：设置视图组件并关联数据模型
    tableView_ = new QTableView(this);
    tableView_->setModel(model_);

    // 选择行为设置：设置整行选择，便于用户操作
    tableView_->setSelectionBehavior(QAbstractItemView::SelectRows);

    // 编辑触发器设置：禁用单元格编辑，防止误操作
    tableView_->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 外观设置：启用交替行颜色，提高可读性
    tableView_->setAlternatingRowColors(true);

    // 行高设置：设置默认行高为50像素，确保内容显示完整
    tableView_->verticalHeader()->setDefaultSectionSize(50);
    // 列宽策略设置：两步法优化列宽显示
    // 1. 先设置所有列为根据内容自动调整大小
    tableView_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // 2. 再设置主要列为拉伸模式，充分利用可用空间
    // 设置前11列（除状态列外）为拉伸模式，确保内容完整显示
    for (int col = 0; col <= 10; ++col) {
        tableView_->horizontalHeader()->setSectionResizeMode(col, QHeaderView::Stretch);
    }

    // 最小列宽设置：确保列宽不会太小，影响可读性
    tableView_->horizontalHeader()->setMinimumSectionSize(120);

    // 表头交互设置：启用表头点击功能，支持排序
    tableView_->horizontalHeader()->setSectionsClickable(true);
    // 关联表头点击信号槽，处理点击排序功能
    connect(tableView_->horizontalHeader(), &QHeaderView::sectionClicked,
            this, &MainWindow::onHeaderSectionClicked);

    // 表格交互设置：启用双击事件，用于显示图书详细信息
    connect(tableView_, &QTableView::doubleClicked, this, &MainWindow::onTableDoubleClicked);

    // 布局集成：将表格视图添加到主窗口的中央布局中
    ui->centralLayout->addWidget(tableView_);
}

// ============================================================================
// 表格数据刷新
// ============================================================================
/**
 * @brief 刷新表格数据显示
 *
 * 功能说明：
 * 1. 清空现有表格数据，准备重新填充
 * 2. 智能数据源选择：
 *    - 如果处于搜索状态，使用搜索结果
 *    - 如果处于普通模式，使用全部数据
 * 3. 应用当前的筛选条件
 * 4. 应用当前的排序设置
 * 5. 更新表格头部显示当前筛选和排序状态
 * 6. 更新状态栏显示统计信息
 *
 * 智能刷新机制：
 * - 搜索状态优先：如果有搜索关键词，保持搜索状态
 * - 筛选条件保持：维持用户的筛选选择
 * - 排序状态保持：维持用户的排序设置
 * - 性能优化：只在必要时重新执行搜索
 */
void MainWindow::refreshTable()
{
    // 清空表格：移除所有现有行，准备重新填充数据
    model_->removeRows(0, model_->rowCount());

    // 智能数据源选择：根据当前状态决定数据来源
    if (isSearchActive_) {
        // 搜索模式：重新执行搜索并应用当前的筛选和排序
        performFuzzySearch(currentSearchKeyword_, currentSearchMode_);
        return;
    }

    // 普通模式：获取所有图书数据
    const QVector<Book> &books = library_.getAll();

    // 数据遍历和筛选：逐行处理图书数据
    for (int row = 0; row < books.size(); ++row) {
        const Book &b = books[row];

        // 多维度筛选条件应用
        // 1. 类别筛选：只显示指定类别的图书
        if (!categoryFilter_.isEmpty() && b.category != categoryFilter_) {
            continue;
        }
        // 2. 位置筛选：只显示指定馆藏地址的图书
        if (!locationFilter_.isEmpty() && b.location != locationFilter_) {
            continue;
        }

        // 副本状态获取：计算该图书的副本情况
        int totalCopies = library_.getTotalCopyCount(b.indexId);
        int availableCopies = library_.getAvailableCopyCount(b.indexId);

        // 3. 状态筛选：根据可用性进行筛选
        if (statusFilter_ == "available" && availableCopies <= 0) {
            continue; // 只显示有可用副本的图书
        }
        if (statusFilter_ == "borrowed" && availableCopies >= totalCopies) {
            continue; // 只显示没有可用副本的图书（即全部被借走）
        }

        // 创建表格行数据：按列顺序添加所有信息
        QList<QStandardItem *> rowItems;
        rowItems << new QStandardItem(b.indexId);                                           // 索引号
        rowItems << new QStandardItem(b.name);                                               // 名称
        rowItems << new QStandardItem(b.author);                                             // 作者
        rowItems << new QStandardItem(b.publisher);                                          // 出版社
        rowItems << new QStandardItem(b.location);                                           // 馆藏地址
        rowItems << new QStandardItem(b.category);                                           // 类别
        rowItems << new QStandardItem(QString::number(totalCopies));                         // 数量：总副本数
        rowItems << new QStandardItem(QString::number(b.price, 'f', 2));                    // 价格：保留2位小数
        rowItems << new QStandardItem(b.inDate.toString("yyyy-MM-dd"));                      // 入库日期

        // 归还日期列：个性化显示逻辑
        // 学生用户可以看到自己的借阅归还日期，管理员看不到
        QString returnDateStr = "";
        if (!currentUsername_.isEmpty() && !isAdminMode_) {  // 学生用户
            // 获取该学生当前借阅的所有副本
            QVector<BookCopy> borrowedCopies = library_.getUserBorrowedCopies(currentUsername_);
            for (const BookCopy &copy : borrowedCopies) {
                if (copy.indexId == b.indexId) {  // 找到该书的借阅记录
                    returnDateStr = copy.dueDate.toString("yyyy-MM-dd");
                    break;  // 找到一个即可（正常情况下学生不会重复借阅同一本书）
                }
            }
        }
        rowItems << new QStandardItem(returnDateStr);  // 归还日期

        rowItems << new QStandardItem(QString::number(b.borrowCount));                       // 借阅次数

        // 状态列：根据副本可用性动态显示状态
        QString statusText = (availableCopies > 0) ? QStringLiteral("可借") : QStringLiteral("不可借");
        rowItems << new QStandardItem(statusText);

        // 将行数据添加到模型中
        model_->appendRow(rowItems);
    }

    // 界面状态更新
    updateStatusBar();      // 更新状态栏统计信息
    updateHeaderLabels();   // 更新表头显示当前筛选和排序状态
}

// ============================================================================
// 核心业务逻辑槽函数
// ============================================================================
/**
 * @brief 借书功能实现
 *
 * 功能流程：
 * 1. 权限验证：只有学生用户可以借书，管理员不能借书
 * 2. 选择验证：检查用户是否选择了要借阅的图书
 * 3. 数据验证：获取选中的图书信息，验证图书是否存在
 * 4. 重复检查：检查该学生是否已经借过这本书的任何副本
 * 5. 副本检查：获取该图书的所有可用副本
 * 6. 用户交互：显示借书对话框，让用户选择具体副本和归还日期
 * 7. 业务处理：调用LibraryManager的borrowBook方法执行借书操作
 * 8. 界面更新：刷新表格显示，显示借书成功/失败信息
 *
 * 涉及的函数：
 * - library_.findByIndexId(): 查找图书信息
 * - library_.getUserBorrowedCopies(): 获取用户已借副本
 * - library_.getAvailableCopies(): 获取可用副本列表
 * - library_.borrowBook(): 执行借书操作
 * - refreshTable(): 刷新表格显示
 */
void MainWindow::onBorrow()
{
    // 权限验证：只有学生用户可以借书
    if (currentUsername_.isEmpty() || isAdminMode_) {
        QMessageBox::warning(this, "借书失败", "只有学生用户可以借书，请使用学生账号登录。");
        return;
    }

    // 选择验证：检查是否选择了图书
    QModelIndexList selectedIndexes = tableView_->selectionModel()->selectedRows();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择要借阅的图书！");
        return;
    }

    // 获取选中图书的信息
    int row = selectedIndexes.first().row();
    QString indexId = model_->item(row, 0)->text();
    QString bookName = model_->item(row, 1)->text();

    // 数据验证：检查图书是否存在
    const Book *book = library_.findByIndexId(indexId);
    if (!book) {
        QMessageBox::warning(this, "错误", "找不到选中的图书信息！");
        return;
    }

    // 重复检查：防止学生重复借阅同一本书的不同副本
    QVector<BookCopy> borrowedCopies = library_.getUserBorrowedCopies(currentUsername_);
    for (const BookCopy &copy : borrowedCopies) {
        if (copy.indexId == indexId) {
            QMessageBox::warning(this, "借书失败",
                QStringLiteral("你已经借过《%1》的副本%2，请先归还再借。")
                .arg(bookName, QString::number(copy.copyNumber)));
            return;
        }
    }

    // 副本检查：获取该图书的所有可用副本
    QVector<BookCopy> availableCopies = library_.getAvailableCopies(indexId);
    if (availableCopies.isEmpty()) {
        QMessageBox::warning(this, "借书失败", "该图书暂无可借副本！");
        return;
    }

    // 用户交互：显示借书对话框，让用户选择具体副本和归还日期
    BorrowDialog dialog(*book, availableCopies, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;  // 用户取消借书
    }

    // 获取用户选择的副本和归还日期
    BookCopy selectedCopy = dialog.getSelectedCopy();
    QDate dueDate = dialog.getDueDate();

    // 业务处理：执行借书操作
    QString error;
    if (library_.borrowBook(indexId, currentUsername_, dueDate, &error)) {
        refreshTable();  // 刷新表格显示
        QMessageBox::information(this, "成功",
            QStringLiteral("成功借阅《%1》的副本%2，归还日期：%3")
            .arg(bookName, QString::number(selectedCopy.copyNumber), dueDate.toString("yyyy-MM-dd")));
    } else {
        QMessageBox::warning(this, "失败", "借阅失败：" + error);
    }
}

/**
 * @brief 还书功能实现
 *
 * 功能流程：
 * 1. 权限验证：只有学生用户可以还书，管理员不能还书
 * 2. 数据获取：获取当前用户所有已借的副本
 * 3. 空值检查：检查用户是否有借阅的图书
 * 4. 智能排序：按到期日期排序，最先到期的排在前面
 * 5. 状态显示：为每个借阅记录计算剩余天数或已过期天数
 * 6. 用户选择：显示借阅列表，让用户选择要归还的图书
 * 7. 确认操作：显示确认对话框，防止误操作
 * 8. 业务处理：调用LibraryManager的returnBook方法执行还书操作
 * 9. 界面更新：刷新表格显示，显示还书成功/失败信息
 *
 * 涉及的函数：
 * - library_.getUserBorrowedCopies(): 获取用户已借副本
 * - library_.findByIndexId(): 查找图书信息
 * - library_.returnBook(): 执行还书操作
 * - refreshTable(): 刷新表格显示
 * - std::sort(): 对借阅记录按到期日期排序
 */
void MainWindow::onReturn()
{
    // 权限验证：只有学生用户可以还书
    if (currentUsername_.isEmpty() || isAdminMode_) {
        QMessageBox::warning(this, "还书失败", "只有学生用户可以还书，请使用学生账号登录。");
        return;
    }

    // 数据获取：获取当前用户所有已借的副本
    QVector<BookCopy> borrowedCopies = library_.getUserBorrowedCopies(currentUsername_);
    if (borrowedCopies.isEmpty()) {
        QMessageBox::information(this, "提示", "你当前没有借阅任何图书！");
        return;
    }

    // 智能排序：按到期日期排序，最先到期的排在前面，方便用户优先归还紧急的图书
    std::sort(borrowedCopies.begin(), borrowedCopies.end(), [](const BookCopy &a, const BookCopy &b) {
        return a.dueDate < b.dueDate;
    });

    // 状态显示：为每个借阅记录创建显示文本，包含状态信息
    QStringList copyNames;
    for (const BookCopy &copy : borrowedCopies) {
        const Book *book = library_.findByIndexId(copy.indexId);
        if (book) {
            QString statusText;
            QDate currentDate = QDate::currentDate();

            // 计算剩余天数或已过期天数
            if (copy.dueDate < currentDate) {
                statusText = QStringLiteral(" (已过期 %1 天)")
                              .arg(currentDate.daysTo(copy.dueDate));
            } else {
                statusText = QStringLiteral(" (剩余 %1 天)")
                              .arg(currentDate.daysTo(copy.dueDate));
            }

            // 创建显示文本，包含书名、副本号、应还日期和状态
            copyNames.append(QStringLiteral("《%1》 - 副本%2 (应还: %3)%4")
                            .arg(book->name)
                            .arg(copy.copyNumber)
                            .arg(copy.dueDate.toString("yyyy-MM-dd"))
                            .arg(statusText));
        }
    }

    // 用户选择：显示借阅列表，让用户选择要归还的图书
    bool ok;
    QString selectedCopy = QInputDialog::getItem(this, "还书", "请选择要归还的图书:",
                                               copyNames, 0, false, &ok);

    if (!ok || selectedCopy.isEmpty()) {
        return;  // 用户取消还书
    }

    // 获取用户选择的副本信息
    int selectedIndex = copyNames.indexOf(selectedCopy);
    if (selectedIndex < 0) return;

    const BookCopy &selectedCopyObj = borrowedCopies[selectedIndex];
    const Book *book = library_.findByIndexId(selectedCopyObj.indexId);

    // 确认操作：显示确认对话框，防止误操作
    auto reply = QMessageBox::question(this, "确认还书",
                                       QStringLiteral("确定要归还《%1》的副本%2吗？\n应还日期：%3")
                                       .arg(book->name)
                                       .arg(selectedCopyObj.copyNumber)
                                       .arg(selectedCopyObj.dueDate.toString("yyyy-MM-dd")),
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No) {
        return;  // 用户取消操作
    }

    // 业务处理：执行还书操作
    QString error;
    if (library_.returnBook(selectedCopyObj.copyId, currentUsername_, &error)) {
        refreshTable();  // 刷新表格显示
        QMessageBox::information(this, "还书成功",
                                 QStringLiteral("成功归还《%1》的副本%2\n感谢您的使用！")
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
    // 清除搜索状态
    isSearchActive_ = false;
    currentSearchKeyword_.clear();
    currentSearchMode_.clear();
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

/**
 * @brief 搜索功能槽函数
 *
 * 功能说明：
 * 1. 搜索状态检查：确保搜索控件已初始化
 * 2. 关键词获取：获取用户输入的搜索关键词并去除空格
 * 3. 空关键词处理：如果关键词为空，显示所有图书并清除搜索状态
 * 4. 搜索模式获取：获取当前选择的搜索模式（书名/作者/索引号/出版社）
 * 5. 搜索执行：调用模糊搜索功能执行搜索
 * 6. 搜索状态设置：标记为搜索状态并保存搜索参数
 * 7. UI更新：更新表格显示搜索结果
 * 8. 状态更新：更新表头显示搜索状态
 *
 * 搜索特性：
 * - 支持空搜索：自动显示所有图书
 * - 支持多种搜索模式：书名、作者、索引号、出版社
 * - 支持模糊匹配：非索引号模式支持模糊搜索
 * - 搜索状态保持：显示搜索结果时保持筛选和排序状态
 * - 性能优化：搜索期间禁用搜索按钮防止重复搜索
 */
void MainWindow::onSearch()
{
    // 搜索控件检查：确保搜索控件已正确初始化
    if (!searchEdit_ || !searchModeComboBox_) {
        qDebug() << "Search widgets not initialized";
        return;
    }

    // 关键词获取和预处理：获取用户输入并去除首尾空格
    QString keyword = searchEdit_->text().trimmed();
    qDebug() << "Search keyword:" << keyword;

    // 空关键词处理：如果关键词为空，显示全部图书并清除搜索状态
    if (keyword.isEmpty()) {
        qDebug() << "Empty keyword, showing all";
        onShowAll();                    // 显示所有图书

        // 清除搜索状态：重置搜索相关变量
        isSearchActive_ = false;        // 标记非搜索状态
        currentSearchKeyword_.clear();  // 清空搜索关键词
        currentSearchMode_.clear();     // 清空搜索模式
        return;
    }

    // 搜索模式获取：从下拉框获取当前选择的搜索模式
    QString searchMode = searchModeComboBox_->currentData().toString();
    qDebug() << "Search mode:" << searchMode;

    // 搜索状态保存：保存当前搜索参数，用于刷新时维持搜索状态
    isSearchActive_ = true;                    // 标记为搜索状态
    currentSearchKeyword_ = keyword;           // 保存搜索关键词
    currentSearchMode_ = searchMode;           // 保存搜索模式

    // 搜索执行控制：禁用搜索按钮防止重复点击
    searchButton_->setEnabled(false);

    // 执行搜索：调用模糊搜索功能进行实际的搜索操作
    performFuzzySearch(keyword, searchMode);

    // 搜索完成恢复：重新启用搜索按钮
    searchButton_->setEnabled(true);

    qDebug() << "Search completed";
}

/**
 * @brief 搜索模式切换槽函数
 *
 * 功能说明：
 * 1. 模式检测：检测搜索控件是否已初始化
 * 2. 模式获取：获取当前选择的搜索模式
 * 3. 占位符设置：根据搜索模式设置相应的提示文本
 * 4. 用户体验优化：为不同搜索模式提供针对性的提示
 *
 * 搜索模式说明：
 * - name模式：按图书名称搜索，支持模糊匹配
 * - indexId模式：按索引号搜索，支持精确匹配和副本号搜索
 * - author模式：按作者搜索，支持模糊匹配
 * - publisher模式：按出版社搜索，支持模糊匹配
 *
 * 用户体验优化：
 * 动态更新搜索框的占位符文本，为用户提供清晰的输入提示
 */
void MainWindow::onSearchModeChanged()
{
    // 控件检查：确保搜索控件已正确初始化
    if (!searchModeComboBox_ || !searchEdit_)
        return;

    // 搜索模式获取：从下拉框获取当前选择的搜索模式
    QString searchMode = searchModeComboBox_->currentData().toString();

    // 占位符文本设置：根据搜索模式设置相应的提示文本
    QString placeholderText;

    // 根据不同的搜索模式设置对应的占位符文本
    if (searchMode == "name") {
        placeholderText = "🔍 搜索图书名称...";  // 书名搜索提示
    } else if (searchMode == "indexId") {
        placeholderText = "🔍 搜索索引号（支持副本号，如 CS001_1）...";  // 索引号搜索提示
    } else if (searchMode == "author") {
        placeholderText = "🔍 搜索作者...";  // 作者搜索提示
    } else if (searchMode == "publisher") {
        placeholderText = "🔍 搜索出版社...";  // 出版社搜索提示
    } else {
        placeholderText = "🔍 输入搜索关键词...";  // 默认搜索提示
    }

    // 应用占位符文本：更新搜索框的提示文本
    searchEdit_->setPlaceholderText(placeholderText);
}

/**
 * @brief 图书数据导入功能
 *
 * 功能说明：
 * 1. 文件选择：弹出文件选择对话框，让用户选择要导入的JSON文件
 * 2. 数据验证：检查选择的文件路径是否有效
 * 3. 数据导入：调用LibraryManager导入JSON数据
 * 4. 界面更新：重新构建筛选菜单并刷新表格显示
 * 5. 结果反馈：显示导入成功或失败的提示消息
 *
 * 使用场景：
 * - 初始化系统时导入图书数据
 * - 从其他系统迁移数据时使用
 * - 备份数据恢复时使用
 *
 * 错误处理：
 * - 文件路径为空时直接返回
 * - 文件格式错误时显示失败消息
 * - 导入失败时保持原有数据不变
 */
void MainWindow::onOpen()
{
    // 文件选择：弹出文件对话框让用户选择JSON文件
    QString path = QFileDialog::getOpenFileName(this, "导入图书数据", "", "JSON Files (*.json)");

    // 路径验证：检查用户是否选择了文件
    if (!path.isEmpty()) {
        // 数据导入：尝试从JSON文件导入图书数据
        if (library_.importFromJson(path)) {
            // 界面更新：导入成功后更新相关UI组件
            rebuildFilterMenus();  // 重新构建筛选菜单，反映新数据中的类别
            refreshTable();        // 刷新表格显示新导入的数据
            QMessageBox::information(this, "成功", "数据导入成功！");
        } else {
            // 错误处理：导入失败时显示错误消息
            QMessageBox::warning(this, "失败", "文件导入失败！");
        }
    }
}

/**
 * @brief 图书数据导出功能
 *
 * 功能说明：
 * 1. 文件选择：弹出保存文件对话框，让用户选择保存位置和文件名
 * 2. 路径验证：检查用户是否输入了有效的保存路径
 * 3. 数据导出：调用LibraryManager将当前数据导出为JSON格式
 * 4. 结果反馈：显示导出成功或失败的提示消息
 *
 * 使用场景：
 * - 数据备份：定期备份图书数据防止丢失
 * - 数据迁移：将数据导出到其他系统
 * - 数据分析：导出数据进行统计分析
 * - 数据共享：与其他图书馆分享图书信息
 *
 * 导出特点：
 * - 默认文件名：library_export.json
 * - 完整数据：包含所有图书和副本信息
 * - 标准格式：使用JSON格式，便于其他程序读取
 *
 * 错误处理：
 * - 文件路径为空时直接返回
 * - 文件写入权限不足时显示失败消息
 * - 磁盘空间不足时显示失败消息
 */
void MainWindow::onSave()
{
    // 文件选择：弹出保存文件对话框，提供默认文件名
    QString path = QFileDialog::getSaveFileName(this, "导出图书数据", "library_export.json", "JSON Files (*.json)");

    // 路径验证：检查用户是否选择了保存位置
    if (!path.isEmpty()) {
        // 数据导出：尝试将当前图书数据导出到JSON文件
        if (library_.exportToJson(path)) {
            // 成功反馈：显示导出成功消息
            QMessageBox::information(this, "成功", "数据导出成功！");
        } else {
            // 错误处理：导出失败时显示错误消息
            QMessageBox::warning(this, "失败", "文件导出失败！");
        }
    }
}

/**
 * @brief 导入图书数据别名函数
 *
 * 功能说明：
 * 这是一个便利函数，直接调用onOpen()函数来保持UI命名的一致性
 * 工具栏按钮使用"导入"标签，对应onImport槽函数
 * 实际功能与onOpen()完全相同
 *
 * 设计目的：
 * - 提高UI文本的语义清晰度
 * - 保持代码结构的统一性
 * - 便于理解和维护
 */
void MainWindow::onImport()
{
    onOpen();  // 直接调用导入功能的实现函数
}

/**
 * @brief 导出图书数据别名函数
 *
 * 功能说明：
 * 这是一个便利函数，直接调用onSave()函数来保持UI命名的一致性
 * 工具栏按钮使用"导出"标签，对应onExport槽函数
 * 实际功能与onSave()完全相同
 *
 * 设计目的：
 * - 提高UI文本的语义清晰度
 * - 保持代码结构的统一性
 * - 便于理解和维护
 */
void MainWindow::onExport()
{
    onSave();  // 直接调用导出功能的实现函数
}

/**
 * @brief 数据刷新功能
 *
 * 功能说明：
 * 1. 数据重新加载：从数据库文件重新加载所有图书数据
 * 2. 菜单重建：重新构建筛选菜单，反映最新的数据分布
 * 3. 界面刷新：更新表格显示，确保界面与数据同步
 * 4. 用户反馈：显示刷新成功的提示消息
 *
 * 使用场景：
 * - 数据文件被外部程序修改后需要重新加载
 * - 系统运行时间较长后需要刷新数据状态
 * - 多用户操作时需要同步最新数据
 * - 排查数据相关问题时需要重置数据状态
 *
 * 刷新特点：
 * - 完整重载：不仅刷新显示，还重新从文件加载数据
 * - 状态同步：确保内存中的数据与文件中的数据完全一致
 * - UI同步：同时更新筛选菜单和表格显示
 *
 * 注意事项：
 * - 此操作会丢失当前未保存的任何修改
 * - 刷新过程中可能会有短暂的界面停顿
 * - 大量数据时刷新时间可能较长
 */
void MainWindow::onRefresh()
{
    // 数据重新加载：从数据库文件重新读取所有图书和副本数据
    library_.loadFromDatabase();

    // 菜单重建：重新构建筛选菜单，更新类别、位置等筛选选项
    rebuildFilterMenus();

    // 界面刷新：更新表格显示，反映最新的数据状态
    refreshTable();

    // 用户反馈：显示刷新成功提示
    QMessageBox::information(this, "成功", "数据已刷新！");
}

/**
 * @brief 类别筛选变更处理函数
 *
 * 功能说明：
 * 1. 参数验证：检查传入的动作对象是否有效
 * 2. 筛选值获取：从动作对象中获取类别筛选条件
 * 3. 状态更新：更新当前类别筛选条件
 * 4. 界面刷新：应用新的筛选条件并更新表格显示
 *
 * 筛选机制：
 * - 动作对象的data()存储了具体的类别值
 * - 空字符串表示"全部"，不进行类别筛选
 * - 非空字符串表示特定的图书类别（如"计算机科学"、"文学"等）
 *
 * 触发场景：
 * - 用户点击了类别筛选菜单中的某个选项
 * - 程序代码中调用了相应动作的trigger()方法
 *
 * 数据流：
 * QAction -> 获取data() -> 更新categoryFilter_ -> refreshTable() -> 更新显示
 */
void MainWindow::onCategoryFilterChanged(QAction *action)
{
    // 参数验证：确保传入的动作对象有效
    if (action) {
        // 筛选条件更新：获取并更新当前类别筛选条件
        categoryFilter_ = action->data().toString();

        // 界面刷新：应用新的筛选条件并更新表格显示
        refreshTable();
    }
}

/**
 * @brief 状态筛选变更处理函数
 *
 * 功能说明：
 * 1. 参数验证：检查传入的动作对象是否有效
 * 2. 筛选值获取：从动作对象中获取状态筛选条件
 * 3. 状态更新：更新当前状态筛选条件
 * 4. 界面刷新：应用新的筛选条件并更新表格显示
 *
 * 筛选类型：
 * - 空字符串或"all"：显示全部图书，不进行状态筛选
 * - "available"：仅显示有可用副本的图书
 * - "borrowed"：仅显示所有副本都被借出的图书
 *
 * 业务逻辑：
 * - available筛选：图书至少有一个副本可借
 * - borrowed筛选：图书所有副本都被借出
 * - 全部显示：不考虑图书的可借状态
 *
 * 数据流：
 * QAction -> 获取data() -> 更新statusFilter_ -> refreshTable() -> 更新显示
 */
void MainWindow::onStatusFilterChanged(QAction *action)
{
    // 参数验证：确保传入的动作对象有效
    if (action) {
        // 筛选条件更新：获取并更新当前状态筛选条件
        statusFilter_ = action->data().toString();

        // 界面刷新：应用新的筛选条件并更新表格显示
        refreshTable();
    }
}

/**
 * @brief 位置筛选变更处理函数
 *
 * 功能说明：
 * 1. 参数验证：检查传入的动作对象是否有效
 * 2. 筛选值获取：从动作对象中获取位置筛选条件
 * 3. 状态更新：更新当前位置筛选条件
 * 4. 界面刷新：应用新的筛选条件并更新表格显示
 *
 * 筛选选项：
 * - 空字符串或"all"：显示全部位置的图书
 * - "三牌楼图书馆"：仅显示三牌楼校区的图书
 * - "仙林图书馆"：仅显示仙林校区的图书
 * - 其他位置：根据实际数据中的location字段动态生成
 *
 * 地理信息：
 * - 支持多校区图书馆管理
 * - 便于用户按位置查找和管理图书
 * - 统计分析各校区的藏书分布
 *
 * 数据流：
 * QAction -> 获取data() -> 更新locationFilter_ -> refreshTable() -> 更新显示
 */
void MainWindow::onLocationFilterChanged(QAction *action)
{
    // 参数验证：确保传入的动作对象有效
    if (action) {
        // 筛选条件更新：获取并更新当前位置筛选条件
        locationFilter_ = action->data().toString();

        // 界面刷新：应用新的筛选条件并更新表格显示
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

    // 搜索方式选择下拉框
    searchModeComboBox_ = new QComboBox();
    searchModeComboBox_->addItem("书名搜索", "name");
    searchModeComboBox_->addItem("索引号搜索", "indexId");
    searchModeComboBox_->addItem("全文搜索", "all");
    searchModeComboBox_->setMinimumWidth(100);
    searchModeComboBox_->setToolTip("选择搜索方式");

    searchEdit_ = new QLineEdit();
    searchEdit_->setPlaceholderText("🔍 输入搜索关键词...");

    searchButton_ = new QPushButton("搜索");

    themeToggleButton_ = new QPushButton("🌙");
    themeToggleButton_->setToolTip("切换深浅色模式");

    searchLayout->addWidget(searchModeComboBox_);
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
    connect(searchModeComboBox_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onSearchModeChanged);
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

    // 更新左侧功能栏容器的背景色
    if (toolBarScrollArea_) {
        QWidget *toolContainer = toolBarScrollArea_->widget();
        if (toolContainer) {
            toolContainer->setStyleSheet(QString("background-color: %1;").arg(isDark ? "#22333B" : "#FEEFF1"));
        }
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

/**
 * @brief 重建筛选和排序菜单
 *
 * 功能流程：
 * 1. 清理资源：删除现有的菜单和动作组，释放内存
 * 2. 类别筛选：创建类别筛选菜单，包含所有图书类别
 * 3. 状态筛选：创建状态筛选菜单（全部、仅可借、仅不可借）
 * 4. 位置筛选：创建位置筛选菜单（全部、三牌楼、仙林）
 * 5. 排序选项：创建排序菜单（默认、按借阅次数排序）
 * 6. 动态数据：根据实际图书数据动态生成筛选选项
 * 7. 状态保持：保持当前的筛选状态和选中状态
 *
 * 涉及的函数：
 * - QMenu: Qt菜单类，用于创建下拉菜单
 * - QActionGroup: Qt动作组，用于实现单选功能
 * - connect(): 连接信号槽，处理用户选择事件
 * - addSeparator(): 添加分隔线
 */
void MainWindow::rebuildFilterMenus()
{
    // 清理资源：删除现有的菜单和动作组，释放内存
    delete categoryFilterMenu_;
    delete statusFilterMenu_;
    delete locationFilterMenu_;
    delete sortMenu_;
    delete categoryActionGroup_;
    delete statusActionGroup_;
    delete locationActionGroup_;
    delete sortActionGroup_;

    // 类别筛选菜单：创建新的类别筛选菜单
    categoryFilterMenu_ = new QMenu(this);
    categoryFilterMenu_->setMinimumWidth(200);  // 设置最小宽度，避免文字被截断
    categoryActionGroup_ = new QActionGroup(categoryFilterMenu_);
    categoryActionGroup_->setExclusive(true);  // 设置为单选模式

    // 使用Lambda函数简化菜单项创建逻辑
    auto addCategoryAction = [this](const QString &label, const QString &value, bool separator = false) {
        if (separator) {
            categoryFilterMenu_->addSeparator();  // 添加分隔线
            return static_cast<QAction *>(nullptr);
        }
        QAction *action = categoryFilterMenu_->addAction(label);
        action->setCheckable(true);  // 设置为可选中
        action->setData(value);      // 存储筛选值
        categoryActionGroup_->addAction(action);
        // 保持当前的选中状态
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
    QAction *borrowedAction = addStatusAction(QStringLiteral("仅不可借"), QStringLiteral("borrowed"));

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

/**
 * @brief 更新表格表头显示当前筛选状态
 *
 * 功能说明：
 * 1. 状态检查：验证表格模型是否已初始化
 * 2. 动态更新：根据当前筛选条件更新表头文本
 * 3. 用户提示：在表头中显示当前应用的筛选条件
 * 4. 视觉反馈：让用户清楚地看到当前的筛选和排序状态
 *
 * 表头更新规则：
 * - 默认状态：显示基础列名和▼符号（表示可点击）
 * - 筛选状态：显示"列名 ▼\n[筛选条件]"
 * - 排序状态：显示"列名 ▼\n[排序方式]"
 * - 箭头符号：▼表示可点击筛选或排序，增强交互性
 *
 * 更新的表头列对应关系：
 * - 第4列（索引4）：馆藏地址，支持位置筛选
 * - 第5列（索引5）：类别，支持类别筛选
 * - 第10列（索引10）：借阅次数，支持排序方式显示
 * - 第11列（索引11）：状态，支持可用性筛选
 *
 * 用户体验特点：
 * - 一目了然：用户可以立即看到当前应用的筛选条件
 * - 交互提示：▼符号提示用户可以点击表头进行操作
 * - 信息紧凑：在表头中既显示列名又显示筛选/排序状态
 * - 实时更新：每次刷新表格时都会更新表头显示
 *
 * 设计目的：
 * 减少界面冗余，将筛选和排序状态信息集成到表头中，提高界面简洁性和信息密度
 */
void MainWindow::updateHeaderLabels()
{
    // 模型验证：确保表格模型存在且可用
    if (!model_)
        return;

    // 类别表头更新：显示当前类别筛选条件（第5列）
    QString categoryLabel = QStringLiteral("类别 ▼");  // 默认显示
    if (!categoryFilter_.isEmpty()) {
        // 有类别筛选时：显示具体类别名称
        categoryLabel = QStringLiteral("类别 ▼\n%1").arg(categoryFilter_);
    }
    model_->setHeaderData(5, Qt::Horizontal, categoryLabel);

    // 位置表头更新：显示当前位置筛选条件（第4列）
    QString locationLabel = QStringLiteral("馆藏地址 ▼");  // 默认显示
    if (!locationFilter_.isEmpty()) {
        // 有位置筛选时：显示具体位置名称
        locationLabel = QStringLiteral("馆藏地址 ▼\n%1").arg(locationFilter_);
    }
    model_->setHeaderData(4, Qt::Horizontal, locationLabel);

    // 状态表头更新：显示当前可用性筛选条件（第11列）
    QString statusLabel = QStringLiteral("状态 ▼");  // 默认显示
    if (statusFilter_ == "available") {
        statusLabel = QStringLiteral("状态 ▼\n可借");      // 仅显示可借图书
    } else if (statusFilter_ == "borrowed") {
        statusLabel = QStringLiteral("状态 ▼\n不可借");   // 仅显示不可借图书
    }
    model_->setHeaderData(11, Qt::Horizontal, statusLabel);

    // 借阅次数表头更新：显示当前排序方式（第10列）
    QString borrowCountLabel = QStringLiteral("借阅次数 ▼");  // 默认显示
    if (currentSortType_ == "borrowCount") {
        borrowCountLabel = QStringLiteral("借阅次数 ▼\n热门排序");    // 按借阅次数排序
    } else if (currentSortType_ == "default") {
        borrowCountLabel = QStringLiteral("借阅次数 ▼\n默认排序");    // 默认排序方式
    }
    model_->setHeaderData(10, Qt::Horizontal, borrowCountLabel);
}

/**
 * @brief 表头点击事件处理函数
 *
 * 功能说明：
 * 1. 列识别：根据点击的列索引确定对应的操作类型
 * 2. 菜单关联：将特定列与其对应的筛选菜单关联
 * 3. 筛选触发：点击不同的表头显示相应的筛选选项
 * 4. 用户交互：提供直观的表头点击筛选功能
 *
 * 可点击的表头列及其功能：
 * - 第4列（索引4）：馆藏地址表头 → 位置筛选菜单
 * - 第5列（索引5）：类别表头 → 类别筛选菜单
 * - 第10列（索引10）：借阅次数表头 → 排序方式菜单
 * - 第11列（索引11）：状态表头 → 可用性筛选菜单
 *
 * 交互流程：
 * 用户点击表头 → 识别列索引 → 显示对应筛选菜单 → 用户选择筛选条件 → 应用筛选并刷新表格
 *
 * 设计特点：
 * - 直观操作：用户直接点击感兴趣的表头进行筛选
 * - 上下文关联：不同列显示与其内容相关的筛选选项
 * - 即时反馈：筛选结果立即在表格中显示
 *
 * 用户体验：
 * 降低学习成本，用户无需在菜单栏中寻找筛选选项，直接点击表头即可
 */
void MainWindow::onHeaderSectionClicked(int section)
{
    // 位置筛选：点击馆藏地址表头时显示位置筛选菜单
    if (section == 4) {
        showFilterMenu(locationFilterMenu_, section);
    }
    // 类别筛选：点击类别表头时显示类别筛选菜单
    else if (section == 5) {
        showFilterMenu(categoryFilterMenu_, section);
    }
    // 排序功能：点击借阅次数表头时显示排序菜单
    else if (section == 10) {
        showFilterMenu(sortMenu_, section);
    }
    // 状态筛选：点击状态表头时显示可用性筛选菜单
    else if (section == 11) {
        showFilterMenu(statusFilterMenu_, section);
    }
}

/**
 * @brief 在指定表头位置显示筛选菜单
 *
 * 功能说明：
 * 1. 参数验证：检查菜单和表格控件是否有效
 * 2. 位置计算：计算表头列的精确屏幕坐标
 * 3. 菜单定位：将筛选菜单精确定位到表头下方
 * 4. 菜单显示：弹出筛选菜单供用户选择
 *
 * 坐标计算逻辑：
 * 1. 获取表头控件引用
 * 2. 计算指定列的视图坐标（sectionViewportPosition）
 * 3. 获取列宽和表头高度
 * 4. 转换为全局屏幕坐标（mapToGlobal）
 * 5. 定位菜单到表头列的底部左角
 *
 * 技术实现：
 * - QHeaderView：Qt的表头控件类
 * - sectionViewportPosition：获取列在视图中的位置
 * - mapToGlobal：将控件坐标转换为屏幕坐标
 * - popup：在指定位置显示菜单
 *
 * 视觉效果：
 * 菜单从点击的表头正下方弹出，与表头列宽度对齐，提供清晰的操作关联
 *
 * @param menu 要显示的筛选菜单指针
 * @param section 表头列索引
 */
void MainWindow::showFilterMenu(QMenu *menu, int section)
{
    // 参数验证：确保菜单和表格控件存在且可用
    if (!menu || !tableView_)
        return;

    // 获取表头控件引用，用于计算坐标
    QHeaderView *header = tableView_->horizontalHeader();

    // 坐标计算：计算表头列的位置和尺寸
    int x = header->sectionViewportPosition(section);  // 列的X坐标
    int width = header->sectionSize(section);          // 列宽度
    int height = header->height();                     // 表头高度

    // 创建矩形区域：表示表头列的几何区域
    QRect sectionRect(x, 0, width, height);

    // 坐标转换：将表头底部左角转换为全局屏幕坐标
    QPoint globalPos = header->viewport()->mapToGlobal(sectionRect.bottomLeft());

    // 菜单显示：在计算出的位置弹出筛选菜单
    menu->popup(globalPos);
}

/**
 * @brief 在表格中显示指定的图书列表
 *
 * 功能说明：
 * 1. 表格清空：清除表格中的所有现有行数据
 * 2. 数据填充：逐行遍历图书数据并填充到表格中
 * 3. 副本统计：计算并显示每本书的副本状态
 * 4. 用户个性化：根据当前用户显示个性化信息
 * 5. 状态更新：更新状态栏统计信息
 *
 * 表格列结构（按添加顺序）：
 * - 第0列：索引号（indexId）
 * - 第1列：书名（name）
 * - 第2列：作者（author）
 * - 第3列：出版社（publisher）
 * - 第4列：馆藏地址（location）
 * - 第5列：类别（category）
 * - 第6列：总副本数（totalCopies）
 * - 第7列：价格（price，保留2位小数）
 * - 第8列：入库日期（inDate，yyyy-MM-dd格式）
 * - 第9列：归还日期（returnDate，仅学生可见）
 * - 第10列：借阅次数（borrowCount）
 * - 第11列：状态（可借/不可借）
 *
 * 个性化显示逻辑：
 * - 学生用户：可以看到自己的借阅归还日期
 * - 管理员用户：归还日期列显示为空
 * - 状态计算：根据副本可用性动态计算显示状态
 *
 * 数据完整性：
 * - 副本统计：实时计算总副本数和可用副本数
 * - 借阅状态：动态判断图书是否可借
 * - 用户关联：显示当前用户的借阅信息
 *
 * @param booksToShow 要在表格中显示的图书列表
 */
void MainWindow::displayBooks(const QVector<Book> &booksToShow)
{
    // 表格清空：移除所有现有行，准备重新填充数据
    model_->removeRows(0, model_->rowCount());

    // 数据填充：逐行处理图书数据
    for (int row = 0; row < booksToShow.size(); ++row) {
        const Book &b = booksToShow[row];  // 获取当前图书引用
        QList<QStandardItem *> rowItems;   // 创建表格行数据容器

        // 基础信息列：图书的核心属性信息
        rowItems << new QStandardItem(b.indexId);                                    // 索引号
        rowItems << new QStandardItem(b.name);                                      // 书名
        rowItems << new QStandardItem(b.author);                                    // 作者
        rowItems << new QStandardItem(b.publisher);                                 // 出版社
        rowItems << new QStandardItem(b.location);                                  // 馆藏地址
        rowItems << new QStandardItem(b.category);                                  // 类别

        // 副本信息列：图书的数量和价格信息
        int totalCopies = library_.getTotalCopyCount(b.indexId);                    // 计算总副本数
        rowItems << new QStandardItem(QString::number(totalCopies));                // 副本数量

        rowItems << new QStandardItem(QString::number(b.price, 'f', 2));           // 价格，保留2位小数
        rowItems << new QStandardItem(b.inDate.toString("yyyy-MM-dd"));             // 入库日期

        // 归还日期列：个性化显示，仅对学生用户显示其借阅信息
        QString returnDateStr = "";
        if (!currentUsername_.isEmpty() && !isAdminMode_) {                         // 学生用户条件判断
            // 获取当前学生的所有借阅副本
            QVector<BookCopy> borrowedCopies = library_.getUserBorrowedCopies(currentUsername_);
            // 查找该书的借阅记录
            for (const BookCopy &copy : borrowedCopies) {
                if (copy.indexId == b.indexId) {                                   // 找到该书的借阅记录
                    returnDateStr = copy.dueDate.toString("yyyy-MM-dd");            // 显示归还日期
                    break;                                                          // 找到一个即可（正常情况下学生不会重复借阅同一本书）
                }
            }
        }
        rowItems << new QStandardItem(returnDateStr);                               // 归还日期（学生可见）

        // 统计信息列：借阅次数和状态信息
        rowItems << new QStandardItem(QString::number(b.borrowCount));              // 借阅次数

        // 状态列：根据副本可用性动态计算状态
        int availableCopies = library_.getAvailableCopyCount(b.indexId);            // 计算可用副本数
        QString statusText = (availableCopies > 0) ? QStringLiteral("可借") : QStringLiteral("不可借");
        rowItems << new QStandardItem(statusText);                                  // 状态

        // 行数据添加：将完整行数据添加到表格模型中
        model_->appendRow(rowItems);
    }

    // 状态栏更新：更新统计信息显示当前数据状态
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

    QVector<BookCopy> borrowedCopies = library_.getUserBorrowedCopies(currentUsername_);
    if (borrowedCopies.isEmpty()) {
        QMessageBox::information(this, "我的借阅", "你当前没有借阅任何图书！");
        return;
    }

    // 按到期日期排序
    std::sort(borrowedCopies.begin(), borrowedCopies.end(), [](const BookCopy &a, const BookCopy &b) {
        return a.dueDate < b.dueDate; // 最先到期的排在前面
    });

    QString borrowText = QStringLiteral("📚 我的借阅记录 (共 %1 本)\n\n").arg(borrowedCopies.size());

    for (const BookCopy &copy : borrowedCopies) {
        const Book *book = library_.findByIndexId(copy.indexId);
        if (book) {
            QString statusIcon;
            QDate currentDate = QDate::currentDate();
            int daysDiff = currentDate.daysTo(copy.dueDate);

            if (daysDiff < 0) {
                statusIcon = "🔴"; // 已过期
            } else if (daysDiff <= 3) {
                statusIcon = "🟡"; // 即将到期
            } else {
                statusIcon = "🟢"; // 正常
            }

            borrowText += QStringLiteral("%1 《%2》\n")
                           .arg(statusIcon)
                           .arg(book->name);
            borrowText += QStringLiteral("   📖 索引号：%1 | 副本：%2\n")
                           .arg(copy.indexId)
                           .arg(copy.copyNumber);
            borrowText += QStringLiteral("   📅 借出：%1 | 应还：%2\n")
                           .arg(copy.borrowDate.toString("yyyy-MM-dd"))
                           .arg(copy.dueDate.toString("yyyy-MM-dd"));

            if (daysDiff < 0) {
                borrowText += QStringLiteral("   ⚠️ 已过期 %1 天！请尽快归还\n")
                               .arg(-daysDiff);
            } else if (daysDiff <= 3) {
                borrowText += QStringLiteral("   ⏰ 剩余 %1 天，即将到期\n")
                               .arg(daysDiff);
            } else {
                borrowText += QStringLiteral("   ✅ 剩余 %1 天\n")
                               .arg(daysDiff);
            }

            borrowText += QStringLiteral("   📋 副本ID：%1\n\n")
                           .arg(copy.copyId);
        }
    }

    QMessageBox::information(this, "我的借阅", borrowText);
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

    // 获取所有副本
    QVector<BookCopy> allCopies = library_.getBookCopies(indexId);
    if (allCopies.isEmpty()) {
        QMessageBox::information(this, "借阅记录", "该图书暂无副本信息。");
        return;
    }

    QString historyText = QStringLiteral("📚 《%1》(索引号: %2) 借阅记录\n\n").arg(bookName, indexId);
    historyText += QStringLiteral("📊 副本总数：%1 本\n").arg(allCopies.size());

    int borrowedCount = 0;
    int availableCount = 0;
    QVector<BookCopy> borrowedCopies;

    for (const BookCopy &copy : allCopies) {
        if (copy.isAvailable()) {
            availableCount++;
        } else {
            borrowedCount++;
            borrowedCopies.append(copy);
        }
    }

    historyText += QStringLiteral("✅ 可借：%1 本\n").arg(availableCount);
    historyText += QStringLiteral("❌ 已借：%1 本\n\n").arg(borrowedCount);

    // 显示当前借阅详情
    if (!borrowedCopies.isEmpty()) {
        historyText += QStringLiteral("🔍 当前借阅详情：\n");
        for (const BookCopy &copy : borrowedCopies) {
            historyText += QStringLiteral("   📋 副本%1 (ID: %2)\n")
                           .arg(copy.copyNumber)
                           .arg(copy.copyId);
            historyText += QStringLiteral("   👤 借阅者：%1\n").arg(copy.borrowedBy);
            historyText += QStringLiteral("   📅 借出：%1 | 应还：%2\n")
                           .arg(copy.borrowDate.toString("yyyy-MM-dd"))
                           .arg(copy.dueDate.toString("yyyy-MM-dd"));

            QDate currentDate = QDate::currentDate();
            int daysDiff = currentDate.daysTo(copy.dueDate);
            if (daysDiff < 0) {
                historyText += QStringLiteral("   ⚠️ 已过期 %1 天！\n")
                               .arg(-daysDiff);
            } else if (daysDiff <= 3) {
                historyText += QStringLiteral("   ⏰ 剩余 %1 天，即将到期\n")
                               .arg(daysDiff);
            } else {
                historyText += QStringLiteral("   ✅ 剩余 %1 天\n")
                               .arg(daysDiff);
            }
            historyText += "\n";
        }
    }

    // 显示可用副本
    if (availableCount > 0) {
        historyText += QStringLiteral("✅ 可用副本列表：\n");
        for (const BookCopy &copy : allCopies) {
            if (copy.isAvailable()) {
                historyText += QStringLiteral("   📋 副本%1 (ID: %2) - 可借\n")
                               .arg(copy.copyNumber)
                               .arg(copy.copyId);
            }
        }
    }

    QMessageBox::information(this, "借阅记录", historyText);
}

// ============================================================================
// 搜索功能增强
// ============================================================================

/**
 * @brief 模糊搜索功能实现
 *
 * 功能说明：
 * 1. 搜索准备：清空现有表格，准备显示搜索结果
 * 2. 数据获取：获取所有图书数据作为搜索源
 * 3. 关键词预处理：转换为小写以实现不区分大小写的搜索
 * 4. 多模式搜索：根据搜索模式在相应字段中进行匹配
 * 5. 结果筛选：应用当前的筛选条件到搜索结果
 * 6. 排序应用：根据当前的排序设置对搜索结果排序
 * 7. 结果显示：将搜索结果显示在表格中
 *
 * 搜索模式支持：
 * - indexId: 在索引号中搜索，支持精确和模糊匹配
 * - name: 在书名中搜索，支持模糊匹配
 * - author: 在作者名中搜索，支持模糊匹配
 * - publisher: 在出版社中搜索，支持模糊匹配
 * - all: 在所有字段中搜索，实现全局搜索
 *
 * 搜索特性：
 * - 不区分大小写：使用小写比较实现大小写不敏感搜索
 * - 模糊匹配：使用contains函数实现部分匹配
 * - 多字段搜索：支持在不同字段中进行搜索
 * - 筛选保持：搜索结果会应用当前的筛选条件
 * - 排序保持：搜索结果会应用当前的排序设置
 *
 * @param keyword 搜索关键词
 * @param searchMode 搜索模式
 */
void MainWindow::performFuzzySearch(const QString &keyword, const QString &searchMode)
{
    qDebug() << "Starting search with keyword:" << keyword << "mode:" << searchMode;

    // 搜索准备：清空现有表格，准备显示搜索结果
    model_->removeRows(0, model_->rowCount());

    // 数据源获取：获取所有图书数据作为搜索范围
    QVector<Book> allBooks = library_.getAll();
    QVector<Book> matchedBooks;     // 存储匹配的图书

    // 关键词预处理：转换为小写以实现不区分大小写的搜索
    QString lowerKeyword = keyword.toLower();

    qDebug() << "Total books to search:" << allBooks.size();

    // 搜索匹配：遍历所有图书，根据搜索模式进行匹配
    for (const Book &book : allBooks) {
        bool match = false;

        // 根据搜索模式进行不同的匹配逻辑
        if (searchMode == "indexId") {
            // 索引号搜索：在索引号中进行匹配
            match = book.indexId.toLower().contains(lowerKeyword);
        } else if (searchMode == "name") {
            // 书名搜索：在书名中进行匹配
            match = book.name.toLower().contains(lowerKeyword);
        } else if (searchMode == "author") {
            // 作者搜索：在作者名中进行匹配
            match = book.author.toLower().contains(lowerKeyword);
        } else if (searchMode == "publisher") {
            // 出版社搜索：在出版社中进行匹配
            match = book.publisher.toLower().contains(lowerKeyword);
        } else if (searchMode == "all") {
            // 全局搜索：在所有字段中进行匹配
            match = (book.name.toLower().contains(lowerKeyword) ||
                    book.indexId.toLower().contains(lowerKeyword) ||
                    book.author.toLower().contains(lowerKeyword) ||
                    book.publisher.toLower().contains(lowerKeyword) ||
                    book.category.toLower().contains(lowerKeyword) ||
                    book.location.toLower().contains(lowerKeyword));
        }

        // 添加匹配结果：如果匹配则添加到结果列表
        if (match) {
            matchedBooks.append(book);
            qDebug() << "Found match:" << book.name << book.indexId;
        }
    }

    qDebug() << "Total matched books:" << matchedBooks.size();

    // 排序应用：根据当前的排序设置对搜索结果进行排序
    if (currentSortType_ == "borrowCount") {
        std::sort(matchedBooks.begin(), matchedBooks.end(), [](const Book &a, const Book &b) {
            return a.borrowCount > b.borrowCount; // 按借阅次数从高到低排序
        });
    }

    // 对搜索结果应用筛选条件并显示
    for (const Book &book : matchedBooks) {
        QList<QStandardItem *> rowItems;

        // 索引号列
        QStandardItem *indexItem = new QStandardItem(book.indexId);
        rowItems << indexItem;

        // 书名列
        QStandardItem *nameItem = new QStandardItem(book.name);
        rowItems << nameItem;

        // 先获取副本数量用于筛选条件判断
        int totalCopies = library_.getTotalCopyCount(book.indexId);
        int availableCopies = library_.getAvailableCopyCount(book.indexId);

        // 应用筛选条件
        if (!categoryFilter_.isEmpty() && book.category != categoryFilter_) {
            continue;
        }
        if (!locationFilter_.isEmpty() && book.location != locationFilter_) {
            continue;
        }
        // 修复筛选逻辑
        if (statusFilter_ == "available" && availableCopies <= 0) {
            continue; // 只显示有可用副本的图书
        }
        if (statusFilter_ == "borrowed" && availableCopies >= totalCopies) {
            continue; // 只显示没有可用副本的图书（即全部被借走）
        }

        // 其他列
        rowItems << new QStandardItem(book.author);
        rowItems << new QStandardItem(book.publisher);
        rowItems << new QStandardItem(book.location);
        rowItems << new QStandardItem(book.category);
        rowItems << new QStandardItem(QString::number(totalCopies));

        rowItems << new QStandardItem(QString::number(book.price, 'f', 2));
        rowItems << new QStandardItem(book.inDate.toString("yyyy-MM-dd"));

        // 归还日期：根据当前用户显示
        QString returnDateStr = "";
        if (!currentUsername_.isEmpty() && !isAdminMode_) {
            QVector<BookCopy> borrowedCopies = library_.getUserBorrowedCopies(currentUsername_);
            for (const BookCopy &copy : borrowedCopies) {
                if (copy.indexId == book.indexId) {
                    returnDateStr = copy.dueDate.toString("yyyy-MM-dd");
                    break;
                }
            }
        }
        rowItems << new QStandardItem(returnDateStr);

        rowItems << new QStandardItem(QString::number(book.borrowCount));

        QString statusText = (availableCopies > 0) ? QStringLiteral("可借") : QStringLiteral("不可借");
        rowItems << new QStandardItem(statusText);

        model_->appendRow(rowItems);
    }

    QString resultText = QStringLiteral("找到 %1 本匹配的图书").arg(matchedBooks.size());
    statusBar()->showMessage(resultText, 5000);

    // 更新表头以显示当前排序状态
    updateHeaderLabels();

    qDebug() << "Search completed successfully";
}

/**
 * @brief 高亮显示表格中匹配搜索关键词的文本
 *
 * 功能说明：
 * 1. 参数验证：检查关键词和表格项是否有效
 * 2. 匹配检测：使用不区分大小写的包含匹配
 * 3. 视觉高亮：通过字体加粗和背景色突出显示匹配项
 * 4. 提示信息：添加工具提示显示匹配详情
 * 5. 数据保持：保留原始文本用于其他用途
 *
 * 高亮效果：
 * - 字体样式：加粗显示匹配的文本
 * - 背景颜色：使用金色背景(#FFD700)突出显示
 * - 工具提示：鼠标悬停时显示"匹配: [原始文本]"
 *
 * 匹配规则：
 * - 不区分大小写的文本包含匹配
 * - 支持部分匹配和完全匹配
 * - 关键词为空时跳过高亮处理
 *
 * 技术实现：
 * - Qt::DisplayRole：存储显示的原始文本
 * - Qt::ToolTipRole：存储工具提示文本
 * - QFont::setBold()：设置字体加粗
 * - QStandardItem::setBackground()：设置背景色
 *
 * 性能考虑：
 * - 简化实现避免复杂的HTML文本处理
 * - 只对匹配项进行样式设置，减少性能开销
 * - 保留原始文本数据，确保数据完整性
 *
 * @param text 要检查和显示的原始文本
 * @param keyword 搜索关键词
 * @param item 要应用高亮效果的表格项
 */
void MainWindow::highlightMatchingText(const QString &text, const QString &keyword, QStandardItem *item)
{
    // 参数验证：关键词为空或表格项无效时直接返回
    if (keyword.isEmpty() || !item) {
        return;
    }

    // 文本预处理：转换为小写进行不区分大小写的匹配
    QString lowerText = text.toLower();
    QString lowerKeyword = keyword.toLower();

    // 匹配检测：检查文本是否包含关键词
    if (lowerText.contains(lowerKeyword)) {
        // 视觉高亮处理：简化实现，避免复杂的HTML文本处理

        // 字体样式设置：将匹配文本加粗显示
        QFont font = item->font();
        font.setBold(true);
        item->setFont(font);

        // 背景色设置：使用金色背景突出显示匹配项
        item->setBackground(QColor("#FFD700")); // 金色背景

        // 数据存储：保持原始文本完整性
        item->setData(text, Qt::DisplayRole);                                    // 存储原始显示文本
        item->setData(QString("匹配: %1").arg(text), Qt::ToolTipRole);           // 存储工具提示文本
    }
}
    }
}

QVector<BookCopy> MainWindow::searchCopiesByKeyword(const QString &keyword)
{
    QVector<BookCopy> result;
    QString lowerKeyword = keyword.toLower();

    const QVector<Book> &allBooks = library_.getAll();
    for (const Book &book : allBooks) {
        QVector<BookCopy> copies = library_.getBookCopies(book.indexId);
        for (const BookCopy &copy : copies) {
            if (copy.copyId.toLower().contains(lowerKeyword) ||
                copy.indexId.toLower().contains(lowerKeyword)) {
                result.append(copy);
            }
        }
    }

    return result;
}

void MainWindow::onTableDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    // 获取行号
    int row = index.row();

    // 获取索引号（第0列）
    QModelIndex indexIdIndex = model_->index(row, 0);
    QString indexId = model_->data(indexIdIndex).toString();

    // 根据索引号查找图书
    const QVector<Book> &allBooks = library_.getAll();
    Book targetBook;
    bool found = false;

    for (const Book &book : allBooks) {
        if (book.indexId == indexId) {
            targetBook = book;
            found = true;
            break;
        }
    }

    if (found) {
        // 显示图书详情对话框
        BookDetailDialog dialog(targetBook, this);
        dialog.exec();
    }
}
