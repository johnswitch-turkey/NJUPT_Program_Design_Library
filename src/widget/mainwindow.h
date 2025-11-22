/**
 * @file mainwindow.h
 * @brief 图书管理系统主窗口头文件
 *
 * 本文件定义了图书管理系统的主窗口类MainWindow，提供完整的图书管理界面。
 * 包括图书列表显示、搜索功能、添加编辑删除功能、管理员权限控制等核心功能。
 *
 * @author 开发团队
 * @date 2023-2024
 * @version 1.0
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QTableView>
#include <QPushButton>
#include <QMenu>
#include <QActionGroup>
#include <QComboBox>
#include "../utils/librarymanager.h"
#include "../utils/bookcopy.h"
#include "bookdetaildialog.h"
#include "../utils/bookdisplay.h"

// Qt组件前向声明，减少编译依赖
QT_BEGIN_NAMESPACE
class QToolBar;
class QScrollArea;
class QDockWidget;
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

/**
 * @class MainWindow
 * @brief 图书管理系统主窗口类
 *
 * 该类是图书管理系统的主界面，集成了所有的图书管理功能。
 * 提供了图书的增删改查、搜索过滤、详情查看、副本管理等核心功能。
 * 支持管理员和普通用户两种权限模式，提供不同的功能访问级别。
 *
 * 主要功能模块：
 * - 图书列表显示：以表格形式展示所有图书信息
 * - 搜索功能：支持按书名、作者、索引号等多种方式搜索
 * - 图书管理：添加、编辑、删除图书信息
 * - 详情查看：双击查看图书的详细信息和副本状态
 * - 副本管理：管理图书的借阅副本信息
 * - 权限控制：管理员模式提供完整功能，普通用户模式受限
 * - 主题切换：支持深色/浅色主题切换
 *
 * UI布局：
 * - 顶部：工具栏包含常用操作按钮
 * - 中部：搜索栏和图书列表表格
 * - 左侧：工具导航栏
 * - 状态栏：显示系统状态和统计信息
 *
 * @note 需要管理员权限才能进行添加、编辑、删除操作
 * @note 支持键盘快捷键和右键菜单操作
 * @note 数据变更会自动保存到本地存储
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // 设置当前登录用户及模式（学生/管理员），以及用户数据文件路径
    void setCurrentUser(const QString &username, bool isAdminMode, const QString &usersFilePath);

private slots:
    // UI交互
    void toggleTheme();
    void onSearch();
    void onSearchModeChanged();
    void onAddBook();
    void onEditBook();
    void onDeleteBook();

    // 核心业务功能
    void onBorrow();
    void onReturn();
    void onRenew();              // 新增：续借功能
    void onWarn();
    void onShowAll();
    // void onSwitchMode();
    void onHeaderSectionClicked(int section);
    void onSortByBorrowCount();
    void onSortDefault();
    void onSortChanged(QAction *action);
    void onTableDoubleClicked(const QModelIndex &index);

    // 数据管理
    void onOpen();
    void onSave();
    void onImport();
    void onExport();
    void onRefresh();
    void onManageCopies();
    // 筛选菜单
    void onCategoryFilterChanged(QAction *action);
    void onStatusFilterChanged(QAction *action);
    void onLocationFilterChanged(QAction *action);
    void onShowMyBorrows();         // 学生查看自己的借阅信息
    void onShowBookBorrowHistory(); // 管理员查看某本书的借阅记录
    void onImportUsers();           // 导入学生数据
    void onExportUsers();           // 导出学生数据

private:
    // UI设置
    void setupTable();
    void setupMenuBar();
    void setupSearchBar();
    void setupActions();
    void updateActionsVisibility();  // 根据用户角色更新按钮显示
    void toggleToolBarOrientation(); // 切换工具栏方向（竖向/横向）
    void onToolBarDockLocationChanged(Qt::DockWidgetArea area); // 工具栏停靠位置变化时的自适应处理
    void setupVerticalToolBarLayout(); // 设置竖向工具栏布局
    void setupHorizontalToolBarLayout(); // 设置横向工具栏布局
    void setupThemeToggle();
    void setupStyles();
    void applyTheme(bool isDark);
    QString getThemeStyles(bool isDark);
    QString getMenuStyles(bool isDark);
    void rebuildFilterMenus();
    void updateHeaderLabels();
    void showFilterMenu(QMenu *menu, int section);
    QDockWidget *createDockWidgetFromScrollArea(class QScrollArea *scrollArea);

    // 数据与显示
    void loadData();     // 加载数据
    void refreshTable(); // 刷新表格显示

    // --- 新增：筛选并显示特定图书列表的辅助函数 ---
    void displayBooks(const QVector<Book> &booksToShow);

    // 搜索功能增强
    void performFuzzySearch(const QString &keyword, const QString &searchMode);
    // void highlightMatchingText(const QString &text, const QString &keyword, QStandardItem *item);
    // QVector<BookCopy> searchCopiesByKeyword(const QString &keyword);

    // UI更新
    void updateStatusBar();
    void showBookDialog(const Book &book = Book(), bool isEdit = false);

    // 用户与借阅相关的辅助函数
    QJsonArray loadUsersJson() const;
    bool saveUsersJson(const QJsonArray &array) const;
    bool currentUserHasBorrowed(const QString &indexId) const;
    void addBorrowRecordForCurrentUser(const Book &book, const QDate &borrowDate, const QDate &dueDate);
    void markBorrowRecordReturnedForCurrentUser(const QString &indexId, const QDate &returnDate);
    QString borrowRecordsForCurrentUserText() const;
    QString borrowHistoryForBookText(const QString &indexId) const;

    // UI成员
    Ui::MainWindow *ui;
    LibraryManager library_;    // 核心数据管理器
    QStandardItemModel *model_; // 表格模型
    QTableView *tableView_;     // 表格视图
    QLineEdit *searchEdit_;
    QPushButton *searchButton_;
    QPushButton *themeToggleButton_;
    QComboBox *searchModeComboBox_;  // 搜索方式选择下拉框
    QMenu *categoryFilterMenu_;
    QMenu *statusFilterMenu_;
    QMenu *locationFilterMenu_; // 新增：馆藏地址筛选菜单
    QMenu *sortMenu_;           // 排序菜单
    QActionGroup *categoryActionGroup_;
    QActionGroup *statusActionGroup_;
    QActionGroup *locationActionGroup_; // 新增：馆藏地址动作组
    QActionGroup *sortActionGroup_;     // 排序动作组
    QString categoryFilter_;
    QString statusFilter_;
    QString locationFilter_;  // 新增：馆藏地址筛选条件
    QString currentSortType_; // 当前排序类型："default" 或 "borrowCount"

    // 搜索状态
    QString currentSearchKeyword_;   // 当前搜索关键词
    QString currentSearchMode_;      // 当前搜索模式
    bool isSearchActive_ = false;    // 是否处于搜索状态

    // 状态和主题
    bool isDarkMode_;
    bool isEditMode_;

    bool isWarn = false;

    // 当前登录用户信息
    QString currentUsername_;
    bool isAdminMode_ = false;      // 是否以管理员模式登录
    QString usersFilePath_;         // 用户数据文件路径

    // 按钮引用，用于控制显示/隐藏
    QAction *borrowAct_ = nullptr;
    QAction *returnAct_ = nullptr;
    QAction *renewAct_ = nullptr;       // 新增：续借按钮
    QAction *warnAct_ = nullptr;
    QAction *myBorrowAct_ = nullptr;
    QAction *allAct_ = nullptr;
    QAction *addBookAct_ = nullptr;
    QAction *editBookAct_ = nullptr;
    QAction *deleteBookAct_ = nullptr;
    QAction *bookHistoryAct_ = nullptr;
    QAction *importBookAct_ = nullptr;
    QAction *exportBookAct_ = nullptr;
    QAction *importUsersAct_ = nullptr;
    QAction *exportUsersAct_ = nullptr;
    QAction *toggleOrientationAct_ = nullptr; // 切换方向按钮

    // 工具栏相关
    QToolBar *actionToolBar_ = nullptr;
    QScrollArea *toolBarScrollArea_ = nullptr;
    QDockWidget *toolBarDockWidget_ = nullptr;
    QAction *manageCopiesAct_ = nullptr;
    bool isToolBarVertical_ = true; // true=竖向(左边), false=横向(顶部)
};

#endif // MAINWINDOW_H
